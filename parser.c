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
 * fix syntax_error func
 * add comments
 *
 * Vyresit to ze muze byt:
 * var a = (TK_UNIT+
 *         T_COMMA+
 *         T_MINUS)
*/
Token tok;


// MAIN, DEDENT, INDENT
// T_AT     T_DEDENT      T_INDENT



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

void parse_expression(){
    switch (tok.type){
        case T_IDENTIFIER:
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_LPAREN:
        case T_PLUS:
        case T_MINUS:
            parse_equal();
            break;
        default:
            syntax_error(-1, "expression");
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
