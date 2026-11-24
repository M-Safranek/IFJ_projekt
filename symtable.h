#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

#include "parser.h"

typedef struct symbol {
  char *name; // name of variable or function
  char *scope;
  DataType type; //type of var or func
  bool defined; //has the symbol already been defined
  bool modifiable; //is it var or val
  bool function; //is it a function
  DataType *param_types; //parameters for functions
  int param_count;
  bool is_main; //is function main
  struct symbol *subtree; //storage for function definitions
  struct symbol *left; //left child
  struct symbol *right; //right child
  int height; //height for balancing
} table_symbol;

table_symbol *search(char *what, table_symbol *where); //searches a symbol in a symtable, if not found, return NULL
table_symbol *insert(table_symbol *where, char *name, bool defined, bool function); //inserts a symbol into a table, returns address of inserted symbol
void delete_tree(table_symbol *top); //deletes an entire symtable

int max(int a, int b); //returns the integer that's higher in value
int get_height(table_symbol* where); //returns the height value of passed symbol
int compare_height(table_symbol* one, table_symbol* two); //compares the height of the symbols
int get_balance(table_symbol* where); //calculates the balance of two AVL tree branches
table_symbol* ll_rotate(table_symbol* top); //right rotation
table_symbol* rr_rotate(table_symbol* top); //left rotation
table_symbol* lr_rotate(table_symbol* top); //left rotation, then right rotation
table_symbol* rl_rotate(table_symbol* top); //right rotation, then left rotation
table_symbol* create_symbol(char* name, bool defined, bool function); //symbol creator to be used by insert()

#endif // SYMTABLE_STRUCT