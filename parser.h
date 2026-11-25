/*
 * Header file for the Parser for the IFJ26 language.
 * Author: Martin Šafránek
 */

#ifndef PARSER_H
#define PARSER_H

#include "Scanner.h"
#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Gets a new token from lexical analyzer
 *
 * Frees the previous token with token_destroy(&tok)
 * Gets a new token from lexical analyzer with get_next_token(&scanner) and checks if it is not an error
 * Terminates the whole program with exit(1) if lexical analyzer returns an error
 *
 * @see token_destroy()
 * @see get_next_token()
 */
void next_token();

/**
 * @brief Prints the given string as a syntax error message, then exits the program with 2
 *
 * @param str The error message you want to be printed
 */
void syntax_error(char* str);

/**
 * @brief Prints a syntax error message with the given token type and exits the program with 2
 *
 * @param t TokenType that was expected
 */
void expect_syntax_error(TokenType t);

/**
 * @brief Checks if the current token is the same as the given type
 *
 * Checks if the current token is the same as the given type and calls next_token() if it is the same as expected
 * Terminates the program with expect_syntax_error(t) if the current token is not the same as expected
 *
 * @param t TokenType that is expected
 *
 * @see next_token()
 * @see expect_syntax_error()
 */
void expect_type(TokenType t);

/**
 * @brief Calls calloc and checks if it succeeded
 *
 * Calls calloc with the given size and checks if it succeeded
 *
 * @param n The size you want to be allocated
 *
 * @return Pointer to the newly allocated memory
 */
void* calloc_and_check(size_t n);

/**
 * @brief Calls strdup and checks if it succeeded
 *
 * Calls strdup with the given string and checks if it succeeded
 *
 * @param s The string you want to strdup
 *
 * @return Pointer to the newly allocated string
 */
char* strdup_and_check(char* s);

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
} DataType;

typedef enum {
    E_INT,
    E_DOUBLE,
    E_STRING,
    E_UNIT,
    E_VAR,
    E_CALL,
    E_UNARY,
    E_BINARY
} ExpressionKind;

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
} Operators;

typedef struct Expression {
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
} Expression;

typedef enum {
    S_VAR_DEF,
    S_ASSIGN,
    S_RETURN,
    S_EXPR,
    S_IF,
    S_WHILE
} StatementKind;

typedef struct Statement {
    StatementKind kind;
    char* name;
    bool is_var;
    DataType type;
    Expression* expr;
    struct Statement* body;
    struct Statement* else_body;
    struct Statement* next;
} Statement;

typedef struct Func {
    char* func_name;
    bool is_main;
    char** param_names;
    DataType* param_types;
    int param_count;
    DataType return_type;
    Statement* body;
    struct Func* next;
} Func;

/**
 * @brief Makes a new Expression
 *
 * Initializes a new Expression of the given kind
 *
 * @param k The kind of expression you want to be made
 *
 * @return Pointer to the new Expression
 */
Expression* new_expr(ExpressionKind k);

/**
 * @brief Makes a new Statement
 *
 * Initializes a new Statement of the given kind
 *
 * @param k The kind of statement you want to be made
 *
 * @return Pointer to the new Statement
 */
Statement* new_stmt(StatementKind k);

/**
 * @brief Makes a new Func
 *
 * Initializes a new Func of the given kind
 *
 * @param k The kind of function you want to be made
 *
 * @return Pointer to the new Func
 */
Func* new_func();

/**
 * @brief Adds a parameter to a Func
 *
 * Appends a parameter of given kind and name to a Func
 *
 * @param f Pointer to the Func
 * @param name Name of the parameter
 * @param type Type of the parameter
 *
 * @warning May terminate the whole program by calling exit(99) when realloc fails
 */
void add_param(Func* f, char* name, DataType type);

/**
 * @brief Appends a Statement to the linked list of Statements
 *
 * Appends the last statement on the end of the first statement->next list
 *
 * @param first The first Statement in the list
 * @param last The Statement you want to append
 *
 * @return Pointer of the start of the Statement list
 */
Statement* append_statement(Statement* first, Statement* last);

/**
 * @brief Makes a new Expression when its name is already known
 *
 * Makes a new Expression of given name
 * Only makes CALL or VAR Expression depending on the current token
 *
 * @param name The name of the expression
 *
 * @return Pointer to the new Expression
 */
