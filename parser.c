/*
 * Parser for the IFJ26 language.
 * Checks syntax of the code from tokens given by Scanner.c
 * and makes an AST tree
 * Author: Martin Šafránek
 */

#define _POSIX_C_SOURCE 200809L // For strdup
#include "parser.h"
#include "Scanner.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token tok;
Scanner scanner;

// For converting enum token type to readable format for error messages
const char* token_names[] = {
    // keywords
    "T_DEF", // 0
    "T_DO", // 1
    "TK_DOUBLE", // 2
    "T_ELSE", // 3
    "T_IF", // 4
    "T_IMPORT", // 5
    "TK_INT", // 6
    "T_RETURN", // 7
    "TK_STRING", // 8
    "T_THEN", // 9
    "TK_UNIT", // 10
    "T_VAL", // 11
    "T_VAR", // 12
    "T_WHILE", // 13

    // special symbols
    "T_LPAREN", // 14
    "T_RPAREN", // 15
    "T_LBRACE", // 16
    "T_RBRACE", // 17
    "T_LBRACKET", // 18
    "T_RBRACKET", // 19
    "T_COMMA", // 20
    "T_SEMICOLON", // 21
    "T_COLON", // 22
    "T_DOT", // 23
    "T_AT", // 24

    "T_INT", // 25
    "T_STRING", // 26
    "T_DOUBLE", // 27
    "T_UNIT", // 28

    // operators
    "T_PLUS", // 29
    "T_MINUS", // 30
    "T_MULTIPLY", // 31
    "T_DIVIDE", // 32
    "T_LESS", // 33
    "T_LESS_EQUAL", // 34
    "T_GREATER", // 35
    "T_GREATER_EQUAL", // 36
    "T_EQUAL", // 37
    "T_EQUAL_EQUAL", // 38
    "T_NOT_EQUAL", // 39

    "T_IDENTIFIER", // 40

    "T_INDENT", // 41
    "T_DEDENT", // 42
    "T_EOL", // 43
    "T_ERROR", // 44
    "T_EOF" // 45
};

// Gets a new token from lexical analyzer
void next_token()
{
    // Frees the previous token
    token_destroy(&tok);
    // Gets a new token
    tok = get_next_token(&scanner);
    if (tok.type == T_ERROR) {
        fprintf(stderr, "Lexical analyzer error.\n");
        exit(1);
    }
}
// Prints the given string as a syntax error message, then exits the program with 2
void syntax_error(char* str)
{
    fprintf(stderr, "Syntax error: %s\n", str);
    exit(2);
}
// Prints a syntax error message with the given token type
void expect_syntax_error(TokenType t)
{
    fprintf(stderr, "Syntax error, EXPECTED %s, got %s, LINE:%d.\n", token_names[t], token_names[tok.type], tok.line);
    exit(2);
}
// Checks if the current token is the same as the given type
void expect_type(TokenType t)
{
    if (tok.type == t) {
        next_token();
    } else {
        expect_syntax_error(t);
    }
}
// Calls calloc and checks if it succeeded
void* calloc_and_check(size_t n)
{
    void* p = calloc(1, n);
    if (p == NULL) {
        fprintf(stderr, "Calloc err.\n");
        exit(99);
    }
    return p;
}
// Calls strdup and checks if it succeeded
char* strdup_and_check(char* s)
{
    char* p = strdup(s);
    if (p == NULL) {
        fprintf(stderr, "Strdup err.\n");
        exit(99);
    }
    return p;
}
/*=========================
 * AST TREE
 * ========================
 */
// Makes a new Expression of given kind and returns its pointer
Expression* new_expr(ExpressionKind k)
{
    Expression* e = calloc_and_check(sizeof(Expression));
    e->kind = k;
    return e;
}
// Makes a new Statement of given kind and returns its pointer
Statement* new_stmt(StatementKind k)
{
    Statement* s = calloc_and_check(sizeof(Statement));
    s->kind = k;
    return s;
}
// Makes a new Func and returns its pointer
Func* new_func()
{
    Func* f = calloc_and_check(sizeof(Func));
    return f;
}

