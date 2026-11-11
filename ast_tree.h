#ifndef AST_TREE
#define AST_TREE

#include "parser.h"

typedef struct ast_tree_node{
    union ast_tree_property{
        Operators operator;
        //value
        //pointer to func
    };
    ast_tree *left;
    ast_tree *right;
}ast_tree;

#endif //AST_TREE