Expression* make_ident_expr(char* name);

/**
 * @brief Frees a whole Func
 *
 * Frees the given Func and all of its allocated memory
 *
 * @param f Pointer to the Func you want to free
 */
void free_func(Func* f);

/**
 * @brief Frees a whole Statement
 *
 * Frees the given Statement and all of its allocated memory
 *
 * @param s Pointer to the Statement you want to free
 */
void free_statement(Statement* s);

/**
 * @brief Frees a whole Expression
 *
 * Frees the given Expression and all of its allocated memory
 *
 * @param e Pointer to the Expression you want to free
 */
void free_expression(Expression* e);

/*==========================
 * PRECEDENCE ANALYZATOR
 * =========================
 */
typedef enum PrecSym {
    P_UNARY,
    P_MULDIV,
    P_PLUSMINUS,
    P_REL,
    P_EQ,
    P_LPAREN,
    P_RPAREN,
    P_ID,
    P_DOLLAR
} PrecSym;

typedef struct PrecItem {
    PrecSym symbol;
    bool is_expression;
    bool has_handle_after;
    Operators op;
    Expression* node;
} PrecItem;

typedef struct Stack {
    PrecItem* data;
    int top;
    int capacity;
} Stack;

/**
 * @brief Initializes a Stack for the precedence analyzator
 *
 * Sets the stack top to the bottom of the stack
 * Sets the capacity to 4
 *
 * @return Pointer to the new Stack
 *
 * @warning May terminate the whole program by calling exit(99) when malloc fails
 */
Stack* stack_init();

/**
 * @brief Doubles the Stack capacity
 *
 * Doubles the capacity of the given stack
 *
 * @param s Pointer to the Stack you want its capacity to be doubled
 *
 * @warning May terminate the whole program by calling exit(99) when realloc fails
 */
void resize_stack(Stack* s);

/**
 * @brief Adds the given item on the top of the given Stack
 *
 * Calls resize_stack(s) when the capacity is full
 *
 * @param s Pointer to the Stack you want the item to be added to
 * @param item The item you want to add to the stack
 *
 * @see resize_stack()
 */
void push(Stack* s, PrecItem item);

/**
 * @brief Removes the top x items from the given stack
 *
 * @param s Pointer to the Stack you want to remove items from
 * @param x Number of items you want to remove
 *
 * @warning Does not check if there is anything to remove
 */
void popx(Stack* s, int x);

/**
 * @brief Frees the whole Stack
 *
 * Frees the given Stack and all of its allocated memory
 *
 * @param s Pointer to the Stack you want to free
 */
void destroy_stack(Stack* s);

/**
 * @brief Returns the index of top most terminal on given Stack
 *
 * @param s Pointer to the Stack you want to get the top most terminal index from
 *
 * @return Index of the top most terminal on given Stack
 *
 * @warning May terminate the whole program by calling syntax_error() when the stack top is < 0
 *
 * @see syntax_error()
 */
int top_terminal_idx(Stack* s);

/**
 * @brief Checks if the given Expression is a CALL expression
 *
 * Terminates the program by calling syntax_error() when the given Expression is a CALL expression
 *
 * @param e Pointer to the Expression you want to check
 *
 * @see syntax_error()
 */
void reject_if_call(Expression* e);

/**
 * @brief Does the Shift action of the precedence analyzator
 *
 * Makes a PrecItem out of given symbol and pushes it on the Stack
 * Changes prev_is_operand and depth depending on the created item
 * Terminates the whole program by calling syntax_error() when the syntax is invalid
 *
 * @param s Pointer to the Stack you want the Shift action to happen on
 * @param symbol Symbol you want to make an item from
 * @param prev_is_operand Pointer to the prev_is_operand
 * @param depth Pointer to the depth
 *
 * @see syntax_error()
 * @see strdup_and_check()
 * @see next_token()
 * @see new_expr()
 * @see push()
 */
void shift(Stack* s, PrecSym symbol, bool* prev_is_operand, int* depth);

/**
 * @brief Adds a HANDLE after the given idx on the stack
 *
 * Sets the stack item on the given index has_handle_after to true
 *
 * @param s Pointer to the Stack
 * @param idx Index of the item you want to insert handle after
 */
void insert_handle(Stack* s, int idx);

