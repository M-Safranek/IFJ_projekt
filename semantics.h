#ifndef SEMANTICS_ANALYZER
#define SEMANTICS_ANALYZER

char builtin_func_list[10][11] = {"print", "readLine", "readInt", "readDouble", "toStr", "toInt", "strLength", "subStr", "ord", "chr"};

struct ast_tree{
    enum{
        CONSTANT,
        BINARY,
        ASSIGNMENT,
        UNARY

    }type;
};

#endif //SEMANTICS_ANALYZER