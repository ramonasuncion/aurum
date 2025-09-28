#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include "lexer.h"

/**
 * @brief Handling macros
*/
typedef struct {
  const char* key;
  Token* tokens;
  int numTokens;
} Macro;


typedef struct {
  Macro** entries;
  int capacity;
  int size;
} HashMap;

/* Hash map */
// extern HashMap* hashmap;

/**
 * @brief Creates a new hashmap
 * @return A pointer to the new hashmap
*/
HashMap* hashmap_create(void);

/**
 * @brief Inserts a macro into the hashmap
 * @param map The hashmap to insert into
 * @param key The key of the macro
 * @param tokens The tokens of the macro
 * @param numTokens The number of tokens in the macro
*/
void hashmap_insert(HashMap* map, const char* key, Token* tokens, int numTokens);

/**
 * @brief Gets a macro from the hashmap
 * @param map The hashmap to get from
 * @param key The key of the macro
 * @return A pointer to the macro
*/
Macro* hashmap_get(HashMap* map, const char* key);

/**
 * @brief Frees the hashmap
 * @param map The hashmap to free
*/
void hashmap_free(HashMap* map);

/**
 * @brief Prints the hashmap
 * @param map The hashmap to print
*/
void hashmap_print(HashMap* map);

#endif /* _HASHMAP_H_ */

