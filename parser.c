#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Scanner.h"
#include "parser.h"
#include <stdbool.h>

/* TODO
 * remove debug things
 *
 * make AST tree generation
 *
 * make memory cleanup
 *
 * fix syntax_error func
 * fix error handling (so far it is just exit(1))
 *
 * add comments
 *
 * Make it so this is valid:
 * var a = (10+
 *         4+
 *         25)
*/
Token tok;


// for debug
const char *token_names[] = {
    //keywords
    "T_DEF",        // 0
    "T_DO",         // 1
    "TK_FLOAT",     // 2
    "T_ELSE",       // 3
    "T_IF",         // 4
    "T_IMPORT",     // 5
    "TK_INT",       // 6
    "T_RETURN",     // 7
    "TK_STRING",    // 8
    "T_THEN",       // 9
    "TK_UNIT",      // 10
    "T_VAL",        // 11
    "T_VAR",        // 12
    "T_WHILE",      // 13

    //special symbols
    "T_LPAREN",     // 14
    "T_RPAREN",     // 15
    "T_LBRACE",     // 16
    "T_RBRACE",     // 17
    "T_LBRACKET",   // 18
    "T_RBRACKET",   // 19
    "T_COMMA",      // 20
    "T_SEMICOLON",  // 21
    "T_COLON",      // 22
    "T_DOT",        // 23
    "T_AT",         // 24

    "T_INT",        // 25
    "T_STRING",     // 26
    "T_FLOAT",      // 27
    "T_UNIT",       // 28


    //operators
    "T_PLUS",       // 29
    "T_MINUS",      // 30
    "T_MULTIPLY",   // 31
    "T_DIVIDE",     // 32
    "T_LESS",       // 33
    "T_LESS_EQUAL", // 34
    "T_GREATER",    // 35
    "T_GREATER_EQUAL",// 36
    "T_EQUAL",      // 37
    "T_EQUAL_EQUAL",// 38


    "T_IDENTIFIER", // 39


    "T_INDENT",      // 40
    "T_DEDENT",     // 41
    "T_EOL",        // 42
    "T_ERROR",      // 43
    "T_EOF"         // 44
};
// for debug
int token_count = T_DO;

// for debug
void print_token(){
    printf("Type: %d aka %s, Attr:[%s], NUMBER:%d\n", tok.type, token_names[tok.type], tok.attr, token_count);
}


void next_token(){
    tok = get_next_token();
    token_count++;
}


void syntax_error(TokenType t, char* str){
    if (t == -1){
        printf("SWITCH FAIL u %s, last token: ", str);
        print_token();
    }
    else{
        printf("Syntax error, EXPECTED %d aka %s, got %d aka %s, NUMBER:%d, LINE:%d\n", t, token_names[t], tok.type, token_names[tok.type], token_count, tok.line);
    }
}

void expect_type(TokenType t){
    if (tok.type == t){
        next_token();
    }
    else{
        syntax_error(t, "abab");
    }
}

void* calloc_and_check(size_t n){
    void* p = calloc(1, n);
    if (p == NULL){
        printf("Calloc err.\n"); //////////////////////C
        exit(1);
    }
    return p;
}

char* strdup_and_check(char* s){
    char* p = strdup(s);
    if (p == NULL){
        printf("Strdup err.\n"); //////////////////////C
        exit(1);
    }
    return p;
}


/*=========================
 * PRECEDENCE ANALYZATOR
 * ========================
 */

typedef enum PrecSym{
    P_UNARY,
    P_MULDIV,
    P_PLUSMINUS,
    P_REL,
    P_EQ,
    P_LPAREN,
    P_RPAREN,
    P_ID,
    P_DOLLAR
}PrecSym;

typedef struct PrecItem{
    PrecSym symbol;
    bool is_expression;
    bool has_handle_after;
    Operators op;
    Expression* node;
}PrecItem;

typedef struct Stack{
    PrecItem* data;
    int top;
    int capacity;
} Stack;

