#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "semantics.h"
#include "symtable.h"
#include "symtable_stack.h"
#include "parser.h"

struct ast_tree ast_create_tree(){
    struct ast_tree top_of_tree;
    top_of_tree.node_type = TOP;
    top_of_tree.properties.func_list = NULL;
    return top_of_tree;
};

struct ast_tree *ast_create_const_int(){

};

struct ast_tree *ast_create_const_double(){

};

struct ast_tree *ast_create_const_string(){

};

struct ast_tree *ast_create_const_unit(){

};

struct ast_tree *ast_create_var(char *var_name){

};

struct ast_tree *ast_create_int_to_double(){

};

struct ast_tree *ast_create_unary(enum ast_operator){

};

struct ast_tree *ast_create_binary(enum ast_operator){

};

struct ast_tree *ast_create_assignment(){

};

struct ast_tree *ast_create_ifthenelse(){

};

struct ast_tree *ast_create_while(){

};

struct ast_tree *ast_create_func_def(struct ast_tree *where, char *name, char *operands){
    struct ast_tree *func_def;
    func_def->node_type = FUNC_DEF;
    strcpy(func_def->properties.func_def.name, name);
    strcpy(func_def->properties.func_def.operands, operands);
    func_def->properties.func_def.body = NULL;
    struct ast_tree *temp = realloc(where->properties.func_list, sizeof(where->properties.func_list + sizeof(struct ast_tree))); //make space for one more
    if(temp != NULL){
        where->properties.func_list[(sizeof(where->properties.func_list) / sizeof(struct ast_tree)) - 1] = *temp; //get the last space possible
    }
    return func_def;
};

struct ast_tree *ast_create_func_call(char *name, char *operands){

};

struct ast_tree *ast_create_return(){

};

void analyze_if(){

};

void analyze_else(){

};

void analyze_while(){

};

void analyze_body(symtable_stack *stack, Statement *func_body, struct ast_tree *ast_position){   
    if(func_body->name == "if"){ //statement is if
        //get all vars in condition, check against symtable and insert into ast
        Statement *if_body = func_body->body;
        while(if_body != NULL){
            analyze_if();
            if_body = if_body->next;
        }
        Statement *if_else_body = func_body->else_body;
        while(if_else_body != NULL){
            analyze_if();
            if_else_body = if_else_body->next;
        }
    }else if(func_body->name == "while"){ //statement is while
        //get all vars in condition, check against symtable and insert into ast
        Statement *while_body = func_body->body;
        while(while_body != NULL){
            analyze_while();
            while_body = while_body->next;
        }
    }else{
        
    }
    return;
};

void analyze(symtable_stack *stack, Func *analyzed_code, struct ast_tree *top_of_tree){
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
        struct ast_tree *ast_position = &top_of_tree; //reset pos to top of tree
        if(search(func->func_name, stack->top_symbol) == NULL){ //second pass or double definition?
            stack->top_symbol = insert(stack->top_symbol, func->func_name, true, true); //get func into tree in global scope
        }else{
            for(int i; i < (sizeof(top_of_tree->properties.func_list) / sizeof(struct ast_tree)); i++){
                if(strcmp(func->func_name, top_of_tree->properties.func_list->properties.func_def.name) == 0){
                    //error, second definition of func
                }
            }
        }
        table_symbol *func_symbol = search(func->func_name, sym_top(stack)->top_symbol); //get the inserted func
        func_symbol->type = func->return_type; //record type
        func_symbol->param_types = func->param_types; //record params
        func_symbol->param_count = func->param_count;
        func_symbol->is_main = func->is_main; //is main

        Statement *func_body = func->body; //get body of func
        if(func_symbol->subtree == NULL){ //are we doing a second pass?
            sym_push(stack, NULL); //push func scope
        }else{
            sym_push(stack, func_symbol->subtree); //push the completed func symtable to stack for checking
        }
        ast_position = ast_create_func_def(top_of_tree->properties.func_list, func->func_name, func->param_types); //insert function into top of ast tree

        while(func_body != NULL){ //analyze all statements in func
            analyze_body(stack, func_body, ast_position);
            func_body = func_body->next; //move on
        }
        func_symbol->subtree = stack->top_symbol; //archive contents of stack for checking in second pass
        stack = sym_pop(stack);
        func = func->next; //move on
    }
    return;
};

struct ast_tree build_table(){
    Func *analyzed_code = parse_program(); //get the parsed code
    symtable_stack *stack = sym_init_stack(); //make stack
    sym_push(stack, NULL); //push global scope
    struct ast_tree return_ast_tree = ast_create_tree();
    analyze(stack, analyzed_code, &return_ast_tree);
    analyze(stack, analyzed_code, &return_ast_tree); //pass 2
    delete_tree(stack->top_symbol); //delete symtable
    sym_delete_stack(stack); //free stack
    return return_ast_tree;
}