#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "symtable.h"

int max(int a, int b){
    if(a < b){
        return b;
    }else{
        return a;
    }
};

int get_height(table_symbol *where){
    if(where == NULL){
        return 0;
    }else{
        return where->height;
    }
};

int compare_height(table_symbol *one, table_symbol *two){
    return max(get_height(one), get_height(two)) + 1;
};

int get_balance(table_symbol *where){
    if(where == NULL){
        return 0;
    }else{
        return get_height(where->left) - get_height(where->right);
    }
};

char *lower_string(char *string[]){
    for(int i = 0; string[i]; i++){
        string[i] = tolower(string[i]);
    }
};

table_symbol *search(char *what[], table_symbol *where){
    if(where == NULL){
        return NULL;
    }else if(strcmp(lower_string(where->name), lower_string(what)) == 0){
        return where;
    }else if(strcmp(lower_string(where->name), lower_string(what)) > 0){
        return search(what, where->left);
    }else{
        return search(what, where->right);
    }
};

table_symbol *ll_rotate(table_symbol *top){ //right_rotate
    table_symbol *left_symbol = top->left;
    top->left = left_symbol->right;
    left_symbol->right = top;
    top->height = compare_height(top->left, top->right);
    left_symbol->height = compare_height(left_symbol->left, top);
    return left_symbol;
};

table_symbol *rr_rotate(table_symbol *top){ //left_rotate
    table_symbol *right_symbol = top->right;
    top->right = right_symbol->left;
    right_symbol->left = top;
    top->height = compare_height(top->left, top->right);
    right_symbol = compare_height(right_symbol->right, top);
    return right_symbol;
};

table_symbol *lr_rotate(table_symbol *top){
    top->left = rr_rotate(top->left);
    return ll_rotate(top); 
};

table_symbol *rl_rotate(table_symbol *top){
    top->right = ll_rotate(top->right);
    return rr_rotate(top);
};

table_symbol *create_symbol(char *name[], bool defined, bool function){
    table_symbol *new_symbol = malloc(sizeof(table_symbol));
    if(new_symbol == NULL){
        return NULL;
    }
    *new_symbol->name = name;
    new_symbol->defined = defined;
    new_symbol->function = function;
    new_symbol->left = NULL;
    new_symbol->right = NULL;
    new_symbol->height = 0;
    return new_symbol;
}

table_symbol *insert(table_symbol *where, char *name[], bool defined, bool function){
    if(where == NULL){
        table_symbol *new_symbol = create_symbol(name, defined, function);
        where = new_symbol;
    }else if(strcmp(lower_string(where->name), lower_string(name)) > 0){
        where->left = insert(where->left, name, defined, function);
        if(get_balance(where) == 2){
            if(strcmp(lower_string(where->left->name), lower_string(name)) > 0){
                where = ll_rotate(where);
            }else{
                where = rl_rotate(where);
            }
        }
    }else if(strcmp(lower_string(where->name), lower_string(name)) < 0){
        where->right = insert(where->right, name, defined, function);
        if(get_balance(where) == -2){
            if(strcmp(lower_string(where->right->name), lower_string(name)) < 0){
                where = rr_rotate(where);
            }else{
                where = lr_rotate(where);
            }
        }
    }
    where->height = compare_height(where->left, where->right);
    return where; //dosáhneme pokud se přiřadí nebo při nalezení přiřazeného symbolu
};

table_symbol *delete_tree(table_symbol *top){
    if(top != NULL){
        delete_tree(top->left);
        delete_tree(top->right);
        free(top);
    }
};
