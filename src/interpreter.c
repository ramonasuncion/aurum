#include "lexer.h"
#include "scanner.h"
#include "stack.h"
#include "interpreter.h"
#include "memory.h"
#include "hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>

static Token token;
static Scanner scanner;

static Stack *stack;
static Stack *loop_stack;
static Stack *end_stack;

static char memory[MEMORY_CAPACITY];

HashMap* hashmap;

typedef void (*action_func_t)(void);
action_func_t* actions;

void print_result(Stack *stack)
{
  if (stack->size > 0) {
    int result_str = pop(stack);
    printf("%d\n", result_str);
  } else {
    fprintf(stderr, "Empty stack.\n");
  }
}

void action_number(void)
{
  if (token.type == TOKEN_CHAR) {
    push(stack, token.lexeme[0]);
    return;
  }
  int value = atoi(token.lexeme);
  push(stack, value);
}

void action_add(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a + b);
}

void action_subtract(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a - b);
}

void action_multiply(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a * b);
}

void action_print(void)
{
  print_result(stack);
}

void action_while(void)
{
  if (top(loop_stack) != scanner.position)
    push(loop_stack, scanner.position);
}

void action_do(void)
{
  int condition = pop(stack);
  if (condition == 0) {
    pop(loop_stack);
    scanner.position = pop(end_stack);
    int keyword_length = strlen("do") + 1;
    scanner.current = scanner.source + scanner.position + keyword_length;
    pop(stack);
  }
}

void action_end(void)
{
  int loop_start = pop(loop_stack);
  if (top(stack) != scanner.position)
    push(end_stack, scanner.position);
  scanner.position = loop_start;
  scanner.current = scanner.source + scanner.position;
}

void action_dup(void)
{
  int a = pop(stack);
  push(stack, a);
  push(stack, a);
}

void action_two_dup(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a);
  push(stack, b);
  push(stack, a);
  push(stack, b);
}

void action_drop(void)
{
  pop(stack);
}

void action_two_drop(void)
{
  pop(stack);
  pop(stack);
}

void action_equal(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a == b);
}

void action_swap(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, b);
  push(stack, a);
}

void action_two_swap(void)
{
  int d = pop(stack);
  int c = pop(stack);
  int b = pop(stack);
  int a = pop(stack);
  push(stack, c);
  push(stack, d);
  push(stack, a);
  push(stack, b);
}

void action_over(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a);
  push(stack, b);
  push(stack, a);
}

void action_two_over(void)
{
  int c = pop(stack);
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a);
  push(stack, b);
  push(stack, c);
  push(stack, a);
  push(stack, b);
  push(stack, c);
}

void action_rot(void)
{
  int c = pop(stack);
  int b = pop(stack);
  int a = pop(stack);
  push(stack, b);
  push(stack, c);
  push(stack, a);
}

void action_peek(void)
{
  int a = pop(stack);
  push(stack, a);
}

void action_if(void) {}

void action_then(void)
{
  int condition = pop(stack);
  if (condition == 0) {
    int block_depth = 1;
    while (block_depth > 0) {
      token = scan_token(&scanner);
      if (token.type == TOKEN_IF) {
        block_depth++;
      } else if (token.type == TOKEN_ELSE && block_depth == 1) {
        break;
      } else if (token.type == TOKEN_END) {
        block_depth--;
      } else {
        continue;
      }
    }
  }
}

void action_else(void)
{
  int block_depth = 1;
  while (block_depth > 0) {
    token = scan_token(&scanner);
    if (token.type == TOKEN_IF) {
      block_depth++;
    } else if (token.type == TOKEN_END) {
      block_depth--;
    }
  }
}

void action_store(void)
{
  char byte = pop(stack);
  intptr_t addr = pop(stack);
  memory[addr] = byte & 0xFF;
}

void action_fetch(void)
{
  intptr_t addr = pop(stack);
  char byte = memory[addr];
  push(stack, byte);
}

void action_memory(void)
{
  push(stack, (intptr_t)memory);
}

void action_lt(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a < b);
}

void action_gt(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a > b);
}

void action_lte(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a <= b);
}

void action_gte(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a >= b);
}

void action_right_shift(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a >> b);
}

void action_left_shift(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a << b);
}

void action_bitwise_and(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a & b);
}

void action_bitwise_or(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a | b);
}

void action_bitwise_not(void)
{
  int a = pop(stack);
  push(stack, ~a);
}

void action_bitwise_xor(void)
{
  int b = pop(stack);
  int a = pop(stack);
  push(stack, a ^ b);
}

