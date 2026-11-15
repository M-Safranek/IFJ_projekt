// codegen
// TODO comment functions, errors
#include <stdio.h>
#include "parser.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


int temp_count = 0;
int lbl_counter = 0;
Func *program_root = NULL;
static DataType cur_return_type;

void codegen_print_expr(Expression *expr);
void codegen_print_stmt(Statement *stmt, bool is_tail);
void codegen_push_param(Func *f, Expression *arg);

// TODO
void print_error(int error_code, char *msg) {
    fprintf(stderr, "Error %d: %s\n ", error_code, msg);
    exit(error_code);
}

void codegen_int_to_double(DataType from, DataType to) {
    if(from == D_INT && to == D_DOUBLE) {
        printf("INT2FLOATS\n");
    }
}

void codegen_print_string_literal(const char *str) {
    printf("PUSHS string@");
    for(int idx = 0; str[idx] != '\0'; idx++) {
        unsigned char c = (unsigned char)str[idx];
        if(c <= 32 || c == '#'  || c == '\\') {
            printf("\\%03d", c);
        } else {
            printf("%c", c);
        }
    }
    printf("\n");
}

void codegen_print_concat(void) {
    int id = temp_count++;
    printf("DEFVAR TF@concat1_%d\n", id);
    printf("DEFVAR TF@concat2_%d\n", id);
    printf("DEFVAR TF@concat_subStr_result_%d\n", id);
    printf("POPS TF@concat2_%d\n", id);
    printf("POPS TF@concat1_%d\n", id);
    printf("CONCAT TF@concat_subStr_result_%d TF@concat1_%d TF@concat2_%d\n", id, id, id);
    printf("PUSHS TF@concat_subStr_result_%d\n", id);
}

void codegen_print_binary(Expression *expr) {
    DataType lt = expr->left->type;
    DataType rt = expr->right->type;
    bool is_double = false;
    if(lt == D_DOUBLE || rt == D_DOUBLE){
        is_double = true;
    }
    codegen_print_expr(expr->left);
    if(is_double) {
        codegen_int_to_double(lt, D_DOUBLE);
    }
    codegen_print_expr(expr->right);
    if(is_double) {
        codegen_int_to_double(rt, D_DOUBLE);
    }
    switch(expr->op) {

        case O_PLUS:
            if(lt == D_STRING && rt == D_STRING) {
                codegen_print_concat();    
            } else {
                printf("ADDS\n");
            }
            break;

        case O_MINUS:
            printf("SUBS\n");
            break;

        case O_MULTIPLY:
            printf("MULS\n");
            break;
        
        case O_DIVIDE:
            if(is_double) {
                printf("DIVS\n");
            } else {
                printf("IDIVS\n");
            }
            break;

        case O_LESS:
            printf("LTS\n");
            break;

        case O_LESS_EQUAL:
            printf("GTS\nNOTS\n");
            break;

        case O_GREATER:
            printf("GTS\n");
            break;

        case O_GREATER_EQUAL:
            printf("LTS\nNOTS\n");
            break;

        case O_EQUAL:
            printf("EQS\n");
            break;

        case O_EQUAL_EQUAL:
            printf("EQS\n");
            break;
        case O_NOT_EQUAL:
            printf("EQS\nNOTS\n");
            break;
        default:
            break;
    }
}

void codegen_push_param(Func *f, Expression *arg) {
    int idx = 0;
    for(Expression *curr = arg; curr != NULL; curr = curr->next) {
        codegen_print_expr(curr);
        if(f != NULL && idx < f->param_count) {
            codegen_int_to_double(curr->type, f->param_types[idx]);
        }
        idx++;
    }
}

void codegen_pop_param(Func *f, int argc){
    for(int i = argc - 1; i >= 0; i--) {
        printf("DEFVAR TF@%s\n", f->param_names[i]);
        printf("POPS TF@%s\n", f->param_names[i]);
    }
}