// Adds a function parameter of given name and type
void add_param(Func* f, char* name, DataType type)
{
    char** names = realloc(f->param_names, (f->param_count + 1) * sizeof(char*));
    DataType* types = realloc(f->param_types, (f->param_count + 1) * sizeof(DataType));
    if (names == NULL || types == NULL) {
        fprintf(stderr, "Realloc err.");
        exit(99);
    }
    f->param_names = names;
    f->param_types = types;
    f->param_names[f->param_count] = name;
    f->param_types[f->param_count] = type;
    f->param_count++;
}
// Appends the last statement on the end of the first statement->next list
Statement* append_statement(Statement* first, Statement* last)
{
    Statement* s = first;
    if (s == NULL) {
        return last;
    }
    while (s->next != NULL) {
        s = s->next;
    }
    s->next = last;
    return first;
}
// Makes a new CALL or VAR Expression when its name was already known
Expression* make_ident_expr(char* name)
{
    Expression* e = { 0 };
    if (tok.type == T_LPAREN) { // ID and then (
        e = new_expr(E_CALL);
        e->name = name;
        next_token();
        e->args = parse_arguments();
        expect_type(T_RPAREN);
    } else { // ID and then not (
        e = new_expr(E_VAR);
        e->name = name;
    }
    return e;
}
// Frees the given Func structure
void free_func(Func* f)
{
    while (f != NULL) {
        Func* next = f->next;
        free(f->func_name);
        for (int i = 0; i < f->param_count; i++) {
            free(f->param_names[i]);
        }

        free(f->param_names);
        free(f->param_types);
        free_statement(f->body);
        free(f);
        f = next;
    }
}
// Frees the given Statement structure
void free_statement(Statement* s)
{
    while (s != NULL) {
        Statement* next = s->next;
        free(s->name);
        free_expression(s->expr);
        free_statement(s->body);
        free_statement(s->else_body);
        free(s);
        s = next;
    }
}
// Frees the given Expression structure
void free_expression(Expression* e)
{
    while (e != NULL) {
        Expression* next = e->next;
        free(e->name);
        free(e->string_val);
        free_expression(e->left);
        free_expression(e->right);
        free_expression(e->args);
        free(e);
        e = next;
    }
}

/*=========================
 * PRECEDENCE ANALYZATOR
 * ========================
 */
// Initializes a stack
Stack* stack_init()
{
    Stack* s = calloc_and_check(sizeof(Stack));
    s->data = malloc(4 * sizeof(PrecItem));

    if (s->data == NULL) {
        fprintf(stderr, "Malloc err.");
        exit(99);
    }

    s->top = -1;
    s->capacity = 4;
    return s;
}
// Doubles the size of the given stack
void resize_stack(Stack* s)
{
    int new_capacity = s->capacity * 2;

    PrecItem* temp = realloc(s->data, new_capacity * sizeof(PrecItem));

    if (temp == NULL) {
        fprintf(stderr, "Realloc err.");
        exit(99);
    }

    s->data = temp;
    s->capacity = new_capacity;
}

// Adds an item to the top of the stack
void push(Stack* s, PrecItem item)
{

    if (s->top + 1 == s->capacity) {
        resize_stack(s);
    }
    s->top++;
    s->data[s->top] = item;
}
// Pop x-times
// Removes the top x items from the stack
void popx(Stack* s, int x)
{
    for (int i = x; i > 0; i--) {
        s->top--;
    }
}

