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
        FUNC_DEF, //func_def
        FUNC_CALL, //func_call
        RETURN, //return_var
        CON_INT, //value_i
        CON_DOUBLE, //value_d
        CON_STRING, //string
        CON_UNIT, //unit
        VAR_INT, //var_name
        VAR_DOUBLE, //var_name
        VAR_STRING, //var_name
        VAR_UNIT, //var_name
        UNARY, //unary
        BINARY, //binary
        ASSIGNMENT, //assignment
        IFTHENELSE, //ifthenelse
        FOR, //ast_for
        WHILE, //ast_while
        INT_TO_DOUBLE //value_i
    };
    union{
        char *return_var;
        int value_i;
        double value_d;
        char *string;
        int unit;
        char *var_name;
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
            struct ast_tree *left;
            struct ast_tree *right;
        }assignment;
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