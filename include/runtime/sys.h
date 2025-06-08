#ifndef FERRUM_RUNTIME_SYS_H
#define FERRUM_RUNTIME_SYS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// Error handling state
#ifdef _WIN32
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif

extern THREAD_LOCAL char error_message[256];
extern THREAD_LOCAL int error_code;

// Error handling
void sys_set_error(int code, const char* message);
int sys_get_error_code(void);
const char* sys_get_error_message(void);
void sys_clear_error(void);

// Time functions
uint64_t sys_time_ms(void);
void sys_sleep_ms(uint32_t milliseconds);

// Environment variables
char* sys_getenv(const char* name);
bool sys_setenv(const char* name, const char* value);

// Process functions
int sys_execute(const char* command);

// Thread functions
typedef void (*ThreadFunc)(void* arg);

#ifdef _WIN32
typedef HANDLE ThreadHandle;
#else
typedef pthread_t ThreadHandle;
#endif

typedef struct Thread {
    ThreadHandle handle;
    ThreadFunc func;
    void* arg;
    bool running;
} Thread;

Thread* sys_thread_create(ThreadFunc func, void* arg);
void sys_thread_join(Thread* thread);
bool sys_thread_is_running(Thread* thread);

// Mutex functions
typedef struct Mutex {
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mutex;
#endif
} Mutex;

Mutex* sys_mutex_create(void);
void sys_mutex_destroy(Mutex* mutex);
void sys_mutex_lock(Mutex* mutex);
void sys_mutex_unlock(Mutex* mutex);

// Platform information
const char* sys_platform(void);
int sys_cpu_count(void);

#endif // FERRUM_RUNTIME_SYS_H 