// Frees the given stack
void free_stack(Stack* s)
{
    free(s->data);
    free(s);
}
// Returns the index of top most terminal on given stack
int top_terminal_idx(Stack* s)
{
    for (int i = s->top; i > -1; i--) {
        if (!(s->data[i].is_expression)) {
            return i;
        }
    }
    syntax_error("top_terminal_idx - stack is empty."); // Should not ever happen
    return 0; // Will never happen because syntax error does exit(2), but it is a warning if it is not there
}
// Does syntax_error, if the given expression is call expression
// Used so the call expression can never be inside of an expression
void reject_if_call(Expression* e)
{
    if (e != NULL && e->kind == E_CALL) {
        syntax_error("Function call inside an expression.");
    }
}
// Table for precedence analyzator
char prec_table[9][9] = {
    // U   M/D  P/M  REL  EQ   L_P  R_P  ID   DOLL
    { '<', '>', '>', '>', '>', '<', '>', '<', '>' }, // UNARY
    { '<', '>', '>', '>', '>', '<', '>', '<', '>' }, // MUL/DIV
    { '<', '<', '>', '>', '>', '<', '>', '<', '>' }, // PLUS/MINUS
    { '<', '<', '<', 'B', '>', '<', '>', '<', '>' }, // REL
    { '<', '<', '<', '<', 'B', '<', '>', '<', '>' }, // EQ
    { '<', '<', '<', '<', '<', '<', '=', '<', 'B' }, // L_PAREN
    { 'B', '>', '>', '>', '>', 'B', '>', 'B', '>' }, // R_PAREN
    { 'B', '>', '>', '>', '>', 'B', '>', 'B', '>' }, // ID
    { '<', '<', '<', '<', '<', '<', 'B', '<', 'A' } // DOLLAR
};
// Makes a PrecItem out of given symbol and pushes it on the stack
void shift(Stack* s, PrecSym symbol, bool* prev_is_operand, int* depth)
{
    // Initializes a stack item
    PrecItem item = { 0 };
    item.symbol = symbol;
    item.is_expression = false;
    item.has_handle_after = false;

    switch (symbol) {
    case P_ID: { // Adds E on the stack
        // Copies the previous token
        char* name = NULL;
        if (tok.type == T_STRING || tok.type == T_IDENTIFIER) {
            name = strdup_and_check(tok.attr.string_value);
        }
        TokenType prev_type = tok.type;
        int prev_int_value;
        double prev_double_value;
        char* prev_string_value;
        switch (prev_type) {
        case T_INT:
            prev_int_value = tok.attr.int_value;
            break;
        case T_DOUBLE:
            prev_double_value = tok.attr.double_value;
            break;
        case T_STRING:
            prev_string_value = name;
            break;
        default:
            break;
        }

        next_token();
        // Makes a CALL expression if the tokens were ID and then (
        if (tok.type == T_LPAREN) {
            if (prev_type == T_IDENTIFIER) {
                item.node = new_expr(E_CALL);
                item.node->name = name;
                next_token();
                item.node->args = parse_arguments();
                expect_type(T_RPAREN);
            } else {
                syntax_error("Lit and then (.");
            }
        } else { // Makes a variable or a value expression
            switch (prev_type) {
            case T_IDENTIFIER:
                item.node = new_expr(E_VAR);
                item.node->name = name;
                break;
            case T_INT:
                item.node = new_expr(E_INT);
                item.node->int_val = prev_int_value;
                item.node->type = D_INT;
                break;
            case T_DOUBLE:
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
                syntax_error("Shift error - invalid symbol.");
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
    default: // Adds an operator on the stack
        switch (tok.type) {
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
            syntax_error("Shift error - invalid symbol.");
        }
        *prev_is_operand = false;
        break;
    }
    push(s, item);
    // If the symbol was ID then next_token was already called
    if (symbol != P_ID) {
        next_token();
    }
}
// Adds a HANDLE after the given idx on the stack
void insert_handle(Stack* s, int idx)
{
    s->data[idx].has_handle_after = true;
}
// Reduces expressions on the stack using precedence analyzator rules
void reduce(Stack* s)
{
    int n = 0; // Tracks how many items are before a HANDLE
    for (int i = s->top; i > -1; i--) { // Looks for a HANDLE and counts how many items are before HANDLE
        if (s->data[i].has_handle_after) {
            switch (n) {
            case 1: // Checks if there is HANDLE and then ID
                if (!(s->data[i + 1].is_expression) && s->data[i + 1].symbol == P_ID) { // Rule i -> E
                    s->data[s->top].is_expression = true;
                } else {
                    syntax_error("Reduce function err - expected HANDLE then i.");
                }
                break;
            case 2:
                if (!(s->data[i + 1].is_expression) && !(s->data[i + 2].is_expression)) {
                    if (s->data[i + 1].symbol == P_LPAREN && s->data[i + 2].symbol == P_RPAREN) { // Rule () -> E
                        PrecItem item = { 0 };
                        item.is_expression = true;
                        item.has_handle_after = false;

                        item.node = new_expr(E_UNIT);
                        item.node->type = D_UNIT;
                        // Removes the ( and )
                        popx(s, 2);
                        // Adds E on the stack
                        push(s, item);
                    } else {
                        syntax_error("Reduce function err - expected ().");
                    }
                } else if (!(s->data[i + 1].is_expression) && s->data[i + 2].is_expression) {
                    if (s->data[i + 1].symbol == P_UNARY) { // Rule UNARY E -> E
                        reject_if_call(s->data[i + 2].node); // Checks if it is a call expression

                        PrecItem item = { 0 };
                        item.is_expression = true;
                        item.has_handle_after = false;

                        item.node = new_expr(E_UNARY);
                        item.node->op = s->data[i + 1].op;
                        item.node->left = s->data[i + 2].node;
                        // Removes the UNARY and E
                        popx(s, 2);
                        // Adds E on the stack
                        push(s, item);
                    } else {
                        syntax_error("Reduce function err - expected UNARY E.");
                    }
                } else {
                    syntax_error("Reduce function err - expected () or UNARY E.");
                }
                break;
            case 3:
                if (!(s->data[i + 1].is_expression) && s->data[i + 2].is_expression && !(s->data[i + 3].is_expression)) {
                    if (s->data[i + 1].symbol == P_LPAREN && s->data[i + 3].symbol == P_RPAREN) { // Rule (E) -> E
                        reject_if_call(s->data[i + 2].node); // Checks if it is a call expression

                        PrecItem item = { 0 };
                        item.is_expression = true;
                        item.has_handle_after = false;

                        item.node = s->data[i + 2].node;
                        // Removes the (E)
                        popx(s, 3);
                        // Adds E on the stack
                        push(s, item);
                    } else {
                        syntax_error("Reduce function err - expected (E).");
                    }
                } else if (s->data[i + 1].is_expression && !(s->data[i + 2].is_expression) && s->data[i + 3].is_expression) {
                    reject_if_call(s->data[i + 1].node); // Checks if it is a call expression
                    reject_if_call(s->data[i + 3].node); // Checks if it is a call expression

                    PrecItem item = { 0 };
                    item.is_expression = true;
                    item.has_handle_after = false;

                    item.node = new_expr(E_BINARY);

                    switch (s->data[i + 2].symbol) { // Rule E op E -> E
                    case P_MULDIV:
                    case P_PLUSMINUS:
                    case P_REL:
                    case P_EQ:
                        item.node->op = s->data[i + 2].op;
                        item.node->left = s->data[i + 1].node;
                        item.node->right = s->data[i + 3].node;
                        // Removes the E op E
                        popx(s, 3);
                        // Adds E on the stack
                        push(s, item);
                        break;
                    default:
                        syntax_error("Reduce function err - expected E op E.");
                        break;
                    }
                } else {
                    syntax_error("Reduce function err - expected (E) or E op E.");
                }
                break;
            default:
                syntax_error("Reduce function err - invalid symbol.");
            }
            s->data[i].has_handle_after = false;
            return;
        }
        n++;
    }
    syntax_error("Reduce function err - did not find any HANDLE."); // Should not ever happen
}

// Makes a PrecSym from current token and returns it
PrecSym tok_to_sym(bool prev_is_operand, int depth)
{
    PrecSym symbol;
    switch (tok.type) {
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
    case T_DOUBLE:
    case T_STRING:
        symbol = P_ID;
        break;
    case T_PLUS:
    case T_MINUS:
        if (prev_is_operand) {
            symbol = P_PLUSMINUS;
        } else {
            symbol = P_UNARY;
        }
        break;
    case T_LPAREN:
        symbol = P_LPAREN;
        break;
    case T_RPAREN:
        if (depth > 0) {
            symbol = P_RPAREN;
        } else {
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
// Checks the syntax of the whole program
Func* parse_program()
{
    if (tok.type != T_IMPORT)
        syntax_error("Missing or invalid prologue.");

    parse_prologue();
    expect_type(T_EOL);
    Func* f = { 0 };
    f = parse_func_list();
    expect_type(T_EOF);
    return f;
}
// Checks the syntax of prologue
void parse_prologue()
{
    expect_type(T_IMPORT);
    if (tok.type == T_IDENTIFIER) {
        if (strcmp(tok.attr.string_value, "ifj26") != 0) {
            syntax_error("Invalid prologue - expected ifj26 after IMPORT.");
        }
    } else {
        syntax_error("Invalid prologue - expected ifj26 after IMPORT.");
    }
    next_token();
    expect_type(T_DOT);
    expect_type(T_MULTIPLY);
}
// Checks syntax of all functions
Func* parse_func_list()
{
    Func* f = { 0 };
    switch (tok.type) {
    case T_DEF:
    case T_AT:
        f = parse_func_def();
        f->next = parse_func_list();
        break;
    case T_EOF:
        return NULL;
    default:
        syntax_error("Invalid function definition.");
    }
    return f;
}
// Checks syntax of a function definition
Func* parse_func_def()
{
    Func* f = new_func();
    switch (tok.type) {
    case T_DEF:
    case T_AT:
        f->is_main = parse_annot();
        expect_type(T_DEF);
        if (tok.type == T_IDENTIFIER) {
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
        syntax_error("Invalid function definition.");
    }
    return f;
}
// Checks syntax of function annotation
bool parse_annot()
{
    bool is_main = false;
    switch (tok.type) {
    case T_AT:
        next_token();
        if (tok.type == T_IDENTIFIER) {
            if (strcmp(tok.attr.string_value, "main") != 0) {
                syntax_error("Expected main after @.");
            } else {
                is_main = true;
            }
        }
        expect_type(T_IDENTIFIER);
        parse_annot_n();
        break;
    case T_DEF:
        break;
    default:
        syntax_error("Invalid function annotation.");
    }
    return is_main;
}
// Checks syntax of function annotation
// Checks if annotation is on its own line or before def
void parse_annot_n()
{
    switch (tok.type) {
    case T_EOL:
        next_token();
        break;
    case T_DEF:
        break;
    default:
        syntax_error("Invalid function annotation.");
    }
}
// Checks syntax of all function parameters
void parse_params(Func* f)
{
    switch (tok.type) {
    case T_RPAREN:
        break;
    case T_IDENTIFIER: {
        char* param_name = strdup_and_check(tok.attr.string_value);
        next_token();
        expect_type(T_COLON);
        DataType param_type = parse_type();
        add_param(f, param_name, param_type);
        parse_params_n(f);
        break;
    }
    default:
        syntax_error("Invalid function parameters.");
    }
}
// Checks syntax of function parameters
void parse_params_n(Func* f)
{
    switch (tok.type) {
    case T_COMMA: {
        int last_token_line = tok.line;
        next_token();
        char* param_name = NULL;
        if (tok.type == T_IDENTIFIER) {
            param_name = strdup_and_check(tok.attr.string_value);
        }
        if (tok.type == T_RPAREN) {
            if (last_token_line == tok.line) {
                syntax_error("Invalid function parameters - Expected a valid parameter after comma.");
            } else {
                break;
            }
        }
        expect_type(T_IDENTIFIER);
        expect_type(T_COLON);
        DataType param_type = parse_type();
        add_param(f, param_name, param_type);
        parse_params_n(f);
        break;
    }
    case T_RPAREN:
        break;
    default:
        syntax_error("Invalid function parameters - Expected a valid parameter after comma.");
    }
}
// Checks syntax of function body
Statement* parse_func_body()
{
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_INT:
    case T_DOUBLE:
    case T_STRING:
    case T_LPAREN:
    case T_PLUS:
    case T_MINUS: {
        Expression* e = parse_expression();
        Statement* s = new_stmt(S_EXPR);
        s->expr = e;
        expect_type(T_EOL);
        return s;
    }
    case T_EOL:
        next_token();
        return parse_block();
    default:
        syntax_error("Invalid function body.");
    }
    return NULL;
}
// Checks syntax of function return type declaration
DataType parse_n_type()
{
    DataType d;
    switch (tok.type) {
    case T_EQUAL:
        d = D_UNKNOWN;
        break;
    case T_COLON:
        next_token();
        d = parse_type();
        break;
    default:
        syntax_error("Invalid function return data type.");
    }
    return d;
}
// Checks syntax of function return type declaration
DataType parse_type()
{
    DataType d;
    switch (tok.type) {
    case TK_UNIT:
        d = D_UNIT;
        break;
    case TK_INT:
        d = D_INT;
        break;
    case TK_DOUBLE:
        d = D_DOUBLE;
        break;
    case TK_STRING:
        d = D_STRING;
        break;
    default:
        syntax_error("Invalid function return or function parameters data type.");
    }
    next_token();
    return d;
}
// Checks syntax of a block of code
Statement* parse_block()
{
    Statement* s = { 0 };
    switch (tok.type) {
    case T_INDENT:
        next_token();
        s = parse_statement();
        expect_type(T_DEDENT);
        break;
    default:
        syntax_error("Invalid block syntax.");
    }
    return s;
}
// Checks syntax of a statement
Statement* parse_statement()
{
    Statement* s = { 0 };
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_IF:
    case T_WHILE:
    case T_RETURN:
    case T_INT:
    case T_DOUBLE:
    case T_STRING:
    case T_LPAREN:
    case T_VAL:
    case T_VAR:
    case T_PLUS:
    case T_MINUS: {
        Statement* line = parse_statement_line();
        Statement* rest = parse_statement_n();
        s = append_statement(line, rest);
        break;
    }
    default:
        syntax_error("Invalid statement syntax.");
    }
    return s;
}
// Checks syntax of a statement
Statement* parse_statement_n()
{
    Statement* s = { 0 };
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_IF:
    case T_WHILE:
    case T_RETURN:
    case T_INT:
    case T_DOUBLE:
    case T_STRING:
    case T_LPAREN:
    case T_VAL:
    case T_VAR:
    case T_PLUS:
    case T_MINUS: {
        Statement* line = parse_statement_line();
        Statement* rest = parse_statement_n();
        s = append_statement(line, rest);
        break;
    }
    case T_DEDENT:
        s = NULL;
        break;
    default:
        syntax_error("Invalid statement syntax.");
    }
    return s;
}
// Checks syntax of statement on one line
Statement* parse_statement_line()
{
    Statement* s = { 0 };
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_RETURN:
    case T_INT:
    case T_DOUBLE:
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
        syntax_error("Invalid statement syntax.");
    }
    return s;
}
// Checks syntax of a statement that is not IF or WHILE
Statement* parse_simple_statement()
{
    Statement* s = { 0 };
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_RETURN:
    case T_INT:
    case T_DOUBLE:
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
        syntax_error("Invalid statement syntax.");
    }
    return s;
}
// Checks syntax of a statement that is not IF or WHILE
Statement* parse_simple_n()
{
    Statement* s = { 0 };
    switch (tok.type) {
    case T_SEMICOLON:
        next_token();
        s = parse_simple_s();
        s->next = parse_simple_n();
        break;
    case T_EOL:
        s = NULL;
        break;
    default:
        syntax_error("Invalid statement syntax.");
    }
    return s;
}
// Checks syntax of a statement that is not IF or WHILE
Statement* parse_simple_s()
{
    Statement* s = { 0 };
    switch (tok.type) {
    case T_IDENTIFIER: {
        char* name = strdup_and_check(tok.attr.string_value);
        next_token();
        if (tok.type == T_EQUAL) {
            next_token();
            s = new_stmt(S_ASSIGN);
            s->name = name;
            s->expr = parse_expression();
        } else {
            Expression* e = make_ident_expr(name);
            s = new_stmt(S_EXPR);
            s->expr = parse_expression_from(e);
        }
        break;
    }
    case T_INT:
    case T_DOUBLE:
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
        syntax_error("Invalid statement syntax.");
    }
    return s;
}
// Checks syntax of an IF statement
Statement* parse_if_statement()
{
    Statement* s = new_stmt(S_IF);
    switch (tok.type) {
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
        syntax_error("Invalid if statement syntax.");
    }
    return s;
}
// Checks syntax of a WHILE statement
Statement* parse_while_statement()
{
    Statement* s = new_stmt(S_WHILE);
    switch (tok.type) {
    case T_WHILE:
        next_token();
        s->expr = parse_condition();
        expect_type(T_DO);
        expect_type(T_EOL);
        s->body = parse_block();
        break;
    default:
        syntax_error("Invalid while statement syntax.");
    }
    return s;
}
// Checks syntax of condition
Expression* parse_condition()
{
    Expression* e = { 0 };
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_INT:
    case T_DOUBLE:
    case T_STRING:
    case T_LPAREN:
    case T_PLUS:
    case T_MINUS:
        e = parse_expression();
        break;
    default:
        syntax_error("Invalid statement condition.");
    }
    return e;
}
// Checks syntax of return statement
Statement* parse_return_statement()
{
    Statement* s = { 0 };
    switch (tok.type) {
    case T_RETURN:
        next_token();
        s = parse_return_val();
        break;
    default:
        syntax_error("Missing return statement.");
    }
    return s;
}
// Checks syntax of return value
Statement* parse_return_val()
{
    Statement* s = new_stmt(S_RETURN);
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_INT:
    case T_DOUBLE:
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
        syntax_error("Invalid return value.");
    }
    return s;
}
// Checks syntax of function arguments
Expression* parse_arguments()
{
    Expression* e = { 0 };
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_INT:
    case T_DOUBLE:
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
        syntax_error("Invalid function arguments.");
    }
    return e;
}
// Checks syntax of function arguments
Expression* parse_arguments_n()
{
    Expression* e = { 0 };
    switch (tok.type) {
    case T_COMMA:
        // next_token();  should be here, but the parse_arguments_nn needs the previous token.line value, so it is in there
        e = parse_arguments_nn();
        break;
    case T_RPAREN:
        e = NULL;
        break;
    default:
        syntax_error("Expected comma between function arguments.");
    }
    return e;
}
// Checks syntax of function arguments
Expression* parse_arguments_nn()
{
    Expression* e = { 0 };
    int last_token_line = tok.line;
    next_token();
    switch (tok.type) {
    case T_IDENTIFIER:
    case T_INT:
    case T_DOUBLE:
    case T_STRING:
    case T_LPAREN:
    case T_MINUS:
        e = parse_term();
        e->next = parse_arguments_n();
        break;
    case T_RPAREN:
        if (last_token_line == tok.line) {
            syntax_error("Invalid function arguments - expected valid argument after comma and before ).");
        } else {
            e = NULL;
        }
        break;
    default:
        syntax_error("Invalid function arguments - expected valid argument after comma.");
    }
    return e;
}
// Checks syntax of function argument values
Expression* parse_term()
{
    Expression* e = { 0 };
    switch (tok.type) {
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
    case T_DOUBLE:
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
        syntax_error("Invalid function arguments.");
    }
    return e;
}
// Checks syntax of function arguments that have an UNARY sign
Expression* parse_num_lit()
{
    Expression* e = { 0 };
    switch (tok.type) {
    case T_INT:
        e = new_expr(E_INT);
        e->int_val = tok.attr.int_value;
        next_token();
        break;
    case T_DOUBLE:
        e = new_expr(E_DOUBLE);
        e->double_val = tok.attr.double_value;
        next_token();
        break;
    default:
        syntax_error("Invalid function arguments - epected float or int after UNARY sign.");
    }
    return e;
}
// Checks syntax of variable or value definition
Statement* parse_var_def()
{
    Statement* s = new_stmt(S_VAR_DEF);
    switch (tok.type) {
    case T_VAL:
    case T_VAR:
        s->is_var = parse_vax();
        if (tok.type == T_IDENTIFIER) {
            s->name = strdup_and_check(tok.attr.string_value);
        }
        expect_type(T_IDENTIFIER);
        s->type = parse_n_type();
        expect_type(T_EQUAL);
        s->expr = parse_expression();
        break;
    default:
        syntax_error("Invalid variable or value definition.");
    }
    return s;
}
// Checks syntax of variable or value definition
bool parse_vax()
{
    bool is_var = false;
    switch (tok.type) {
    case T_VAL:
        next_token();
        break;
    case T_VAR:
        is_var = true;
        next_token();
        break;
    default:
        syntax_error("Invalid variable or value definition.");
    }
    return is_var;
}
/* ==================
 * END OF LL TABLE FUNCTIONS
 * ==================
 */