Stack* stack_init() {
    Stack* s = calloc_and_check(sizeof(Stack));
    s->data = malloc(4 * sizeof(PrecItem));

    if (s->data == NULL) {
        printf("Malloc err"); ////////////////////////////C
        exit(1);
    }

    s->top = -1;
    s->capacity = 4;
    return s;
}

void resize_stack(Stack* s) {
    int new_capacity = s->capacity * 2;

    PrecItem* temp = realloc(s->data, new_capacity * sizeof(PrecItem));

    if (temp == NULL) {
        printf("Realloc err"); ////////////////////////////C
        exit(1);
    }

    s->data = temp;
    s->capacity = new_capacity;
}


void push(Stack* s, PrecItem item) {

    if (s->top + 1 == s->capacity) {
        resize_stack(s);
    }
    s->top++;
    s->data[s->top] = item;
}

// does not yet check if there is something to pop
void popx(Stack* s, int x) {
    for (int i = x; i > 0; i--){
        ///////////// mby free item
        s->top--;
    }
}


void destroy_stack(Stack *s) {
    free(s->data);

    s->data = NULL;
    s->top = -1;
    s->capacity = 0;
}

int top_terminal_idx(Stack* s){
    for (int i = s->top; i>-1; i--){
        if (!(s->data[i].is_expression)){
            return i;
        }
    }
    printf("Top terminal idx - stack is empty\n"); // Should not ever happen, I hope
    exit(1);///////need to free//////////////////////////////////////////////////////////////
}

char prec_table[9][9] = {
    //U   M/D  P/M  REL  EQ   L_P  R_P  ID   DOLL
    {'<', '>', '>', '>', '>', '<', '>', '<', '>'}, // UNARY
    {'<', '>', '>', '>', '>', '<', '>', '<', '>'}, // MUL/DIV
    {'<', '<', '>', '>', '>', '<', '>', '<', '>'}, // PLUS/MINUS
    {'<', '<', '<', 'B', '>', '<', '>', '<', '>'}, // REL
    {'<', '<', '<', '<', 'B', '<', '>', '<', '>'}, // EQ
    {'<', '<', '<', '<', '<', '<', '=', '<', 'B'}, // L_PAREN
    {'B', '>', '>', '>', '>', 'B', '>', 'B', '>'}, // R_PAREN
    {'B', '>', '>', '>', '>', 'B', '>', 'B', '>'}, // ID
    {'<', '<', '<', '<', '<', '<', 'B', '<', 'A'}  // DOLLAR
};

void shift(Stack* s, PrecSym symbol, bool* prev_is_operand, int* depth){
    PrecItem item = {0};
    item.symbol = symbol;
    item.is_expression = false;
    item.has_handle_after = false;
    switch(symbol){
        case P_ID:{
            char* name = strdup_and_check(tok.attr);
            next_token();
            if (tok.type == T_LPAREN){ // Will be used for making AST tree - E_CALL
                next_token();
                parse_arguments();
                expect_type(T_RPAREN);
            }
            /* Will be used for making AST tree - E_VAR
            else{

            }*/

            *prev_is_operand = true;
            break;
        }
        case P_RPAREN:
            *prev_is_operand = true;
            (*depth)--;
            break;
        case P_LPAREN:
            *prev_is_operand = false;
            (*depth)++;
            break;
        case P_DOLLAR:
            break;
        default:// Will be used for making AST tree - operators
            *prev_is_operand = false;
            break;
    }
    push(s,item);
    if (symbol != P_ID){
        next_token();
    }

}

void insert_handle(Stack* s, int idx){
    s->data[idx].has_handle_after = true;
}

