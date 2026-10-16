#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Scanner.h"
#include <stdbool.h>

/* TODO
 * remove debug things
 *
 * fix syntax_error func
 * add comments
 *
 * Vyresit to ze muze byt:
 * var a = (10+
 *         20+
 *         30)
*/
Token tok;

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

// MAIN, DEDENT, INDENT
// 24     72      73



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


    "T_IDENT",      // 40
    "T_EOL",        // 41
    "T_ERROR",      // 42
    "T_EOF"         // 43
};
// for debug
int token_count = 1;

// for debug
void print_token(){
    printf("Type: %d aka %s, Attr:[%s], NUMBER:%d\n", tok.type, token_names[tok.type], tok.attr, token_count);
}

// AST tree structs

typedef enum {
    E_INT,
    E_DOUBLE,
    E_STRING,
    E_UNIT,
    E_VAR,
    E_CALL,
    E_UNARY,
    E_BINARY
}StatementKind;

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

// end of AST tree structs

void next_token(){
    tok = get_next_token();
    token_count++;
    if (tok.type == 40){
        token_count--;
        next_token();
    }
}


void syntax_error(TokenType t, char* str){
    if (t == -1){
        printf("SWITCH FAIL u %s, last token: ", str);
        print_token();
    }
    else{
        printf("Syntax error, EXPECTED %d aka %s, got %d aka %s, NUMBER:%d\n", t, token_names[t], tok.type, token_names[tok.type], token_count);
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

/*========================
 * ALL THE RULES
 * =======================
 */

void parse_program(){
    if (tok.type != 5 ) syntax_error(5, "abab");

    parse_prologue();
    expect_type(41);
    parse_func_list();
    expect_type(43);
}

void parse_prologue(){
    expect_type(5);
    expect_type(39);
    expect_type(23);
    expect_type(31);
}

void parse_func_list(){
    switch (tok.type){
        case 0:
        case 24:
            parse_func_def();
            parse_func_list();
            break;
        case 43:
            break;
        default:
            syntax_error(-1,"func_list");
    }
}
void parse_func_def(){
    switch (tok.type){
        case 0:
        case 24:
            parse_annot();
            expect_type(0);
            expect_type(39);
            expect_type(14);
            parse_params();
            expect_type(15);
            parse_n_type();
            expect_type(37);
            parse_func_body();
            break;
        default:
            syntax_error(-1, "func_def");
    }
}

void parse_annot(){
    switch (tok.type){
        case 24:
            next_token();
            expect_type(39);
            parse_annot_n();
            break;
        case 0:
            break;
        default:
            syntax_error(-1, "annot");
    }
}

void parse_annot_n(){
    switch (tok.type){
        case 41:
            next_token();
            break;
        case 0:
            break;
        default:
            syntax_error(-1, "annot_n");
    }
}

void parse_params(){
    switch (tok.type){
        case 15:
            break;
        case 39:
            next_token();
            expect_type(22);
            parse_type();
            parse_params_n();
            break;
        default:
            syntax_error(-1, "params");
    }
}

void parse_params_n(){
    switch (tok.type){
        case 20:
            next_token();
            expect_type(39);
            expect_type(22);
            parse_type();
            parse_params_n();
            break;
        case 15:
            break;
        default:
            syntax_error(-1, "params_n");
    }
}


void parse_func_body(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_expression();
            expect_type(41);
            break;
        case 41:
            next_token();
            parse_block();
            break;
        default:
            syntax_error(-1, "func_body");
    }
}

void parse_n_type(){
    switch (tok.type){
        case 37:
            break;
        case 22:
            next_token();
            parse_type();
            break;
        default:
            syntax_error(-1, "n_type");
    }
}

void parse_type(){
    switch (tok.type){
        case 10:
        case 6:
        case 2:
        case 8:
            next_token();
            break;
        default:
            syntax_error(-1, "type");
    }
}

void parse_block(){
    switch (tok.type){
        case 73:
            parse_statement();
            expect_type(72);
            break;
        default:
            syntax_error(-1, "block");
    }
}

void parse_statement(){
    switch (tok.type){
        case 39:
        case 4:
        case 13:
        case 7:
        case 25:
        case 27:
        case 26:
        case 14:
        case 11:
        case 12:
        case 29:
        case 30:
            parse_statement_line();
            parse_statement_n();
            break;
        default:
            syntax_error(-1, "statement");
    }
}

void parse_statement_n(){
    switch (tok.type){
        case 39:
        case 4:
        case 13:
        case 7:
        case 25:
        case 27:
        case 26:
        case 14:
        case 11:
        case 12:
        case 29:
        case 30:
            parse_statement_line();
            parse_statement_n();
            break;
        case 72:
            break;
        default:
            syntax_error(-1, "statement_n");
    }
}

void parse_statement_line(){
    switch (tok.type){
        case 39:
        case 7:
        case 25:
        case 27:
        case 26:
        case 14:
        case 11:
        case 12:
        case 29:
        case 30:
            parse_simple_statement();
            expect_type(41);
            break;
        case 4:
            parse_if_statement();
            break;
        case 13:
            parse_while_statement();
            break;
        default:
            syntax_error(-1, "statement_line");
    }
}

void parse_simple_statement(){
    switch (tok.type){
        case 39:
        case 7:
        case 25:
        case 27:
        case 26:
        case 14:
        case 11:
        case 12:
        case 29:
        case 30:
            parse_simple_s();
            parse_simple_n();
            break;
        default:
            syntax_error(-1, "simple_statement");
    }
}

void parse_simple_n(){
    switch (tok.type){
        case 21:
            next_token();
            parse_simple_s();
            parse_simple_n();
            break;
        case 41:
            break;
        default:
            syntax_error(-1, "simple_n");
    }
}

void parse_simple_s(){
    switch (tok.type){
        case 39:
            next_token();
            parse_simple_id();
            break;
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_unary_noid();
            parse_expression_n();
            break;
        case 11:
        case 12:
            parse_var_def();
            break;
        case 7:
            parse_return_statement();
            break;
        default:
            syntax_error(-1, "simple_s");
    }
}

void parse_simple_id(){
    switch (tok.type){
        case 41:
        case 31:
        case 21:
        case 14:
        case 38:
        //case NEQ:
        case 35:
        case 36:
        case 33:
        case 34:
        case 29:
        case 30:
        case 32:
            parse_primary_id();
            parse_expression_n();
            break;
        case 37:
            next_token();
            parse_expression();
            break;
        default:
            syntax_error(-1, "simple_id");
    }
}

void parse_if_statement(){
    switch (tok.type){
        case 4:
            next_token();
            parse_condition();
            expect_type(9);
            expect_type(41);
            parse_block();
            expect_type(3);
            expect_type(41);
            parse_block();
            break;
        default:
            syntax_error(-1, "if_statement");
    }
}

void parse_while_statement(){
    switch (tok.type){
        case 13:
            next_token();
            parse_condition();
            expect_type(1);
            expect_type(41);
            parse_block();
            break;
        default:
            syntax_error(-1, "while_statement");
    }
}
/////////////////////////////////////////////////////// CONDITION
void parse_condition(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            break;
        default:
            syntax_error(-1, "condition");
    }
}

