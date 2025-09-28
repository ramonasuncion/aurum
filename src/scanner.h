#ifndef _scanner_h_
#define _scanner_h_

#include <stdbool.h>

#include "lexer.h"

/**
 * @brief Scanner structure.
 */
typedef struct {
  const char *source;
  const char *start;
  const char *current;
  int start_column;
  int column;
  int line;
  int position;
} Scanner;

/**
 * @brief Initialize a scanner.
 * @param scanner Pointer to the scanner structure.
 * @param source Source code string.
 */
void init_scanner(Scanner *scanner, const char *source);

/**
 * @brief Check if the scanner is at the end of the source code.
 * @param scanner Pointer to the scanner structure.
 * @return true if at the end, false otherwise.
 */
bool is_at_end(Scanner *scanner);

/**
 * @brief Advance the scanner to the next character.
 * @param scanner Pointer to the scanner structure.
 * @return The current character.
 */
char advance(Scanner *scanner);

/**
 * @brief Peek the current character without advancing the scanner.
 * @param scanner Pointer to the scanner structure.
 * @return The current character.
 */
char peek(Scanner *scanner);

/**
 * @brief Peek the next character without advancing the scanner.
 * @param scanner Pointer to the scanner structure.
 * @return The next character.
 */
char peek_next(Scanner *scanner);

/**
 * @brief Check if the current character matches the expected character and advance the scanner if it does.
 * @param scanner Pointer to the scanner structure.
 * @param expected The expected character.
 * @return true if the characters match, false otherwise.
 */
bool match(Scanner *scanner, char expected);

/**
 * @brief Create a token with the specified type.
 * @param type The token type.
 * @param scanner Pointer to the scanner structure.
 * @return The created token.
 */
Token create_token(TokenType type, Scanner *scanner);

/**
 * @brief Scan the next token from the source code.
 * @param scanner Pointer to the scanner structure.
 * @return The scanned token.
 */
Token scan_token(Scanner *scanner);

#endif // _scanner_h_

