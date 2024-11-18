#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"
#include "hashmap.h"
#include "scanner.h"

#define ALPHABET_SIZE (128)

// HashMap *hashmap = NULL;
extern HashMap *hashmap;

TrieNode *trie_node_create(TokenType type)
{
  TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
  int i;
  node->type = type;
  for (i = 0; i < ALPHABET_SIZE; ++i) {
    node->children[i] = NULL;
  }
  return node;
}

void trie_insert(TrieNode *root, const char *keyword, TokenType type)
{
  TrieNode *node = root;
  while (*keyword) {
    if (node->children[(unsigned char)*keyword] == NULL) {
      node->children[(unsigned char)*keyword] = trie_node_create(TOKEN_IDENTIFIER);
    }
    node = node->children[(unsigned char)*keyword];
    keyword++;
  }
  node->type = type;
}

TrieNode *init_keyword_trie(void)
{
  TrieNode *root = trie_node_create(TOKEN_IDENTIFIER);
  trie_insert(root, "while", TOKEN_WHILE);
  trie_insert(root, "do", TOKEN_DO);
  trie_insert(root, "if", TOKEN_IF);
  trie_insert(root, "then", TOKEN_THEN);
  trie_insert(root, "end", TOKEN_END);
  trie_insert(root, "else", TOKEN_ELSE);
  trie_insert(root, "dup", TOKEN_DUP);
  trie_insert(root, "2dup", TOKEN_TWO_DUP);
  trie_insert(root, "swap", TOKEN_SWAP);
  trie_insert(root, "2swap", TOKEN_TWO_SWAP);
  trie_insert(root, "over", TOKEN_OVER);
  trie_insert(root, "2over", TOKEN_TWO_OVER);
  trie_insert(root, "peek", TOKEN_PEEK);
  trie_insert(root, "memory", TOKEN_MEMORY);
  trie_insert(root, "write", TOKEN_WRITE);
  trie_insert(root, "read", TOKEN_READ);
  trie_insert(root, "store", TOKEN_STORE);
  trie_insert(root, "fetch", TOKEN_FETCH);
  trie_insert(root, "rot", TOKEN_ROT);
  trie_insert(root, "drop", TOKEN_DROP);
  trie_insert(root, "2drop", TOKEN_TWO_DROP);
  trie_insert(root, "systemcall", TOKEN_SYSCALL);
  trie_insert(root, "dump", TOKEN_DUMP);
  trie_insert(root, "define", TOKEN_DEFINE);
  trie_insert(root, "include", TOKEN_INCLUDE);
  return root;
}

TokenType trie_search(TrieNode *root, Scanner *scanner)
{
  TrieNode *node = root;
  const char *source = scanner->start;

  while (node && *source) {
    if (!isalnum(*source)) {
      break;
    }
    node = node->children[(unsigned char)*source];
    source++;
  }

  if ((!node || !isalnum(*source)) && node && node->type != TOKEN_IDENTIFIER) {
    return node->type;
  }

  return TOKEN_IDENTIFIER;
}

TokenType check_keyword(Scanner *scanner)
{
  static TrieNode *keyword_trie = NULL;
  if (keyword_trie == NULL) {
    keyword_trie = init_keyword_trie();
  }
  return trie_search(keyword_trie, scanner);
}

void init_scanner(Scanner *scanner, const char *source)
{
  scanner->source = source;
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;
  scanner->column = 1;
  scanner->start_column = 1;
}

bool is_at_end(Scanner *scanner)
{
  return *scanner->current == '\0';
}

char advance(Scanner *scanner)
{
  scanner->column++;
  if (*scanner->current == '\n') {
    scanner->line++;
    scanner->column = 1;
  }
  return *scanner->current++;
}

char* get_token(Scanner *scanner)
{
  size_t length = (size_t)(scanner->current - scanner->start);
  char *lexeme = (char *)malloc(length + 1);
  memcpy(lexeme, scanner->start, length);
  lexeme[length] = '\0';
  return lexeme;
}

char peek(Scanner *scanner)
{
  return *scanner->current;
}

char peek_next(Scanner *scanner)
{
  if (is_at_end(scanner)) return '\0';
  return scanner->current[1];
}

void skip_whitespace(Scanner *scanner)
{
  while (isspace(peek(scanner))) {
    advance(scanner);
  }
}

bool is_whitespace(Scanner *scanner)
{
  return isspace(peek(scanner));
}

bool match(Scanner *scanner, char expected)
{
  if (is_at_end(scanner)) return false;
  if (*scanner->current != expected) return false;

  scanner->current++;
  scanner->column++;

  return true;
}

Token create_token(TokenType type, Scanner *scanner)
{
  Token token;
  token.type = type;
  token.line = scanner->line;

  size_t length = (size_t)(scanner->current - scanner->start);
  char *lexeme = (char *)malloc(length + 1);
  memcpy(lexeme, scanner->start, length);
  lexeme[length] = '\0';

  token.lexeme = lexeme;

  return token;
}

