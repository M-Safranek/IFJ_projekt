/*
 * Header file for the Scanner for the IFJ26 language.
 * Author: Daniel Vrba
 */

#ifndef SCANNER_H
#define SCANNER_H
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_INDENT_LEVELS 1024

typedef struct {
  char *data;
  unsigned long attr_length;
  unsigned long capacity;
} attr;

typedef struct {
  FILE *file;

  attr current_attr;

  int line_number;

  bool has_newline;
  int block_comment_depth;
  bool after_block_comment_check_flag;
  bool count_indent_flag;
  int paren_depth;
  int is_running;
  bool comment_at_line_start;

  int pending_dedents;

  long unsigned int indent_stack[MAX_INDENT_LEVELS];
  unsigned int indent_top;
} Scanner;

typedef enum {
  // keywords
  T_DEF,
  T_DO,
  TK_DOUBLE,
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

  // special symbols
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
  T_DOUBLE,
  T_UNIT,

  // operators
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
  T_NOT_EQUAL,

  T_IDENTIFIER,

  T_INDENT,
  T_DEDENT,
  T_EOL,
  T_ERROR,
  T_EOF
} TokenType;

typedef enum {
  START,

  UNDERSCORE,

  IDENTIFIER,

  /* Numbers */
  INT,
  INT_LAST_UNDERSCORE,

  ZERO,
  DOT,
  DECIMAL_START,
  DOUBLE,
  DOUBLE_LAST_UNDERSCORE,
  DOUBLE_EXPONENT_START,
  DOUBLE_SIGNED_EXPONENT_START,
  DOUBLE_EXPONENT,
  DOUBLE_EXPONENT_LAST_UNDERSCORE,

  /* Strings */
  STRING_START,
  DOUBLE_DOUBLE_QUOTE,
  STRING,
  MULTILINE_STRING,
  MULTILINE_STRING_END_1,
  MULTILINE_STRING_END_FINAL,
  ESCAPE_SEQUENCE,

  /* Operators */
  PLUS,
  MINUS,
  TIMES,
  SLASH,
  LESS,
  LESS_EQUAL,
  GREATER,
  GREATER_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  T_EXCLAMATION,

  /* Punctuation */
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  COMMA,
  SEMICOLON,

  L_COMMENT,
  ML_COMMENT,
  ML_COMMENT_STAR,
  ML_COMMENT_SLASH,
  ML_COMMENT_END_CHECK,

  L_START,
  L_START_SLASH,
  L_START_L_COMMENT,
  L_START_ML_COMMENT,
  L_START_ML_COMMENT_SLASH,
  L_START_ML_COMMENT_STAR,

  FALSE

} State;

#include <stdint.h>

typedef union {
  int32_t int_value;
  double double_value;
  char *string_value;
} TokenValue;

typedef struct {
  TokenType type;
  int line;
  TokenValue attr;
} Token;

Scanner scanner_init(FILE *source);
void scanner_destroy(Scanner *s);
Token get_next_token(Scanner *s);
void create_token(TokenType new_type, Token *current_token, Scanner *scanner);
void token_destroy(Token *token);
#endif