Func *codegen_find_func(const char *name) {
    for(Func *curr_f = program_root; curr_f != NULL; curr_f = curr_f->next) {
        if(strcmp(curr_f->func_name, name) == 0) {
            return curr_f;
        }
    }
    return NULL;
} 

bool codegen_is_builtin(const char *func_name) {
    static const char *names[] = {
        "print", "readInt", "readDouble", "readLine", "strLength",
        "ord", "chr", "toInt", "toStr", "subStr", NULL
    };
    for(int idx = 0; names[idx] != NULL; idx++) {
        if(strcmp(func_name, names[idx]) == 0) {
            return true;
        } 
    }
    return false;
}
// builtin functions
void codegen_print_builtin() {
    // print
    printf("LABEL $builtin_print\n");
    printf("PUSHFRAME\n");
    printf("WRITE LF@arg1\n");
    printf("PUSHS nil@nil\n");
    printf("POPS GF@%%retval\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    // readLine
    printf("LABEL $builtin_readLine\n");
    printf("PUSHFRAME\n");
    printf("READ GF@%%retval string\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    // readInt
    printf("LABEL $builtin_readInt\n");
    printf("PUSHFRAME\n");
    printf("READ GF@%%retval int\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    // readDouble
    printf("LABEL $builtin_readDouble\n");
    printf("PUSHFRAME\n");
    printf("READ GF@%%retval float\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    // toStr
    printf("LABEL $builtin_toStr\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR TF@arg_type\n");
    printf("TYPE TF@arg_type LF@arg1\n");
    printf("JUMPIFEQ $tostr_int TF@arg_type string@int\n");
    printf("JUMPIFEQ $tostr_double TF@arg_type string@double\n");
    printf("JUMPIFEQ $tostr_string TF@arg_type string@string\n");
    printf("LABEL $tostr_int\n");
    printf("INT2STR GF@%%retval LF@arg1\n");
    printf("JUMP $tostr_end\n");
    printf("LABEL $tostr_double\n");
    printf("FLOAT2STR GF@%%retval LF@arg1\n");
    printf("JUMP $tostr_end\n");
    printf("LABEL $tostr_string\n");
    printf("MOVE GF@%%retval LF@arg1\n");
    printf("LABEL $tostr_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    //toInt
    printf("LABEL $builtin_toInt\n");
    printf("PUSHFRAME\n");
    printf("FLOAT2INT GF@%%retval LF@arg1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    //strLength
    printf("LABEL $builtin_strLength\n");
    printf("PUSHFRAME\n");
    printf("STRLEN GF@%%retval LF@arg1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    // subStr
    printf("LABEL $builtin_subStr\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR TF@subStr_len\n");
    printf("DEFVAR TF@subStr_end_idx\n");
    printf("DEFVAR TF@subStr_cond\n");
    printf("DEFVAR LF@subStr_result\n");
    printf("DEFVAR LF@subStr_char\n");
    printf("STRLEN TF@subStr_len LF@arg1\n");
    printf("LT TF@subStr_cond int@0 LF@arg2\n");
    printf("JUMPIFEQ $subStr_invalid TF@subStr_cond bool@true\n");
    printf("LT LF@subStr_cond int@0 LF@arg3\n");
    printf("JUMPIFEQ $subStr_invalid TF@subStr_cond bool@true\n");
    printf("GT TF@subStr_cond LF@arg2 LF@arg3\n");
    printf("JUMPIFEQ $subStr_invalid TF@subStr_cond bool@true\n");
    printf("LT LF@subStr_cond TF@subStr_len LF@arg2\n");
    printf("JUMPIFEQ $subStr_invalid TF@subStr_cond bool@true\n");
    printf("GT TF@subStr_cond LF@arg3 TF@subStr_len\n");
    printf("JUMPIFEQ $subStr_invalid TF@subStr_cond bool@true\n");
    printf("ADD TF@subStr_end_idx LF@arg2 LF@arg3\n");
    printf("MOVE LF@subStr_result string@\n");
    printf("LABEL $subStr_loop\n");
    printf("LT LF@cond LF@arg2 LF@subStr_end_idx\n");
    printf("JUMPIFEQ $subStr_done LF@cond bool@false\n");
    printf("GETCHAR LF@char LF@arg1 LF@arg2\n");
    printf("CONCAT LF@subStr_result LF@subStr_result LF@char\n");
    printf("ADD LF@arg2 LF@arg2 int@1\n");
    printf("JUMP $subStr_loop\n");
    printf("LABEL $subStr_done\n");
    printf("MOVE GF@%%retval LF@subStr_result\n");
    printf("JUMP $subStr_end\n");
    printf("LABEL $subStr_invalid\n");
    printf("MOVE GF@%%retval int@40\n");
    printf("LABEL $subStr_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    // ord
    printf("LABEL $builtin_ord\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR TF@ord_len\n");
    printf("DEFVAR TF@ord_cond\n");
    printf("STRLEN TF@ord_len LF@arg1\n");
    printf("JUMPIFEQ $ord_is_invalid TF@ord_len int@0\n");
    printf("LT TF@ord_cond LF@arg2 TF@ord_len\n");
    printf("JUMPIFEQ $ord_is_invalid TF@ord_cond bool@false\n");
    printf("LT TF@ord_cond  LF@arg2 int@0 \n");
    printf("JUMPIFEQ $ord_is_invalid TF@ord_cond bool@true\n");
    printf("STRI2INT GF@%%retval LF@arg1 TF@arg2\n");
    printf("JUMP $ord_end\n");
    printf("LABEL $ord_is_invalid\n");
    printf("MOVE GF@%%retval int@-1\n");
    printf("LABEL $ord_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    // chr
    printf("LABEL $builtin_chr\n");
    printf("PUSHFRAME\n");
    printf("INT2CHAR GF@%%retval LF@arg1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
}

void codegen_print_call(Expression *expr){

    if(codegen_is_builtin(expr->name)) {
        int argc = 0;
        for(Expression *arg = expr->args; arg != NULL; arg = arg->next) {
            argc++;
        }
        
        for(Expression *arg = expr->args; arg != NULL; arg = arg->next) {
            codegen_print_expr(arg);
        }
        printf("CREATEFRAME\n");
        for(int idx = argc; idx >= 0; idx--) {
            printf("DEFVAR TF@arg%d\n", idx);
            printf("POPS TF@arg%d\n", idx);

        }
        printf("CALL $builtin_%s\n", expr->name);
        printf("PUSHS GF@%%retval\n");
        return;
    }

    Func *f = codegen_find_func(expr->name);
    if(f == NULL) {
        print_error(3, "calling undefined function");
    }

    int argc = 0;
    for(Expression *arg = expr->args; arg != NULL; arg = arg->next) {
        argc++;
    }

    if(argc != f->param_count) {
        print_error(5, "wrong number of arguments when calling the function");
    }

    codegen_push_param(f, expr->args);

    printf("CREATEFRAME\n");
    codegen_pop_param(f, argc);

    printf("CALL $F_%s\n", f->func_name);
    if(f->return_type == D_UNIT) {
        printf("PUSHS nil@nil\n");
    } else {
        printf("PUSHS GF@%%retval\n");
    }
}

void codegen_print_expr(Expression *expr) {
    if(!expr) return;
    switch (expr->kind) {

        case E_INT:
            printf("PUSHS int@%d\n",expr->int_val);
            break;

        case E_DOUBLE:
            printf("PUSHS float@%a\n",expr->double_val);
            break;

        case E_STRING:
            codegen_print_string_literal(expr->string_val);
            break;
        case E_UNIT:
            printf("PUSHS nil@nil\n");
            break;
        case E_VAR:
            printf("PUSHS LF@%s\n",expr->name);
            break;

        case E_BINARY:
            codegen_print_binary(expr);
            break;

        case E_CALL:
            codegen_print_call(expr);
            break;

        case E_UNARY:
            if(expr->op == O_MINUS) {
                if(expr->left->type == D_DOUBLE) {
                    printf("PUSHS float@0x0p+0\n");
                } else {
                    printf("PUSHS int@0\n");
                }
                codegen_print_expr(expr->left);
                printf("SUBS\n");
            } else {
                codegen_print_expr(expr->left);
            }
            break;

        default:
            break;
    }
}

void codegen_print_body(Statement *stmt, bool is_tail) {
    for(Statement *curr = stmt; curr != NULL; curr = curr->next){
        codegen_print_stmt(curr, is_tail && curr->next == NULL);
    }
}

void codegen_print_stmt(Statement *stmt, bool is_tail) {
    if(!stmt) return;

    switch(stmt->kind) {
        case S_VAR_DEF:
            codegen_print_expr(stmt->expr);
            codegen_int_to_double(stmt->expr->type, stmt->type);
            printf("DEFVAR LF@%s\n", stmt->name);
            printf("POPS LF@%s\n", stmt->name);
            break;  

        case S_ASSIGN:
            codegen_print_expr(stmt->expr);
            printf("POPS LF@%s\n", stmt->name);
            break;
        
        case S_RETURN:
            if(stmt->expr) {
                codegen_print_expr(stmt->expr);
                if(cur_return_type != D_UNIT) {
                    printf("POPS GF@%%retval\n");
                } else {
                    printf("CLEARS\n");
                }
            } else if(cur_return_type == D_UNIT) {
                printf("PUSHS nil@nil\nPOPS GF@%%retval\n");
            }
            printf("POPFRAME\n");
            printf("RETURN\n");
            break;

        case S_EXPR:
            codegen_print_expr(stmt->expr);
            if(is_tail && cur_return_type != D_UNIT) {
                printf("POPS GF@%%retval\n");
            } else {
                printf("CLEARS\n");
            }
            break;

        case S_IF: {
            int lbl_else = lbl_counter++;
            int lbl_end = lbl_counter++;
            codegen_print_expr(stmt->expr);
            printf("PUSHS bool@false\n");
            printf("JUMPIFEQS $L_%d\n", lbl_else);
            codegen_print_body(stmt->body, false);
            printf("JUMP $L_%d\n", lbl_end);
            printf("LABEL $L_%d\n", lbl_else);
            codegen_print_body(stmt->else_body, false);
            printf("LABEL $L_%d\n", lbl_end);
            break;
        }

        case S_WHILE: {
            int lbl_start = lbl_counter++;
            int lbl_end = lbl_counter++;
            printf("LABEL $L_%d\n", lbl_start);
            codegen_print_expr(stmt->expr);
            printf("PUSHS bool@false\n");
            printf("JUMPIFEQS $L_%d\n", lbl_end);
            codegen_print_body(stmt->body, false);
            printf("JUMP $L_%d\n", lbl_start);
            printf("LABEL $L_%d\n", lbl_end);
            break;
        }
        default:
            break;
    }
}

void codegen_print_func(Func *f) {
    cur_return_type = f->return_type;
    printf("LABEL $F_%s\n", f->func_name);
    printf("PUSHFRAME\n");
    
    codegen_print_body(f->body, true);
    printf("POPFRAME\n");
    printf("RETURN\n");
}

void codegen_print(Func *ast_root) {
    program_root = ast_root;
    Func *main_f = NULL;
    for(Func *f = ast_root; f != NULL; f = f->next) {
        if(f->is_main) {
            main_f =f;
            break;
        }
    }

    printf(".IFJcode26\n");
    printf("DEFVAR GF@%%retval\n");

    codegen_print_builtin();

    printf("JUMP $main\n");

    for(Func *f = ast_root; f != NULL; f = f->next) {
        codegen_print_func(f);
    }

    printf("LABEL $main\n");
    printf("CREATEFRAME\n");
    printf("CALL $F_%s\n", main_f->func_name);

    printf("EXIT int@0\n");    
}


