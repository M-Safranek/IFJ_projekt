#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Scanner.h"
#include <stdbool.h>

/* TODO
 * remove print_token and token_names
 * predelat DEF na 0 atd...
 * fix syntax_error func
 *
 * DAN
 * co s main
 * nema indent a dedent
*/
Token tok;

void parse_program();
void parse_prologue();
void parse_func_list();
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
void print_token(){
    printf("Type: %d aka %s, Attr:[%s]\n", tok.type, token_names[tok.type], tok.attr);
}

void next_token(){
    tok = get_next_token();
}

void syntax_error(TokenType t){
    if (t == -1){
        printf("Failed u switch, last token:\n");
        print_token();
    }
    else{
        printf("Syntax error, EXPECTED %d aka %s, got %d aka %s\n", t, token_names[t], tok.type, token_names[tok.type]);
    }
}

void expect_type(TokenType t){
    if (tok.type == t){
        next_token();
    }
    else{
        syntax_error(t);
    }
}


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
