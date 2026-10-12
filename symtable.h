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
};

struct symbol *search(char what, struct symbol *where);
struct symbol rotate_left(struct symbol top);
struct symbol rotate_right(struct symbol top);
struct symbol *create_symbol(char name[]);
struct symbol *insert(struct symbol *where, char name[]);
struct symbol getdata(struct symbol what);
int delete(struct symbol what);

#endif //SYMTABLE_STRUCT