void action_syscall(void)
{
  int argument_count = pop(stack);

  if (argument_count < 1 || argument_count > 3) {
    fprintf(stderr, "Invalid number of arguments for syscall.\n");
    exit(1);
  }

  int syscall_number = pop(stack);
  int arg1 = pop(stack);
  int arg2 = (argument_count >= 2) ? pop(stack) : 0;
  int arg3 = (argument_count >= 3) ? pop(stack) : 0;

  switch (syscall_number) {
    case SYS_READ: {
      int fd = arg1;
      int buf = arg2;
      int count = arg3;
      char* data = malloc(count * sizeof(char));
      int bytes_read = read(fd, data, count);
      memcpy(memory + buf, data, bytes_read);
      free(data);
      push(stack, bytes_read);
      break;
    }
    case SYS_WRITE: {
      int fd = arg1;
      int buf = arg2;
      int count = arg3;
      int bytes_written = write(fd, memory + buf, count);
      push(stack, bytes_written);
      break;
    }
    case SYS_EXIT: {
      int status = arg1;
      exit(status);
    }
  }
}

void action_dump(void)
{
  dump(stack);
}

void action_string_literal(void)
{
  int memory_index = 0;
  int string_length = strlen(token.lexeme);
  char* string = malloc((string_length + 1) * sizeof(char));
  strcpy(string, token.lexeme);

  // Remove the quotes from the string literal
  char* literal = string + 1;
  literal[string_length - 2] = '\0';

  // Store the characters in memory
  int literal_length = strlen(literal);
  for (int i = 0; i < literal_length; ++i) {
    if (literal[i] == '\\') {
      // Handle escape sequences
      switch (literal[i + 1]) {
        case 'n': // Newline character
          memory[memory_index++] = '\n';
          i++; // Skip the next character ('\n')
          break;
        case 't': // Tab character
          memory[memory_index++] = '\t';
          i++; // Skip the next character ('\t')
          break;
        default: // Treat other characters as regular characters
          memory[memory_index++] = literal[i];
          break;
      }
    } else {
      memory[memory_index++] = literal[i];
    }
  }
  memory[memory_index] = '\0';

  push(stack, literal_length);
  push(stack, memory_index - literal_length);

  free(string);
}

void action_define(void)
{
  // Grab the name of the macro.
  Token macro_name = scan_token(&scanner); // Save the first token as macro_name
  Token* macros = malloc(sizeof(Token) * 100);
  int i = 0;
  Token token = scan_token(&scanner); // Start scanning tokens after the macro_name
  int num_tokens = 0;
  while (token.type != TOKEN_END) {
    macros[i++] = token;
    token = scan_token(&scanner);
    num_tokens++;
  }
  macros[i] = token;

  // Insert the macro into the hashmap
  hashmap_insert(hashmap, macro_name.lexeme, macros, num_tokens);

  free(macros);
}

void action_macro(void)
{
  const char* macro_name = token.lexeme;

  // Error handling to check if hashmap exists and macro is found.
  if (hashmap == NULL || hashmap->size == 0) {
    printf("Hashmap is null.\n");
    exit(0);
  }

  Macro* macro = hashmap_get(hashmap, macro_name);
  if (macro == NULL) {
    fprintf(stderr, "Error: Macro '%s' not found.\n", macro_name);
    return;
  }

  if (macro->tokens == NULL) {
    fprintf(stderr, "Error: Tokens for macro '%s' are NULL.\n", macro_name);
    return;
  }

  // Push the tokens onto the stack
  for (int i = 0; i < macro->numTokens; ++i) {
    if (i < 0 || i >= macro->numTokens) {
      fprintf(stderr, "Error: Index %d out of bounds for macro '%s'.\n", i, macro_name);
      return;
    }
    if (isdigit(macro->tokens[i].lexeme[0])) {
      push(stack, atoi(macro->tokens[i].lexeme));
    } else {
      action_func_t action = actions[macro->tokens[i].type];
      if (action != NULL) {
        action();
      } else {
        fprintf(stderr, "[%d:%d] ERROR: Unknown token type: %s\n",
                scanner.line, scanner.column, macro->tokens[i].lexeme);
        exit(1);
      }
    }
  }
}