void parse_return_statement(){
    switch (tok.type){
        case 7:
            next_token();
            parse_return_val();
            break;
        default:
            syntax_error(-1, "return_statement");
    }
}

void parse_return_val(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_expression();
            break;
        case 41:
        case 21:
            break;
        default:
            syntax_error(-1, "return_val");
    }
}

void parse_arguments(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 30:
            parse_term();
            parse_arguments_n();
            break;
        case 15:
            break;
        default:
            syntax_error(-1, "arguments");
    }
}

void parse_arguments_n(){
    switch (tok.type){
        case 20:
            next_token();
            parse_arguments_nn();
            break;
        case 15:
            break;
        default:
            syntax_error(-1, "arguments_n");
    }
}

void parse_arguments_nn(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 30:
            parse_term();
            parse_arguments_n();
            break;
        default:
            syntax_error(-1, "arguments_nn");
    }
}

void parse_term(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
            next_token();
            break;
        case 14:
            next_token();
            expect_type(15);
            break;
        case 30:
            next_token();
            parse_num_lit();
            break;
        default:
            syntax_error(-1, "term");
    }
}

void parse_num_lit(){
    switch (tok.type){
        case 25:
        case 27:
            next_token();
            break;
        default:
            syntax_error(-1, "num_lit");
    }
}

void parse_var_def(){
    switch (tok.type){
        case 11:
        case 12:
            parse_vax();
            expect_type(39);
            parse_n_type();
            expect_type(37);
            parse_expression();
            break;
        default:
            syntax_error(-1, "var_def");
    }
}

