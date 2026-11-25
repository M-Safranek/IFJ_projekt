/**
 * @file symtable_stack.h
 * @brief Header file for the stack management of symbol tables for the IFJ26 language
 * @author Václav Tomaník, xtomanv00
 */

#ifndef SYMTABLE_STACK
#define SYMTABLE_STACK

#include "symtable.h"

typedef struct symtable_stack_item {
  table_symbol *top_symbol;         // pointer to the top of a symtable
  struct symtable_stack_item *prev; // pointer to previous stack item
} symtable_stack;

/**
 * @brief Creates a stack and returns the header.
 */
symtable_stack *sym_init_stack();

/**
 * @brief Check if a given stack is empty (only consisting of a header).
 */
bool sym_is_empty(symtable_stack *stack);

/**
 * @brief Pushes a new symbol table onto the stack.
 */
symtable_stack *sym_push(symtable_stack *stack, table_symbol *item);

/**
 * @brief Pops the symbol table on the top of the stack.
 */
symtable_stack *sym_pop(symtable_stack *stack);

/**
 * @brief Returns the top of the stack.
 */
symtable_stack *sym_top(symtable_stack *stack);

/**
 * @brief Pops the entire stack one by one while freeing symbol tables.
 */
void sym_delete_stack(symtable_stack *stack);

/**
 * @brief Searches through the stack to find a symbol.
 */
table_symbol *sym_stack_search(symtable_stack *stack, char *name);

#endif // SYMTABLE_STACK