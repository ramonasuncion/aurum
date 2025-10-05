#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <stdbool.h>

#include "lexer.h"

struct scanner {
  const char *source;
  const char *start;
  const char *current;
  int start_column;
  int column;
  int line;
  int position;
};

/**
 * @brief Initialize a scanner.
 * @param scanner Pointer to the scanner structure.
 * @param source Source code string.
 */
void init_scanner(struct scanner *scanner, const char *source);

/**
 * @brief Check if the scanner is at the end of the source code.
 * @param scanner Pointer to the scanner structure.
 * @return true if at the end, false otherwise.
 */
bool is_at_end(struct scanner *scanner);

/**
 * @brief Advance the scanner to the next character.
 * @param scanner Pointer to the scanner structure.
 * @return The current character.
 */
char advance(struct scanner *scanner);

/**
 * @brief Peek the current character without advancing the scanner.
 * @param scanner Pointer to the scanner structure.
 * @return The current character.
 */
char peek(struct scanner *scanner);

/**
 * @brief Peek the next character without advancing the scanner.
 * @param scanner Pointer to the scanner structure.
 * @return The next character.
 */
char peek_next(struct scanner *scanner);

/**
 * @brief Check if the current character matches the expected character and advance the scanner if it does.
 * @param scanner Pointer to the scanner structure.
 * @param expected The expected character.
 * @return true if the characters match, false otherwise.
 */
bool match(struct scanner *scanner, char expected);

/**
 * @brief Create a token with the specified type.
 * @param type The token type.
 * @param scanner Pointer to the scanner structure.
 * @return The created token.
 */
struct token create_token(enum token_type type, struct scanner *scanner);

/**
 * @brief Scan the next token from the source code.
 * @param scanner Pointer to the scanner structure.
 * @return The scanned token.
 */
struct token scan_token(struct scanner *scanner);

#endif /* _SCANNER_H_ */

