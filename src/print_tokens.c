#include <stdio.h>

#include "lexer.h"
#include "print_tokens.h"

void print_token(Token token, Scanner *scanner, int token_id)
{
  if (token.type == TOKEN_UNKNOWN) return;

  printf("ID: %-3d Type: ", token_id);

  switch (token.type) {
    case TOKEN_ADD: printf("%-6s", "+"); break;
    case TOKEN_SUBTRACT: printf("%-6s", "-"); break;
    case TOKEN_MULTIPLY: printf("%-6s", "*"); break;
    case TOKEN_IDENTIFIER: printf("%-6s", "IDEN"); break;
    case TOKEN_NUMBER: printf("%-6s", "DIGIT"); break;
    case TOKEN_WHILE: printf("%-6s", "while"); break;
    case TOKEN_ELSE: printf("%-6s", "else"); break;
    case TOKEN_IF: printf("%-6s", "if"); break;
    case TOKEN_DUP: printf("%-6s", "dup"); break;
    case TOKEN_SWAP: printf("%-6s", "swap"); break;
    case TOKEN_OVER: printf("%-6s", "over"); break;
    case TOKEN_PEEK: printf("%-6s", "peek"); break;
    case TOKEN_MEMORY: printf("%-6s", "memory"); break;
    case TOKEN_WRITE: printf("%-6s", "write"); break;
    case TOKEN_READ: printf("%-6s", "read"); break;
    case TOKEN_STORE: printf("%-6s", "store"); break;
    case TOKEN_DROP: printf("%-6s", "drop"); break;
    case TOKEN_FETCH: printf("%-6s", "fetch"); break;
    case TOKEN_EQUAL: printf("%-6s", "="); break;
    case TOKEN_LESS: printf("%-6s", "<"); break;
    case TOKEN_LESS_EQUAL: printf("%-6s", "<="); break;
    case TOKEN_GREATER: printf("%-6s", ">"); break;
    case TOKEN_GREATER_EQUAL: printf("%-6s", ">="); break;
    case TOKEN_BITWISE_XOR: printf("%-6s", "^"); break;
    case TOKEN_BITWISE_AND: printf("%-6s", "&"); break;
    case TOKEN_BITWISE_OR: printf("%-6s", "|"); break;
    case TOKEN_BITWISE_NOT: printf("%-6s", "~"); break;
    case TOKEN_LEFT_SHIFT: printf("%-6s", "<<"); break;
    case TOKEN_RIGHT_SHIFT: printf("%-6s", ">>"); break;
    case TOKEN_ESCAPE_SEQUENCE: printf("%-6s", "ESC"); break;
    case TOKEN_NEW_LINE: printf("%-6s", "NL"); break;
    case TOKEN_CARRIAGE_RETURN: printf("%-6s", "CR"); break;
    case TOKEN_ROT: printf("%-6s", "rot"); break;
    case TOKEN_EOF: printf("%-6s", "EOF"); break;
    case TOKEN_END: printf("%-6s", "end"); break;
    case TOKEN_THEN: printf("%-6s", "then"); break;
    case TOKEN_QUESTION: printf("%-6s", "?"); break;
    case TOKEN_UNKNOWN: printf("%-6s", "UNKNOWN"); break;
    case TOKEN_SYSCALL: printf("%-6s", "systemcall"); break;
    case TOKEN_DUMP: printf("%-6s", "dump"); break;
    case TOKEN_TWO_DUP: printf("%-6s", "2dup"); break;
    case TOKEN_DO: printf("%-6s", "do"); break;
    case TOKEN_TWO_SWAP: printf("%-6s", "2swap"); break;
    case TOKEN_TWO_OVER: printf("%-6s", "2over"); break;
    case TOKEN_TWO_DROP: printf("%-6s", "2drop"); break;
    case TOKEN_STRING_LITERAL: printf("%-6s", "STR"); break;
    case TOKEN_DEFINE: printf("%-6s", "define"); break;
    case TOKEN_INCLUDE: printf("%-6s", "include"); break;
    case TOKEN_CHAR: printf("%-6s", "CHAR"); break;
    default: printf("%-6s", "UNKNOWN"); break;
  }

  printf("Line: %d[%-2d] Symbol: ", scanner->line, scanner->start_column);
  if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_NUMBER) {
    printf("%.*s", (int)(scanner->current - scanner->start), scanner->start);
  }
  printf("\n");
}

