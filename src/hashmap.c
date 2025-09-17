#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

HashMap* hashmap_create(void)
{
  HashMap* map = malloc(sizeof(HashMap));
  map->capacity = 16;
  map->size = 0;
  map->entries = malloc(map->capacity * sizeof(Macro*));
  for (int i = 0; i < map->capacity; ++i) {
    map->entries[i] = NULL;
  }
  return map;
}

void hashmap_insert(HashMap* map, const char* key, Token* tokens, int numTokens)
{
  Macro* macro = malloc(sizeof(Macro));
  macro->key = strdup(key);
  macro->tokens = malloc(numTokens * sizeof(Token));
  memcpy(macro->tokens, tokens, numTokens * sizeof(Token));
  macro->numTokens = numTokens;

  size_t index = 0;
  for (size_t i = 0; i < strlen(key); ++i) {
    index += key[i];
  }
  index %= map->capacity;

  while (map->entries[index] != NULL) {
    index++;
    index %= map->capacity;
  }

  map->entries[index] = macro;
  map->size++;
}

Macro* hashmap_get(HashMap* map, const char* key)
{
  if (!map) {
    fprintf(stderr, "Error: Map is null.\n");
    return NULL;
  }

  size_t index = 0;
  for (size_t i = 0; i < strlen(key); ++i) {
    index += key[i];
  }
  index %= map->capacity;

  while (map->entries[index] != NULL) {
    if (strcmp(map->entries[index]->key, key) == 0) {
      return map->entries[index];
    }
    index++;
    index %= map->capacity;
  }
  return NULL;
}

void hashmap_free(HashMap* map)
{
  for (int i = 0; i < map->capacity; ++i) {
    if (map->entries[i] != NULL) {
      // free(map->entries[i]->key);
      free(map->entries[i]->tokens);
      free(map->entries[i]);
    }
  }
  free(map->entries);
  free(map);
}

void hashmap_print(HashMap* map)
{
  for (int i = 0; i < map->size; ++i) {
    if (map->entries[i] != NULL) {
      printf("Macro: %s\n", map->entries[i]->key);
      printf("Number of tokens: %d\n", map->entries[i]->numTokens);

      printf("Tokens: ");
      for (int j = 0; j < map->entries[i]->numTokens; ++j) {
        printf("%s ", map->entries[i]->tokens[j].lexeme);
      }
      printf("\n");
    }
  }
}

