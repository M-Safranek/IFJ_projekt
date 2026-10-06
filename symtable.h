#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

enum token_type{
    INT
};

typedef struct symbol{
    char name[21];
    enum token_type type;
    char value[101];
    struct symbol* left;
    struct symbol* right;
};

int init_table();
struct symbol search(char what, struct symbol where);
struct symbol rotate_left(struct symbol top);
struct symbol rotate_right(struct symbol top);
int insert(struct symbol what);
struct symbol getdata(struct symbol what);
int delete(struct symbol what);

#endif //SYMTABLE_STRUCT