#include "io.h"
#include "../../include/common.h"


#ifdef FERRUM_OS_WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

FerrumFile ferrum_file_open(const char* path, uint8_t flags) {
    FerrumFile file = { .fd = -1, .is_open = false };
    
#ifdef FERRUM_OS_WINDOWS
    DWORD access = 0;
    DWORD disposition = OPEN_EXISTING;
    
    if (flags & FR_WRITE) access |= GENERIC_WRITE;
    if (flags & FR_READ) access |= GENERIC_READ;
    
    if (flags & FR_CREATE) {
        disposition = (flags & FR_TRUNC) ? CREATE_ALWAYS : OPEN_ALWAYS;
    } else if (flags & FR_TRUNC) {
        disposition = TRUNCATE_EXISTING;
    }
    
    HANDLE h = CreateFileA(path, access, 0, NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        file.fd = _open_osfhandle((intptr_t)h, 0);
        file.is_open = (file.fd != -1);
    }
#else
    int posix_flags = 0;
    
    if ((flags & FR_WRITE) && (flags & FR_READ)) posix_flags |= O_RDWR;
    else if (flags & FR_WRITE) posix_flags |= O_WRONLY;
    else posix_flags |= O_RDONLY;
    
    if (flags & FR_CREATE) posix_flags |= O_CREAT;
    if (flags & FR_TRUNC) posix_flags |= O_TRUNC;
    if (flags & FR_APPEND) posix_flags |= O_APPEND;
    
    file.fd = open(path, posix_flags, 0644);
    file.is_open = (file.fd != -1);
#endif
    
    return file;
}

void ferrum_file_close(FerrumFile* file) {
    if (file->is_open) {
#ifdef FERRUM_OS_WINDOWS
        _close(file->fd);
#else
        close(file->fd);
#endif
        file->is_open = false;
    }
}

size_t ferrum_file_read(FerrumFile* file, void* buf, size_t count) {
    if (!file->is_open) return 0;
#ifdef FERRUM_OS_WINDOWS
    return _read(file->fd, buf, (unsigned int)count);
#else
    return read(file->fd, buf, count);
#endif
}

size_t ferrum_file_write(FerrumFile* file, const void* buf, size_t count) {
    if (!file->is_open) return 0;
#ifdef FERRUM_OS_WINDOWS
    return _write(file->fd, buf, (unsigned int)count);
#else
    return write(file->fd, buf, count);
#endif
}