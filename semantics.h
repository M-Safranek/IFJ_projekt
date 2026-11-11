#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

#include "symtable.h"
#include "ast_tree.h"

char builtin_func_list[10][11] = {"print", "readLine", "readInt", "readDouble", "toStr", "toInt", "strLength", "subStr", "ord", "chr"};

typedef struct semantics_return_value{
    table_symbol *return_symtable;
    ast_tree *return_ast_tree;
}semantics_return;

#endif //SEMANTICS_ANALYZER