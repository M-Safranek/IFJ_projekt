#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Scanner.h"
#include <stdbool.h>
FILE* file;
unsigned long current_token_attr_size = 0;
char current_token_attr[1000];
//TODO dynamically allocate memory for attribute

int is_running = 0; // used to stop the token reader when a token is created(in create token)
Token current_token;
int line_number = 0;


bool has_newline = false;// used in multi-line comments based on specification
int block_comment_depth = 0;

bool after_block_comment_check_flag = false; // used to check that after a multi-line comment there is a OEL preceded bty whitespaces
bool count_indent_flag = true; // used to read the indent at the start of the line state is set to L_START instead of start

int paren_depth = 0;


// remembers pending dedent count
int pending_dedents = 0;


//indents
//TODO dynamically allocate memory for stack
#define MAX_INDENT_LEVELS 1024

int indentStack[MAX_INDENT_LEVELS] = {0};
int indentTop = 0;

void pushIndent(int indent) {
    if (indentTop + 1 >= MAX_INDENT_LEVELS) {
        return; 
    }
    indentTop++;
    indentStack[indentTop] = indent;
}

void popIndent() {
    if (indentTop > 0) {
        indentTop--;
    }
}



// returns false on indentation error
bool new_indent(int current_indent) {
    int previous_indent = indentStack[indentTop];
    if (current_indent > previous_indent) {
        pushIndent(current_indent);
        create_token(T_INDENT);
    }
    else if (current_indent < previous_indent) {
        while (current_indent < indentStack[indentTop]) {
            popIndent();
            create_token(T_DEDENT);
        }

        if (current_indent != indentStack[indentTop]) {
            is_running = 0;
            return false;
        }
    }
    return true;
}


const char *keywords[] = {
    "def",
    "do",
    "Double",
    "else",
    "if",
    "import",
    "Int",
    "return",
    "String",
    "then",
    "Unit",
    "val",
    "var",
    "while"
};

/*
 * Initializes the scanner.
 */
void scanner_init(FILE *source)
{
    file = source;
}

//checks if the current identifier is a keywords only works because of the keywords being at the start of the enum and in the same order as in the array 
bool keyword_check(){
    current_token_attr[current_token_attr_size] = '\0';
    for (int i = T_DEF; i <= T_WHILE; i++) {
        if (strcmp(current_token_attr, keywords[i]) == 0) {
            create_token(i);
            return true;
        }
    }
    return false;
}


/*
 * Returns the next token from the input.
 */
