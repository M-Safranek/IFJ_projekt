/**
 * @file codegen.h
 * @brief Header file for the Codegen for the IFJ26 lenguage
 * @author Vojtěch Daniel Eliáš, xeliasv00
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>
#include "parser.h"

/**
 * @brief Convert integer on top of data stack to a float
 */
void codegen_int_to_double(DataType from, DataType to);

/**
 * @brief Predeclare variables in while cycle
 */
void codegen_predeclare_while_var(Statement *stmt);

/**
 * @brief Pushes a string literal to top of data stack with correct escaped special characters
 */
void codegen_print_string_literal(const char *str);

/**
 * @brief Concat two strings on top of data stack
 */
void codegen_print_concat(void);

/**
 * @brief Generates code for binary operations in post-order
 */
void codegen_print_binary(Expression *expr);

/**
 * @brief Pushes function call arguments onto the data stack
 */
void codegen_push_param(Func *f, Expression *arg);

/**
 * @brief Pops arguments from the data stack in reverse order onto the TF
 */
void codegen_pop_param(Func *f, int argc);

/**
 * @brief Searches the AST tree for user define function by name
 */
Func *codegen_find_func(const char *name);

/**
 * @brief Evaluates if the given function is builtin
 */
bool codegen_is_builtin(const char *func_name);

/**
 * @brief Implementation of builtin functions IFJ26
 */
void codegen_print_builtin(void);

/**
 * @brief Generates code for bultin and user defined function calls
 */
void codegen_print_call(Expression *expr);

/**
 * @brief Generates code for one expression node and pushes it onto data stack
 */
void codegen_print_expr(Expression *expr);

/**
 * @brief Generate sequence of statetemets within one block
 */
void codegen_print_body(Statement *stmt, bool is_tail, bool in_while);

/**
 * @brief Generates code for one statement
 */
void codegen_print_stmt(Statement *stmt, bool is_tail, bool in_while);

/**
 * @brief Generates label, frame, body and return of a function
 */
void codegen_print_func(Func *f);

/**
 * @brief Entry point for the codegen.c
 */
void codegen_print(Func *ast_root);

#endif
