#ifndef FERRUM_SYS_H
#define FERRUM_SYS_H

#include "../../include/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Cross-platform System Operations
 */

// Process management
int sys_getpid(void);
void sys_exit(int status);

// Memory management
void* sys_mmap(size_t length);
int sys_munmap(void* addr, size_t length);

// Timing
uint64_t sys_nanotime(void);
void sys_sleep(uint64_t milliseconds);

// System info
uint64_t sys_page_size(void);
uint64_t sys_total_memory(void);

// File system
bool sys_file_exists(const char* path);
int64_t sys_file_size(const char* path);

#ifdef __cplusplus
}
#endif

#endif // FERRUM_SYS_H