#include <stdio.h>
#include <stdlib.h>

#include "codegen.h"
#include "parser.h"
#include "semantics.h"

int main(void)
{
    Func* parsed_code = parser_interface(stdin);
    parsed_code = build_table(parsed_code);
    codegen_print(parsed_code);

    free_func(parsed_code);
    return 1;
}
