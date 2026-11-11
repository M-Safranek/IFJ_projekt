#include "ast_tree.h"

ast_tree *insert(ast_tree *where, ast_tree *what, bool left){
    if(left){
        where->left = what;
    }else{
        where->right = what;
    }
    return what;
};