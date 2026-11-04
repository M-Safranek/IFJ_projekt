/*
 * Scanner for the IFJ26 language.
 * Reads source code and returns lexical tokens.
 * Author: Daniel Vrba
 */

#include "Scanner.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Scanner scanner;
void forget_char(char c);

void scanner_init(FILE *source) {
  scanner.file = source;

  scanner.attr_size = 0;
  scanner.attr[0] = '\0';

  scanner.is_running = 0;
  scanner.line_number = 1;

  scanner.has_newline = false;
  scanner.block_comment_depth = 0;
  scanner.after_block_comment_check_flag = false;
  scanner.count_indent_flag = true;

  scanner.paren_depth = 0;
  scanner.pending_dedents = 0;

  scanner.indent_top = 0;
  scanner.indent_stack[0] = 0;
}

// gets rid of _ and d/D before conversion
void normalize_number(char *number) {
  int read = 0;
  int write = 0;

  while (number[read] != '\0') {
    if (number[read] != '_') {
      number[write] = number[read];
      write++;
    }
    read++;
  }

  number[write] = '\0';

  // Remove optional Double suffix.
  if (write > 0 && (number[write - 1] == 'd' || number[write - 1] == 'D')) {
    number[write - 1] = '\0';
  }
}

void token_destroy(Token *token) {
  if (token->type == T_IDENTIFIER || token->type == T_STRING) {
    free(token->attr.string_value);
    token->attr.string_value = NULL;
  }
}

bool push_indent(int indent) {
  if (scanner.indent_top + 1 >= MAX_INDENT_LEVELS) {
    return false;
  }

  scanner.indent_top++;
  scanner.indent_stack[scanner.indent_top] = indent;
  return true;
}

void pop_indent(void) {
  if (scanner.indent_top > 0) {
    scanner.indent_top--;
  }
}

// Returns false on indentation error.
bool new_indent(int current_indent, Token *current_token) {
  int previous_indent = scanner.indent_stack[scanner.indent_top];

  if (current_indent > previous_indent) {
    if (!push_indent(current_indent)) {
      return false;
    }

    create_token(T_INDENT, current_token);
  } else if (current_indent < previous_indent) {
    while (scanner.indent_top > 0 &&
           current_indent < scanner.indent_stack[scanner.indent_top]) {
      pop_indent();
      scanner.pending_dedents++;
    }

    if (current_indent != scanner.indent_stack[scanner.indent_top]) {
      scanner.pending_dedents = 0;
      return false;
    }

    if (scanner.pending_dedents > 0) {
      scanner.pending_dedents--;
      create_token(T_DEDENT, current_token);
    }
  }

  return true;
}

const char *keywords[] = {"def",    "do",  "Double", "else",   "if",
                          "import", "Int", "return", "String", "then",
                          "Unit",   "val", "var",    "while"};

// checks if the current identifier is a keywords only works because of the
// keywords being at the start of the enum and in the same order as in the array
bool keyword_check(Token *current_token) {
  scanner.attr[scanner.attr_size] = '\0';
  for (int i = T_DEF; i <= T_WHILE; i++) {
    if (strcmp(scanner.attr, keywords[i]) == 0) {
      create_token(i, current_token);
      return true;
    }
  }
  return false;
}

/*
 * Returns the next token from the input.
 */
