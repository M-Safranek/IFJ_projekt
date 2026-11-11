#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

#include "symtable.h"

typedef struct semantics_return_value{
    table_symbol *return_symtable;
    //+return built ast tree
}semantics_return;

#endif //SEMANTICS_ANALYZER