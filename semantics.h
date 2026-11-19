#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

#include <stdbool.h>
#include "parser.h"
#include "symtable.h"
#include "symtable_stack.h"

char *make_scope_name(char *name);
void analyze_expression(symtable_stack *stack, Expression *expr);
void analyze_condition(symtable_stack *stack, Expression *condition);
void analyze_body(symtable_stack *stack, Statement *func_body, Func *func);
bool func_has_return(Statement *stmt);
void register_functions(symtable_stack *stack, Func *analyzed_code);
void analyze_functions(symtable_stack *stack, Func *analyzed_code);
Func *build_table(Func* analyzed_code);

#endif // SEMANTICS_ANALYZER