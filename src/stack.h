#ifndef _STACK_H_
#define _STACK_H_

#define DEFAULT_CAPACITY 16

#include <stdbool.h>

/**
 * @brief Stack structure.
 */
struct stack {
  int *data;
  int top;
  int capacity;
  int size;
};

/**
 * @brief Creates a new stack with the given size.
 * @return A pointer to the newly created stack.
 */
struct stack *create_stack(void);

/**
 * @brief Pushes a value onto the stack.
 * @param stack The stack to push the value onto.
 * @param value The value to push onto the stack.
 */
void push(struct stack *stack, int value);

/**
 * @brief Pops a value from the stack.
 * @param stack The stack to pop the value from.
 * @return The value popped from the stack.
 */
int pop(struct stack *stack);

/**
 * @brief Returns the top value from the stack.
 * @param stack The stack to top the value from.
 * @return The value dropped from the stack.
 */
int top(struct stack *stack);

/**
 * @brief Prints the stack.
 * @param stack The stack to print.
 */
void dump(struct stack *stack);

/**
 * @brief Checks if the stack is empty.
 * @param stack The stack to check.
 * @return true if the stack is empty, false otherwise.
 */
bool is_empty(struct stack *stack);

#endif /* _STACK_H_ */

