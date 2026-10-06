#include <stdio.h>
#include "symtable.h"

int init_table(){

};

struct symbol search(char what, struct symbol where){
//    if(where == NULL){ //existuje to, kam jsme se dostali?
//        return NULL;
//    }elseif(what < where){ //menší hodnota, domluv která
//        where = where.left;
//    }elseif(what > where){ //vetší hodnota
//        where = what.right;
//    }else{ //našli jsme
//        return where;
//    }
};

struct symbol rotate_left(struct symbol top){
    //rightsymbol = top right 
    //top right = rightsymbol left
    //rightsymbol left = top
    //return rightsymbol //jak otočení oznámit rodiči?
};

struct symbol rotate_right(struct symbol top){
    //leftsymbol = top left
    //top left = leftsymbol right
    //leftsymbol right = top
    //return leftsymbol
};

int insert(struct symbol what){
    //je hodnota menší? jdi doleva
    //je větší? jdi doprava
    //opakuj dokud nenarazíme na konec stromu (možná použij search)
    //pak to dáme nejspíše doleva a otočíme dle potřeby
    //asi by bylo ideální použít malloc
};

struct symbol getdata(struct symbol what){
    
};

int delete(struct symbol what){
    //(možná použij search)
    //najdi symbol který má odkaz na what
    //jestli má what děti, přiřaď je někde jinde
    //odeber odkaz z rodiče
};