Token get_next_token(void) {
  Token current_token;
  if (scanner.pending_dedents > 0) {
    scanner.attr_size = 0;
    scanner.pending_dedents--;
    create_token(T_DEDENT, &current_token);
    return current_token;
  }

  State state = START;
  scanner.attr_size = 0;
  scanner.is_running = 1;

  while (scanner.is_running) {
    int c = getc(scanner.file);

    // ignore
    if (state == START && c != EOF && c != '\n' && isspace((unsigned char)c) &&
        !scanner.count_indent_flag) {
      continue;
    }

    // add to the string
    if (scanner.attr_size < sizeof(scanner.attr) - 1) {
      if (c != EOF)
        scanner.attr[scanner.attr_size++] = (char)c;
    }

    if (scanner.after_block_comment_check_flag) {
      state = ML_COMMENT_END_CHECK;
      scanner.after_block_comment_check_flag = false;
    }

    if (scanner.count_indent_flag) {
      state = L_START;
      scanner.count_indent_flag = false;
    }

    switch (state) {
    // starting case
    case START:
      if (c == '_')
        state = UNDERSCORE;
      else if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
        state = IDENTIFIER;
      else if (c == '0')
        state = ZERO;
      else if (isdigit(c))
        state = INT;
      else if (c == '.')
        state = DOT;
      else if (c == '"')
        state = STRING_START;
      else if (c == '+')
        create_token(T_PLUS, &current_token);
      else if (c == '-')
        create_token(T_MINUS, &current_token);
      else if (c == '*')
        create_token(T_MULTIPLY, &current_token);
      else if (c == '/')
        state = SLASH;
      else if (c == '=')
        state = EQUAL;
      else if (c == '<')
        state = LESS;
      else if (c == '>')
        state = GREATER;

      else if (c == '!')
        state = T_EXCLAMATION;

      else if (c == '(') {
        create_token(T_LPAREN, &current_token);
        scanner.paren_depth++;
      }

      else if (c == ')') {
        create_token(T_RPAREN, &current_token);
        if (scanner.paren_depth > 0) {
          scanner.paren_depth--;
        }
      }

      else if (c == '[')
        create_token(T_LBRACKET, &current_token);
      else if (c == ']')
        create_token(T_RBRACKET, &current_token);
      else if (c == ';')
        create_token(T_SEMICOLON, &current_token);
      else if (c == ':')
        create_token(T_COLON, &current_token);
      else if (c == ',')
        create_token(T_COMMA, &current_token);
      else if (c == '@')
        create_token(T_AT, &current_token);
      else if (c == '\n') {
        scanner.line_number += 1;
        if (scanner.paren_depth == 0)
          create_token(T_EOL, &current_token);
        scanner.count_indent_flag = true;
      } else if (c == EOF) {
        if (scanner.indent_top > 0) {
          scanner.pending_dedents = scanner.indent_top - 1;
          scanner.indent_top = 0;
          create_token(T_DEDENT, &current_token);

        } else {
          create_token(T_EOF, &current_token);
        }
      }

      else
        state = FALSE;

      break;

    case UNDERSCORE:
      if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_') ||
          isdigit(c))
        state = IDENTIFIER;

      else {
        state = FALSE;
        forget_char(c);
      }

      break;

    case T_EXCLAMATION:
      if (c == '=') {
        create_token(T_NOT_EQUAL, &current_token);
      } else {
        state = FALSE;
        forget_char(c);
      }
      break;

    case IDENTIFIER:
      if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_') ||
          isdigit(c))
        state = IDENTIFIER;

      else {
        forget_char(c);
        if (!keyword_check(&current_token))
          create_token(T_IDENTIFIER, &current_token);
      }

      break;

      /* ==========================
       * Numbers
       * ========================== */

    case INT:
      if (isdigit(c))
        state = INT;

      else if (c == '_')
        state = INT_LAST_UNDERSCORE;

      else if (c == '.')
        state = DECIMAL_START;

      else if (c == 'e' || c == 'E')
        state = FLOAT_EXPONENT_START;

      else {
        forget_char(c);
        create_token(T_INT, &current_token);
      }

      break;

    case INT_LAST_UNDERSCORE:
      if (isdigit(c))
        state = INT;

      else {
        state = FALSE;
        forget_char(c);
      }

      break;

    case ZERO:
      if (c == '0')
        state = ZERO;
      else if (c == '.')
        state = DECIMAL_START;

      else if (isdigit(c)) {
        state = FALSE;
        forget_char(c);
      }

      else {
        forget_char(c);
        create_token(T_INT, &current_token);
      }

      break;

    case DOT:
      if (isdigit(c))
        state = FLOAT;
      else {
        forget_char(c);
        create_token(T_DOT, &current_token);
      }
      break;

    case DECIMAL_START:
      if (isdigit(c))
        state = FLOAT;

      else {
        state = FALSE;
        forget_char(c);
      }
      break;

    case FLOAT:
      if (isdigit(c))
        state = FLOAT;

      else if (c == '_')
        state = FLOAT_LAST_UNDERSCORE;

      else if (c == 'e' || c == 'E')
        state = FLOAT_EXPONENT_START;

      else if (((c == 'd') || (c == 'D'))) {
        create_token(T_FLOAT, &current_token);
      } else {
        forget_char(c);
        create_token(T_FLOAT, &current_token);
      }
      break;

    case FLOAT_LAST_UNDERSCORE:
      if (isdigit(c))
        state = FLOAT;

      else {
        state = FALSE;
        forget_char(c);
      }
      break;

    case FLOAT_EXPONENT_START:
      if ((c == '+') || (c == '-'))
        state = FLOAT_SIGNED_EXPONENT_START;

      else if (isdigit(c))
        state = FLOAT_EXPONENT;

      else {
        state = FALSE;
        forget_char(c);
      }
      break;

    case FLOAT_SIGNED_EXPONENT_START:
      if (isdigit(c))
        state = FLOAT_EXPONENT;

      else {
        state = FALSE;
        forget_char(c);
      }
      break;

    case FLOAT_EXPONENT:
      if (isdigit(c))
        state = FLOAT_EXPONENT;

      else if (c == '_')
        state = FLOAT_EXPONENT_LAST_UNDERSCORE;

      else if (((c == 'd') || (c == 'D'))) {
        create_token(T_FLOAT, &current_token);
      } else {
        forget_char(c);
        create_token(T_FLOAT, &current_token);
      }

      break;

    case FLOAT_EXPONENT_LAST_UNDERSCORE:
      if (isdigit(c))
        state = FLOAT_EXPONENT;
      else {
        state = FALSE;
        forget_char(c);
      }
      break;

      /* ==========================
       * Strings
       * ========================== */

    case STRING_START:
      if (c == '"')
        state = DOUBLE_DOUBLE_QUOTE;
      else if (c >= 0 && c <= 127)
        state = STRING;
      else {
        state = FALSE;
        forget_char(c);
      }

      break;

    case DOUBLE_DOUBLE_QUOTE:
      if (c == '"')
        state = MULTILINE_STRING;

      else {
        create_token(T_STRING, &current_token);
        forget_char(c);
      }
      break;

    case STRING:
      if ((c >= 0 && c <= 127) && (c != '\n' && c != '\\' && c != '"'))
        state = STRING;

      else if (c == '\\')
        state = ESCAPE_SEQUENCE;

      else if (c == '"')
        create_token(T_STRING, &current_token);

      else {
        state = FALSE;
        forget_char(c);
      }

      break;

    case ESCAPE_SEQUENCE:

      state = STRING;
      if (c == 'n') {
        scanner.attr[scanner.attr_size - 2] = '\n';
        scanner.attr_size -= 1;
      } else if (c == 'r') {
        scanner.attr[scanner.attr_size - 2] = '\r';
        scanner.attr_size -= 1;
      } else if (c == 't') {
        scanner.attr[scanner.attr_size - 2] = '\t';
        scanner.attr_size -= 1;
      } else if (c == '\\') {
        scanner.attr[scanner.attr_size - 2] = '\\';
        scanner.attr_size -= 1;
      } else if (c == '"') {
        scanner.attr[scanner.attr_size - 2] = '\"';
        scanner.attr_size -= 1;
      } else {
        {
          state = FALSE;
          forget_char(c);
        }
      }
      break;

    case MULTILINE_STRING:
      if (c == '"')
        state = MULTILINE_STRING_END_1;

      else if ((c >= 0 && c <= 127) && c != '"') {
        state = MULTILINE_STRING;
        if (c == '\n') {
          scanner.line_number++;
        }
      }

      else {
        state = FALSE;
      }
      break;

    case MULTILINE_STRING_END_1:
      if (c == '"')
        state = MULTILINE_STRING_END_FINAL;

      else if (c >= 0 && c <= 127) {

        if (c == '\n') {
          scanner.line_number++;
        }

        state = MULTILINE_STRING;
      }

      else
        state = FALSE;
      break;

    case MULTILINE_STRING_END_FINAL:
      if (c == '"')
        create_token(T_STRING, &current_token);

      else if (c >= 0 && c <= 127) {

        if (c == '\n') {
          scanner.line_number++;
        }

        state = MULTILINE_STRING;
      } else
        state = FALSE;
      break;

      /* ==========================
       * Operators
       * ========================== */

    case LESS:
      if (c == '=')
        create_token(T_LESS_EQUAL, &current_token);

      else {
        forget_char(c);
        create_token(T_LESS, &current_token);
      }
      break;

    case GREATER:
      if (c == '=')
        create_token(T_GREATER_EQUAL, &current_token);

      else {
        forget_char(c);
        create_token(T_GREATER, &current_token);
      }
      break;

    case EQUAL:
      if (c == '=')
        create_token(T_EQUAL_EQUAL, &current_token);

      else {
        forget_char(c);
        create_token(T_EQUAL, &current_token);
      }
      break;

    case SLASH:
      if (c == '/')
        state = L_COMMENT;

      else if (c == '*') {
        scanner.has_newline = false;
        scanner.block_comment_depth = 1;
        state = ML_COMMENT;
      } else {
        forget_char(c);
        create_token(T_DIVIDE, &current_token);
      }
      break;

      /* ==========================
       * Strings
       * ========================== */

    case L_COMMENT:
      if (c == '\n') {
        forget_char(c);
        state = START;
      }

      else if (c == EOF) {
        create_token(T_EOF, &current_token);
      }
      break;

    case ML_COMMENT:
      if (c == EOF) {
        create_token(T_ERROR, &current_token);
      }

      else if (c == '/') {
        state = ML_COMMENT_SLASH;
      }

      else if (c == '*') {
        state = ML_COMMENT_STAR;
      }

      else if (c == '\n') {
        scanner.has_newline = true;
        scanner.line_number++;
      }
      break;

    case ML_COMMENT_SLASH:
      if (c == EOF) {
        create_token(T_ERROR, &current_token);
      }

      else if (c == '*') {
        scanner.block_comment_depth++;
        state = ML_COMMENT;
      } else {
        state = ML_COMMENT;
        forget_char(c);
      }
      break;

    case ML_COMMENT_STAR:
      if (c == EOF) {
        create_token(T_ERROR, &current_token);
      }

      else if (c == '/') {
        scanner.block_comment_depth--;
        if (scanner.block_comment_depth == 0) {
          if (scanner.has_newline) {
            if (scanner.paren_depth == 0)
              create_token(T_EOL, &current_token);
            scanner.after_block_comment_check_flag = true;
          } else {
            scanner.attr_size = 0;
            state = START;
          }
        } else {
          state = ML_COMMENT;
        }
      } else {
        state = ML_COMMENT;
        forget_char(c);
      }
      break;

    case ML_COMMENT_END_CHECK:
      if (c == EOF) {
        create_token(T_EOF, &current_token);
      }

      else if (c == '\n') {
        forget_char(c);
        state = START;
      }

      else if (isspace((unsigned char)c)) {
        state = ML_COMMENT_END_CHECK;
      }

      else {
        forget_char(c);
        state = FALSE;
      }
      break;

    //--------------------------//

    // line start(indent check)
    case L_START:
      if (c == '\n') {
        scanner.line_number++;
        scanner.count_indent_flag = true;
        scanner.attr_size = 0;
        state = START;
      }

      else if (c != ' ') {
        forget_char(c);
        if (!new_indent(scanner.attr_size, &current_token)) {
          create_token(T_ERROR, &current_token);
        }
        scanner.attr_size = 0;

        state = START;
      }

      break;

    // false state keeps reading until end of line or EOF
    case FALSE:
      if (c != EOF && c != '\n') {
        state = FALSE;
      }

      else {
        if (c == '\n') {
          scanner.line_number += 1;
        }
        create_token(T_ERROR, &current_token);
      }
      break;

    default:
      break;
    }
  }
  return current_token;
}