// TODO Check if E is E_CALL because function call has to be a whole expression (not func(a) + 4)
void reduce(Stack* s){
    int n = 0;
    for (int i = s->top; i>-1; i--){
        if (s->data[i].has_handle_after){
            switch (n){
                case 1: // i -> E
                    if (!(s->data[i+1].is_expression && s->data[i+1].symbol == P_ID)){
                       s->data[s->top].is_expression = true;
                    }
                    else{
                        syntax_error(-1,"Reduce function err - not <i.");//////////////////////////////
                        exit(1);
                    }
                    break;
                case 2:
                    if (!(s->data[i+1].is_expression) && !(s->data[i+2].is_expression)){
                        if (s->data[i+1].symbol == P_LPAREN && s->data[i+2].symbol == P_RPAREN){ // () -> E
                            // Will be used for making AST tree - makes E_UNIT
                            PrecItem item = {0};
                            item.is_expression = true;
                            item.has_handle_after = false;
                            popx(s, 2);
                            push(s, item);
                        }
                        else{
                            syntax_error(-1,"Reduce function err - not ().");/////////////////////////////
                            exit(1);
                        }
                    }
                    else if (!(s->data[i+1].is_expression) && s->data[i+2].is_expression){
                        if (s->data[i+1].symbol == P_UNARY){// UNARY E -> E
                            // Will be used for making AST tree - makes E_UNARY
                            PrecItem item = {0};
                            item.is_expression = true;
                            item.has_handle_after = false;
                            popx(s, 2);
                            push(s, item);
                        }
                        else{
                            syntax_error(-1,"Reduce function err - not UNARY E.");/////////////////////////////
                            exit(1);
                        }
                    }
                    else{
                        syntax_error(-1,"Reduce function err - not () or UNARY E.");/////////////////////////////
                        exit(1);
                    }
                    break;
                case 3:
                    if (!(s->data[i+1].is_expression) && s->data[i+2].is_expression && !(s->data[i+3].is_expression)){
                        if (s->data[i+1].symbol == P_LPAREN && s->data[i+3].symbol == P_RPAREN){ // (E) -> E
                            PrecItem item = {0};
                            item.is_expression = true;
                            item.has_handle_after = false;
                            popx(s, 3);
                            push(s, item);
                        }
                        else{
                            syntax_error(-1,"Reduce function err - not (E).");/////////////////////////////
                            exit(1);
                        }
                    }
                    else if (s->data[i+1].is_expression && !(s->data[i+2].is_expression) && s->data[i+3].is_expression){
                        switch(s->data[i+2].symbol){// E op E -> E
                            case P_MULDIV:
                            case P_PLUSMINUS:
                            case P_REL:
                            case P_EQ:{
                                PrecItem item = {0};
                                item.is_expression = true;
                                item.has_handle_after = false;
                                popx(s, 3);
                                push(s, item);
                                break;
                            }
                            default:
                                syntax_error(-1,"Reduce function err - not E op E.");/////////////////////////////
                                exit(1);
                                break;
                        }

                    }
                    else{
                        syntax_error(-1,"Reduce function err - not (E) or E op E.");/////////////////////////////
                        exit(1);
                    }
                    break;
                default:
                    syntax_error(-1,"Reduce function err.");
                    exit(1);////need to free////////////////////////////////////////////
            }
            s->data[i].has_handle_after = false;
            return;
        }
        n++;
    }
    printf("Did not find any HANDLE.\n"); // Should not ever happen, I hope
    exit(1);
}



PrecSym tok_to_sym(bool prev_is_operand, int depth){
    PrecSym symbol;
    switch (tok.type){
        case T_MULTIPLY:
        case T_DIVIDE:
            symbol = P_MULDIV;
            break;
        case T_LESS:
        case T_LESS_EQUAL:
        case T_GREATER:
        case T_GREATER_EQUAL:
            symbol = P_REL;
            break;
        case T_EQUAL_EQUAL:
        //case T_NEQ
            symbol = P_EQ;
            break;
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
            symbol = P_ID;
            break;
        case T_PLUS:
        case T_MINUS:
            if (prev_is_operand){
                symbol = P_PLUSMINUS;
            }
            else{
                symbol = P_UNARY;
            }
            break;
        case T_LPAREN:
            symbol = P_LPAREN;
            break;
        case T_RPAREN:
            if (depth > 0){
                symbol = P_RPAREN;
            }
            else{
                symbol = P_DOLLAR;
            }
            break;
        default:
            symbol = P_DOLLAR;
    }
    return symbol;
}


