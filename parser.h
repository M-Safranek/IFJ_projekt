#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

/*==================
 * AST tree
 * =================
*/
typedef enum {
    D_UNKNOWN, // first so calloc initializes it
    D_INT,
    D_DOUBLE,
    D_STRING,
    D_UNIT
}DataType;

typedef enum {
    E_INT,
    E_DOUBLE,
    E_STRING,
    E_UNIT,
    E_VAR,
    E_CALL,
    E_UNARY,
    E_BINARY
}ExpressionKind;

typedef enum {
    O_PLUS,
    O_MINUS,
    O_MULTIPLY,
    O_DIVIDE,
    O_LESS,
    O_LESS_EQUAL,
    O_GREATER,
    O_GREATER_EQUAL,
    O_EQUAL,
    O_EQUAL_EQUAL,
    O_NOT_EQUAL
}Operators;

typedef struct Expression{
    ExpressionKind kind;
    DataType type;
    Operators op;
    char* name;
    int int_val;
    double double_val;
    char* string_val;
    struct Expression* left;
    struct Expression* right;
    struct Expression* args;
    struct Expression* next;
}Expression;

typedef enum {
    S_VAR_DEF,
    S_ASSIGN,
    S_RETURN,
    S_EXPR,
    S_IF,
    S_WHILE
}StatementKind;

typedef struct Statement{
    StatementKind kind;
    char* name;
    bool is_var;
    DataType type;
    Expression* expr;
    struct Statement* body;
    struct Statement* else_body;
    struct Statement* next;
}Statement;

typedef struct Func{
    char* func_name;
    bool is_main;
    char** param_names;
    DataType* param_types;
    int param_count;
    DataType return_type;
    Statement* body;
    struct Func* next;
}Func;

Expression* new_expr(ExpressionKind k);
Statement* new_stmt(StatementKind k);
Func* new_func();
void add_param(Func* f, char* name, DataType type);
Statement* append_statement(Statement* line, Statement* rest);
Expression* make_ident_expr(char* name);

void free_func(Func* f);
void free_statement(Statement* s);
void free_expression(Expression* e);

/*==========================
 * PRECEDENCE ANALYZATOR
 * =========================
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
}Stack;

Stack* stack_init();
void resize_stack(Stack* s);
void push(Stack* s, PrecItem item);
void popx(Stack* s, int x);
void destroy_stack(Stack *s);
int top_terminal_idx(Stack* s);
void shift(Stack* s, PrecSym symbol, bool* prev_is_operand, int* depth);
void insert_handle(Stack* s, int idx);
void reduce(Stack* s);
PrecSym tok_to_sym(bool prev_is_operand, int depth);

Expression* parse_expression_from(Expression* first);

/*=========================
 *LL TABLE FUNCTIONS
 * ========================
 */

Func* parse_program();
void parse_prologue();
Func* parse_func_list();
Func* parse_func_def();
bool parse_annot();
void parse_annot_n();
void parse_params(Func* f);
void parse_params_n(Func* f);
Statement* parse_func_body();
DataType parse_n_type();
DataType parse_type();
Statement* parse_block();
Statement* parse_statement();
Statement* parse_statement_n();
Statement* parse_statement_line();
Statement* parse_simple_statement();
Statement* parse_simple_n();
Statement* parse_simple_s();
Statement* parse_if_statement();
Statement* parse_while_statement();
Expression* parse_condition();
Statement* parse_return_statement();
Statement* parse_return_val();
Expression* parse_arguments();
Expression* parse_arguments_n();
Expression* parse_arguments_nn();
Expression* parse_term();
Expression* parse_num_lit();
Statement* parse_var_def();
bool parse_vax();
Expression* parse_expression();

#endif
