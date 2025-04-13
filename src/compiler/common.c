#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Bellek yönetimi
void* f_malloc(usize size) {
    void* ptr = malloc(size);
    if (!ptr && size != 0) {
        panic("Memory allocation failed for size: %zu", size);
    }
    return ptr;
}

void* f_calloc(usize count, usize size) {
    void* ptr = calloc(count, size);
    if (!ptr && count != 0 && size != 0) {
        panic("Memory allocation failed for count: %zu, size: %zu", count, size);
    }
    return ptr;
}

void* f_realloc(void* ptr, usize size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr && size != 0) {
        panic("Memory reallocation failed for size: %zu", size);
    }
    return new_ptr;
}

void f_free(void* ptr) {
    free(ptr);
}

void f_memcpy(void* dest, const void* src, usize size) {
    if (size > 0) {
        memcpy(dest, src, size);
    }
}

void f_memset(void* ptr, u8 value, usize size) {
    if (size > 0) {
        memset(ptr, value, size);
    }
}

// Debug fonksiyonları
FERRUM_NO_RETURN void panic(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[FERRUM PANIC] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

void debug_log(const char* fmt, ...) {
#ifdef FERRUM_DEBUG
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[DEBUG] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
#endif
}

// String işlemleri
usize f_strlen(const char* str) {
    return str ? strlen(str) : 0;
}

char* f_strdup(const char* str) {
    if (!str) return NULL;
    
    usize len = f_strlen(str) + 1;
    char* copy = f_malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

bool f_streq(const char* a, const char* b) {
    if (!a || !b) return a == b;
    return strcmp(a, b) == 0;
}

// ByteBuffer implementasyonu
ByteBuffer byte_buffer_new(usize initial_capacity) {
    ByteBuffer buf = {0};
    if (initial_capacity > 0) {
        buf.data = f_malloc(initial_capacity);
        buf.capacity = initial_capacity;
    }
    return buf;
}

void byte_buffer_free(ByteBuffer* buf) {
    if (buf) {
        f_free(buf->data);
        buf->data = NULL;
        buf->length = 0;
        buf->capacity = 0;
    }
}

void byte_buffer_append(ByteBuffer* buf, const void* data, usize size) {
    if (!buf || !data || size == 0) return;
    
    if (buf->length + size > buf->capacity) {
        usize new_capacity = buf->capacity * 2;
        if (new_capacity < buf->length + size) {
            new_capacity = buf->length + size;
        }
        
        u8* new_data = f_realloc(buf->data, new_capacity);
        if (!new_data) {
            panic("Failed to expand ByteBuffer to %zu bytes", new_capacity);
        }
        
        buf->data = new_data;
        buf->capacity = new_capacity;
    }
    
    memcpy(buf->data + buf->length, data, size);
    buf->length += size;
}

void byte_buffer_append_byte(ByteBuffer* buf, u8 byte) {
    byte_buffer_append(buf, &byte, sizeof(byte));
}

// Dinamik dizi implementasyonu
DynamicArray da_new(usize item_size, usize initial_capacity) {
    DynamicArray arr = {0};
    arr.item_size = item_size;
    if (initial_capacity > 0) {
        arr.items = f_malloc(item_size * initial_capacity);
        arr.capacity = initial_capacity;
    }
    return arr;
}

void da_free(DynamicArray* arr) {
    if (arr) {
        f_free(arr->items);
        arr->items = NULL;
        arr->count = 0;
        arr->capacity = 0;
    }
}

void da_append(DynamicArray* arr, const void* item) {
    if (!arr || !item) return;
    
    if (arr->count >= arr->capacity) {
        usize new_capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        void* new_items = f_realloc(arr->items, arr->item_size * new_capacity);
        if (!new_items) {
            panic("Failed to expand DynamicArray to %zu elements", new_capacity);
        }
        arr->items = new_items;
        arr->capacity = new_capacity;
    }
    
    memcpy((char*)arr->items + (arr->count * arr->item_size), item, arr->item_size);
    arr->count++;
}