#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

char builtin_func_list[10][11] = {"print", "readLine", "readInt", "readDouble", "toStr", "toInt", "strLength", "subStr", "ord", "chr"};

enum{
    CONSTANT,
    UNARY,
    BINARY,
    IFTHENELSE,
    FOR,
    WHILE
}ast_type;

struct ast_tree{
    ast_type;
    union{
        int value;
        char string[100];
        char operator;
        struct{
            char name[20];
            char operands[50][100];
        }func_call;
        struct{
            char operator;
            struct ast_tree *left;
            struct ast_tree *right;
        }binary;
        struct{
            struct ast_tree *condition;
            struct ast_tree *body;
            struct ast_tree *elsebody;
        }ifthenelse;
        struct{
            struct ast_tree *condition;
            struct ast_tree *body;
        }ast_for;
        struct{
            struct ast_tree *condition;
            struct ast_tree *body;
        }ast_while;
    };
};

#endif //SEMANTICS_ANALYZER