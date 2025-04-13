#include "sys.h"
#include "../../include/common.h"
#include <string.h>

#ifdef FERRUM_OS_WINDOWS
#include <windows.h>
#include <psapi.h>
#include <timeapi.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#endif

// Process functions
int sys_getpid(void) {
#ifdef FERRUM_OS_WINDOWS
    return (int)GetCurrentProcessId();
#else
    return getpid();
#endif
}

void sys_exit(int status) {
#ifdef FERRUM_OS_WINDOWS
    ExitProcess(status);
#else
    _exit(status);
#endif
}

// Memory mapping
void* sys_mmap(size_t length) {
#ifdef FERRUM_OS_WINDOWS
    return VirtualAlloc(NULL, length, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    return mmap(NULL, length, PROT_READ | PROT_WRITE, 
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
}

int sys_munmap(void* addr, size_t length) {
#ifdef FERRUM_OS_WINDOWS
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