#ifndef SYMTABLE_STACK
#define SYMTABLE_STACK

typedef struct symtable_stack_item{
    TableSymbol *top_symbol;
    TableSymbol *prev;
}symtable_stack;

symtable_stack *init_stack();
symtable_stack *is_empty(symtable_stack *stack);
symtable_stack *push(symtable_stack *stack, TableSymbol *item);
symtable_stack *pop(symtable_stack *stack);
symtable_stack *top(symtable_stack *stack);
void delete_stack(symtable_stack *stack);

#endif //SYMTABLE_STACK