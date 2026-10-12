#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

enum token_type{
    INT
};

typedef struct symbol{
    char name[21];
    enum token_type type;
    struct symbol* left;
    struct symbol* right;
    int height;
};

struct symbol *search(char what, struct symbol *where);
struct symbol *insert(struct symbol *where, char name[], enum token_type type);
struct symbol *delete(struct symbol *what);

#endif //SYMTABLE_STRUCT