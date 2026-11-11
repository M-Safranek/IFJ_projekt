#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

#include "symtable.h"
#include "ast_tree.h"

typedef struct semantics_return_value{
    table_symbol *return_symtable;
    ast_tree *return_ast_tree;
}semantics_return;

#endif //SEMANTICS_ANALYZER