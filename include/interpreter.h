#ifndef _interpreter_h_
#define _interpreter_h_

static Token token;
static Scanner scanner;
static Stack *stack, *loop_stack, *end_stack;
static char memory[MEMORY_CAPACITY];
HashMap* hashmap;
typedef void (*action_func_t)(void);
action_func_t* actions;

/**
 * @brief Runs the interpreter.
*/
void run_interpreter(const char *source_code);

#endif // _interpreter_h_