void action_include(void)
{
  // Get the filename from the current token
  const char* filename = scan_token(&scanner).lexeme;

  // Remove the quotes from the filename
  size_t filename_length = strlen(filename);
  char* cleaned_filename = malloc(filename_length - 1);
  strncpy(cleaned_filename, filename + 1, filename_length - 2);
  cleaned_filename[filename_length - 2] = '\0';

  // Open the file
  FILE* file = fopen(cleaned_filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Could not open file: %s\n", cleaned_filename);
    exit(1);
  }

  // Get the size of the file
  fseek(file, 0L, SEEK_END);
  int file_size = ftell(file);
  rewind(file);

  // Read the file into a buffer
  char* buffer = malloc(file_size + 1);
  int fread_code = fread(buffer, sizeof(char), file_size, file);

  if (fread_code != file_size) {
    fprintf(stderr, "Could not read file into buffer: %s\n", cleaned_filename);
    exit(1);
  }

  buffer[file_size] = '\0';

  // Close the file
  fclose(file);

  // Remove line breaks and add a space at the end of each line
  for (int i = 0; i < file_size; ++i) {
    if (buffer[i] == '\n') {
      buffer[i] = ' ';
    }
  }
  buffer[file_size] = ' '; // Add a space at the end
  memmove(buffer + 1, buffer, file_size); // Shift the buffer to make space for the space at the beginning
  buffer[0] = ' '; // Add a space at the beginning

  // Calculate the position for the buffer
  int position = scanner.position + strlen(cleaned_filename) + 2;

  // Create a new source string with the buffer inserted after the current token
  char* new_source = malloc(strlen(scanner.source) + strlen(buffer) + 1);
  strncpy(new_source, scanner.source, position);
  strcat(new_source, buffer);
  strcat(new_source, scanner.source + position);

  // Update the scanner with the new source and position
  scanner.source = new_source;
  scanner.current = scanner.source + position;

  // Free the buffer
  free(buffer);

  // Free the cleaned filename
  free(cleaned_filename);
}

void free_resources(void)
{
  while (stack->size > 0) {
    pop(stack);
  }

  free(loop_stack);
  free(end_stack);
  free(stack);
  hashmap_free(hashmap);
}

void run_interpreter(const char *source_code)
{

  //InterpreterState state;
  //init_scanner(&state.scanner, source_code);
  //state.actions = (action_func_t[]) {
  //    // Initialization of action functions
  //};
  //state.hashmap = hashmap_create();

  //Stack *stack = create_stack();
  //Stack *loop_stack = create_stack();
  //Stack *end_stack = create_stack();

  //if (stack == NULL) {
  stack = create_stack();
  loop_stack = create_stack();
  end_stack = create_stack();
  hashmap = hashmap_create();

  // atexit(free_resources);
  //}

  init_scanner(&scanner, source_code);

  actions = (action_func_t[]) {
    [TOKEN_NUMBER] = action_number,
    [TOKEN_ADD] = action_add,
    [TOKEN_SUBTRACT] = action_subtract,
    [TOKEN_MULTIPLY] = action_multiply,
    [TOKEN_PERIOD] = action_print,
    [TOKEN_WHILE] = action_while,
    [TOKEN_THEN] = action_then,
    [TOKEN_DO] = action_do,
    [TOKEN_END] = action_end,
    [TOKEN_GREATER] = action_gt,
    [TOKEN_LESS] = action_lt,
    [TOKEN_GREATER_EQUAL] = action_gte,
    [TOKEN_LESS_EQUAL] = action_lte,
    [TOKEN_DUP] = action_dup,
    [TOKEN_TWO_DUP] = action_two_dup,
    [TOKEN_DROP] = action_drop,
    [TOKEN_TWO_DROP] = action_two_drop,
    [TOKEN_EQUAL] = action_equal,
    [TOKEN_SWAP] = action_swap,
    [TOKEN_TWO_SWAP] = action_two_swap,
    [TOKEN_OVER] = action_over,
    [TOKEN_TWO_OVER] = action_two_over,
    [TOKEN_ROT] = action_rot,
    [TOKEN_PEEK] = action_peek,
    [TOKEN_IF] = action_if,
    [TOKEN_ELSE] = action_else,
    [TOKEN_STORE] = action_store,
    [TOKEN_FETCH] = action_fetch,
    [TOKEN_MEMORY] = action_memory,
    [TOKEN_RIGHT_SHIFT] = action_right_shift,
    [TOKEN_LEFT_SHIFT] = action_left_shift,
    [TOKEN_BITWISE_AND] = action_bitwise_and,
    [TOKEN_BITWISE_OR] = action_bitwise_or,
    [TOKEN_BITWISE_NOT] = action_bitwise_not,
    [TOKEN_BITWISE_XOR] = action_bitwise_xor,
    [TOKEN_SYSCALL] = action_syscall,
    [TOKEN_DUMP] = action_dump,
    [TOKEN_STRING_LITERAL] = action_string_literal,
    [TOKEN_CHAR] = action_number,
    [TOKEN_DEFINE] = action_define,
    [TOKEN_MACRO] = action_macro,
    [TOKEN_INCLUDE] = action_include,
  };

  // while ((state.token = scan_token(&state.scanner)).type != TOKEN_EOF) {
  while ((token = scan_token(&scanner)).type != TOKEN_EOF) {
    // action_func_t action = state.actions[state.token.type];
    action_func_t action = actions[token.type];
    if (action != NULL) {
      action();
    } else {
      fprintf(stderr, "[%d:%d] ERROR: Unknown token type: %s\n",
              scanner.line, scanner.column, token.lexeme);
      exit(1);
    }
  }

  while (stack->size > 0) {
    pop(stack);
  }

  free(loop_stack);
  free(end_stack);
  free(stack);
  hashmap_free(hashmap);
}

