#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantics.h"

typedef struct
{
    const char *names;
    DataType return_type;
    int param_count;
    DataType param_types[3];
} BuiltinSpec;

static const BuiltinSpec builtins[] = {
    {"print", D_UNIT, 1, {D_UNKNOWN}},
    {"readLine", D_STRING, 0, {0}},
    {"readInt", D_INT, 0, {0}},
    {"readDouble", D_DOUBLE, 0, {0}},
    {"toStr", D_STRING, 1, {D_UNKNOWN}},
    {"toInt", D_INT, 1, {D_DOUBLE}},
    {"strLength", D_INT, 1, {D_STRING}},
    {"subStr", D_STRING, 3, {D_STRING, D_INT, D_INT}},
    {"ord", D_INT, 2, {D_STRING, D_INT}},
    {"chr", D_STRING, 1, {D_INT}},
    {NULL, D_UNKNOWN, 0, {0}}};

int scope_var_counter = 0;

char *make_scope_name(char *name)
{
    int local_scope_counter = scope_var_counter++;
    size_t len = strlen(name) + 32;
    char *var_scope_name = malloc(len * sizeof(char));
    if (!var_scope_name)
    {
        exit(99);
    }
    snprintf(var_scope_name, len, "%s$%d", name, local_scope_counter);
    return var_scope_name;
}

static const BuiltinSpec *find_builtin(const char *name)
{
    for (int idx = 0; builtins[idx].names != NULL; idx++)
    {
        if (strcmp(name, builtins[idx].names) == 0)
        {
            return &builtins[idx];
        }
    }
    return NULL;
}

void analyze_expression(symtable_stack *stack, Expression *expr)
{
    if (!expr)
        return;

    switch (expr->kind)
    {
    case E_INT:
        expr->type = D_INT;
        break;

    case E_DOUBLE:
        expr->type = D_DOUBLE;
        break;

    case E_STRING:
        expr->type = D_STRING;
        break;

    case E_UNIT:
        expr->type = D_UNIT;
        break;

    case E_VAR:
    {
        table_symbol *sym_var = sym_stack_search(stack, expr->name);
        if (sym_var == NULL)
        {
            exit(3); // error var not defined
        }
        else if (sym_var->function == true)
        {
            exit(6); // var is func
        }
        expr->type = sym_var->type;
        if (sym_var->scope != NULL)
        {
            char *name_copy = strdup(sym_var->scope);
            if (!name_copy)
            {
                exit(99);
            }
            free(expr->name);
            expr->name = name_copy;
        }
        break;
    }

    case E_BINARY:
    {
        analyze_expression(stack, expr->left);
        analyze_expression(stack, expr->right);

        DataType left_type = expr->left->type;
        DataType right_type = expr->right->type;

        switch (expr->op)
        {
        case O_PLUS:
            if (left_type == D_STRING && right_type == D_STRING)
            {
                expr->type = D_STRING;
                break;
            }
            if ((left_type == D_INT || left_type == D_DOUBLE) && (right_type == D_INT || right_type == D_DOUBLE))
            {
                expr->type = (left_type == D_DOUBLE || right_type == D_DOUBLE)
                                 ? D_DOUBLE
                                 : D_INT;
            }
            else
            {
                exit(6); // error incompatible types at arithmetic op
            }
            break;
        case O_MINUS:
        case O_DIVIDE:
        case O_MULTIPLY:
            if ((left_type == D_INT || left_type == D_DOUBLE) && (right_type == D_INT || right_type == D_DOUBLE))
            {
                expr->type = (left_type == D_DOUBLE || right_type == D_DOUBLE)
                                 ? D_DOUBLE
                                 : D_INT;
            }
            else
            {
                exit(6); // error incompatible types at arithmetic op
            }
            break;
        case O_LESS:
        case O_LESS_EQUAL:
        case O_GREATER:
        case O_GREATER_EQUAL:
            if (((left_type == D_INT || left_type == D_DOUBLE) && (right_type == D_INT || right_type == D_DOUBLE)) || (left_type == D_STRING && right_type == D_STRING))
            {
                expr->type = D_INT;
            }
            else
            {
                exit(6); // error incompatible types at relation op
            }
            break;
        case O_EQUAL_EQUAL:
        case O_NOT_EQUAL:
            if ((left_type == right_type) || ((left_type == D_INT || left_type == D_DOUBLE) && (right_type == D_INT || right_type == D_DOUBLE)))
            {
                expr->type = D_INT;
                break;
            }
            else
            {
                exit(6); // error incompatible types at equality comparison op
            }
        default:
            exit(6); // error not binary operator
            break;
        }
        break;
    }
    case E_CALL:
    {
        for (Expression *arg = expr->args; arg != NULL; arg = arg->next)
        {
            analyze_expression(stack, arg);
        }
        const BuiltinSpec *is_builtin = NULL;
        for (int idx = 0; builtins[idx].names != NULL; idx++)
        {
            if (strcmp(expr->name, builtins[idx].names) == 0)
            {
                is_builtin = &builtins[idx];
                break;
            }
        }
        if (is_builtin != NULL)
        {
            int idx = 0;
            for (Expression *arg = expr->args; arg != NULL; arg = arg->next)
            {
                if (idx >= is_builtin->param_count)
                {
                    exit(5); // error too many arguments at builtin
                }
                DataType expected = is_builtin->param_types[idx];
                if ((expected != D_UNKNOWN) && (arg->type != expected) && !(expected == D_DOUBLE && arg->type == D_INT))
                {
                    exit(5); // error wrong argument type at builtin
                }
                idx++;
            }
            if (idx != is_builtin->param_count)
            {
                exit(5); // error too few arguments at builtin
            }
            expr->type = is_builtin->return_type;
            break;
        }
        table_symbol *func = sym_stack_search(stack, expr->name);
        if (func == NULL)
        {
            exit(3); // call to undefined function
        }
        int idx = 0;
        for (Expression *arg = expr->args; arg != NULL; arg = arg->next)
        {
            if (idx >= func->param_count)
            {
                exit(5); // error too many arguments at function
            }
            DataType expected = func->param_types[idx];
            if ((expected != D_UNKNOWN) && (arg->type != expected) && !(expected == D_DOUBLE && arg->type == D_INT))
            {
                exit(5); // error wrong argument type at function
            }
            idx++;
        }
        if (idx != func->param_count)
        {
            exit(5); // error too few arguments at function
        }
        expr->type = func->type;
        break;
    }
    case E_UNARY:
        analyze_expression(stack, expr->left);
        if (expr->left->type != D_DOUBLE && expr->left->type != D_INT)
        {
            exit(6); // error invalid operand for unary
        }
        expr->type = expr->left->type;
        break;

    default:
        break;
    }
}

