#ifndef SYMTABLE_STACK
#define SYMTABLE_STACK

typedef struct symtable_stack_item{
    table_symbol *top_symbol;
    table_symbol *prev;
}symtable_stack;

symtable_stack *sym_init_stack();
symtable_stack *sym_is_empty(symtable_stack *stack);
symtable_stack *sym_push(symtable_stack *stack, table_symbol *item);
symtable_stack *sym_pop(symtable_stack *stack);
symtable_stack *sym_top(symtable_stack *stack);
void sym_delete_stack(symtable_stack *stack);

#endif //SYMTABLE_STACK