#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

char builtin_func_list[10][11] = {"print", "readLine", "readInt", "readDouble", "toStr", "toInt", "strLength", "subStr", "ord", "chr"};

enum{
    CONSTANT,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    ASSIGNMENT,
    UNARY
}ast_type;

struct ast_tree{
    ast_type;
    union{
        int value;
        struct{
            struct ast_tree *left;
            struct ast_tree *right;
        }ast_child;
    };
};

#endif //SEMANTICS_ANALYZER