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
    printf("Type: %d aka %s, NUMBER:%d\n", tok.type, token_names[tok.type], token_count);
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
    exit(1);///////////////////////
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
 * AST TREE
 * ========================
 */

Expression* new_expr(ExpressionKind k){
    Expression* e = calloc_and_check(sizeof(Expression));
    e->kind = k;
    return e;
}

Statement* new_stmt(StatementKind k){
    Statement* s = calloc_and_check(sizeof(Statement));
    s->kind = k;
    return s;
}

Func* new_func(){
    Func* f = calloc_and_check(sizeof(Func));
    return f;
}

void add_param(Func* f, char* name, DataType type){
    char** names = realloc(f->param_names, (f->param_count + 1) * sizeof(char*));
    DataType* types = realloc(f->param_types, (f->param_count + 1) * sizeof(DataType));
    if (names == NULL || types == NULL){
        printf("Realloc err"); ////////////////////////////C
        exit(1);
    }
    f->param_names = names;
    f->param_types = types;
    f->param_names[f->param_count] = name;
    f->param_types[f->param_count] = type;
    f->param_count++;
}

Statement* append_statement(Statement* line, Statement* rest){
    Statement* s = line;
    if (s == NULL){
        return rest;
    }
    while (s->next != NULL){
        s = s->next;
    }
    s->next = rest;
    return line;
}

Expression* make_ident_expr(char* name){
    Expression* e = {0};
    if (tok.type == T_LPAREN){
        e = new_expr(E_CALL);
        e->name = name;
        next_token();
        e->args = parse_arguments();
        expect_type(T_RPAREN);
    }
    else{
        e = new_expr(E_VAR);
        e->name = name;
    }
    return e;
}

