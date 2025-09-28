#ifndef _stack_h_
#define _stack_h_

#define DEFAULT_CAPACITY 16

#include <stdbool.h>

/**
 * @brief Stack structure.
*/
typedef struct {
    int *data;
    int top;
    int capacity;
    int size;
} Stack;


/**
 * @brief Creates a new stack with the given size.
 * @return A pointer to the newly created stack.
 */
Stack *create_stack(void);

/**
 * @brief Pushes a value onto the stack.
 * @param stack The stack to push the value onto.
 * @param value The value to push onto the stack.
 */
void push(Stack *stack, int value);

/**
 * @brief Pops a value from the stack.
 * @param stack The stack to pop the value from.
 * @return The value popped from the stack.
 */
int pop(Stack *stack);

/**
 * @brief Returns the top value from the stack.
 * @param stack The stack to top the value from.
 * @return The value dropped from the stack.
*/
int top(Stack *stack);

/**
 * @brief Prints the stack.
 * @param stack The stack to print.
*/
void dump(Stack *stack);

/**
 * @brief Checks if the stack is empty.
 * @param stack The stack to check.
 * @return true if the stack is empty, false otherwise.
*/
bool is_empty(Stack *stack);

#endif // _stack_h_

