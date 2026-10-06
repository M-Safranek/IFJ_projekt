#ifndef SYMTABLE_STRUCT
#define SYMTABLE_STRUCT

typedef struct symbol{
    char name[21];
    int type;
    char value[101];
    struct symbol* left;
    struct symbol* right;
};

int inittable();
struct symbol search(char what);
int insert();
struct symbol getdata();
int delete();
int rotate_left();
int rotate_right();

#endif //SYMTABLE_STRUCT