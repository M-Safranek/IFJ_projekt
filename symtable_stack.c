#include <stdio.h>
#include "symtable_stack.h"
#include "symtable.h"

symtable_stack *sym_init_stack(){
    symtable_stack *first = malloc(sizeof(symtable_stack));
    first->top_symbol = NULL;
    first->prev = NULL;
    return first;
};

symtable_stack *sym_is_empty(symtable_stack *stack){
    if(stack->top_symbol == NULL && stack->prev == NULL){
        return true;
    }else{
        return false;
    }
};

symtable_stack *sym_push(symtable_stack *stack ,table_symbol *item){
    symtable_stack *pushed = malloc(sizeof(symtable_stack));
    pushed->top_symbol = item;
    pushed->prev = stack;
    return pushed;
};

symtable_stack *sym_pop(symtable_stack *stack){
    if(sym_is_empty == false){
        symtable_stack *previous = stack->prev;
        free(stack);
        return previous;
    }
}

symtable_stack *sym_top(symtable_stack *stack){
    if(sym_is_empty == false){
        return stack;
    }else{
        return NULL;
    }
};

void sym_delete_stack(symtable_stack *stack){
    while(sym_is_empty == false){
        sym_pop(stack);
    }
    free(stack);
}