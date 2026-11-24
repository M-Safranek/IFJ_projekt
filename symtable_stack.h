#ifndef SYMTABLE_STACK
#define SYMTABLE_STACK

#include "symtable.h"

typedef struct symtable_stack_item {
  table_symbol *top_symbol; //pointer to the top of a symtable
  struct symtable_stack_item *prev; //pointer to previous stack item
} symtable_stack;

symtable_stack *sym_init_stack(); //create a stack, returns header
bool sym_is_empty(symtable_stack *stack); //checks if a stack is empty (consisting only of a header)
symtable_stack *sym_push(symtable_stack *stack, table_symbol *item); //pushes a new symtable onto the stack
symtable_stack *sym_pop(symtable_stack *stack); //pops the symtable on top
symtable_stack *sym_top(symtable_stack *stack); //returns the symtable on top
void sym_delete_stack(symtable_stack *stack); //pops the entire stack one by one, deleting it
table_symbol *sym_stack_search(symtable_stack *stack, char *name); //searches through the stack to find a symbol

#endif // SYMTABLE_STACK