#ifndef FERRUM_RUNTIME_IO_H
#define FERRUM_RUNTIME_IO_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

// I/O mode flags
typedef enum {
    IO_READ = 1,
    IO_WRITE = 2,
    IO_APPEND = 4,
    IO_READ_WRITE = IO_READ | IO_WRITE
} IOMode;

// Seek origin
typedef enum {
    IO_SEEK_START,
    IO_SEEK_CURRENT,
    IO_SEEK_END
} IOSeek;

// Forward declaration of file handle
typedef struct FileHandle {
    FILE* handle;
    char* path;
    IOMode mode;
} FileHandle;

// I/O system initialization and cleanup
void io_init(void);
void io_cleanup(void);

// File operations
FileHandle* io_open(const char* path, IOMode mode);
void io_close(FileHandle* handle);
size_t io_read(FileHandle* handle, void* buffer, size_t size);
size_t io_write(FileHandle* handle, const void* buffer, size_t size);
bool io_seek(FileHandle* handle, long offset, IOSeek origin);
long io_tell(FileHandle* handle);
bool io_flush(FileHandle* handle);

// String I/O operations
char* io_read_line(FileHandle* handle);
bool io_write_line(FileHandle* handle, const char* str);

// Standard streams
FileHandle* io_stdin(void);
FileHandle* io_stdout(void);
FileHandle* io_stderr(void);

#endif // FERRUM_RUNTIME_IO_H 