void parse_vax(){
    switch (tok.type){
        case 11:
        case 12:
            next_token();
            break;
        default:
            syntax_error(-1, "vax");
    }
}

void parse_expression(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_equal();
            break;
        default:
            syntax_error(-1, "expression");
    }
}

void parse_expression_n(){
    switch (tok.type){
        case 41:
        case 31:
        case 21:
        case 38:
        //case NEQ:
        case 35:
        case 36:
        case 33:
        case 34:
        case 29:
        case 30:
        case 32:
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
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_comparison();
            parse_equal_n();
            break;
        default:
            syntax_error(-1, "equal");
    }
}

void parse_equal_n(){
    switch (tok.type){
        case 41:
        case 21:
        case 9:
        case 1:
        case 15:
            break;
        case 38:
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
        case 38:
        //case NEQ:
            next_token();
            break;
        default:
            syntax_error(-1, "equal_op");
    }
}

void parse_comparison(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_addition();
            parse_comparison_n();
            break;
        default:
            syntax_error(-1, "comparison");
    }
}

void parse_comparison_n(){
    switch (tok.type){
        case 41:
        case 21:
        case 9:
        case 1:
        case 15:
        case 38:
        //case NEQ:
            break;
        case 35:
        case 36:
        case 33:
        case 34:
            parse_comparison_op();
            parse_addition();
            break;
        default:
            syntax_error(-1, "comparison_n");
    }
}

void parse_comparison_op(){
    switch (tok.type){
        case 35:
        case 36:
        case 33:
        case 34:
            next_token();
            break;
        default:
            syntax_error(-1, "comparison_op");
    }
}

void parse_addition(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_mult();
            parse_addition_n();
            break;
        default:
            syntax_error(-1, "addition");
    }
}

void parse_addition_n(){
    switch (tok.type){
        case 41:
        case 21:
        case 9:
        case 1:
        case 15:
        case 38:
        //case NEQ:
        case 35:
        case 36:
        case 33:
        case 34:
            break;
        case 29:
            next_token();
            parse_mult();
            parse_addition_n();
            break;
        case 30:
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
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_unary();
            parse_mult_n();
            break;
        default:
            syntax_error(-1, "mult");
    }
}

void parse_mult_n(){
    switch (tok.type){
        case 41:
        case 21:
        case 9:
        case 1:
        case 15:
        case 38:
        //case NEQ:
        case 35:
        case 36:
        case 33:
        case 34:
        case 29:
        case 30:
            break;
        case 31:
            next_token();
            parse_unary();
            parse_mult_n();
            break;
        case 32:
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
        case 39:
            next_token();
            parse_primary_id();
            break;
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_unary_noid();
            break;
        default:
            syntax_error(-1, "unary");
    }
}

void parse_unary_noid(){
    switch (tok.type){
        case 25:
            next_token();
            break;
        case 27:
            next_token();
            break;
        case 26:
            next_token();
            break;
        case 14:
            next_token();
            parse_primary_n();
            break;
        case 29:
            next_token();
            parse_unary();
            break;
        case 30:
            next_token();
            parse_unary();
            break;
        default:
            syntax_error(-1, "unary_noid");
    }
}

void parse_primary_id(){
    switch (tok.type){
        case 41:
        case 31:
        case 21:
        case 9:
        case 1:
        case 15:
        case 38:
        //case NEQ:
        case 35:
        case 36:
        case 33:
        case 34:
        case 29:
        case 30:
        case 32:
            break;
        case 14:
            next_token();
            parse_arguments();
            expect_type(15);
            break;
        default:
            syntax_error(-1, "primary_id");
    }
}

void parse_primary_n(){
    switch (tok.type){
        case 39:
        case 25:
        case 27:
        case 26:
        case 14:
        case 29:
        case 30:
            parse_expression();
            expect_type(15);
            break;
        case 15:
            next_token();
            break;
        default:
            syntax_error(-1, "primary_n");
    }
}
/* ==================
 * END OF RULES
 * ==================
 */


int main (){
    FILE* test_file = fopen("test.txt","r");
    if (test_file == NULL){
        return -1;
    }
    scanner_init(test_file);

    next_token();


    parse_program();
    printf("uz je konec hej\n");

    fclose(test_file);
    return 0;
}
