#ifndef FERRUM_RUNTIME_MEMORY_H
#define FERRUM_RUNTIME_MEMORY_H

#include <stddef.h>
#include <stdbool.h>

// Memory management functions
void memory_init(void);
void memory_cleanup(void);

// Memory allocation functions
void* f_malloc(size_t size);
void* f_realloc(void* ptr, size_t old_size, size_t new_size);
void f_free(void* ptr);

#endif // FERRUM_RUNTIME_MEMORY_H 