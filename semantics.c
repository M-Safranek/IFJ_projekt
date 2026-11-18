#include "parser.h"
#include "symtable.h"
#include "symtable_stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
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

void analyze_expression(symtable_stack *stack, Expression *expr) {
  if (!expr)
    return;

  switch (expr->kind) {
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

  case E_VAR: {
    table_symbol *sym_var = sym_stack_search(stack, expr->name);
    if (sym_var == NULL) {
      exit(3); // error var not defined
    }
    expr->type = sym_var->type;
    break;
  }

  case E_BINARY: {
    analyze_expression(stack, expr->left);
    analyze_expression(stack, expr->right);

    DataType left_type = expr->left->type;
    DataType right_type = expr->right->type;

    switch (expr->op) {
    case O_PLUS:
      if (expr->op == O_PLUS && left_type == D_STRING &&
          right_type == D_STRING) {
        expr->type = D_STRING;
      }
      break;
    case O_MINUS:
    case O_DIVIDE:
    case O_MULTIPLY:
      if ((left_type == D_INT || left_type == D_DOUBLE) &&
          (right_type == D_INT || right_type == D_DOUBLE)) {
        expr->type = (left_type == D_DOUBLE || right_type == D_DOUBLE)
                         ? D_DOUBLE
                         : D_INT;
      } else {
        exit(6); // error incompatible types at arithmetic op
      }
      break;
    case O_LESS:
    case O_LESS_EQUAL:
    case O_GREATER:
    case O_GREATER_EQUAL:
      if (((left_type == D_INT || left_type == D_DOUBLE) &&
           (right_type == D_INT || right_type == D_DOUBLE)) ||
          (left_type == D_STRING && right_type == D_STRING)) {
        expr->type = D_INT;
      } else {
        exit(6); // error incompatible types at relation op
      }
      break;
    case O_EQUAL_EQUAL:
    case O_NOT_EQUAL:
      if ((left_type == right_type) ||
          ((left_type == D_INT || left_type == D_DOUBLE) &&
           (right_type == D_INT || right_type == D_DOUBLE))) {
        expr->type = D_INT;
        break;
      } else {
        exit(6); // error incompatible types at equality comparison op
      }
    default:
      exit(6); // error not binary operator
      break;
    }
    break;
  }
  case E_CALL: {
    for (Expression *arg = expr->args; arg != NULL; arg = arg->next) {
      analyze_expression(stack, arg);
    }
    const BuiltinSpec *is_builtin = NULL;
    for (int idx = 0; builtins[idx].names != NULL; idx++) {
      if (strcmp(expr->name, builtins[idx].names) == 0) {
        is_builtin = &builtins[idx];
        break;
      }
    }
    if (is_builtin != NULL) {
      int idx = 0;
      for (Expression *arg = expr->args; arg != NULL; arg = arg->next) {
        if (idx >= is_builtin->param_count) {
          exit(5); // error too many arguments at builtin
        }
        DataType expected = is_builtin->param_types[idx];
        if ((expected != D_UNKNOWN) && (arg->type != expected) &&
            !(expected == D_DOUBLE && arg->type == D_INT)) {
          exit(5); // error wrong argument type at builtin
        }
        idx++;
      }
      if (idx != is_builtin->param_count) {
        exit(5); // error too few arguments at builtin
      }
      expr->type = is_builtin->return_type;
      break;
    }
    table_symbol *func = sym_stack_search(stack, expr->name);
    if (func == NULL) {
      exit(3); // call to undefined function
    }
    int idx = 0;
    for (Expression *arg = expr->args; arg != NULL; arg = arg->next) {
      if (idx >= func->param_count) {
        exit(5); // error too many arguments at function
      }
      DataType expected = func->param_types[idx];
      if ((expected != D_UNKNOWN) && (arg->type != expected) &&
          !(expected == D_DOUBLE && arg->type == D_INT)) {
        exit(5); // error wrong argument type at function
      }
      idx++;
    }
    if (idx != func->param_count) {
      exit(5); // error too few arguments at function
    }
    expr->type = func->type;
    break;
  }
  case E_UNARY:
    analyze_expression(stack, expr->left);
    if (expr->left->type != D_DOUBLE && expr->left->type != D_INT) {
      exit(6); // error invalid operand for unary
    }
    expr->type = expr->left->type;
    break;

  default:
    break;
  }
}

void analyze_condition(symtable_stack *stack, Expression *condition) {
  analyze_expression(stack, condition);

  if (condition->op != O_EQUAL_EQUAL && condition->op != O_GREATER &&
      condition->op != O_GREATER_EQUAL && condition->op != O_NOT_EQUAL &&
      condition->op != O_LESS && condition->op != O_LESS_EQUAL) {
    exit(6); // error condition is not relation expression
  }
};

