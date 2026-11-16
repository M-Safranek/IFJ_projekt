#ifndef SYMTABLE_STACK
#define SYMTABLE_STACK

#include "symtable.h"

typedef struct symtable_stack_item{
    table_symbol *top_symbol;
    struct symtable_stack_item *prev;
}symtable_stack;

symtable_stack *sym_init_stack();
bool sym_is_empty(symtable_stack *stack);
symtable_stack *sym_push(symtable_stack *stack, table_symbol *item);
symtable_stack *sym_pop(symtable_stack *stack);
symtable_stack *sym_top(symtable_stack *stack);
void sym_delete_stack(symtable_stack *stack);
table_symbol *sym_stack_search(symtable_stack *stack, char *name[]);

#endif //SYMTABLE_STACK