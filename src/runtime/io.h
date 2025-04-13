#ifndef FERRUM_IO_H
#define FERRUM_IO_H

#include "common.h"

// Ferrum File Mode Flags
#define FR_READ   0x01
#define FR_WRITE  0x02
#define FR_CREATE 0x04
#define FR_APPEND 0x08
#define FR_TRUNC  0x10

typedef struct {
    int fd;
    bool is_open;
} FerrumFile;

FerrumFile ferrum_file_open(const char* path, uint8_t flags);
void ferrum_file_close(FerrumFile* file);
size_t ferrum_file_read(FerrumFile* file, void* buf, size_t count);
size_t ferrum_file_write(FerrumFile* file, const void* buf, size_t count);

#endif