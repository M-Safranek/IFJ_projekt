#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

char builtin_func_list[10][11] = {"print", "readLine", "readInt", "readDouble", "toStr", "toInt", "strLength", "subStr", "ord", "chr"};

enum{
    PLUS_UN,
    MINUS_UN,
    MUL,
    DIV,
    PLUS_BI,
    MIN_BI,
    LESS,
    LESS_EQ,
    MORE,
    MORE_EQ,
    IS,
    NOT
}operator;

struct ast_tree{
    enum{
        CONSTANT,
        VARIABLE,
        UNARY,
        BINARY,
        IFTHENELSE,
        FOR,
        WHILE
    }type;
    union{
        int value_i;
        double value_d;
        char *string;
        bool bolean;
        int unit;
        operator;
        struct{
            char *name;
            char *operands;
        }func_call;
        struct{
            operator;
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
    }properties;
};

#endif //SEMANTICS_ANALYZER