/**
 * @file symtable.h
 * @brief Header file for the symbol table for the IFJ26 language
 * @author Václav Tomaník, xtomanv00
 */

#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

#include "parser.h"

typedef struct symbol {
  char *name; // name of variable or function
  char *scope;
  DataType type;         // type of var or func
  bool defined;          // has the symbol already been defined
  bool modifiable;       // is it var or val
  bool function;         // is it a function
  DataType *param_types; // parameters for functions
  int param_count;
  bool is_main;           // is function main
  struct symbol *subtree; // storage for function definitions
  struct symbol *left;    // left child
  struct symbol *right;   // right child
  int height;             // height for balancing
} table_symbol;

/**
 * @brief Searches through a symbol table to find a symbol with a given name. If not found, returns NULL
 */
table_symbol *search(char *what, table_symbol *where);

/**
 * @brief Inserts a symbol into a symbol table and performs the necessary adjustments. Returns address of the newly created symbol or NULL in case of error.
 */
table_symbol *insert(table_symbol *where, char *name, bool defined, bool function);

/**
 * @brief Deletes a symbol table.
 */
void delete_tree(table_symbol *top);



/**
 * @brief Returns the higher value.
 */
int max(int a, int b);

/**
 * @brief Returns the height value of a given symbol.
 */
int get_height(table_symbol *where);

/**
 * @brief Compares the height of two given symbols.
 */
int compare_height(table_symbol *one, table_symbol *two);

/**
 * @brief Calculates the balance of two AVL tree branches.
 */
int get_balance(table_symbol *where);

/**
 * @brief Performs a right rotation.
 */
table_symbol *ll_rotate(table_symbol *top);

/**
 * @brief Performs a left rotation.
 */
table_symbol *rr_rotate(table_symbol *top);

/**
 * @brief First performs a left rotation, then performs a right rotation.
 */
table_symbol *lr_rotate(table_symbol *top);

/**
 * @brief First performs a right rotation, then performs a left rotation.
 */
table_symbol *rl_rotate(table_symbol *top);

/**
 * @brief Symbol creator used by insert().
 */
table_symbol *create_symbol(char *name, bool defined, bool function);

#endif // SYMTABLE_STRUCT