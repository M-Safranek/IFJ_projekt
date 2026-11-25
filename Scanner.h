/**
 * @file Scanner.h
 * @brief Lexical analyzer header for the IFJ26 language.
 * @author Daniel Vrba
 *
 * @details Defines the scanner structure, token types,
 * finite automaton states and functions used for lexical analysis.
 */

#ifndef SCANNER_H
#define SCANNER_H
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INDENT_LEVELS 1024

/**
 * @brief Dynamically allocated buffer that holds the current attr as its being
 * read
 */
typedef struct {
  char *data;
  unsigned long attr_length;
  unsigned long capacity;
} attr;

/**
 * @brief Scanner struct that represents the state the scanner is in
 *
 */
typedef struct {
  FILE *file; /**< input file(gets set to stdin in main for this project)*/

  attr current_attr; /**< holds the current_attr, is altered while reading the
                        input stream */

  int line_number; /**< holds the line count in the input stream*/

  bool has_newline; /**< used in the comment states to represent whether a
                       comment is multiline or oneline*/
  int block_comment_depth; /**< represents block_comment_depth when in a
                              multiline comment*/

  bool count_indent_flag; /**< states, whether to count indent on the next token
                             call or not, set in start when reading a newline*/

  int paren_depth; /**< count the paren depth in ()*/

  int is_running; /**< used as while loop condition in get_next_token()  while
                     (scanner->is_running). is set to false in create_token*/

  bool comment_at_line_start; /**< remembers whether a multiline comment started
a line or not is later used to decide whether to emit a newline or not */

  bool last_token_was_eol; /**< remembers whether the last token was a newline
                              or not, used to detect files without a trailing
                              newline */

  int pending_dedents; /**< remembers the amount of dedents that are needed to
                          emit(used at EOF) */

  long unsigned int indent_stack[MAX_INDENT_LEVELS]; /**< indent stack*/
  unsigned int indent_top;                           /**< top used index*/
} Scanner;

/**
 * @brief Types of tokens recognized by the lexical analyzer.
 *
 * @details Includes keywords, literals, operators, punctuation,
 * identifiers and indentation-related tokens.
 */
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

/**
 * @brief The states of the lexical analyzer.
 *
 */
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

/**
 * @brief union that represents a token's value
 *
 */
typedef union {
  int32_t int_value;
  double double_value;
  char *string_value;
} TokenValue;

/**
 * @brief token struct
 *
 */
typedef struct {
  TokenType type;
  int line;
  TokenValue attr;
} Token;

/**
 * @brief Initializes the lexical analyzer.
 *
 * @param source Input source file to be analyzed.
 * @return Initialized scanner structure.
 */
Scanner scanner_init(FILE *source);

/**
 * @brief Destroys the lexical analyzer state(mainly
 * free(scanner->current_attr.data);)
 *
 * @param s A pointer to the scanner
 *
 */
void scanner_destroy(Scanner *s);

/**
 * @brief returns the next token from the input stream
 *
 * @param s a pointer to the scanner state
 * @return The next token
 */
Token get_next_token(Scanner *s);

/**
 * @brief creates a token and sets is_running to false to stop the main while
 * loop in get_next_token()
 *
 * @param new_type  the token type
 * @param current_token  a pointer to the current_token a pointer to the
 * current_token(in get_next_token())
 * @param scanner a pointer the scanner state
 */
void create_token(TokenType new_type, Token *current_token, Scanner *scanner);

/**
 * @brief destroys a token if needed ( free(token->attr.string_value);
    token->attr.string_value = NULL;)
 *
 * @param token A pointer to the token
 */
void token_destroy(Token *token);
#endif