Token get_next_token(void)
{

      if (pending_dedents > 0) {
        current_token_attr_size = 0;
        popIndent();
        pending_dedents--;
        create_token(T_DEDENT);
        return current_token;
    }

    State state = START;
    current_token_attr_size = 0 ;
    is_running = 1;


    while (is_running)
    {
        int c = getc(file);

        //ignore
        if (state == START &&
        c != EOF &&
        c != '\n' &&
        isspace((unsigned char)c)
        && ! count_indent_flag)
            {
                continue;
            }
        

        //add to the string 
        if (current_token_attr_size < sizeof(current_token_attr) - 1)
        {
            if(c != EOF)
                current_token_attr[current_token_attr_size++] = (char)c;
        }

        if (after_block_comment_check_flag){
            state =  ML_COMMENT_END_CHECK;
            after_block_comment_check_flag = false;}

        if (count_indent_flag){
            state = L_START;
            count_indent_flag = false;
        }

            
        switch (state)
        {
            //starting case
            case START:
                if (c == '_')
                    state = UNDERSCORE;
                else if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
                    state = IDENTIFIER;
                else if (c == '0')
                    state = ZERO;
                else if (isdigit(c))
                    state = INT;
                else if (c == '.')
                    state = DOT;
                else if(c == '"')
                    state = STRING_START;
                else if(c == '+')
                    create_token(T_PLUS);
                else if(c == '-')
                    create_token(T_MINUS);
                else if(c == '*')
                    create_token(T_MULTIPLY);
                else if(c == '/')
                    state = SLASH;
                else if(c == '=')
                    state = EQUAL;
                else if(c == '<')
                    state = LESS;
                else if(c == '>')
                     state = GREATER;

                else if(c == '!')
                     state = T_EXCLAMATION;

                else if(c == '(')
                    {
                    create_token(T_LPAREN);
                    paren_depth ++;
                    }

                else if(c == ')'){
                    create_token(T_RPAREN);
                    paren_depth --;
                    }

                else if(c == '[')
                   create_token(T_LBRACKET);
                else if(c == ']')
                    create_token(T_RBRACKET);
                else if(c == ';')
                    create_token(T_SEMICOLON);
                else if(c == ':')
                    create_token(T_COLON);
                else if(c == ',')
                    create_token(T_COMMA);
                else if(c == '@')
                    create_token(T_AT);
                else if(c == '\n'){
                    line_number += 1;
                    if (paren_depth == 0)
                        create_token(T_EOL);
                    count_indent_flag = true;
                }
                else if(c == EOF)
                    create_token(T_EOF);

        
                else 
                    state = FALSE;
                 
                break;


            case UNDERSCORE:
                if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_') || isdigit(c))
                    state = IDENTIFIER;

                else
                    {state = FALSE;
                    forget_char(c);}

            break;

                        
            case T_EXCLAMATION:
                if (c == '='){
                    create_token(T_NOT_EQUAL);
                }
                else{
                    state = FALSE;
                    forget_char(c);
                }
            break;

            case IDENTIFIER:
                if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_') || isdigit(c))
                    state = IDENTIFIER;
                    
                else {
                    forget_char(c);
                    if (!keyword_check())
                        create_token(T_IDENTIFIER);}

                break;


            /* ==========================
             * Numbers
             * ========================== */

            case INT:
                if (isdigit(c))
                    state = INT;

                else if (c == '_')
                    state = INT_LAST_UNDERSCORE;

                else if (c == '.')
                    state = DECIMAL_START;

                else if (c == 'e' || c =='E')
                    state = FLOAT_EXPONENT_START;

                else {
                    forget_char(c);
                    create_token(T_INT);
                }
                    
                break;



            case INT_LAST_UNDERSCORE:
                if (isdigit(c))
                    state = INT;

                else
                    {state = FALSE;
                    forget_char(c);}

                break;

            
            case ZERO:
                if (c == '0')
                    state = ZERO;
                else if (c =='.')
                    state = DECIMAL_START;
        
                else if (isdigit(c))
                    {state = FALSE;
                    forget_char(c);}


                else{
                    forget_char(c);
                    create_token(T_INT);
                }

                break;


            case DOT:
                if(isdigit(c))
                    state = FLOAT;
                else {
                    forget_char(c);
                    create_token(T_DOT);}
                break;


            case DECIMAL_START:
                if (isdigit(c))
                    state = FLOAT;

                else
                    {state = FALSE;
                    forget_char(c);}
                break;

            case FLOAT:
                if (isdigit(c))
                    state = FLOAT;

                else if (c == '_')
                    state = FLOAT_LAST_UNDERSCORE;

                else if (c == 'e' || c =='E')
                    state = FLOAT_EXPONENT_START;

                else if (((c == 'd') || (c == 'D'))){
                    create_token(T_FLOAT);
                }
                else{
                    forget_char(c);
                    create_token(T_FLOAT);
                }
                break;

            case FLOAT_LAST_UNDERSCORE:
                if (isdigit(c))
                    state = FLOAT;

                else
                    {state = FALSE;
                    forget_char(c);}
                break;

            case FLOAT_EXPONENT_START:
                if ((c == '+') || (c == '-'))
                    state = FLOAT_SIGNED_EXPONENT_START;

                else if (isdigit(c))
                    state = FLOAT_EXPONENT;

                else{
                    state = FALSE;
                    forget_char(c);
                }
                break;

            case FLOAT_SIGNED_EXPONENT_START:
                if (isdigit(c))
                    state = FLOAT_EXPONENT;

                else
                    {state = FALSE;
                    forget_char(c);}
                break;

            case FLOAT_EXPONENT:
                if (isdigit(c))
                    state = FLOAT_EXPONENT;

                else if (c == '_')
                    state = FLOAT_EXPONENT_LAST_UNDERSCORE;

                
                else if (((c == 'd') || (c == 'D'))){
                    create_token(T_FLOAT);
                }
                else{
                    forget_char(c);
                    create_token(T_FLOAT);
                }
                
                break;

            case FLOAT_EXPONENT_LAST_UNDERSCORE:
                if (isdigit(c))
                    state = FLOAT_EXPONENT;
                else
                    {state = FALSE;
                    forget_char(c);}
                break;


            /* ==========================
             * Strings
             * ========================== */

            case  STRING_START:
                if (c =='"')
                    state = DOUBLE_DOUBLE_QUOTE;
                else if (c >= 0 && c <= 127)
                    state = STRING;
                else
                    {state = FALSE;
                    forget_char(c);}

                break;

            case DOUBLE_DOUBLE_QUOTE:
                if (c =='"')
                    state = MULTILINE_STRING;
                
                else
                    {state = FALSE;
                    forget_char(c);}
                break;

            case STRING:
                if ((c >= 0 && c <= 127) && (c != '\n' && c!= '\\' && c!= '"'))
                    state = STRING;

                else if (c == '\\')
                    state = ESCAPE_SEQUENCE;

                else if (c == '"')
                    create_token(T_STRING);

                else
                    {state = FALSE;
                    forget_char(c);}


                break;

           case ESCAPE_SEQUENCE:
           
                state = STRING;
                if (c == 'n') {
                    current_token_attr[current_token_attr_size - 2] = '\n';
                    current_token_attr_size -= 1;
                }
                else if (c == 'r') {
                    current_token_attr[current_token_attr_size - 2] = '\r';
                    current_token_attr_size -= 1;
                }
                else if (c == 't') {
                    current_token_attr[current_token_attr_size - 2] = '\t';
                    current_token_attr_size -= 1;
                }
                else if (c == '\\') {
                    current_token_attr[current_token_attr_size - 2] = '\\';
                    current_token_attr_size -= 1;
                }
                else if (c == '"') {
                    current_token_attr[current_token_attr_size - 2] = '\"';
                    current_token_attr_size -= 1;
                }
                else {
                    {state = FALSE;
                    forget_char(c);}
                }
                break;
    

            case MULTILINE_STRING:
                if (c == '"')
                    state = MULTILINE_STRING_END_1;

                else if((c >= 0 && c <= 127) && c!= '"')
                    state = MULTILINE_STRING;

                else{
                    state = FALSE;
                }
                break;

            case MULTILINE_STRING_END_1:
                if (c == '"')
                    state = MULTILINE_STRING_END_FINAL;
                else if (c >= 0 && c <= 127)
                    state = MULTILINE_STRING;    

                else
                    state = FALSE;      
                break;
        
            case MULTILINE_STRING_END_FINAL:
                if (c == '"')
                   create_token(T_STRING);
                else if (c >= 0 && c <= 127)
                    state = MULTILINE_STRING;      
                else 
                    state = FALSE;  
                break;



            /* ==========================
             * Operators
             * ========================== */


            case LESS:
                if (c == '=')
                    create_token(T_LESS_EQUAL);

                else{
                    forget_char(c);
                    create_token(T_LESS);
                    }
                break;


            case GREATER:
                 if (c == '=')
                    create_token(T_GREATER_EQUAL);

                else{
                    forget_char(c);
                    create_token(T_GREATER);
                    }
                break;


            case EQUAL:
                if (c == '=')
                    create_token(T_EQUAL_EQUAL);

                else{
                    forget_char(c);
                    create_token(T_EQUAL);
                    }
                  break;
            

            case SLASH:
                if (c == '/')
                    state = L_COMMENT;

                else if(c == '*'){
                    has_newline = false;
                    block_comment_depth = 1;
                    state = ML_COMMENT;
                }
                else{
                    forget_char(c);
                    create_token(T_DIVIDE);
                }
                break;


            /* ==========================
             * Strings
             * ========================== */


            case L_COMMENT:
                if (c == '\n') {
                    forget_char(c);
                    state = START;
                 }

                else if (c == EOF) {
                    create_token(T_EOF); 
                }
                break;

                 
                 
            case ML_COMMENT:
                if (c == EOF) {
                    create_token(T_ERROR);
                }

                else if (c == '/') {
                    state = ML_COMMENT_SLASH;
                }

                else if (c == '*') {
                    state = ML_COMMENT_STAR;
                }

                else if (c == '\n') {
                    has_newline = true;
                    line_number++;
                }
                break;

            case ML_COMMENT_SLASH:
                 if (c == EOF) {
                    create_token(T_ERROR);
                    }

                else if (c == '*') {
                    block_comment_depth++;
                    state = ML_COMMENT;
                }
                else {
                    state = ML_COMMENT;
                    forget_char(c);
                }
                break;

            case ML_COMMENT_STAR:
                if (c == EOF) {
                 create_token(T_ERROR);
                }

                else if (c == '/') {
                    block_comment_depth--;
                    if (block_comment_depth == 0) {
                        if (has_newline) {
                            if(paren_depth == 0)
                                create_token(T_EOL);
                            after_block_comment_check_flag = true;
                        }
                        else {
                        current_token_attr_size = 0;
                        state = START;
                        }
                    }
                    else {
                        state = ML_COMMENT;
                    }
                }
                else {
                    state = ML_COMMENT;
                    forget_char(c);
                }
                break;

            case ML_COMMENT_END_CHECK:
                if (c == EOF) {
                    create_token(T_EOF); 
                }
                
                else if (c == '\n'){
                    forget_char(c);
                    state = START;
                }

                else if(isspace(c)){
                    state = ML_COMMENT_END_CHECK;
                }
              
                else{
                    forget_char(c);
                    state = FALSE;
                }
               break;
                            

            //--------------------------//

            //line start(indent check)
            case L_START:
                if (c == EOF) {
                    if(indentTop > 0){
                    pending_dedents = indentTop -1;
                    popIndent();
                    create_token(T_DEDENT);
                    }

                    else{
                        create_token(T_EOF);
                    }
                }

                else if (c == '\n') {
                    line_number++;
                    count_indent_flag = true;
                    current_token_attr_size = 0;
                    state = START;
                    }

                else if (c != ' '){
                    forget_char(c);
                    if (!new_indent(current_token_attr_size)) {
                        create_token(T_ERROR);
                    }
                    current_token_attr_size = 0;

                    state = START;
                    }
    
            break;

            // false state keeps reading until end of line or EOF
            case FALSE:
                if (c != EOF && c!= '\n')
                    {state = FALSE;}
                
                else
                    {
                        if( c== '\n'){
                            line_number += 1;
                        }
                    create_token(T_ERROR);
                    }
                break;


            
            default:
                break;
        }
    }
    return current_token;
    
}

// creates a token using current_token_attr and current_token_attr_size and sets is_running = 0
void create_token(TokenType new_type){
    Token new_token;
    current_token_attr[current_token_attr_size] = '\0';
    new_token.type = new_type;
    new_token.attr = malloc(sizeof(char)*(current_token_attr_size+1));
    new_token.line = line_number;
    strcpy(new_token.attr,current_token_attr);
    is_running = 0;
    current_token = new_token;
}


void forget_char(char c){
    ungetc(c,file);
    current_token_attr_size -= 1;
}




int main()
{
    FILE* test_file = fopen("test.txt","r");
    if (test_file == NULL){
        return -1;
    }
    file = test_file;

    Token i = get_next_token();
    while  (i.type != T_EOF){
        printf("type=%d, attr=[%s] line: %d \n", i.type, i.attr, i.line);
        i = get_next_token();
    }
    printf("type=%d, attr=[%s]\n", i.type, i.attr);

    fclose(test_file);
    return 0;
}