/*========================
 * LL TABLE FUNCTIONS
 * =======================
 */

void parse_program(){
    if (tok.type != T_IMPORT ) syntax_error(T_IMPORT, "abab");

    parse_prologue();
    expect_type(T_EOL);
    parse_func_list();
    expect_type(T_EOF);
}

void parse_prologue(){
    expect_type(T_IMPORT);
    expect_type(T_IDENTIFIER);
    expect_type(T_DOT);
    expect_type(T_MULTIPLY);
}

void parse_func_list(){
    switch (tok.type){
        case T_DEF:
        case T_AT:
            parse_func_def();
            parse_func_list();
            break;
        case T_EOF:
            break;
        default:
            syntax_error(-1,"func_list");
    }
}
void parse_func_def(){
    switch (tok.type){
        case T_DEF:
        case T_AT:
            parse_annot();
            expect_type(T_DEF);
            expect_type(T_IDENTIFIER);
            expect_type(T_LPAREN);
            parse_params();
            expect_type(T_RPAREN);
            parse_n_type();
            expect_type(T_EQUAL);
            parse_func_body();
            break;
        default:
            syntax_error(-1, "func_def");
    }
}

void parse_annot(){
    switch (tok.type){
        case T_AT:
            next_token();
            expect_type(T_IDENTIFIER);
            parse_annot_n();
            break;
        case T_DEF:
            break;
        default:
            syntax_error(-1, "annot");
    }
}

void parse_annot_n(){
    switch (tok.type){
        case T_EOL:
            next_token();
            break;
        case T_DEF:
            break;
        default:
            syntax_error(-1, "annot_n");
    }
}

void parse_params(){
    switch (tok.type){
        case T_RPAREN:
            break;
        case T_IDENTIFIER:
            next_token();
            expect_type(T_COLON);
            parse_type();
            parse_params_n();
            break;
        default:
            syntax_error(-1, "params");
    }
}

void parse_params_n(){
    switch (tok.type){
        case T_COMMA:
            next_token();
            expect_type(T_IDENTIFIER);
            expect_type(T_COLON);
            parse_type();
            parse_params_n();
            break;
        case T_RPAREN:
            break;
        default:
            syntax_error(-1, "params_n");
    }
}


void parse_func_body(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_expression();
            expect_type(T_EOL);
            break;
        case T_EOL:
            next_token();
            parse_block();
            break;
        default:
            syntax_error(-1, "func_body");
    }
}

void parse_n_type(){
    switch (tok.type){
        case T_EQUAL:
            break;
        case T_COLON:
            next_token();
            parse_type();
            break;
        default:
            syntax_error(-1, "n_type");
    }
}

void parse_type(){
    switch (tok.type){
        case TK_UNIT:
        case TK_INT:
        case TK_FLOAT:
        case TK_STRING:
            next_token();
            break;
        default:
            syntax_error(-1, "type");
    }
}

void parse_block(){
    switch (tok.type){
        case T_INDENT:
            next_token();
            parse_statement();
            expect_type(T_DEDENT);
            break;
        default:
            syntax_error(-1, "block");
    }
}

void parse_statement(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_IF:
        case T_WHILE:
        case T_RETURN:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_VAL:
        case T_VAR:
        case T_PLUS:
        case T_MINUS:
            parse_statement_line();
            parse_statement_n();
            break;
        default:
            syntax_error(-1, "statement");
    }
}

