#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "semantics.h"
#include "codegen.h"

int main(int argc, char **argv){
    if(argc != 2){
        FILE* code_file = fopen(argv[1], "r");
        if (code_file == NULL) {
            fprintf(stderr, "File couldn't be opened\n");
            exit(99);
        }

        Func* parsed_code = parser_interface(code_file);
        printf("Parsing success\n");
        parsed_code = build_table(parsed_code);
        printf("Analyzing success\n");
        codegen_print(parsed_code);

        free_func(parsed_code);
        fclose(code_file);
        return 1;
    }else{
        fprintf(stderr, "Provide a file to compile\n");
    }
}
