#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

#include "parser.h"

typedef struct symbol{
    char *name;
    DataType type;
    bool defined;
    bool modifiable;
    bool function;
    DataType *param_types;
    int param_count;
    char **param_names;
    bool is_main;
    struct symbol *subtree;
    struct symbol *left;
    struct symbol *right;
    int height;
}table_symbol;

table_symbol *search(char *what, table_symbol *where);
table_symbol *insert(table_symbol *where, char *name, bool defined, bool function);
void delete_tree(table_symbol *top);

#endif //SYMTABLE_STRUCT