void parse_statement_n(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_IF:
        case T_WHILE:
        case T_RETURN:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_VAL:
        case T_VAR:
        case T_PLUS:
        case T_MINUS:
            parse_statement_line();
            parse_statement_n();
            break;
        case T_DEDENT:
            break;
        default:
            syntax_error(-1, "statement_n");
    }
}

void parse_statement_line(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_RETURN:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_VAL:
        case T_VAR:
        case T_PLUS:
        case T_MINUS:
            parse_simple_statement();
            expect_type(T_EOL);
            break;
        case T_IF:
            parse_if_statement();
            break;
        case T_WHILE:
            parse_while_statement();
            break;
        default:
            syntax_error(-1, "statement_line");
    }
}

void parse_simple_statement(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_RETURN:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_VAL:
        case T_VAR:
        case T_PLUS:
        case T_MINUS:
            parse_simple_s();
            parse_simple_n();
            break;
        default:
            syntax_error(-1, "simple_statement");
    }
}

void parse_simple_n(){
    switch (tok.type){
        case T_SEMICOLON:
            next_token();
            parse_simple_s();
            parse_simple_n();
            break;
        case T_EOL:
            break;
        default:
            syntax_error(-1, "simple_n");
    }
}

void parse_simple_s(){
    switch (tok.type){
        case T_IDENTIFIER:
            next_token();
            parse_simple_id();
            break;
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_unary_noid();
            parse_expression_n();
            break;
        case T_VAL:
        case T_VAR:
            parse_var_def();
            break;
        case T_RETURN:
            parse_return_statement();
            break;
        default:
            syntax_error(-1, "simple_s");
    }
}

void parse_simple_id(){
    switch (tok.type){
        case T_EOL:
        case T_MULTIPLY:
        case T_SEMICOLON:
        case T_LPAREN:
        case T_EQUAL_EQUAL:
        //case NEQ:
        case T_GREATER:
        case T_GREATER_EQUAL:
        case T_LESS:
        case T_LESS_EQUAL:
        case T_PLUS:
        case T_MINUS:
        case T_DIVIDE:
            parse_primary_id();
            parse_expression_n();
            break;
        case T_EQUAL:
            next_token();
            parse_expression();
            break;
        default:
            syntax_error(-1, "simple_id");
    }
}

void parse_if_statement(){
    switch (tok.type){
        case T_IF:
            next_token();
            parse_condition();
            expect_type(T_THEN);
            expect_type(T_EOL);
            parse_block();
            expect_type(T_ELSE);
            expect_type(T_EOL);
            parse_block();
            break;
        default:
            syntax_error(-1, "if_statement");
    }
}

void parse_while_statement(){
    switch (tok.type){
        case T_WHILE:
            next_token();
            parse_condition();
            expect_type(T_DO);
            expect_type(T_EOL);
            parse_block();
            break;
        default:
            syntax_error(-1, "while_statement");
    }
}
/////////////////////////////////////////////////////// CONDITION
void parse_condition(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            break;
        default:
            syntax_error(-1, "condition");
    }
}

void parse_return_statement(){
    switch (tok.type){
        case T_RETURN:
            next_token();
            parse_return_val();
            break;
        default:
            syntax_error(-1, "return_statement");
    }
}

void parse_return_val(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_expression();
            break;
        case T_EOL:
        case T_SEMICOLON:
            break;
        default:
            syntax_error(-1, "return_val");
    }
}

void parse_arguments(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_MINUS:
            parse_term();
            parse_arguments_n();
            break;
        case T_RPAREN:
            break;
        default:
            syntax_error(-1, "arguments");
    }
}

void parse_arguments_n(){
    switch (tok.type){
        case T_COMMA:
            next_token();
            parse_arguments_nn();
            break;
        case T_RPAREN:
            break;
        default:
            syntax_error(-1, "arguments_n");
    }
}

void parse_arguments_nn(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_MINUS:
            parse_term();
            parse_arguments_n();
            break;
        default:
            syntax_error(-1, "arguments_nn");
    }
}

