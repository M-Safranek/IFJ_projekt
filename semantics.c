#include <stdio.h>
#include "symtable.h"
#include "symtable_stack.h"
#include "parser.h"

Statement *analyze_statement(symtable_stack *stack, Statement *analyzed_statement){
    
};

Func *analyze_func(symtable_stack *stack, Func *analyzed_code){
    while(analyzed_code->body != NULL){
        Statement *analyzed_statement = analyze_statement(stack, analyzed_code->body);
        delete_tree(stack->top_symbol);
        stack = sym_pop(stack);
    }
    return;
};

void analyze(Func *analyzed_code){
    symtable_stack *stack = sym_init_stack();
    //analyze first func, push global scope
    //analyze inside func, push func scope
    //pop func scope, analyze second func, push second func scope
    //analyze block inside second func, push block scope
    //outside block, pop block scope
    //co se stromy co mají danou paměť? delete tree při pop?

    if(analyzed_code == NULL){
        return;
    }
    
    Func *func = analyzed_code;
    table_symbol *symbol = insert(NULL, func->func_name, func->return_type, true);
    stack = sym_push(stack, symbol); //push global scope

    while(analyzed_code != NULL){
        Func analyzed_func = *analyze_func(stack, analyzed_code);
        analyzed_code = analyzed_code->next;
    }

    //konec
    sym_delete_stack(stack);
    return;
};

int main(){
    Func *analyzed_code = parse_program();
    analyze(analyzed_code);
}