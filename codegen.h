#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>
#include "parser.h"

void print_error(int error_code, char *msg);
void codegen_int_to_double(DataType from, DataType to);
void codegen_print_string_literal(const char *str);
void codegen_print_concat(void);
void codegen_print_binary(Expression *expr);
void codegen_push_param(Func *f, Expression *arg);
void codegen_pop_param(Func *f, int argc);
Func *codegen_find_func(const char *name);
bool codegen_is_builtin(const char *func_name);
void codegen_print_builtin(void);
void codegen_print_call(Expression *expr);
void codegen_print_expr(Expression *expr);
void codegen_print_body(Statement *stmt, bool is_tail);
void codegen_print_stmt(Statement *stmt, bool is_tail);
void codegen_print_func(Func *f);
void codegen_print(Func *ast_root);

#endif 