/*=========================
 * PRECEDENCE ANALYZATOR
 * ========================
 */

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
            char* name;
            if (tok.type == T_STRING || tok.type == T_IDENTIFIER){
                name = strdup_and_check(tok.attr.string_value);
            }
            TokenType prev_type = tok.type;
            int prev_int_value;
            double prev_double_value;
            char* prev_string_value;

            switch(prev_type){
                case T_INT:
                    prev_int_value = tok.attr.int_value;
                    break;
                case T_FLOAT:
                    prev_double_value = tok.attr.double_value;
                    break;
                case T_STRING:
                    prev_string_value = name;
                    break;
                default:
                    break;
            }
            next_token();
            if (tok.type == T_LPAREN){
                if (prev_type == T_IDENTIFIER){
                    item.node = new_expr(E_CALL);
                    item.node->name = name;
                    next_token();
                    item.node->args = parse_arguments();
                    expect_type(T_RPAREN);
                }
                else{
                    syntax_error(-1, "Lit and then (");////////////////////////////
                }
            }
            else{
                switch(prev_type){
                    case T_IDENTIFIER:
                        item.node = new_expr(E_VAR);
                        item.node->name = name;
                        break;
                    case T_INT:
                        item.node = new_expr(E_INT);
                        item.node->int_val = prev_int_value;
                        item.node->type = D_INT;
                        break;
                    case T_FLOAT:
                        item.node = new_expr(E_DOUBLE);
                        item.node->double_val = prev_double_value;
                        item.node->type = D_DOUBLE;
                        break;
                    case T_STRING:
                        item.node = new_expr(E_STRING);
                        item.node->string_val = prev_string_value;
                        item.node->type = D_STRING;
                        break;
                    default:
                        printf("Shift error that should never happen.");
                        exit(1);/////////////////////////////////////////////////
                }
            }
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
        default:
            switch(tok.type){
                case T_PLUS:
                    item.op = O_PLUS;
                    break;
                case T_MINUS:
                    item.op = O_MINUS;
                    break;
                case T_DIVIDE:
                    item.op = O_DIVIDE;
                    break;
                case T_MULTIPLY:
                    item.op = O_MULTIPLY;
                    break;
                case T_GREATER:
                    item.op = O_GREATER;
                    break;
                case T_GREATER_EQUAL:
                    item.op = O_GREATER_EQUAL;
                    break;
                case T_LESS:
                    item.op = O_LESS;
                    break;
                case T_LESS_EQUAL:
                    item.op = O_LESS_EQUAL;
                    break;
                case T_EQUAL_EQUAL:
                    item.op = O_EQUAL_EQUAL;
                    break;
                case T_NOT_EQUAL:
                    item.op = O_NOT_EQUAL;
                    break;
                default:
                    syntax_error(-1, "Shift err");
                    exit(1);//////////////////////////////////////

            }
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
                    if (!(s->data[i+1].is_expression) && s->data[i+1].symbol == P_ID){
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
                            PrecItem item = {0};
                            item.is_expression = true;
                            item.has_handle_after = false;

                            item.node = new_expr(E_UNIT);
                            item.node->type = D_UNIT;

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
                            PrecItem item = {0};
                            item.is_expression = true;
                            item.has_handle_after = false;

                            item.node = new_expr(E_UNARY);
                            item.node->op = s->data[i+1].op;
                            item.node->left = s->data[i+2].node;

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

                            item.node = s->data[i+2].node;

                            popx(s, 3);
                            push(s, item);
                        }
                        else{
                            syntax_error(-1,"Reduce function err - not (E).");/////////////////////////////
                            exit(1);
                        }
                    }
                    else if (s->data[i+1].is_expression && !(s->data[i+2].is_expression) && s->data[i+3].is_expression){
                        PrecItem item = {0};
                        item.is_expression = true;
                        item.has_handle_after = false;

                        item.node = new_expr(E_BINARY);

                        switch(s->data[i+2].symbol){// E op E -> E
                            case P_MULDIV:
                            case P_PLUSMINUS:
                            case P_REL:
                            case P_EQ:
                                item.node->op = s->data[i+2].op;
                                item.node->left = s->data[i+1].node;
                                item.node->right = s->data[i+3].node;

                                popx(s, 3);
                                push(s, item);
                                break;
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
        case T_NOT_EQUAL:
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

Func* parse_program(){
    if (tok.type != T_IMPORT ) syntax_error(T_IMPORT, "abab");

    parse_prologue();
    expect_type(T_EOL);
    Func* f = {0};
    f = parse_func_list();
    expect_type(T_EOF);
    return f;
}

void parse_prologue(){
    expect_type(T_IMPORT);
    expect_type(T_IDENTIFIER);
    expect_type(T_DOT);
    expect_type(T_MULTIPLY);
}

Func* parse_func_list(){
    Func* f = {0};
    switch (tok.type){
        case T_DEF:
        case T_AT:
            f = parse_func_def();
            f->next = parse_func_list();
            break;
        case T_EOF:
            return NULL;
        default:
            syntax_error(-1,"func_list");
    }
    return f;
}

Func* parse_func_def(){
    Func* f = new_func();
    switch (tok.type){
        case T_DEF:
        case T_AT:
            f->is_main = parse_annot();
            expect_type(T_DEF);
            if (tok.type == T_IDENTIFIER){
               f->func_name = strdup_and_check(tok.attr.string_value);
            }
            expect_type(T_IDENTIFIER);
            expect_type(T_LPAREN);
            parse_params(f);
            expect_type(T_RPAREN);
            f->return_type = parse_n_type();
            expect_type(T_EQUAL);
            f->body = parse_func_body();
            break;
        default:
            syntax_error(-1, "func_def");
    }
    return f;
}

bool parse_annot(){
    bool is_main = false;
    switch (tok.type){
        case T_AT:
            next_token();
            if (tok.type == T_IDENTIFIER){
                if (strcmp(tok.attr.string_value, "main") != 0){
                    printf("At and then not main.\n"); /////////////////////// Change later
                    exit(1);
                }
                else{
                    is_main = true;
                }
            }
            expect_type(T_IDENTIFIER);
            parse_annot_n();
            break;
        case T_DEF:
            break;
        default:
            syntax_error(-1, "annot");
    }
    return is_main;
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

void parse_params(Func* f){
    switch (tok.type){
        case T_RPAREN:
            break;
        case T_IDENTIFIER:{
            char* param_name = strdup_and_check(tok.attr.string_value);
            next_token();
            expect_type(T_COLON);
            DataType param_type = parse_type();
            add_param(f, param_name, param_type);
            parse_params_n(f);
            break;
        }
        default:
            syntax_error(-1, "params");
    }
}

void parse_params_n(Func* f){
    switch (tok.type){
        case T_COMMA:
            next_token();
            if (tok.type == T_IDENTIFIER){
                char* param_name = strdup_and_check(tok.attr.string_value);
            }
            expect_type(T_IDENTIFIER);
            expect_type(T_COLON);
            DataType param_type = parse_type();
            add_param(f, param_name, param_type);
            parse_params_n(f);
            break;
        case T_RPAREN:
            break;
        default:
            syntax_error(-1, "params_n");
    }
}

Statement* parse_func_body(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:{
            Expression* e = parse_expression();
            Statement* s = new_stmt(S_EXPR);
            s->expr = e;
            expect_type(T_EOL);
            return s;
            break;
        }
        case T_EOL:
            next_token();
            return parse_block();
        default:
            syntax_error(-1, "func_body");
    }
    return NULL;
}

DataType parse_n_type(){
    DataType d;
    switch (tok.type){
        case T_EQUAL:
            d = D_UNKNOWN;
            break;
        case T_COLON:
            next_token();
            d = parse_type();
            break;
        default:
            syntax_error(-1, "n_type");
    }
    return d;
}

DataType parse_type(){
    DataType d;
    switch (tok.type){
        case TK_UNIT:
            d = D_UNIT;
            break;
        case TK_INT:
            d = D_INT;
            break;
        case TK_FLOAT:
            d = D_DOUBLE;
            break;
        case TK_STRING:
            d = D_STRING;
            break;
        default:
            syntax_error(-1, "type");
    }
    next_token();
    return d;
}

Statement* parse_block(){
    Statement* s = {0};
    switch (tok.type){
        case T_INDENT:
            next_token();
            s = parse_statement();
            expect_type(T_DEDENT);
            break;
        default:
            syntax_error(-1, "block");
    }
    return s;
}

Statement* parse_statement(){
    Statement* s = {0};
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
        case T_MINUS:{
            Statement* line = parse_statement_line();
            Statement* rest = parse_statement_n();
            s = append_statement(line, rest);
            break;
        }
        default:
            syntax_error(-1, "statement");
    }
    return s;
}

Statement* parse_statement_n(){
    Statement* s = {0};
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
        case T_MINUS:{
            Statement* line = parse_statement_line();
            Statement* rest = parse_statement_n();
            s = append_statement(line, rest);
            break;
        }
        case T_DEDENT:
            s = NULL;
            break;
        default:
            syntax_error(-1, "statement_n");
    }
    return s;
}