void analyze_condition(symtable_stack *stack, Expression *condition)
{
    analyze_expression(stack, condition);

    if (condition->op != O_EQUAL_EQUAL && condition->op != O_GREATER && condition->op != O_GREATER_EQUAL && condition->op != O_NOT_EQUAL && condition->op != O_LESS && condition->op != O_LESS_EQUAL)
    {
        exit(6); // error condition is not relation expression
    }
}

void analyze_body(symtable_stack *stack, Statement *func_body, Func *func)
{
    if (func_body->kind == S_IF)
    { // statement is if
        // get all vars in condition and check against symtable
        Expression *condition = func_body->expr;
        while (condition != NULL)
        {
            analyze_condition(stack, condition);
            condition = condition->next;
        }

        stack = sym_push(stack, NULL);
        Statement *if_body = func_body->body;
        while (if_body != NULL)
        {
            analyze_body(stack, if_body, func);
            if_body = if_body->next;
        }
        stack = sym_pop(stack);

        if (func_body->else_body != NULL)
        {
            stack = sym_push(stack, NULL);
            Statement *if_else_body = func_body->else_body;
            while (if_else_body != NULL)
            {
                analyze_body(stack, if_else_body, func);
                if_else_body = if_else_body->next;
            }
            stack = sym_pop(stack);
        }
    }
    else if (func_body->kind == S_WHILE)
    { // statement is while
        // get all vars in condition and check against symtable
        stack = sym_push(stack, NULL);

        Expression *condition = func_body->expr;
        while (condition != NULL)
        {
            analyze_condition(stack, condition);
            condition = condition->next;
        }

        Statement *while_body = func_body->body;
        while (while_body != NULL)
        {
            analyze_body(stack, while_body, func);
            while_body = while_body->next;
        }

        stack = sym_pop(stack);
    }
    else if (func_body->kind == S_VAR_DEF)
    {
        if (search(func_body->name, stack->top_symbol) != NULL)
        {            // defined in the same block twice
            exit(4); // error 4
        }
        DataType var_type = func_body->type;
        if (func_body->expr != NULL)
        {
            analyze_expression(stack, func_body->expr);
            DataType rhs_type = func_body->expr->type;
            if (rhs_type == D_UNKNOWN)
            {
                exit(6);
            }
            if (var_type == D_UNKNOWN)
            {
                var_type = rhs_type;
            }
            else if (var_type != rhs_type && !(var_type == D_DOUBLE && rhs_type == D_INT))
            {
                exit(6); // error incompatible types in variable definition
            }
        }
        stack->top_symbol = insert(stack->top_symbol, func_body->name, true, false);
        table_symbol *var_symbol = search(func_body->name, stack->top_symbol);
        var_symbol->type = var_type;
        var_symbol->modifiable = func_body->is_var;
        var_symbol->scope = make_scope_name(func_body->name);

        char *name_copy = malloc(strlen(var_symbol->scope) + 1);
        if (!name_copy)
            exit(99);
        strcpy(name_copy, var_symbol->scope);

        free(func_body->name);
        func_body->name = name_copy;
        func_body->type = var_type;
    }
    else if (func_body->kind == S_ASSIGN)
    {
        table_symbol *sym_var = sym_stack_search(stack, func_body->name);
        if (sym_var == NULL)
        {
            exit(3); // assignment to undefined variable
        }
        else if (sym_var->modifiable == false)
        {
            exit(7); // cannot assign to val
        }
        analyze_expression(stack, func_body->expr);
        if (sym_var->type != func_body->expr->type && !(sym_var->type == D_DOUBLE && func_body->expr->type == D_INT))
        {
            exit(6); // incompatible types at assignment
        }
        func_body->type = sym_var->type;
        if (sym_var->scope != NULL)
        {
            char *name_copy = strdup(sym_var->scope);
            if (!name_copy)
            {
                exit(99);
            }
            free(func_body->name);
            func_body->name = name_copy;
        }
    }
    else if (func_body->kind == S_EXPR)
    {
        analyze_expression(stack, func_body->expr);
    }
    else if (func_body->kind == S_RETURN)
    {
        if (func_body->expr == NULL)
        {
            if (func->return_type != D_UNIT)
            {
                exit(6); // error missing return from no-unit function
            }
        }
        else
        {
            analyze_expression(stack, func_body->expr);

            if (func->return_type != D_UNIT &&
                func->return_type != func_body->expr->type && !(func->return_type == D_DOUBLE && func_body->expr->type == D_INT))
            {
                exit(6); // error return value doesnt match function return type
            }
        }
    }
    return;
}
// checks if function has return
bool func_has_return(Statement *stmt)
{
    if (!stmt)
    {
        return false;
    }
    Statement *last = stmt;
    while (last->next != NULL)
    {
        last = last->next;
    }
    if (last->kind == S_RETURN)
    {
        return true;
    }
    else if (last->kind == S_IF)
    {
        return func_has_return(last->body) && func_has_return(last->else_body);
    }
    return false;
}

