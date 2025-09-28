#ifndef _memory_h_
#define _memory_h_

#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_EXIT  60

// Should be enough.
#define MEMORY_CAPACITY (250 * 1024 * 1024)

/**
 * @brief Display the memory content.
 * @param memory The memory to display.
 * @param size The size of the memory.
*/
void print_memory(char *memory, int size);

#endif // _memory_h_

