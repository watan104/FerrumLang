#ifndef FERRUM_MEMORY_H
#define FERRUM_MEMORY_H

#include "../../include/common.h"

/**
 * Ferrum Memory Management API
 * - Platform-independent memory operations
 * - Debugging support with allocation tracking
 */

typedef struct {
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    size_t peak_usage;
} MemoryStats;

// Core memory functions
void* fmalloc(size_t size);
void ffree(void* ptr, size_t size);
void* fmemcpy(void* dest, const void* src, size_t n);
void* fmemset(void* s, uint8_t c, size_t n);

// Statistics and debugging
MemoryStats get_memory_stats(void);
void reset_memory_stats(void);

// Alignment helpers
void* faligned_alloc(size_t alignment, size_t size);
void faligned_free(void* ptr);

#endif // FERRUM_MEMORY_H