void register_functions(symtable_stack *stack, Func *analyzed_code)
{
    Func *func = analyzed_code;
    int main_count = 0;

    while (func != NULL)
    {
        // Checks if the function name is a builtin function or duplicate
        if (find_builtin(func->func_name) != NULL || search(func->func_name, stack->top_symbol) != NULL)
        {
            exit(4);
        }

        // Checks @main rules
        if (func->is_main)
        {
            main_count++;
            if (main_count > 1)
            {
                exit(4); // More than one @main
            }
            if (func->param_count > 0)
            {
                exit(5); // @main cannot have parameters
            }
            if (func->return_type != D_UNIT)
            {
                exit(6); // @main must be Unit
            }
        }

        // Puts function header into global scope
        stack->top_symbol = insert(stack->top_symbol, func->func_name, true, true);
        table_symbol *func_symbol = search(func->func_name, stack->top_symbol);

        func_symbol->type = func->return_type;
        func_symbol->param_types = func->param_types;
        func_symbol->param_count = func->param_count;
        func_symbol->is_main = func->is_main;
        func_symbol->modifiable = false;

        func = func->next;
    }

    // Check if there is exactly one @main
    if (main_count == 0)
    {
        exit(3);
    }
}

void analyze_functions(symtable_stack *stack, Func *analyzed_code)
{
    Func *func = analyzed_code;

    while (func != NULL)
    {
        // Makes a new scope
        stack = sym_push(stack, NULL);

        // Parameters as
        for (int i = 0; i < func->param_count; i++)
        {
            // Checks if two parameters have the same name
            if (search(func->param_names[i], stack->top_symbol) != NULL)
            {
                exit(4); // Duplicate param name
            }

            stack->top_symbol = insert(stack->top_symbol, func->param_names[i], true, false);
            table_symbol *param_sym = search(func->param_names[i], stack->top_symbol);
            param_sym->type = func->param_types[i];
            param_sym->modifiable = false;
        }

        // Analyzes function body
        Statement *func_body = func->body;
        Statement *last = NULL;

        while (func_body != NULL)
        {
            analyze_body(stack, func_body, func);
            last = func_body;
            func_body = func_body->next;
        }

        // checks final statemnt for no-unit function
        if (func->return_type != D_UNIT && func->return_type != D_UNKNOWN)
        {
            if (last == NULL)
            {
                exit(6);
            }
            else if (last->kind == S_EXPR)
            {
                DataType actual = last->expr->type;
                DataType expected = func->return_type;

                if (actual != expected &&
                    !(expected == D_DOUBLE && actual == D_INT))
                {
                    exit(6);
                }
            }
            else if (last->kind == S_IF)
            {
                if (!func_has_return(last))
                {
                    exit(6);
                }
            }
            else if (last->kind == S_VAR_DEF || last->kind == S_ASSIGN || last->kind == S_WHILE)
            {
                exit(6);
            }
        }
        // Frees local tree and makes a new scope
        stack = sym_pop(stack);
        func = func->next;
    }
}

Func *build_table()
{
    Func *analyzed_code = parse_program(); // AST from parser
    if (analyzed_code == NULL)
    {
        exit(3); // Empty code with no @main
    }

    symtable_stack *stack = sym_init_stack();
    stack = sym_push(stack, NULL);

    register_functions(stack, analyzed_code);
    analyze_functions(stack, analyzed_code);

    delete_tree(stack->top_symbol);
    sym_delete_stack(stack);

    return analyzed_code;
}