void parse_term(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
            next_token();
            break;
        case T_LPAREN:
            next_token();
            expect_type(T_RPAREN);
            break;
        case T_MINUS:
            next_token();
            parse_num_lit();
            break;
        default:
            syntax_error(-1, "term");
    }
}

void parse_num_lit(){
    switch (tok.type){
        case T_INT:
        case T_FLOAT:
            next_token();
            break;
        default:
            syntax_error(-1, "num_lit");
    }
}

void parse_var_def(){
    switch (tok.type){
        case T_VAL:
        case T_VAR:
            parse_vax();
            expect_type(T_IDENTIFIER);
            parse_n_type();
            expect_type(T_EQUAL);
            parse_expression();
            break;
        default:
            syntax_error(-1, "var_def");
    }
}

void parse_vax(){
    switch (tok.type){
        case T_VAL:
        case T_VAR:
            next_token();
            break;
        default:
            syntax_error(-1, "vax");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void parse_expression(){
    Stack* s = stack_init();

    PrecItem item = {0};
    item.symbol = P_DOLLAR;
    item.is_expression = false;
    item.has_handle_after = false;

    push(s,item);
    int depth = 0;
    bool prev_is_operand = false;

    while(true){
        PrecSym b = tok_to_sym(prev_is_operand, depth);
        PrecSym a = s->data[top_terminal_idx(s)].symbol;

        switch (prec_table[a][b]){
            case '=':
                shift(s, b, &prev_is_operand, &depth);
                break;
            case '<':
                insert_handle(s, top_terminal_idx(s));
                shift(s, b, &prev_is_operand, &depth);
                break;
            case '>':
                reduce(s);
                break;
            case 'A':
                if (s->top == 1 && s->data[1].is_expression){
                    return; // Will return s->data[1].node
                }
                else{
                    syntax_error(-1, "Prec table - ACCEPT but stack is not $E");
                    exit(1);/////////////////////////////////////////
                }
                break;
            default:
                syntax_error(-1,"Prec Table.");
                exit(1);/////////////////////////////////////////
        }
    }

}

void parse_expression_n(){
    switch (tok.type){
        case T_EOL:
        case T_MULTIPLY:
        case T_SEMICOLON:
        case T_EQUAL_EQUAL:
        //case NEQ:
        case T_GREATER:
        case T_GREATER_EQUAL:
        case T_LESS:
        case T_LESS_EQUAL:
        case T_PLUS:
        case T_MINUS:
        case T_DIVIDE:
            parse_mult_n();
            parse_addition_n();
            parse_comparison_n();
            parse_equal_n();
            break;
        default:
            syntax_error(-1, "expression_n");
    }
}

void parse_equal(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_comparison();
            parse_equal_n();
            break;
        default:
            syntax_error(-1, "equal");
    }
}

void parse_equal_n(){
    switch (tok.type){
        case T_EOL:
        case T_SEMICOLON:
        case T_THEN:
        case T_DO:
        case T_RPAREN:
            break;
        case T_EQUAL_EQUAL:
        //case NEQ:
            parse_equal_op();
            parse_comparison();
            break;
        default:
            syntax_error(-1, "equal_n");
    }
}

void parse_equal_op(){
    switch (tok.type){
        case T_EQUAL_EQUAL:
        //case NEQ:
            next_token();
            break;
        default:
            syntax_error(-1, "equal_op");
    }
}

void parse_comparison(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_addition();
            parse_comparison_n();
            break;
        default:
            syntax_error(-1, "comparison");
    }
}

void parse_comparison_n(){
    switch (tok.type){
        case T_EOL:
        case T_SEMICOLON:
        case T_THEN:
        case T_DO:
        case T_RPAREN:
        case T_EQUAL_EQUAL:
        //case NEQ:
            break;
        case T_GREATER:
        case T_GREATER_EQUAL:
        case T_LESS:
        case T_LESS_EQUAL:
            parse_comparison_op();
            parse_addition();
            break;
        default:
            syntax_error(-1, "comparison_n");
    }
}