// Parses the expression using precedence analyzator
Expression* parse_expression()
{
    return parse_expression_from(NULL);
}
// Parses the expression using precedence analyzator and adds an expression on the stack if given
Expression* parse_expression_from(Expression* first)
{
    Stack* s = stack_init();
    // Puts dollar on the stack
    PrecItem item = { 0 };
    item.symbol = P_DOLLAR;
    item.is_expression = false;
    item.has_handle_after = false;
    push(s, item);

    bool prev_is_operand;
    // Puts the first Expression on the stack if it is provided
    if (first != NULL) {
        PrecItem item2 = { 0 };
        item2.is_expression = true;
        item2.has_handle_after = false;
        item2.node = first;
        push(s, item2);

        prev_is_operand = true;
    } else {
        prev_is_operand = false;
    }
    int depth = 0; // Tracks the depth of parentheses

    while (true) {
        PrecSym b = tok_to_sym(prev_is_operand, depth);
        PrecSym a = s->data[top_terminal_idx(s)].symbol;

        switch (prec_table[a][b]) {
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
            if (s->top == 1 && s->data[1].is_expression) {
                Expression* result = s->data[1].node;
                free_stack(s);
                return result;
            } else {
                syntax_error("Prec analysis - ACCEPT but stack is not only $E");
            }
            break;
        default: // Blank space in precedence table
            syntax_error("Prec analysis - Invalid expression.");
        }
    }
}
/*
int main()
{
    FILE* test_file = fopen("test.txt", "r");
    if (test_file == NULL) {
        return -1;
    }
    scanner = scanner_init(test_file);

    next_token();

    Func* f = { 0 };
    f = parse_program();

    token_destroy(&tok);
    scanner_destroy(&scanner);
    free_func(f);

    fclose(test_file);
    return 0;
}
*/
