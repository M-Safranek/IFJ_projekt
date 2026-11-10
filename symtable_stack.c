#include <stdio.h>
#include "symtable_stack.h"
#include "symtable.h"

symtable_stack *init_stack(){
    symtable_stack *first = malloc(sizeof(symtable_stack));
    first->top_symbol = NULL;
    first->prev = NULL;
    return first;
};

symtable_stack *is_empty(symtable_stack *stack){
    if(stack->top_symbol == NULL && stack->prev == NULL){
        return true;
    }else{
        return false;
    }
};

symtable_stack *push(symtable_stack *stack ,TableSymbol *item){
    symtable_stack *pushed = malloc(sizeof(symtable_stack));
    pushed->top_symbol = item;
    pushed->prev = stack;
    return pushed;
};

symtable_stack *pop(symtable_stack *stack){
    if(is_empty == false){
        symtable_stack *previous = stack->prev;
        free(stack);
        return previous;
    }
}

symtable_stack *top(symtable_stack *stack){
    if(is_empty == false){
        return stack;
    }else{
        return NULL;
    }
};

void delete_stack(symtable_stack *stack){
    while(is_empty == false){
        pop(stack);
    }
}