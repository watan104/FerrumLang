#include "../../include/runtime/io.h"
#include "../../include/runtime/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Standard streams
static FileHandle stdin_handle = {NULL, "stdin", IO_READ};
static FileHandle stdout_handle = {NULL, "stdout", IO_WRITE};
static FileHandle stderr_handle = {NULL, "stderr", IO_WRITE};

void io_init(void) {
    stdin_handle.handle = stdin;
    stdout_handle.handle = stdout;
    stderr_handle.handle = stderr;
}

void io_cleanup(void) {
    // Close all open files except standard streams
}

FileHandle* io_open(const char* path, IOMode mode) {
    const char* mode_str;
    switch (mode) {
        case IO_READ: mode_str = "rb"; break;
        case IO_WRITE: mode_str = "wb"; break;
        case IO_APPEND: mode_str = "ab"; break;
        case IO_READ_WRITE: mode_str = "r+b"; break;
        default: return NULL;
    }

    FILE* file = fopen(path, mode_str);
    if (!file) return NULL;

    FileHandle* handle = f_malloc(sizeof(FileHandle));
    if (!handle) {
        fclose(file);
        return NULL;
    }

    handle->path = f_malloc(strlen(path) + 1);
    if (!handle->path) {
        fclose(file);
        f_free(handle);
        return NULL;
    }

    strcpy(handle->path, path);
    handle->handle = file;
    handle->mode = mode;

    return handle;
}

void io_close(FileHandle* handle) {
    if (!handle) return;
    if (handle->handle && handle->handle != stdin && 
        handle->handle != stdout && handle->handle != stderr) {
        fclose(handle->handle);
    }
    f_free(handle->path);
    f_free(handle);
}

size_t io_read(FileHandle* handle, void* buffer, size_t size) {
    if (!handle || !handle->handle || !(handle->mode & IO_READ)) return 0;
    return fread(buffer, 1, size, handle->handle);
}

size_t io_write(FileHandle* handle, const void* buffer, size_t size) {
    if (!handle || !handle->handle || !(handle->mode & IO_WRITE)) return 0;
    return fwrite(buffer, 1, size, handle->handle);
}

bool io_seek(FileHandle* handle, long offset, IOSeek origin) {
    if (!handle || !handle->handle) return false;

    int whence;
    switch (origin) {
        case IO_SEEK_START: whence = SEEK_SET; break;
        case IO_SEEK_CURRENT: whence = SEEK_CUR; break;
        case IO_SEEK_END: whence = SEEK_END; break;
        default: return false;
    }

    return fseek(handle->handle, offset, whence) == 0;
}

long io_tell(FileHandle* handle) {
    if (!handle || !handle->handle) return -1;
    return ftell(handle->handle);
}

bool io_flush(FileHandle* handle) {
    if (!handle || !handle->handle) return false;
    return fflush(handle->handle) == 0;
}

// String I/O functions
char* io_read_line(FileHandle* handle) {
    if (!handle || !handle->handle || !(handle->mode & IO_READ)) return NULL;

    size_t capacity = 128;
    size_t size = 0;
    char* buffer = f_malloc(capacity);
    if (!buffer) return NULL;

    int c;
    while ((c = fgetc(handle->handle)) != EOF) {
        if (size + 1 >= capacity) {
            size_t new_capacity = capacity * 2;
            char* new_buffer = f_realloc(buffer, capacity, new_capacity);
            if (!new_buffer) {
                f_free(buffer);
                return NULL;
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }

        buffer[size++] = (char)c;
        if (c == '\n') break;
    }

    if (size == 0 && c == EOF) {
        f_free(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    return buffer;
}

bool io_write_line(FileHandle* handle, const char* str) {
    if (!handle || !handle->handle || !(handle->mode & IO_WRITE)) return false;
    
    size_t len = strlen(str);
    if (io_write(handle, str, len) != len) return false;
    if (io_write(handle, "\n", 1) != 1) return false;
    
    return true;
}

// Standard stream accessors
FileHandle* io_stdin(void) { return &stdin_handle; }
FileHandle* io_stdout(void) { return &stdout_handle; }
FileHandle* io_stderr(void) { return &stderr_handle; }