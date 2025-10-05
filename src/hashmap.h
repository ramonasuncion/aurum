#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include "lexer.h"

struct macro {
  const char *key;
  struct token *tokens;
  int num_tokens;
};

struct hashmap {
  struct macro **entries;
  int capacity;
  int size;
};

/**
 * @brief Creates a new hashmap
 * @return A pointer to the new hashmap
 */
struct hashmap *hashmap_create(void);

/**
 * @brief Inserts a macro into the hashmap
 * @param map The hashmap to insert into
 * @param key The key of the macro
 * @param tokens The tokens of the macro
 * @param num_tokens The number of tokens in the macro
 */
void hashmap_insert(struct hashmap *map, const char *key,
    struct token *tokens, int num_tokens);

/**
 * @brief Gets a macro from the hashmap
 * @param map The hashmap to get from
 * @param key The key of the macro
 * @return A pointer to the macro
 */
struct macro *hashmap_get(struct hashmap *map, const char *key);

/**
 * @brief Frees the hashmap
 * @param map The hashmap to free
 */
void hashmap_free(struct hashmap *map);

/**
 * @brief Prints the hashmap
 * @param map The hashmap to print
 */
void hashmap_print(struct hashmap *map);

#endif /* _HASHMAP_H_ */

