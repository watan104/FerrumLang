#include "runtime/memory.h"
#include "common.h"
#include <string.h>

#ifdef FERRUM_OS_WINDOWS
#include <malloc.h>
#else
#include <stdlib.h>
#endif

// Thread-safe memory statistics
static MemoryStats stats = {0};

void* fmalloc(size_t size) {
    if (size == 0) return NULL;

    void* ptr = malloc(size);
    if (ptr) {
        stats.total_allocated += size;
        stats.current_usage += size;
        if (stats.current_usage > stats.peak_usage) {
            stats.peak_usage = stats.current_usage;
        }
    }
    return ptr;
}

void ffree(void* ptr, size_t size) {
    if (!ptr) return;

    free(ptr);
    stats.total_freed += size;
    stats.current_usage -= size;
}

void* fmemcpy(void* dest, const void* src, size_t n) {
    return memcpy(dest, src, n);
}

void* fmemset(void* s, uint8_t c, size_t n) {
    return memset(s, c, n);
}

MemoryStats get_memory_stats(void) {
    return stats;
}

void reset_memory_stats(void) {
    stats = (MemoryStats){0};
}

void* faligned_alloc(size_t alignment, size_t size) {
#ifdef FERRUM_OS_WINDOWS
    void* ptr = _aligned_malloc(size, alignment);
#else
    void* ptr = aligned_alloc(alignment, size);
#endif
    if (ptr) {
        stats.total_allocated += size;
        stats.current_usage += size;
        if (stats.current_usage > stats.peak_usage) {
            stats.peak_usage = stats.current_usage;
        }
    }
    return ptr;
}

void faligned_free(void* ptr) {
    if (!ptr) return;

#ifdef FERRUM_OS_WINDOWS
    size_t size = _aligned_msize(ptr, 0, 0);
    _aligned_free(ptr);
#else
    // Note: aligned_alloc doesn't provide size query
    size_t size = 0; // Requires custom tracking for exact size
    free(ptr);
#endif

    if (size > 0) {
        stats.total_freed += size;
        stats.current_usage -= size;
    }
}