#include <stdio.h>
#include "semantics.h"
#include "symtable.h"
#include "symtable_stack.h"
#include "ast_tree.h"
#include "parser.h"

void analyze_body(symtable_stack *stack, Statement *func_body){
    if(func_body->name != "if" || func_body->name != "for" || func_body->name != "while"){ //statement is not if, for or while
        stack->top_symbol = insert(stack->top_symbol, func_body->name, false, true); //get statement into tree in func scope
    }else{

    }
    return;
};

void analyze(symtable_stack *stack, Func *analyzed_code){
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
        stack->top_symbol = insert(stack->top_symbol, func->func_name, true, true); //get func into tree in global scope
        table_symbol *func_symbol = search(func->func_name, sym_top(stack)->top_symbol); //get the inserted func
        func_symbol->type = func->return_type; //record type
        func_symbol->param_types = func->param_types; //record params
        func_symbol->param_count = func->param_count;
        func_symbol->is_main = func->is_main; //is main

        Statement *func_body = func->body; //get body of func
        sym_push(stack, NULL); //push func scope
        while(func_body != NULL){ //analyze all statements in func
            analyze_body(stack, func_body);
            func_symbol->subtree = sym_top(stack);
            func_body = func_body->next;
        }
        stack = sym_pop(stack);
        func = func->next;
    }
    return;
};

semantics_return build_table(){
    Func *analyzed_code = parse_program();
    symtable_stack *stack = sym_init_stack();
    analyze(stack, analyzed_code);
    analyze(stack, analyzed_code); //pass 2
    semantics_return return_value;
    return_value.return_symtable = sym_top(stack)->top_symbol;
    sym_delete_stack(stack);
    return return_value;
}