/**
 * @brief Does the Reduce action of the precedence analyzator
 *
 * Reduces expressions on the stack using precedence analyzator rules
 * Terminates the whole program by calling syntax_error() when the syntax is invalid
 *
 * @param s Pointer to the Stack you want the Reduce action to happen on
 *
 * @see syntax_error()
 * @see reject_if_call()
 * @see new_expr()
 * @see popx()
 * @see push()
 */
void reduce(Stack* s);

/**
 * @brief Makes a PrecSym from the current token
 *
 * Symbol created also depends on the given prev_is_operand and depth
 *
 * @param prev_is_operand If the previous symbol is an operand or not
 * @param depth Depth of parentheses
 *
 * @return Symbol made from the token
 */
PrecSym tok_to_sym(bool prev_is_operand, int depth);

/**
 * @brief Parses the expression using precedence analyzator
 *
 * @return Return value of parse_expression_from(NULL)
 *
 * @see parse_expression_from()
 */
Expression* parse_expression();

/**
 * @brief Parses the expression using precedence analyzator and adds an expression on the stack if given
 *
 * Makes a stack with stack_init()
 * Adds a DOLLAR on the bottom using push()
 * Adds the given Expression to the stack using push()
 * Does '=', '<', '>', 'ACCEPT', 'BLANK' actions according to the precedence table
 * Terminates the whole program using syntax_error() on invalid syntax
 *
 * @param first Pointer to the Expression to be added to the stack
 *
 * @return Pointer to the parsed Expression
 *
 * @see syntax_error()
 * @see stack_init()
 * @see free_stack()
 * @see shift()
 * @see reduce()
 * @see insert_handle()
 * @see top_terminal_idx()
 * @see push()
 */
Expression* parse_expression_from(Expression* first);

/*=========================
 *LL TABLE FUNCTIONS
 * ========================
 */
/* All the LL table functions terminate the program using
 * syntax_error() or expect_type() on invalid syntax
 *
 * All the LL table functions are based on the LL table
 */

/**
 * @brief Checks the syntax of the whole program
 *
 * Checks the syntax of the whole program and creates an AST tree from it
 *
 * @return AST tree of the program
 */
Func* parse_program();

/**
 * @brief Checks the syntax of prologue
 *
 */
void parse_prologue();

/**
 * @brief Checks syntax of all functions
 *
 * Checks syntax of all functions and makes a linked list of functions
 *
 * @return Pointer to the first Func of the function linked list
 */
Func* parse_func_list();

/**
 * @brief Checks syntax of a function definition
 *
 * Checks syntax of a function definition and makes a new Func and fills it
 *
 * @return Pointer to the created Func
 */
Func* parse_func_def();
/**
 * @brief Checks syntax of function annotation
 *
 * Checks syntax of function annotation and returns if it is main
 *
 * @return true if the function is main, false otherwise
 */
bool parse_annot();

/**
 * @brief Checks syntax of function annotation
 *
 * Checks if annotation is on its own line or before def
 */
void parse_annot_n();

/**
 * @brief Checks syntax of all function parameters
 *
 * Checks syntax of all function parameters and fills the parameter information in Func
 *
 * @param f Pointer to the current Func that is getting checked
 */
void parse_params(Func* f);

/**
 * @brief Checks syntax of function parameters
 *
 * Checks syntax of function parameters and fills the parameter information in Func
 *
 * @param f Pointer to the current Func that is getting checked
 */
void parse_params_n(Func* f);

/**
 * @brief Checks syntax of function body
 *
 * Checks syntax of function body and fills Func body
 *
 * @return Pointer to the new Func body Statement
 */
Statement* parse_func_body();

/**
 * @brief Checks syntax of function return type declaration
 *
 * Checks syntax of function return type declaration and sets its return type
 *
 * @return Type of the return value
 */
DataType parse_n_type();

/**
 * @brief Checks syntax of function return type declaration
 *
 * Checks syntax of function return type declaration and sets its return type
 *
 * @return Type of the return value
 */
DataType parse_type();

/**
 * @brief Checks syntax of a block of code
 *
 * Checks syntax of a block of code and returns the statement linked list of the block
 *
 * @return Pointer to the first Statement of the statement list
 */
Statement* parse_block();

/**
 * @brief Checks syntax of a statement
 *
 * Checks syntax of a statement and makes a linked list of the Statements
 *
 * @return Pointer to the first Statement of the statement list
 */
