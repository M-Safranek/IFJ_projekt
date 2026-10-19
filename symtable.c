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

int get_height(struct symbol *where){
    if(where == NULL){
        return -1;
    }else{
        return max(get_height(where->left), get_height(where->right)) + 1;
    }
};

char *lower_string(char *string[]){
    for(int i = 0; string[i]; i++){
        string[i] = tolower(string[i]);
    }
};

struct symbol *search(char *what[], struct symbol *where){
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

struct symbol *ll_rotate(struct symbol *top){ //right_rotate
    struct symbol *left_symbol = top->left;
    top->left = left_symbol->right;
    left_symbol->right = top;
    return left_symbol;
};

struct symbol *rr_rotate(struct symbol *top){ //left_rotate
    struct symbol *right_symbol = top->right;
    top->right = right_symbol->left;
    right_symbol->left = top;
    return right_symbol;
};

struct symbol *lr_rotate(struct symbol *top){
    top->left = ll_rotate(top->left);
    return rr_rotate(top); 
};

struct symbol *rl_rotate(struct symbol *top){
    top->right = ll_rotate(top->right);
    return rr_rotate(top);
};

struct symbol *create_symbol(char *name[], TokenType type, char *scope[]){
    struct symbol *new_symbol = malloc(sizeof(struct symbol));
    if(new_symbol == NULL){
        return NULL;
    }
    *new_symbol->name = name;
    new_symbol->type = type;
    *new_symbol->scope = scope;
    new_symbol->left = NULL;
    new_symbol->right = NULL;
    new_symbol->height = 0;
    return new_symbol;
}

struct symbol *insert(struct symbol *where, char *name[], TokenType type, char *scope[]){
    if(where == NULL){
        struct symbol *new_symbol = create_symbol(name, type, scope);
        //výška
        //rotace
        where = new_symbol;
    }else if(strcmp(lower_string(where->name), lower_string(name)) > 0){
        where->left = insert(where->left, name, type, scope);
    }else if(strcmp(lower_string(where->name), lower_string(name)) < 0){
        where->right = insert(where->right, name, type, scope);
    }
    return where; //dosáhneme pokud se přiřadí nebo při nalezení přiřazeného symbolu
};

struct symbol *delete_symbol(struct symbol *what, struct symbol *top){
    if(search(what, top) != NULL){
        //najdi symbol který má odkaz na what
        //jestli má what děti, přiřaď je někde jinde
        //odeber odkaz z rodiče
        free(what);
    }
};

struct symbol *delete_tree(struct symbol *top){
    if(top != NULL){
        delete_tree(top->left);
        delete_tree(top->right);
        free(top);
    }
};
