#include <stdio.h>
#include "symtable.h"
#include "symtable_stack.h"
#include "parser.h"

void analyze_body(symtable_stack *stack, Statement *func_body){
    

    return
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
    sym_push(stack, NULL); //push global scope
    while(func != NULL){ //analyze all funcs in code
        stack->top_symbol = insert(stack->top_symbol, analyzed_code->func_name, true, true); //get func into tree in global scope
        table_symbol *func_symbol = search(analyzed_code->func_name, stack->top_symbol); //get the inserted func
        func_symbol->type = analyzed_code->return_type; //record type
        Statement *func_body = analyzed_code->body; //get body of func
        sym_push(stack, NULL); //push func scope
        while(func_body != NULL){ //analyze all statements in func
            analyze_body(stack, func_body);
            func_body = func_body->next;
        }
        delete_tree(stack->top_symbol);
        stack = sym_pop(stack);
        func = func->next;
    }

    //konec
    sym_delete_stack(stack);
    return;
};

int main(){
    Func *analyzed_code = parse_program();
    analyze(analyzed_code);
}