#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

char builtin_func_list[10][11] = {"print", "readLine", "readInt", "readDouble", "toStr", "toInt", "strLength", "subStr", "ord", "chr"};

enum ast_operator{
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
};

struct ast_tree{
    enum type{
        FUNC_DEF,
        FUNC_CALL,
        RETURN,
        CON_INT,
        CON_DOUBLE,
        CON_STRING,
        CON_UNIT,
        VAR_INT,
        VAR_DOUBLE,
        VAR_STRING,
        VAR_UNIT,
        UNARY,
        BINARY,
        ASSIGNMENT,
        IFTHENELSE,
        FOR,
        WHILE
    };
    union{
        char *return_var;
        int value_i;
        double value_d;
        char *string;
        int unit;
        struct{
            char *name;
            char *operands;
            struct ast_tree *body;
        }func_def;
        struct{
            char *name;
            char *operands;
        }func_call;
        struct{
            ast_operator;
            struct ast_tree *var;
        }unary;
        struct{
            ast_operator;
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