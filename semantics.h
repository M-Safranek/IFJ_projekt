/**
 * @file semantics.h
 * @brief Header file for the Semantic analyser for the IFJ26 language
 * @author Vojtěch Daniel Eliáš, xeliasv00
 * @author Václav Tomaník, xtomanv00
 */
#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

#include <stdbool.h>
#include "parser.h"
#include "symtable.h"
#include "symtable_stack.h"

/**
 * @brief Makes a unique scoped name for a local variable.
 */
char *make_scope_name(char *name);

/**
 * @brief Semantic and type analysis of an expression node
 */
void analyze_expression(symtable_stack *stack, Expression *expr);

/**
 * @brief Validates that a condition expression is a relational operator
 */
void analyze_condition(symtable_stack *stack, Expression *condition);

/**
 * @brief Semantic analysis of statement, manages scopes for inner blocks.
 */
void analyze_body(symtable_stack *stack, Statement *func_body, Func *func);

/**
 * @brief Checks if function has 'return' on all paths.
 */
bool func_has_return(Statement *stmt);

void register_functions(symtable_stack *stack, Func *analyzed_code);
void analyze_functions(symtable_stack *stack, Func *analyzed_code);
Func *build_table(Func *analyzed_code);

#endif // SEMANTICS_ANALYZER