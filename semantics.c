#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "semantics.h"
#include "symtable.h"
#include "symtable_stack.h"
#include "parser.h"

void analyze_if(symtable_stack *stack, Statement *if_body){

};

void analyze_else(symtable_stack *stack, Statement *else_body){

};

void analyze_while(symtable_stack *stack, Statement *while_body){

};

void analyze_body(symtable_stack *stack, Statement *func_body){   
    if(strcmp(func_body->name, "if") == 0){ //statement is if
        //get all vars in condition, check against symtable and insert into ast
        Statement *if_body = func_body->body;
        while(if_body != NULL){
            analyze_if(stack, if_body);
            if_body = if_body->next;
        }
        Statement *if_else_body = func_body->else_body;
        while(if_else_body != NULL){
            analyze_if(stack, if_else_body);
            if_else_body = if_else_body->next;
        }
    }else if(strcmp(func_body->name, "while")){ //statement is while
        //get all vars in condition, check against symtable and insert into ast
        Statement *while_body = func_body->body;
        while(while_body != NULL){
            analyze_while(stack, while_body);
            while_body = while_body->next;
        }
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
    if(analyzed_code == NULL){
        return;
    }
    
    Func *func = analyzed_code;
    
    while(func != NULL){ //analyze all funcs in code
        if(search(func->func_name, stack->top_symbol) == NULL){ //second pass or double definition?
            stack->top_symbol = insert(stack->top_symbol, func->func_name, true, true); //get func into tree in global scope
        }else if(search(func->func_name, stack->top_symbol)->defined == true){ //defined for the second time, not allowed
            return; //TODO return some error
        }

        table_symbol *func_symbol = search(func->func_name, sym_top(stack)->top_symbol); //get the inserted func
        func_symbol->type = func->return_type; //record type
        func_symbol->param_types = func->param_types; //record params
        func_symbol->param_count = func->param_count;
        func_symbol->param_names = func->param_names;
        func_symbol->is_main = func->is_main; //is main
        func_symbol->defined = true; //currently defining a function

        Statement *func_body = func->body; //get body of func
        if(func_symbol->subtree == NULL){ //are we doing a second pass?
            sym_push(stack, NULL); //push func scope
        }else{
            sym_push(stack, func_symbol->subtree); //push the completed func symtable to stack for checking
        }

        while(func_body != NULL){ //analyze all statements in func
            analyze_body(stack, func_body);
            func_body = func_body->next; //move on
        }
        func_symbol->subtree = stack->top_symbol; //archive contents of stack for checking in second pass
        stack = sym_pop(stack);
        func = func->next; //move on
    }
    return;
};

Func *build_table(){
    Func *analyzed_code = parse_program(); //get the parsed code
    symtable_stack *stack = sym_init_stack(); //make stack
    sym_push(stack, NULL); //push global scope

    analyze(stack, analyzed_code);
    analyze(stack, analyzed_code); //pass 2

    delete_tree(stack->top_symbol); //delete symtable
    symtable_stack *tree_delete_check = stack;
    while(tree_delete_check != NULL){
        delete_tree(tree_delete_check->top_symbol);
        tree_delete_check = tree_delete_check->prev;
    }
    sym_delete_stack(stack); //free stack

    return analyzed_code;
}