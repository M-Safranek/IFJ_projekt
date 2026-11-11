#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

#include "parser.h"

typedef struct symbol{
    char name[21];
    DataType type;
    bool function;
    bool defined;
    bool modifiable;
    DataType *param_types; //is this needed?
    int param_count; //ditto
    bool is_main; //ditto
    table_symbol *left;
    table_symbol *right;
    int height;
    table_symbol *subtree;
}table_symbol;

table_symbol *search(char *what[], table_symbol *where);
table_symbol *insert(table_symbol *where, char *name[], bool defined, bool function);
table_symbol *delete_tree(table_symbol *top);

#endif //SYMTABLE_STRUCT