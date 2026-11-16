#include <stdio.h>
#include <stdlib.h>
#include "symtable_stack.h"
#include "symtable.h"

symtable_stack *sym_init_stack(){ //allocate momory for stack
    symtable_stack *first = malloc(sizeof(symtable_stack)); //header
    if(first == NULL){
        return NULL;
    }
    first->top_symbol = NULL; 
    first->prev = NULL;
    return first;
};

bool sym_is_empty(symtable_stack *stack){ //check if stack only contains the header
    if(stack != NULL){
        if(stack->top_symbol == NULL && stack->prev == NULL){
            return true;
        }else{
            return false;
        }
    }
    return NULL;
};

symtable_stack *sym_push(symtable_stack *stack ,table_symbol *item){ //push onto stack
    symtable_stack *pushed = malloc(sizeof(symtable_stack));
    if(pushed == NULL){
        return NULL;
    }
    pushed->top_symbol = item;
    pushed->prev = stack;
    stack = pushed;
    return stack;
};

symtable_stack *sym_pop(symtable_stack *stack){ //pop stack
    if(sym_is_empty(stack) == false){
        symtable_stack *previous = stack->prev;
        free(stack);
        return previous;
    }
    return stack;
}

symtable_stack *sym_top(symtable_stack *stack){ //get first item on stack
    if(sym_is_empty(stack) == false){
        return stack;
    }else{
        return NULL;
    }
};

void sym_delete_stack(symtable_stack *stack){ //delete whole stack
    while(sym_is_empty(stack) == false){
        stack = sym_pop(stack);
    }
    free(stack);
};

table_symbol *sym_stack_search(symtable_stack *stack, char *name){ //search all trees in the stack
    symtable_stack *searched_pos = stack;
    table_symbol *searched_sym = NULL;
    while(searched_pos->prev != NULL && searched_sym == NULL){
        searched_sym = search(name, searched_pos->top_symbol);
        searched_pos = searched_pos->prev;
    }
    return searched_sym;
}