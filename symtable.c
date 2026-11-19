#include "symtable.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int max(int a, int b)
{
    // return max value out of the two provided
    if (a < b) {
        return b;
    } else {
        return a;
    }
}

int get_height(table_symbol* where)
{
    // get height of symbol if it exists
    if (where == NULL) {
        return 0;
    } else {
        return where->height;
    }
}

int compare_height(table_symbol* one, table_symbol* two)
{
    // quick comparison of height between two symbols
    return max(get_height(one), get_height(two)) + 1;
}

int get_balance(table_symbol* where)
{
    // tree balance calculation
    if (where == NULL) {
        return 0;
    } else {
        return get_height(where->left) - get_height(where->right);
    }
}

table_symbol* search(char* what, table_symbol* where)
{
    // searches for a symbol, returns NULL if nonexistent
    if (where == NULL) {
        return NULL;
    } else if (strcmp(where->name, what) == 0) {
        return where;
    } else if (strcmp(where->name, what) > 0) {
        return search(what, where->left);
    } else {
        return search(what, where->right);
    }
}

table_symbol* ll_rotate(table_symbol* top)
{ // right_rotate
    table_symbol* left_symbol = top->left;
    top->left = left_symbol->right;
    left_symbol->right = top;
    top->height = compare_height(top->left, top->right);
    left_symbol->height = compare_height(left_symbol->left, top);
    return left_symbol;
}

table_symbol* rr_rotate(table_symbol* top)
{ // left_rotate
    table_symbol* right_symbol = top->right;
    top->right = right_symbol->left;
    right_symbol->left = top;
    top->height = compare_height(top->left, top->right);
    right_symbol->height = compare_height(right_symbol->right, top);
    return right_symbol;
}

table_symbol* lr_rotate(table_symbol* top)
{ // left, then right
    top->left = rr_rotate(top->left);
    return ll_rotate(top);
}

table_symbol* rl_rotate(table_symbol* top)
{ // right, then left
    top->right = ll_rotate(top->right);
    return rr_rotate(top);
}

table_symbol* create_symbol(char* name, bool defined, bool function)
{
    // record necessary values for symbol creation
    table_symbol* new_symbol = malloc(sizeof(table_symbol));
    if (new_symbol == NULL) {
        exit(99);
    }
    new_symbol->name = malloc(strlen(name) + 1);
    if (new_symbol->name == NULL) {
        free(new_symbol);
        exit(99);
    }
    strcpy(new_symbol->name, name);
    new_symbol->scope = NULL;
    new_symbol->defined = defined;
    new_symbol->function = function;
    new_symbol->left = NULL;
    new_symbol->right = NULL;
    new_symbol->height = 0;
    new_symbol->subtree = NULL;
    new_symbol->param_types = NULL;
    new_symbol->param_count = 0;
    new_symbol->type = D_UNKNOWN;
    new_symbol->is_main = false;
    new_symbol->modifiable = false;
    return new_symbol;
}

table_symbol* insert(table_symbol* where, char* name, bool defined,
    bool function)
{
    // insert a symbol into a tree, search first
    if (where == NULL) { // symbol doesn't exist yet, create
        table_symbol* new_symbol = create_symbol(name, defined, function);
        if (new_symbol == NULL) {
            return NULL;
        }
        where = new_symbol;
    } else if (strcmp(where->name, name) > 0) { // symbol is in the left child
        where->left = insert(where->left, name, defined, function);
        if (get_balance(where) == 2) {
            if (strcmp(where->left->name, name) > 0) {
                where = ll_rotate(where);
            } else {
                where = lr_rotate(where);
            }
        }
    } else if (strcmp(where->name, name) < 0) { // symbol is in the right child
        where->right = insert(where->right, name, defined, function);
        if (get_balance(where) == -2) {
            if (strcmp(where->right->name, name) < 0) {
                where = rr_rotate(where);
            } else {
                where = rl_rotate(where);
            }
        }
    }
    where->height = compare_height(where->left, where->right); // calculate height
    return where; // reached after inserting or finding already inserted symbol
                  // with the same name, thus we search first
}

void delete_tree(table_symbol* top)
{
    if (top != NULL) {
        free(top->name);
        free(top->scope);
        delete_tree(top->subtree);
        delete_tree(top->left);
        delete_tree(top->right);
        free(top);
    }
}
