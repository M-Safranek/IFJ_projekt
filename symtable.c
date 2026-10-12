#include <stdio.h>
#include <string.h>
#include "symtable.h"

int max(int a, int b){
    if(a < b){
        return b;
    }else{
        return a;
    }
};

int get_height(struct symbol *where){
    if(where == NULL){
        return -1;
    }else{
        return max(get_height(where->left), get_height(where->right)) + 1;
    }
};

struct symbol *search(char what, struct symbol *where){
    if(where == NULL){
        return NULL;
    }else if(strcmp(where->name, what) == 0){
        return where;
    }else if(where->name > what){
        return search(what, where->left);
    }else{
        return search(what, where->right);
    }
};

struct symbol *rotate_left(struct symbol *top){
    struct symbol *right_symbol = top->right;
    top->right = right_symbol->left;
    right_symbol->left = top;
    return right_symbol;
};

struct symbol *rotate_right(struct symbol *top){
    struct symbol *left_symbol = top->left;
    top->left = left_symbol->right;
    left_symbol->right = top;
    return left_symbol;
};

struct symbol *create_symbol(char name[], enum token_type type){
    struct symbol *new_symbol = malloc(sizeof(struct symbol));
    if(new_symbol == NULL){
        return NULL;
    }
    *new_symbol->name = name;
    new_symbol->type = type;
    new_symbol->left = NULL;
    new_symbol->right = NULL;
    new_symbol->height = 0;
    return new_symbol;
}

struct symbol *insert(struct symbol *where, char name[], enum token_type type){
    if(where == NULL){
        return create_symbol(name, type);
    }else if(strcmp(where->name, name) == 0 & where->type == type){
        return where;
    }else if(where->name > name){
        where->left = insert(where->left, name, type);
    }else if(where->name < name){
        where->right = insert(where->right, name, type);
    }
    //sem vlož výšku a rotace
    return where;
};

struct symbol *delete(struct symbol *what){
    //najdi symbol který má odkaz na what
    //jestli má what děti, přiřaď je někde jinde
    //odeber odkaz z rodiče
    //free
};
