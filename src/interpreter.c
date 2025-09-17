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

void print_result(Stack *stack)
{
  if (stack->size > 0)
    printf("%d\n", pop(stack));
  else
    fprintf(stderr, "Empty stack.\n");
}

void action_print(void)
{
  print_result(stack);
}

void action_dump(void)
{
  dump(stack);
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

void action_number(void)
{
  int value = (token.type == TOKEN_CHAR) ?
    token.lexeme[0] : atoi(token.lexeme);
  push(stack, value);
}

void action_arithmetic(void)
{
  int b = pop(stack);
  int a = pop(stack);
  int result = 0;

  switch (token.type) {
    case TOKEN_ADD: result = a + b; break;
    case TOKEN_SUBTRACT: result = a - b; break;
    case TOKEN_MULTIPLY: result = a * b; break;
    default: return;
  }

  push(stack, result);
}

void action_comparison(void)
{
  int b = pop(stack);
  int a = pop(stack);
  int result = 0;

  switch (token.type) {
    case TOKEN_EQUAL: result = (a == b); break;
    case TOKEN_GREATER: result = (a > b); break;
    case TOKEN_LESS: result = (a < b); break;
    case TOKEN_GREATER_EQUAL: result = (a >= b); break;
    case TOKEN_LESS_EQUAL: result = (a <= b); break;
    default: return;
  }

  push(stack, result);
}

void action_bitwise(void)
{
  int b = pop(stack);
  int a = pop(stack);
  int result = 0;

  switch (token.type) {
    case TOKEN_BITWISE_AND: result = a & b; break;
    case TOKEN_BITWISE_OR: result = a | b; break;
    case TOKEN_BITWISE_XOR: result = a ^ b; break;
    case TOKEN_BITWISE_NOT: result = ~a; push(stack, result); return;
    default: return;
  }

  push(stack, result);
}

void action_memory(void)
{
  push(stack, (intptr_t)memory);
}

void action_syscall(void)
{
  int args[3];
  int argument_count = pop(stack);

  if (argument_count < 1 || argument_count > 3) {
    fprintf(stderr, "Invalid number of arguments for syscall.\n");
    exit(1);
  }

  for (int i = 0; i < argument_count; ++i) {
    args[i] = pop(stack);
  }

  int syscall_number = pop(stack);
  switch (syscall_number) {
    case SYS_READ: {
      int fd = args[0], buf = args[1], count = args[2];
      char* data = malloc(count * sizeof(char));
      int bytes_read = read(fd, data, count);
      memcpy(memory + buf, data, bytes_read);
      free(data);
      push(stack, bytes_read);
      break;
    }
    case SYS_WRITE: {
      int fd = args[0], buf = args[1], count = args[2];
      int bytes_written = write(fd, memory + buf, count);
      push(stack, bytes_written);
      break;
    }
    case SYS_EXIT: exit(args[0]);
  }
}

void action_include(void)
{
  const char* filename = scan_token(&scanner).lexeme;
  size_t filename_length = strlen(filename);
  char* cleaned_filename = malloc(filename_length - 1);
  strncpy(cleaned_filename, filename + 1, filename_length - 2);
  cleaned_filename[filename_length - 2] = '\0';

  FILE* file = fopen(cleaned_filename, "r");
  if (!file) {
    fprintf(stderr, "Could not open file: %s\n", cleaned_filename);
    exit(1);
  }

  fseek(file, 0L, SEEK_END);
  int file_size = ftell(file);
  rewind(file);

  char* buffer = malloc(file_size + 1);
  fread(buffer, sizeof(char), file_size, file);
  buffer[file_size] = '\0';
  fclose(file);

  for (int i = 0; i < file_size; ++i) {
    if (buffer[i] == '\n') buffer[i] = ' ';
  }

  int position = scanner.position + strlen(cleaned_filename) + 2;
  char* new_source = malloc(strlen(scanner.source) + strlen(buffer) + 1);
  strncpy(new_source, scanner.source, position);
  strcat(new_source, buffer);
  strcat(new_source, scanner.source + position);

  scanner.source = new_source;
  scanner.current = new_source + position;

  free(buffer);
  free(cleaned_filename);
}

void action_define(void)
{
  Token macro_name = scan_token(&scanner);
  Token* macros = malloc(sizeof(Token) * 100);
  int i = 0;

  while ((token = scan_token(&scanner)).type != TOKEN_END) {
    macros[i++] = token;
  }

  hashmap_insert(hashmap, macro_name.lexeme, macros, i);
}

void action_macro(void)
{
  const char* macro_name = token.lexeme;
  Macro* macro = hashmap_get(hashmap, macro_name);

  if (!macro) {
    fprintf(stderr, "Error: Macro '%s' not found.\n", macro_name);
    return;
  }

  for (int i = 0; i < macro->numTokens; ++i) {
    if (isdigit(macro->tokens[i].lexeme[0])) {
      push(stack, atoi(macro->tokens[i].lexeme));
    } else {
      action_func_t action = actions[macro->tokens[i].type];
      if (action) action();
    }
  }
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

void action_string_literal(void)
{
  int memory_index = 0;
  int string_length = strlen(token.lexeme);
  char* string = malloc((string_length + 1) * sizeof(char));
  strcpy(string, token.lexeme);

  char* literal = string + 1;
  literal[string_length - 2] = '\0';

  int literal_length = strlen(literal);
  for (int i = 0; i < literal_length; ++i) {
    if (literal[i] == '\\') {
      switch (literal[i + 1]) {
        case 'n':
          memory[memory_index++] = '\n';
          i++;
          break;
        case 't':
          memory[memory_index++] = '\t';
          i++;
          break;
        default:
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


void free_resources(void)
{
  while (stack->size > 0) pop(stack);
  free(stack);
  free(loop_stack);
  free(end_stack);
  hashmap_free(hashmap);
}

void run_interpreter(const char *source_code) {
  stack = create_stack();
  loop_stack = create_stack();
  end_stack = create_stack();
  hashmap = hashmap_create();

  init_scanner(&scanner, source_code);

  actions = (action_func_t[]) {
    [TOKEN_NUMBER] = action_number,
    [TOKEN_ADD] = action_arithmetic,
    [TOKEN_SUBTRACT] = action_arithmetic,
    [TOKEN_MULTIPLY] = action_arithmetic,
    [TOKEN_QUESTION] = action_print,
    [TOKEN_WHILE] = action_while,
    [TOKEN_DO] = action_do,
    [TOKEN_DUMP] = action_dump,
    [TOKEN_END] = action_end,
    [TOKEN_GREATER] = action_comparison,
    [TOKEN_GREATER_EQUAL] = action_comparison,
    [TOKEN_LESS] = action_comparison,
    [TOKEN_EQUAL] = action_comparison,
    [TOKEN_LESS_EQUAL] = action_comparison,
    [TOKEN_BITWISE_AND] = action_bitwise,
    [TOKEN_BITWISE_OR] = action_bitwise,
    [TOKEN_BITWISE_XOR] = action_bitwise,
    [TOKEN_BITWISE_NOT] = action_bitwise,
    [TOKEN_STRING_LITERAL] = action_string_literal,
    [TOKEN_SYSCALL] = action_syscall,
    [TOKEN_DEFINE] = action_define,
    [TOKEN_MACRO] = action_macro,
    [TOKEN_INCLUDE] = action_include,
    [TOKEN_DUP] = action_dup,
    [TOKEN_TWO_DUP] = action_two_dup,
    [TOKEN_DROP] = action_drop,
    [TOKEN_TWO_DROP] = action_two_drop,
    [TOKEN_SWAP] = action_swap,
    [TOKEN_TWO_SWAP] = action_two_swap,
    [TOKEN_OVER] = action_over,
    [TOKEN_TWO_OVER] = action_two_over,
    [TOKEN_ROT] = action_rot,
    [TOKEN_PEEK] = action_peek,
  };

  while ((token = scan_token(&scanner)).type != TOKEN_EOF) {
    action_func_t action = actions[token.type];
    if (action) {
      action();
    } else {
      fprintf(stderr, "[%d:%d] ERROR: Unknown token type: %s\n",
              scanner.line, scanner.column, token.lexeme);
      exit(1);
    }
  }

  free_resources();
}
