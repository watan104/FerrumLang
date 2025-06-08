#include "../../include/runtime/memory.h"
#include <stdlib.h>
#include <string.h>

typedef struct GCObject {
    struct GCObject* next;
    bool marked;
    size_t size;
    void* data;
} GCObject;

typedef struct {
    GCObject* objects;
    size_t bytes_allocated;
    size_t next_gc;
    size_t threshold;
} GC;

static GC gc;

void memory_init() {
    gc.objects = NULL;
    gc.bytes_allocated = 0;
    gc.next_gc = 1024 * 1024; // 1MB initial threshold
    gc.threshold = gc.next_gc;
}

static void mark_object(GCObject* object) {
    if (object == NULL || object->marked) return;
    object->marked = true;
}

static void mark_all() {
    // TODO: Traverse root set (globals, stack)
    // For now, just mark everything (conservative)
    for (GCObject* obj = gc.objects; obj != NULL; obj = obj->next) {
        mark_object(obj);
    }
}

static void sweep() {
    GCObject** object = &gc.objects;
    while (*object) {
        if (!(*object)->marked) {
            GCObject* unreached = *object;
            *object = unreached->next;
            free(unreached->data);
            free(unreached);
            gc.bytes_allocated -= unreached->size;
        } else {
            (*object)->marked = false;
            object = &(*object)->next;
        }
    }
}

void* f_malloc(size_t size) {
    gc.bytes_allocated += size;

    if (gc.bytes_allocated > gc.next_gc) {
        mark_all();
        sweep();
        gc.next_gc = gc.bytes_allocated * 2;
    }

    void* ptr = malloc(size);
    if (ptr == NULL) return NULL;

    GCObject* object = (GCObject*)malloc(sizeof(GCObject));
    if (object == NULL) {
        free(ptr);
        return NULL;
    }

    object->next = gc.objects;
    object->marked = false;
    object->size = size;
    object->data = ptr;
    gc.objects = object;

    return ptr;
}

void* f_realloc(void* ptr, size_t old_size, size_t new_size) {
    if (ptr == NULL) return f_malloc(new_size);
    
    gc.bytes_allocated = gc.bytes_allocated - old_size + new_size;
    
    // Find the GC object
    GCObject* object = gc.objects;
    while (object != NULL && object->data != ptr) {
        object = object->next;
    }
    
    if (object == NULL) return NULL;
    
    void* new_ptr = realloc(ptr, new_size);
    if (new_ptr == NULL) return NULL;
    
    object->data = new_ptr;
    object->size = new_size;
    
    return new_ptr;
}

void f_free(void* ptr) {
    if (ptr == NULL) return;

    GCObject** object = &gc.objects;
    while (*object != NULL && (*object)->data != ptr) {
        object = &(*object)->next;
    }

    if (*object != NULL) {
        GCObject* found = *object;
        *object = found->next;
        gc.bytes_allocated -= found->size;
        free(found->data);
        free(found);
    }
}

void memory_cleanup() {
    GCObject* object = gc.objects;
    while (object != NULL) {
        GCObject* next = object->next;
        free(object->data);
        free(object);
        object = next;
    }
    gc.objects = NULL;
    gc.bytes_allocated = 0;
}