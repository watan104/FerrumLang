#ifndef FERRUM_COMMON_H
#define FERRUM_COMMON_H

// Platform tespiti
#if defined(_WIN32) || defined(_WIN64)
    #define _CRT_SECURE_NO_WARNINGS
    #define FERRUM_OS_WINDOWS 1
    #ifdef _WIN64
        #define FERRUM_64BIT 1
    #else
        #define FERRUM_32BIT 1
    #endif
#elif defined(__linux__)
    #define FERRUM_OS_LINUX 1
    #if defined(__x86_64__) || defined(__ppc64__)
        #define FERRUM_64BIT 1
    #else
        #define FERRUM_32BIT 1
    #endif
#elif defined(__APPLE__)
    #define FERRUM_OS_MACOS 1
    #include <TargetConditionals.h>
    #if TARGET_CPU_X86_64 || TARGET_CPU_ARM64
        #define FERRUM_64BIT 1
    #else
        #define FERRUM_32BIT 1
    #endif
#else
    #error "Unsupported platform"
#endif

// Temel tipler
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;
typedef size_t   usize;
typedef ptrdiff_t isize;

#if defined(__GNUC__) || defined(__clang__)
    #define FERRUM_PACKED __attribute__((packed))
    #define FERRUM_NO_RETURN __attribute__((noreturn))
    #define FERRUM_UNUSED __attribute__((unused))
#else
    #define FERRUM_PACKED
    #define FERRUM_NO_RETURN
    #define FERRUM_UNUSED
#endif

// Hata yönetimi
#define FERRUM_CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            panic("%s:%d: Assertion failed: %s", __FILE__, __LINE__, (msg)); \
        } \
    } while (0)

// Bellek yönetimi
void* f_malloc(size_t size);
void* f_calloc(size_t count, size_t size);
void* f_realloc(void* ptr, size_t old_size, size_t new_size);
void f_free(void* ptr);
void f_memcpy(void* dest, const void* src, size_t size);
void f_memset(void* ptr, u8 value, size_t size);

// Debug fonksiyonları
FERRUM_NO_RETURN void panic(const char* fmt, ...);
void debug_log(const char* fmt, ...);

// String işlemleri
usize f_strlen(const char* str);
char* f_strdup(const char* str);
bool f_streq(const char* a, const char* b);

// Byte buffer yapısı
typedef struct {
    u8* data;
    usize length;
    usize capacity;
} ByteBuffer;

ByteBuffer byte_buffer_new(usize initial_capacity);
void byte_buffer_free(ByteBuffer* buf);
void byte_buffer_append(ByteBuffer* buf, const void* data, usize size);
void byte_buffer_append_byte(ByteBuffer* buf, u8 byte);

// Dinamik dizi için basit implementasyon
typedef struct DynamicArray {
    void* items;
    usize count;
    usize capacity;
    usize item_size;
} DynamicArray;

DynamicArray da_new(usize item_size, usize initial_capacity);
void da_free(DynamicArray* arr);
void da_append(DynamicArray* arr, const void* item);
void* da_get(DynamicArray* arr, usize index);
void da_set(DynamicArray* arr, usize index, const void* item);
void da_remove(DynamicArray* arr, usize index);
void da_clear(DynamicArray* arr);
void da_resize(DynamicArray* arr, usize new_capacity);

#endif // FERRUM_COMMON_H