Statement* parse_statement_line(){
    Statement* s = {0};
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
            s = parse_simple_statement();
            expect_type(T_EOL);
            break;
        case T_IF:
            s = parse_if_statement();
            break;
        case T_WHILE:
            s = parse_while_statement();
            break;
        default:
            syntax_error(-1, "statement_line");
    }
    return s;
}

Statement* parse_simple_statement(){
    Statement* s = {0};
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
            s = parse_simple_s();
            s->next = parse_simple_n();
            break;
        default:
            syntax_error(-1, "simple_statement");
    }
    return s;
}

Statement* parse_simple_n(){
    Statement* s = {0};
    switch (tok.type){
        case T_SEMICOLON:
            next_token();
            s = parse_simple_s();
            s->next = parse_simple_n();
            break;
        case T_EOL:
            s = NULL;
            break;
        default:
            syntax_error(-1, "simple_n");
    }
    return s;
}

Statement* parse_simple_s(){
    Statement* s = {0};
    switch (tok.type){
        case T_IDENTIFIER:{
            char* name = strdup_and_check(tok.attr.string_value);
            next_token();
            if (tok.type == T_EQUAL){
                next_token();
                s = new_stmt(S_ASSIGN);
                s->name = name;
                s->expr = parse_expression();
            }
            else{
                Expression* e = make_ident_expr(name);
                s = new_stmt(S_EXPR);
                s->expr = parse_expression_from(e);
            }
            break;
        }
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            s = new_stmt(S_EXPR);
            s->expr = parse_expression();
            break;
        case T_VAL:
        case T_VAR:
            s = parse_var_def();
            break;
        case T_RETURN:
            s = parse_return_statement();
            break;
        default:
            syntax_error(-1, "simple_s");
    }
    return s;
}

Statement* parse_if_statement(){
    Statement* s = new_stmt(S_IF);
    switch (tok.type){
        case T_IF:
            next_token();
            s->expr = parse_condition();
            expect_type(T_THEN);
            expect_type(T_EOL);
            s->body = parse_block();
            expect_type(T_ELSE);
            expect_type(T_EOL);
            s->else_body = parse_block();
            break;
        default:
            syntax_error(-1, "if_statement");
    }
    return s;
}

Statement* parse_while_statement(){
    Statement* s = new_stmt(S_WHILE);
    switch (tok.type){
        case T_WHILE:
            next_token();
            s->expr = parse_condition();
            expect_type(T_DO);
            expect_type(T_EOL);
            s->body = parse_block();
            break;
        default:
            syntax_error(-1, "while_statement");
    }
    return s;
}

Expression* parse_condition(){
    Expression* e = {0};
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            e = parse_expression();
            break;
        default:
            syntax_error(-1, "condition");
    }
    return e;
}

Statement* parse_return_statement(){
    Statement* s = {0};
    switch (tok.type){
        case T_RETURN:
            next_token();
            s = parse_return_val();
            break;
        default:
            syntax_error(-1, "return_statement");
    }
    return s;
}

