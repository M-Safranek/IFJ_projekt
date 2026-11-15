#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

#include "parser.h"

typedef struct symbol{
    char name[21];
    DataType type;
    bool defined;
    bool modifiable;
    bool function;
    DataType *param_types;
    int param_count;
    bool is_main;
    table_symbol *subtree;
    table_symbol *left;
    table_symbol *right;
    int height;
}table_symbol;

table_symbol *search(char *what[], table_symbol *where);
table_symbol *insert(table_symbol *where, char *name[], bool defined, bool function);
table_symbol *delete_tree(table_symbol *top);

#endif //SYMTABLE_STRUCT