void update_position(Scanner *scanner)
{
  scanner->position = scanner->current - scanner->source;
}

Token scan_token(Scanner *scanner)
{
  skip_whitespace(scanner);
  scanner->start = scanner->current;
  scanner->start_column = scanner->column;
  update_position(scanner);
  if (is_at_end(scanner)) return create_token(TOKEN_EOF, scanner);

  char c = advance(scanner);
  switch (c) {
    case '*': return create_token(TOKEN_MULTIPLY, scanner);
    case '~': return create_token(TOKEN_BITWISE_NOT, scanner);
    case '^': return create_token(TOKEN_BITWISE_XOR, scanner);
    case '\\': return create_token(TOKEN_ESCAPE_SEQUENCE, scanner);
    case '\n': return create_token(TOKEN_NEW_LINE, scanner);
    case '\r': return create_token(TOKEN_CARRIAGE_RETURN, scanner);
    case '=': return create_token(TOKEN_EQUAL, scanner);
    case '+': return create_token(TOKEN_ADD, scanner);
    case '-': return create_token(TOKEN_SUBTRACT, scanner);
    case '&': return create_token(TOKEN_BITWISE_AND, scanner);
    case '|':  return create_token(TOKEN_BITWISE_OR, scanner);
    case '.': return create_token(TOKEN_PERIOD, scanner);
    case '"':
      while (peek(scanner) != '"' && !is_at_end(scanner)) {
        if (peek(scanner) == '\n') {
          scanner->line++;
        }
        advance(scanner);
      }
      if (is_at_end(scanner)) {
        return create_token(TOKEN_UNKNOWN, scanner);
      }
      advance(scanner);
      return create_token(TOKEN_STRING_LITERAL, scanner);
    case '\'':
      while (peek(scanner) != '\'' && !is_at_end(scanner)) {
        if (peek(scanner) == '\n') {
          scanner->line++;
        }
        advance(scanner);
      }
      if (is_at_end(scanner)) {
        return create_token(TOKEN_UNKNOWN, scanner);
      }
      advance(scanner);
      return create_token(TOKEN_CHAR, scanner);
    case '>':
      if (match(scanner, '=')) {
        return create_token(TOKEN_GREATER_EQUAL, scanner);
      } else if (match(scanner, '>')) {
        return create_token(TOKEN_RIGHT_SHIFT, scanner);
      } else {
        return create_token(TOKEN_GREATER, scanner);
      }
    case '<':
      if (match(scanner, '=')) {
        return create_token(TOKEN_LESS_EQUAL, scanner);
      } else if (match(scanner, '<')) {
        return create_token(TOKEN_LEFT_SHIFT, scanner);
      } else {
        return create_token(TOKEN_LESS, scanner);
      }
    case '/':
      if (match(scanner, '/')) {
        // Single-line comment
        while (peek(scanner) != '\n' && !is_at_end(scanner)) {
          advance(scanner);
        }
        if (peek(scanner) == '\n') {
          advance(scanner); // Consume '\n'
        }
        // Recursively get the next token after the comment
        return scan_token(scanner);
      } else if (match(scanner, '*')) {
        // Multi-line comment
        while (!(peek(scanner) == '*' && peek_next(scanner) == '/') && !is_at_end(scanner)) {
          if (peek(scanner) == '\n') {
            scanner->line++; // Increment line count
          }
          advance(scanner);
        }
        if (!is_at_end(scanner)) {
          advance(scanner); // Consume '*'
          advance(scanner); // Consume '/'
        }
        // Recursively get the next token after the comment
        return scan_token(scanner);
      } else {
        return create_token(TOKEN_UNKNOWN, scanner);
      }
    default:
      if (isdigit(c)) {
        while (isdigit(peek(scanner))) advance(scanner);
        if (isalpha(peek(scanner))) {
          while (isalnum(peek(scanner)) || peek(scanner) == '_') advance(scanner);
          TokenType type = check_keyword(scanner);
          return create_token(type, scanner);
        } else {
          return create_token(TOKEN_NUMBER, scanner);
        }
      } else if (isalpha(c) || c == '_') {
        while (isalnum(peek(scanner)) || peek(scanner) == '_') advance(scanner);
        TokenType type = check_keyword(scanner);
        if (type == TOKEN_IDENTIFIER) {
          if (hashmap != NULL) {
            Macro *macro = hashmap_get(hashmap, get_token(scanner));
            if (macro != NULL) {
              return create_token(TOKEN_MACRO, scanner);
            }
          }
          return create_token(type, scanner);
        }
        return create_token(type, scanner);
      } else {
        return create_token(TOKEN_UNKNOWN, scanner);
      }
  }
}