// Creates a token from scanner.attr and scanner.attr_size.
void create_token(TokenType new_type, Token *current_token) {
  scanner.attr[scanner.attr_size] = '\0';

  current_token->type = new_type;
  current_token->line = scanner.line_number;

  // Initialize the union.
  current_token->attr.string_value = NULL;

  switch (new_type) {
  case T_INT: {
    errno = 0;
    char *end = NULL;
    normalize_number(scanner.attr);
    long converted_int = strtol(scanner.attr, &end, 10);

    if (errno == ERANGE || end == scanner.attr || *end != '\0' ||
        converted_int < INT32_MIN || converted_int > INT32_MAX) {
      current_token->type = T_ERROR;
      break;
    }

    current_token->attr.int_value = (int32_t)converted_int;
    break;
  }

  case T_FLOAT: {
    errno = 0;
    char *end = NULL;
    normalize_number(scanner.attr);
    double converted_float = strtod(scanner.attr, &end);

    if (errno == ERANGE || end == scanner.attr || *end != '\0') {
      current_token->type = T_ERROR;
      break;
    }

    current_token->attr.double_value = converted_float;
    break;
  }

  case T_IDENTIFIER:
  case T_STRING: {
    current_token->attr.string_value = malloc(scanner.attr_size + 1);

    if (current_token->attr.string_value == NULL) {
      current_token->type = T_ERROR;
      break;
    }

    strcpy(current_token->attr.string_value, scanner.attr);
    break;
  }

  default:
    // Keywords, punctuation, EOF, INDENT, etc.
    // Don't need a dynamically allocated value.
    break;
  }

  scanner.is_running = 0;
}

void forget_char(char c) {
  if (c == EOF) {
    return;
  }

  ungetc(c, scanner.file);
  scanner.attr_size -= 1;
}

/*
int main()
{
    FILE* test_file = fopen("id_test","r");

    if (test_file == NULL){
        return -1;
    }
    scanner_init(test_file);

    Token i = get_next_token();
    while  (i.type != T_EOF){



        if (i.type == T_IDENTIFIER || i.type == T_STRING) {
            printf("type=%d, attr=[%s] line: %d\n",
           i.type, i.attr.string_value, i.line);
        }
        else if (i.type == T_INT) {
            printf("type=%d, attr=[%d] line: %d\n",
                i.type, i.attr.int_value, i.line);
        }
        else if (i.type == T_FLOAT) {
            printf("type=%d, attr=[%f] line: %d\n",
                i.type, i.attr.double_value, i.line);
        }
        else {
            printf("type=%d, line: %d\n", i.type, i.line);
        }

        i = get_next_token();
    }
    printf("type=%d\n", i.type);

    fclose(test_file);
    return 0;
}
*/