Statement* parse_return_val(){
    Statement* s = new_stmt(S_RETURN);
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            s->expr = parse_expression();
            break;
        case T_EOL:
        case T_SEMICOLON:
            s->type = D_UNKNOWN;
            break;
        default:
            syntax_error(-1, "return_val");
    }
    return s;
}

Expression* parse_arguments(){
    Expression* e = {0};
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_MINUS:
            e = parse_term();
            e->next = parse_arguments_n();
            break;
        case T_RPAREN:
            e = NULL;
            break;
        default:
            syntax_error(-1, "arguments");
    }
    return e;
}

Expression* parse_arguments_n(){
    Expression* e = {0};
    switch (tok.type){
        case T_COMMA:
            next_token();
            e = parse_arguments_nn();
            break;
        case T_RPAREN:
            e = NULL;
            break;
        default:
            syntax_error(-1, "arguments_n");
    }
    return e;
}

Expression* parse_arguments_nn(){
    Expression* e = {0};
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_MINUS:
            e = parse_term();
            e->next = parse_arguments_n();
            break;
        default:
            syntax_error(-1, "arguments_nn");
    }
    return e;
}

Expression* parse_term(){
    Expression* e = {0};
    switch (tok.type){
        case T_IDENTIFIER:
            e = new_expr(E_VAR);
            e->name = strdup_and_check(tok.attr.string_value);
            next_token();
            break;
        case T_INT:
            e = new_expr(E_INT);
            e->int_val = tok.attr.int_value;
            next_token();
            break;
        case T_FLOAT:
            e = new_expr(E_DOUBLE);
            e->double_val = tok.attr.double_value;
            next_token();
            break;
        case T_STRING:
            e = new_expr(E_STRING);
            e->string_val = strdup_and_check(tok.attr.string_value);
            next_token();
            break;
        case T_LPAREN:
            next_token();
            expect_type(T_RPAREN);
            e = new_expr(E_UNIT);
            e->type = D_UNIT;
            break;
        case T_MINUS:
            e = new_expr(E_UNARY);
            e->op = O_MINUS;
            next_token();
            e->left = parse_num_lit();
            break;
        default:
            syntax_error(-1, "term");
    }
    return e;
}

Expression* parse_num_lit(){
    Expression* e = {0};
    switch (tok.type){
        case T_INT:
            e = new_expr(E_INT);
            e->int_val = tok.attr.int_value;
            next_token();
            break;
        case T_FLOAT:
            e = new_expr(E_DOUBLE);
            e->double_val = tok.attr.double_value;
            next_token();
            break;
        default:
            syntax_error(-1, "num_lit");
    }
    return e;
}

Statement* parse_var_def(){
    Statement* s = new_stmt(S_VAR_DEF);
    switch (tok.type){
        case T_VAL:
        case T_VAR:
            s->is_var = parse_vax();
            if (tok.type == T_IDENTIFIER){
                s->name = strdup_and_check(tok.attr.string_value);
            }
            expect_type(T_IDENTIFIER);
            s->type = parse_n_type();
            expect_type(T_EQUAL);
            s->expr = parse_expression();
            break;
        default:
            syntax_error(-1, "var_def");
    }
    return s;
}

bool parse_vax(){
    bool is_var = false;
    switch (tok.type){
        case T_VAL:
            next_token();
            break;
        case T_VAR:
            is_var = true;
            next_token();
            break;
        default:
            syntax_error(-1, "vax");
    }
    return is_var;
}

Expression* parse_expression(){
    return parse_expression_from(NULL);
}

Expression* parse_expression_from(Expression* first){
    Stack* s = stack_init();

    PrecItem item = {0};
    item.symbol = P_DOLLAR;
    item.is_expression = false;
    item.has_handle_after = false;
    push(s,item);

    bool prev_is_operand;
    if (first != NULL){
        PrecItem item2 = {0};
        item2.is_expression = true;
        item2.has_handle_after = false;
        item2.node = first;
        push(s,item2);

        prev_is_operand = true;
    }
    else{
        prev_is_operand = false;
    }
    int depth = 0;

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
                    return s->data[1].node;
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

/* ==================
 * END OF LL TABLE FUNCTIONS
 * ==================
 */


int main (){
    FILE* test_file = fopen("test.txt","r");
    if (test_file == NULL){
        return -1;
    }
    scanner_init(test_file);

    next_token();

    Func* f = {0};
    f = parse_program();
    printf("uz je konec hej\n");

    fclose(test_file);
    return 0;
}
