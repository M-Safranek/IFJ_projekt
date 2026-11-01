#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

/*=========================
 *LL TABLE FUNCTIONS
 * ========================
 */

void parse_program();
void parse_prologue();
void parse_func_list();
void parse_func_def();
void parse_annot();
void parse_annot_n();
void parse_params();
void parse_params_n();
void parse_func_body();
void parse_n_type();
void parse_type();
void parse_block();
void parse_statement();
void parse_statement_n();
void parse_statement_line();
void parse_simple_statement();
void parse_simple_n();
void parse_simple_s();
void parse_simple_id();
void parse_if_statement();
void parse_while_statement();
void parse_condition();
void parse_return_statement();
void parse_return_val();
void parse_arguments();
void parse_arguments_n();
void parse_arguments_nn();
void parse_term();
void parse_num_lit();
void parse_var_def();
void parse_vax();
void parse_expression();
void parse_expression_n();
void parse_equal();
void parse_equal_n();
void parse_equal_op();
void parse_comparison();
void parse_comparison_n();
void parse_comparison_op();
void parse_addition();
void parse_addition_n();
void parse_mult();
void parse_mult_n();
void parse_unary();
void parse_unary_noid();
void parse_primary_id();
void parse_primary_n();

/*==================
 * AST tree
 * =================
*/
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
}Operators;

typedef struct {
    ExpressionKind kind;
    TokenType type;
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

typedef struct {
    StatementKind kind;
    char* name;
    bool is_var;
    TokenType type;
    Expression* expr;
    struct Statement* body;
    struct Statement* else_body;
    struct Statement* next;
}Statement;

typedef struct {
    char* func_name;
    bool is_main;
    char** param_names;
    TokenType type;
    int param_count;
    TokenType return_type;
    Statement* body;
    struct Func* next;
}Func;
/*==========================
 * END of AST tree
 * =========================
 */
#endif
