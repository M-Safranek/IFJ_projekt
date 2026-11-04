/*
 * Header file for the Scanner for the IFJ26 language.
 * Author: Daniel Vrba
 */

#ifndef SCANNER_H
#define SCANNER_H
#include <stdbool.h>
#define MAX_INDENT_LEVELS 1024

typedef struct {
  FILE *file;

  unsigned long attr_size;
  char attr[1000];

  int line_number;

  bool has_newline;
  int block_comment_depth;
  bool after_block_comment_check_flag;
  bool count_indent_flag;
  int paren_depth;
  int is_running;

  int pending_dedents;

  int indent_stack[MAX_INDENT_LEVELS];
  int indent_top;
} Scanner;

typedef enum {
  // keywords
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
  T_FLOAT,
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
  FLOAT,
  FLOAT_LAST_UNDERSCORE,
  FLOAT_EXPONENT_START,
  FLOAT_SIGNED_EXPONENT_START,
  FLOAT_EXPONENT,
  FLOAT_EXPONENT_LAST_UNDERSCORE,

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

void create_token(TokenType new_type, Token *token);
void scanner_init(FILE *source);
void token_destroy(Token *token);
#endif