#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

#include "parser.h"

char builtin_func_list[10][11] = {"print", "readLine", "readInt", "readDouble", "toStr", "toInt", "strLength", "subStr", "ord", "chr"};

typedef enum{
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
}ast_operator;

struct func_arguments{
    char *arg_name;
    DataType arg_type;
    struct func_arguments *next;
};

typedef struct ast_tree_node{
    enum type{
        TOP, //func_def
        FUNC_DEF, //func_def
        FUNC_CALL, //func_call
        RETURN, //link
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
        WHILE, //ast_while
        INT_TO_DOUBLE //link
    }node_type;
    union{
        int value_i;
        double value_d;
        char *string;
        int unit;
        char *var_name;
        struct ast_tree_node *link;
        struct{
            char *name;
            struct func_arguments arguments;
            int arg_count;
            struct ast_tree_node *body;
            struct ast_tree_node *next;
        }func_def;
        struct{
            char *name;
            struct func_arguments arguments;
        }func_call;
        struct{
            ast_operator operator;
            struct ast_tree_node *var;
        }unary;
        struct{
            ast_operator operator;
            struct ast_tree_node *left;
            struct ast_tree_node *right;
        }binary;
        struct{
            struct ast_tree_node *left;
            struct ast_tree_node *right;
        }assignment;
        struct{
            struct ast_tree_node *condition;
            struct ast_tree_node *body;
            struct ast_tree_node *elsebody;
        }ifthenelse;
        struct{
            struct ast_tree_node *condition;
            struct ast_tree_node *body;
        }ast_while;
    }properties;
}ast_tree;

#endif //SEMANTICS_ANALYZER