Statement* parse_statement();

/**
 * @brief Checks syntax of a statement
 *
 * Checks syntax of a statement and makes a linked list of the Statements
 *
 * @return Pointer to the first Statement of the statement list
 */
Statement* parse_statement_n();

/**
 * @brief Checks syntax of statement on one line
 *
 * Checks syntax of statement on one line and fills it
 *
 * @return Pointer to the parsed Statement
 */
Statement* parse_statement_line();

/**
 * @brief Checks syntax of a statement that is not IF or WHILE
 *
 * @return Pointer to the parsed Statement
 */
Statement* parse_simple_statement();

/**
 * @brief Checks syntax of a statement that is not IF or WHILE
 *
 * @return Pointer to the parsed Statement
 */
Statement* parse_simple_n();

/**
 * @brief Checks syntax of a statement that is not IF or WHILE
 *
 * Checks syntax of a statement that is not IF or WHILE and fills it
 * Makes an ASSIGN or EXPR Statement if it has an identifier and fills it
 *
 * @return Pointer to the parsed Statement
 */
Statement* parse_simple_s();

/**
 * @brief Checks syntax of an IF statement
 *
 * Checks syntax of an IF statement and makes a new IF Statement and fills it
 *
 * @return Pointer to the new Statement
 */
Statement* parse_if_statement();

/**
 * @brief Checks syntax of a WHILE statement
 *
 * Checks syntax of a WHILE statement and makes a new WHILE Statement and fills it
 *
 * @return Pointer to the new Statement
 */
Statement* parse_while_statement();

/**
 * @brief Checks syntax of condition
 *
 * @return Pointer to the Expression made from the condition
 */
Expression* parse_condition();

/**
 * @brief Checks syntax of return statement
 *
 * Checks syntax of return statement and gets its value
 *
 * @return Pointer to the parsed Statement
 */
Statement* parse_return_statement();

/**
 * @brief Checks syntax of return value
 *
 * Checks syntax of return value and makes a new RETURN Statement and fills it
 *
 * @return Pointer to the new Statement
 */
Statement* parse_return_val();

/**
 * @brief Checks syntax of function arguments
 *
 * Checks syntax of function arguments and makes a linked list of function arguments
 *
 * @return Pointer to the first Expression of the argument list
 */
Expression* parse_arguments();

/**
 * @brief Checks syntax of function arguments
 *
 * Checks syntax of function arguments and makes a linked list of function arguments
 *
 * @return Pointer to the first Expression of the argument list
 */
Expression* parse_arguments_n();

/**
 * @brief Checks syntax of function arguments
 *
 * Checks syntax of function arguments and makes a linked list of function arguments
 *
 * @return Pointer to the first Expression of the argument list
 */
Expression* parse_arguments_nn();

/**
 * @brief Checks syntax of function argument values
 *
 * Checks syntax of function argument values and makes a new Expression depending on the current token and fills it
 *
 * @return Pointer to the new Expression
 */
Expression* parse_term();

/**
 * @brief Checks syntax of function arguments that have an UNARY sign
 *
 * Checks syntax of function arguments that have an UNARY sign and makes a new Expression depending on the current token and fills it
 *
 * @return Pointer to the new Expression
 */
Expression* parse_num_lit();

/**
 * @brief Checks syntax of variable or value definition
 *
 * Checks syntax of variable or value definition and makes a new VAR_DEF Statement and fills it
 *
 * @return Pointer to the new Statement
 */
Statement* parse_var_def();

/**
 * @brief Checks syntax of variable or value definition
 *
 * Checks syntax of variable or value definition returns if its VAR or VAL
 *
 * @return true if it is a variable, false if it is a value
 */
bool parse_vax();

// Parser innit
/* @brief Does whole syntactic analysis and then cleans up after
 *
 * Initializes lexical analyzator using scanner_init()
 * Does whole syntactic analysis on the program
 * Cleans up the lexical analyzator because it is no longer needed
 *
 * @param code_file Pointer to the file or input the lexical analyzer reads from
 *
 * @return Pointer to the created AST tree
 *
 * @see scanner_init()
 * @see next_token()
 * @see parse_program()
 * @see token_destroy()
 * @see scanner_destroy()
 */
Func* parser_interface(FILE* code_file);

#endif
