#include "../../include/runtime/sys.h"
#include "../../include/common.h"
#include "../../include/runtime/memory.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#endif

// Error handling state
THREAD_LOCAL char error_message[256] = {0};
THREAD_LOCAL int error_code = 0;

void sys_set_error(int code, const char* message) {
    error_code = code;
    strncpy(error_message, message, sizeof(error_message) - 1);
    error_message[sizeof(error_message) - 1] = '\0';
}

int sys_get_error_code(void) {
    return error_code;
}

const char* sys_get_error_message(void) {
    return error_message;
}

void sys_clear_error(void) {
    error_code = 0;
    error_message[0] = '\0';
}

// Time functions
uint64_t sys_time_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

void sys_sleep_ms(uint32_t milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// Environment variables
char* sys_getenv(const char* name) {
    const char* value = getenv(name);
    if (!value) return NULL;
    
    char* copy = f_malloc(strlen(value) + 1);
    if (copy) strcpy(copy, value);
    return copy;
}

bool sys_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value) == 0;
#else
    return setenv(name, value, 1) == 0;
#endif
}

// Process functions
int sys_execute(const char* command) {
#ifdef _WIN32
    return system(command);
#else
    return system(command);
#endif
}

// Thread functions
#ifdef _WIN32
static DWORD WINAPI thread_wrapper(LPVOID param) {
#else
static void* thread_wrapper(void* param) {
#endif
    Thread* thread = (Thread*)param;
    thread->func(thread->arg);
    thread->running = false;
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

Thread* sys_thread_create(ThreadFunc func, void* arg) {
    Thread* thread = f_malloc(sizeof(Thread));
    if (!thread) return NULL;

    thread->func = func;
    thread->arg = arg;
    thread->running = true;

#ifdef _WIN32
    thread->handle = CreateThread(NULL, 0, thread_wrapper, thread, 0, NULL);
    if (!thread->handle) {
        f_free(thread);
        return NULL;
    }
#else
    if (pthread_create(&thread->handle, NULL, thread_wrapper, thread) != 0) {
        f_free(thread);
        return NULL;
    }
#endif

    return thread;
}

void sys_thread_join(Thread* thread) {
    if (!thread) return;

#ifdef _WIN32
    WaitForSingleObject(thread->handle, INFINITE);
    CloseHandle(thread->handle);
#else
    pthread_join(thread->handle, NULL);
#endif

    f_free(thread);
}

bool sys_thread_is_running(Thread* thread) {
    return thread && thread->running;
}

// Mutex functions
Mutex* sys_mutex_create(void) {
    Mutex* mutex = f_malloc(sizeof(Mutex));
    if (!mutex) return NULL;

#ifdef _WIN32
    InitializeCriticalSection(&mutex->cs);
#else
    pthread_mutex_init(&mutex->mutex, NULL);
#endif

    return mutex;
}

void sys_mutex_destroy(Mutex* mutex) {
    if (!mutex) return;

#ifdef _WIN32
    DeleteCriticalSection(&mutex->cs);
#else
    pthread_mutex_destroy(&mutex->mutex);
#endif

    f_free(mutex);
}

void sys_mutex_lock(Mutex* mutex) {
    if (!mutex) return;

#ifdef _WIN32
    EnterCriticalSection(&mutex->cs);
#else
    pthread_mutex_lock(&mutex->mutex);
#endif
}

void sys_mutex_unlock(Mutex* mutex) {
    if (!mutex) return;

#ifdef _WIN32
    LeaveCriticalSection(&mutex->cs);
#else
    pthread_mutex_unlock(&mutex->mutex);
#endif
}

// Platform detection
const char* sys_platform(void) {
#ifdef _WIN32
    return "windows";
#elif defined(__APPLE__)
    return "macos";
#elif defined(__linux__)
    return "linux";
#else
    return "unknown";
#endif
}

// CPU core count
int sys_cpu_count(void) {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

// Memory mapping
void* sys_mmap(size_t length) {
#ifdef _WIN32
    return VirtualAlloc(NULL, length, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    return mmap(NULL, length, PROT_READ | PROT_WRITE, 
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
}

int sys_munmap(void* addr, size_t length) {
#ifdef _WIN32
    return VirtualFree(addr, 0, MEM_RELEASE) ? 0 : -1;
#else
    return munmap(addr, length);
#endif
}

// Timing functions
uint64_t sys_nanotime(void) {
#ifdef FERRUM_OS_WINDOWS
    LARGE_INTEGER freq, time;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&time);
    return (time.QuadPart * 1000000000ULL) / freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif
}

void sys_sleep(uint64_t milliseconds) {
#ifdef FERRUM_OS_WINDOWS
    Sleep((DWORD)milliseconds);
#else
    struct timespec ts = {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = (milliseconds % 1000) * 1000000
    };
    nanosleep(&ts, NULL);
#endif
}

// System information
uint64_t sys_page_size(void) {
#ifdef FERRUM_OS_WINDOWS
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
#else
    return sysconf(_SC_PAGESIZE);
#endif
}

uint64_t sys_total_memory(void) {
#ifdef FERRUM_OS_WINDOWS
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#else
    return (uint64_t)sysconf(_SC_PHYS_PAGES) * sys_page_size();
#endif
}

// File system
bool sys_file_exists(const char* path) {
#ifdef FERRUM_OS_WINDOWS
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && 
           !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
#endif
}

int64_t sys_file_size(const char* path) {
#ifdef FERRUM_OS_WINDOWS
    HANDLE hFile = CreateFileA(path, GENERIC_READ, 
                             FILE_SHARE_READ, NULL, 
                             OPEN_EXISTING, 
                             FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return -1;
    
    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return -1;
    }
    
    CloseHandle(hFile);
    return size.QuadPart;
#else
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return st.st_size;
#endif
}