void parse_comparison_op(){
    switch (tok.type){
        case T_GREATER:
        case T_GREATER_EQUAL:
        case T_LESS:
        case T_LESS_EQUAL:
            next_token();
            break;
        default:
            syntax_error(-1, "comparison_op");
    }
}

void parse_addition(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_mult();
            parse_addition_n();
            break;
        default:
            syntax_error(-1, "addition");
    }
}

void parse_addition_n(){
    switch (tok.type){
        case T_EOL:
        case T_SEMICOLON:
        case T_THEN:
        case T_DO:
        case T_RPAREN:
        case T_EQUAL_EQUAL:
        //case NEQ:
        case T_GREATER:
        case T_GREATER_EQUAL:
        case T_LESS:
        case T_LESS_EQUAL:
            break;
        case T_PLUS:
            next_token();
            parse_mult();
            parse_addition_n();
            break;
        case T_MINUS:
            next_token();
            parse_mult();
            parse_addition_n();
            break;
        default:
            syntax_error(-1, "addition_n");
    }
}

void parse_mult(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_unary();
            parse_mult_n();
            break;
        default:
            syntax_error(-1, "mult");
    }
}

void parse_mult_n(){
    switch (tok.type){
        case T_EOL:
        case T_SEMICOLON:
        case T_THEN:
        case T_DO:
        case T_RPAREN:
        case T_EQUAL_EQUAL:
        //case NEQ:
        case T_GREATER:
        case T_GREATER_EQUAL:
        case T_LESS:
        case T_LESS_EQUAL:
        case T_PLUS:
        case T_MINUS:
            break;
        case T_MULTIPLY:
            next_token();
            parse_unary();
            parse_mult_n();
            break;
        case T_DIVIDE:
            next_token();
            parse_unary();
            parse_mult_n();
            break;
        default:
            syntax_error(-1, "mult_n");
    }
}

void parse_unary(){
    switch (tok.type){
        case T_IDENTIFIER:
            next_token();
            parse_primary_id();
            break;
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_unary_noid();
            break;
        default:
            syntax_error(-1, "unary");
    }
}

void parse_unary_noid(){
    switch (tok.type){
        case T_INT:
            next_token();
            break;
        case T_FLOAT:
            next_token();
            break;
        case T_STRING:
            next_token();
            break;
        case T_LPAREN:
            next_token();
            parse_primary_n();
            break;
        case T_PLUS:
            next_token();
            parse_unary();
            break;
        case T_MINUS:
            next_token();
            parse_unary();
            break;
        default:
            syntax_error(-1, "unary_noid");
    }
}

void parse_primary_id(){
    switch (tok.type){
        case T_EOL:
        case T_MULTIPLY:
        case T_SEMICOLON:
        case T_THEN:
        case T_DO:
        case T_RPAREN:
        case T_EQUAL_EQUAL:
        //case NEQ:
        case T_GREATER:
        case T_GREATER_EQUAL:
        case T_LESS:
        case T_LESS_EQUAL:
        case T_PLUS:
        case T_MINUS:
        case T_DIVIDE:
            break;
        case T_LPAREN:
            next_token();
            parse_arguments();
            expect_type(T_RPAREN);
            break;
        default:
            syntax_error(-1, "primary_id");
    }
}

void parse_primary_n(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_expression();
            expect_type(T_RPAREN);
            break;
        case T_RPAREN:
            next_token();
            break;
        default:
            syntax_error(-1, "primary_n");
    }
}
/* ==================
 * END OF LL TABLE FUNCTIONS
 * ==================
 */


int main (){
    FILE* test_file = fopen("test.txt","r");
    if (test_file == NULL){
        return -T_DO;
    }
    scanner_init(test_file);

    next_token();


    parse_program();
    printf("uz je konec hej\n");

    fclose(test_file);
    return T_DEF;
}
