#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

typedef enum {
    //keywords
    T_DEF,
    T_DO,
    TK_FLOAT,
    T_ELSE,
    T_IF,
    T_IMPORT,
    TK_INT,
    T_RETURN,
    TK_STRING,
    T_THEN,
    TK_UNIT,
    T_VAL,
    T_VAR,
    T_WHILE,

    //special symbols
    T_LPAREN,
    T_RPAREN,
    T_LBRACE,
    T_RBRACE,
    T_LBRACKET,
    T_RBRACKET,
    T_COMMA,
    T_SEMICOLON,
    T_COLON,
    T_DOT,
    T_AT,

    T_INT,
    T_STRING,
    T_FLOAT,
    T_UNIT,
    
    //operators
    T_PLUS,
    T_MINUS,
    T_MULTIPLY,
    T_DIVIDE,
    T_LESS,
    T_LESS_EQUAL,
    T_GREATER,
    T_GREATER_EQUAL,
    T_EQUAL,
    T_EQUAL_EQUAL,

    T_IDENTIFIER,

    T_IDENT,
    T_EOL,
    T_ERROR,
    T_EOF
}TokenType;

typedef struct symbol{
    char name[21];
    TokenType type;
    bool defined;
    TableSymbol *left;
    TableSymbol *right;
    int height;
}TableSymbol;

TableSymbol *search(char *what[], TableSymbol *where);
TableSymbol *insert(TableSymbol *where, char *name[], TokenType type, bool defined);
TableSymbol *delete_tree(TableSymbol *top);

#endif //SYMTABLE_STRUCT