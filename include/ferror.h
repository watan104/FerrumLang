#ifndef FERRUM_ERROR_H
#define FERRUM_ERROR_H

#include <stdbool.h>
#include <stdint.h>
#include "common.h"

typedef enum {
    ERR_LEXER,          // Lexer hatası
    ERR_PARSER,         // Parser hatası
    ERR_SEMANTIC,       // Semantik analiz hatası
    ERR_CODEGEN,        // Kod üretim hatası
    ERR_RUNTIME,        // Runtime hatası
    ERR_TYPE,           // Tip sistemi hatası
    ERR_SYNTAX,         // Genel syntax hatası
    ERR_IO,             // Dosya I/O hatası
    ERR_MEMORY          // Bellek yönetim hatası
} ErrorType;

typedef struct {
    ErrorType type;
    char* message;
    uint32_t line;
    uint32_t column;
    char* filename;
    bool is_fatal;
} Error;

// Hata işleme API'si
void error_init();
void error_report(Error err);
void error_cleanup();

// Hata oluşturma yardımcıları
Error error_create(ErrorType type, const char* msg, uint32_t line, uint32_t col, const char* filename, bool fatal);
Error error_lexer(const char* msg, uint32_t line, uint32_t col, const char* filename);
Error error_parser(const char* msg, uint32_t line, uint32_t col, const char* filename);
Error error_semantic(const char* msg, uint32_t line, uint32_t col, const char* filename);

// Global hata durumu
extern bool had_error;
extern bool had_runtime_error;

#endif // FERRUM_ERROR_H