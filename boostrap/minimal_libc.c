#include "../../include/common.h"

void* memset(void* dest, int ch, size_t count) {
    unsigned char* p = dest;
    while(count--) *p++ = (unsigned char)ch;
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    char* d = dest;
    const char* s = src;
    while(count--) *d++ = *s++;
    return dest;
}

int memcmp(const void* lhs, const void* rhs, size_t count) {
    const unsigned char *p1 = lhs, *p2 = rhs;
    while(count--) {
        if(*p1 != *p2)
            return *p1 - *p2;
        p1++, p2++;
    }
    return 0;
}

size_t strlen(const char* str) {
    const char* s;
    for(s = str; *s; ++s);
    return s - str;
}

char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while((*dest++ = *src++));
    return ret;
}

int strcmp(const char* lhs, const char* rhs) {
    while(*lhs && (*lhs == *rhs)) lhs++, rhs++;
    return *(const unsigned char*)lhs - *(const unsigned char*)rhs;
}

int putchar(int ch) {
    #ifdef FERRUM_OS_WINDOWS
    _write(1, &ch, 1);
    #else
    asm volatile (
        "mov $1, %%rax\n"
        "mov %0, %%rdi\n"
        "mov $1, %%rsi\n"
        "mov $1, %%rdx\n"
        "syscall"
        :: "r"(ch) : "rax", "rdi", "rsi", "rdx"
    );
    #endif
    return ch;
}

int puts(const char* str) {
    size_t len = strlen(str);
    #ifdef FERRUM_OS_WINDOWS
    _write(1, str, len);
    #else
    asm volatile (
        "mov $1, %%rax\n"
        "mov $1, %%rdi\n"
        "mov %0, %%rsi\n"
        "mov %1, %%rdx\n"
        "syscall"
        :: "r"(str), "r"(len) : "rax", "rdi", "rsi", "rdx"
    );
    #endif
    putchar('\n');
    return len + 1;
}

void _exit(int status) {
    #ifdef FERRUM_OS_WINDOWS
    ExitProcess(status);
    #else
    asm volatile (
        "mov $60, %%rax\n"
        "mov %0, %%rdi\n"
        "syscall"
        :: "r"(status) : "rax", "rdi"
    );
    #endif
    __builtin_unreachable();
}