void analyze_body(symtable_stack *stack, Statement *func_body, Func *func) {
  if (func_body->kind == S_IF) { // statement is if
    // get all vars in condition and check against symtable
    Expression *condition = func_body->expr;
    while (condition != NULL) {
      analyze_condition(stack, condition);
      condition = condition->next;
    }

    stack = sym_push(stack, NULL);
    Statement *if_body = func_body->body;
    while (if_body != NULL) {
      analyze_body(stack, if_body, func);
      if_body = if_body->next;
    }
    stack = sym_pop(stack);

    if (func_body->else_body != NULL) {
      stack = sym_push(stack, NULL);
      Statement *if_else_body = func_body->else_body;
      while (if_else_body != NULL) {
        analyze_body(stack, if_else_body, func);
        if_else_body = if_else_body->next;
      }
      stack = sym_pop(stack);
    }

  } else if (func_body->kind == S_WHILE) { // statement is while
    // get all vars in condition and check against symtable
    stack = sym_push(stack, NULL);

    Expression *condition = func_body->expr;
    while (condition != NULL) {
      analyze_condition(stack, condition);
      condition = condition->next;
    }

    Statement *while_body = func_body->body;
    while (while_body != NULL) {
      analyze_body(stack, while_body, func);
      while_body = while_body->next;
    }

    stack = sym_pop(stack);

  } else if (func_body->kind == S_VAR_DEF) {
    if (search(func_body->expr->left->name, stack->top_symbol) !=
        NULL) { // defined in the same block twice
      exit(4);  // error 4
    }
    DataType var_type = func_body->expr->left->type;
    if (func_body->expr->right != NULL) {
      analyze_expression(stack, func_body->expr->right);
      DataType rhs_type = func_body->expr->right->type;
      if (rhs_type == D_UNIT || rhs_type == D_UNKNOWN) {
        exit(6);
      }
      if (var_type == D_UNKNOWN) {
        var_type = rhs_type;
      } else if (var_type != rhs_type &&
                 !(var_type == D_DOUBLE && rhs_type == D_INT)) {
        exit(6); // error incompatible types in variable definition
      }
      stack->top_symbol =
          insert(stack->top_symbol, func_body->expr->left->name, true, false);
      table_symbol *var_symbol =
          search(func_body->expr->left->name, stack->top_symbol);
      var_symbol->type = var_type;
      var_symbol->modifiable = func_body->is_var;
    }
  } else if (func_body->kind == S_ASSIGN) {
    table_symbol *var_sym =
        sym_stack_search(stack, func_body->expr->left->name);
    if (var_sym == NULL) {
      exit(3); // assignment to undefined variable
    } else if (var_sym->modifiable == false) {
      exit(7); // cannot assign to val
    }
    analyze_expression(stack, func_body->expr->right);
    if (var_sym->type != func_body->expr->right->type &&
        !(var_sym->type == D_DOUBLE && func_body->expr->right->type == D_INT)) {
      exit(6); // incompatible types at assignment
    }
    func_body->expr->left->type = var_sym->type;

  } else if (func_body->kind == S_EXPR) {
    analyze_expression(stack, func_body->expr);

  } else if (func_body->kind == S_RETURN) {
    if (func_body->expr == NULL) {
      if (func->return_type != D_UNIT) {
        exit(6); // error missing return from no-unit function
      }
    } else {
      analyze_expression(stack, func_body->expr);
      if (func->return_type == D_UNIT) {
        exit(6); // error returning a value from unit function
      }
      if (func->return_type != func_body->expr->type &&
          !(func->return_type == D_DOUBLE && func_body->expr->type == D_INT)) {
        exit(6); // error return value doesnt match function return type
      }
    }
  }
  return;
};

void analyze(symtable_stack *stack, Func *analyzed_code) {
  if (analyzed_code == NULL) {
    return;
  }

  Func *func = analyzed_code;

  while (func != NULL) { // analyze all funcs in code
    if (search(func->func_name, stack->top_symbol) ==
        NULL) { // second pass or double definition?
      stack->top_symbol = insert(stack->top_symbol, func->func_name, true,
                                 true); // get func into tree in global scope
    } else if (search(func->func_name, stack->top_symbol)->defined ==
               true) { // defined for the second time, not allowed
      exit(4);         // error 4
    }

    table_symbol *func_symbol = search(
        func->func_name, sym_top(stack)->top_symbol); // get the inserted func
    func_symbol->type = func->return_type;            // record type
    func_symbol->param_types = func->param_types;     // record params
    func_symbol->param_count = func->param_count;
    func_symbol->is_main = func->is_main; // is main

    Statement *func_body = func->body;  // get body of func
    if (func_symbol->subtree == NULL) { // are we doing a second pass?
      stack = sym_push(stack, NULL);    // push func scope
    } else {
      stack = sym_push(stack,
                       func_symbol->subtree); // push the completed func
                                              // symtable to stack for checking
    }

    while (func_body != NULL) { // analyze all statements in func
      analyze_body(stack, func_body, func);
      func_body = func_body->next; // move on
    }
    func_symbol->subtree = stack->top_symbol; // archive contents of stack for
                                              // checking in second pass
    stack = sym_pop(stack);
    func = func->next; // move on
  }
  return;
};

Func *build_table() {
  Func *analyzed_code = parse_program();    // get the parsed code
  symtable_stack *stack = sym_init_stack(); // make stack
  stack = sym_push(stack, NULL);            // push global scope

  analyze(stack, analyzed_code);
  analyze(stack, analyzed_code); // pass 2

  delete_tree(stack->top_symbol); // delete symtable
  symtable_stack *tree_delete_check = stack;
  while (tree_delete_check != NULL) {
    delete_tree(tree_delete_check->top_symbol);
    tree_delete_check = tree_delete_check->prev;
  }
  sym_delete_stack(stack); // free stack

  return analyzed_code;
}