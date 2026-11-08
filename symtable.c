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

int get_height(TableSymbol *where){
    if(where == NULL){
        return 0;
    }else{
        return where->height;
    }
};

int compare_height(TableSymbol *one, TableSymbol *two){
    return max(get_height(one), get_height(two)) + 1;
};

int get_balance(TableSymbol *where){
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

TableSymbol *search(char *what[], TableSymbol *where){
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

TableSymbol *ll_rotate(TableSymbol *top){ //right_rotate
    TableSymbol *left_symbol = top->left;
    top->left = left_symbol->right;
    left_symbol->right = top;
    top->height = compare_height(top->left, top->right);
    left_symbol->height = compare_height(left_symbol->left, top);
    return left_symbol;
};

TableSymbol *rr_rotate(TableSymbol *top){ //left_rotate
    TableSymbol *right_symbol = top->right;
    top->right = right_symbol->left;
    right_symbol->left = top;
    top->height = compare_height(top->left, top->right);
    right_symbol = compare_height(right_symbol->right, top);
    return right_symbol;
};

TableSymbol *lr_rotate(TableSymbol *top){
    top->left = rr_rotate(top->left);
    return ll_rotate(top); 
};

TableSymbol *rl_rotate(TableSymbol *top){
    top->right = ll_rotate(top->right);
    return rr_rotate(top);
};

TableSymbol *create_symbol(char *name[], TokenType type, char *scope[], bool defined){
    TableSymbol *new_symbol = malloc(sizeof(TableSymbol));
    if(new_symbol == NULL){
        return NULL;
    }
    *new_symbol->name = name;
    new_symbol->type = type;
    *new_symbol->scope = scope;
    new_symbol->defined = defined;
    new_symbol->left = NULL;
    new_symbol->right = NULL;
    new_symbol->height = 0;
    return new_symbol;
}

TableSymbol *insert(TableSymbol *where, char *name[], TokenType type, char *scope[], bool defined){
    if(where == NULL){
        TableSymbol *new_symbol = create_symbol(name, type, scope, defined);
        where = new_symbol;
    }else if(strcmp(lower_string(where->name), lower_string(name)) > 0){
        where->left = insert(where->left, name, type, scope, defined);
        if(get_balance(where) == 2){
            if(strcmp(lower_string(where->left->name), lower_string(name)) > 0){
                where = ll_rotate(where);
            }else{
                where = rl_rotate(where);
            }
        }
    }else if(strcmp(lower_string(where->name), lower_string(name)) < 0){
        where->right = insert(where->right, name, type, scope, defined);
        if(get_balance(where) == -2){
            if(strcmp(lower_string(where->right->name), lower_string(name)) < 0){
                where = rr_rotate(where);
            }else{
                where = lr_rotate(where);
            }
        }
    }else{
        where->type = type;
        where->defined = defined;
    }
    where->height = compare_height(where->left, where->right);
    return where; //dosáhneme pokud se přiřadí nebo při nalezení přiřazeného symbolu
};

TableSymbol *delete_symbol(TableSymbol *what, TableSymbol *top){
    if(search(what, top) != NULL){
        //najdi symbol který má odkaz na what
        //jestli má what děti, přiřaď je někde jinde
        //odeber odkaz z rodiče
        free(what);
    }
};

TableSymbol *delete_tree(TableSymbol *top){
    if(top != NULL){
        delete_tree(top->left);
        delete_tree(top->right);
        free(top);
    }
};

//udělej zásobník pro scope management?