#include "codegen.h"
#include "semantics.h"

int main(){
    Func *test_func = parser_do_it();
    codegen_print(build_table());
    free_func(test_func);
    return 0;
}