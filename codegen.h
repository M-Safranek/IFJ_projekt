/**
 * @file codegen.h
 * @brief Header file for the Codegen for the IFJ26 language
 * @author Vojtěch Daniel Eliáš, xeliasv00
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>
#include "parser.h"

/**
 * @brief Convert integer on top of data stack to a float
 * 
 * @param from Source data type of the variable
 * @param to Target data type required
 */
void codegen_int_to_double(DataType from, DataType to);

/**
 * @brief Predeclare variables in while cycle
 * 
 * @param stmt First statement in the while body
 * 
 */
void codegen_predeclare_while_var(Statement *stmt);

/**
 * @brief Pushes a string literal to top of data stack with correct escaped special characters
 * 
 * @param str String literal to emit
 */
void codegen_print_string_literal(const char *str);

/**
 * @brief Concat two strings on top of data stack
 */
void codegen_print_concat(void);

/**
 * @brief Generates code for binary operations in post-order
 * 
 * @param expr Binary expression AST node
 */
void codegen_print_binary(Expression *expr);

/**
 * @brief Pushes function call arguments onto the data stack
 * 
 * @param f Function pointer used for implicit type conversion
 * @param arg Head of the argument expression list
 */
void codegen_push_param(Func *f, Expression *arg);

/**
 * @brief Pops arguments from the data stack in reverse order onto the TF
 * 
 * @param f Function pointer containing parameter names
 * @param argc Number of arguments
 */
void codegen_pop_param(Func *f, int argc);

/**
 * @brief Searches the AST tree for user define function by name
 * @param name Name of the function to find
 * @return Pointer to the matching function node, or NULL if not found
 */
Func *codegen_find_func(const char *name);

/**
 * @brief Evaluates if the given function is builtin
 * 
 * @param func_name Function identifier to check
 * @return true if the function is builtin, else false
 */
bool codegen_is_builtin(const char *func_name);

/**
 * @brief Implementation of builtin functions IFJ26
 */
void codegen_print_builtin(void);

/**
 * @brief Generates code for bultin and user defined function calls
 * 
 * @param expr Call expression AST node
 */
void codegen_print_call(Expression *expr);

/**
 * @brief Generates code for one expression node and pushes it onto data stack
 * 
 * @param expr Expression AST node
 */
void codegen_print_expr(Expression *expr);

/**
 * @brief Generate sequence of statetemets within one block
 * 
 * @param stmt First statement in the block
 * @param is_tail If the block is in tail position in the function
 * @param in_while If the block is inside a while loop
 * 
 */
void codegen_print_body(Statement *stmt, bool is_tail, bool in_while);

/**
 * @brief Generates code for one statement
 * 
 * @param stmt Statement AST node
 * @param is_tail If the statement is in tail position of the function
 * @param in_while If the statement is inside a while loop
 */
void codegen_print_stmt(Statement *stmt, bool is_tail, bool in_while);

/**
 * @brief Generates label, frame, body and return of a function
 * 
 * @param f Function AST node
 */
void codegen_print_func(Func *f);

/**
 * @brief Entry point for the codegen.c
 * 
 * @param ast_root Root of the AST
 */
void codegen_print(Func *ast_root);

#endif
