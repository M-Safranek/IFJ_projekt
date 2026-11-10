#include <stdio.h>
#include "symtable.h"
#include "symtable_stack.h"
#include "parser.h"

void analyze(Func *analyzed_code){
    symtable_stack *stack = sym_init_stack();
    //analyze first func, push global scope
    //analyze inside func, push func scope
    //pop func scope, analyze second func, push second func scope
    //analyze block inside second func, push block scope
    //outside block, pop block scope
    //co se stromy co mají danou paměť? delete tree při pop?

    Func *func = analyzed_code;
    TableSymbol *symbol = insert(NULL, func->func_name, func->return_type, true);
    stack = sym_push(stack, symbol);

    //konec
    sym_delete_stack(stack);
    return;
};

int main(){
    Func *analyzed_code = parse_program();
    analyze(analyzed_code);
}