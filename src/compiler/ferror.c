#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/ferror.h"

bool had_error = false;
bool had_runtime_error = false;

static Error* errors = NULL;
static size_t error_count = 0;
static size_t error_capacity = 0;

void error_init() {
    errors = malloc(sizeof(Error) * 10);
    error_capacity = 10;
    error_count = 0;
}

void error_cleanup() {
    for (size_t i = 0; i < error_count; i++) {
        free(errors[i].message);
        free(errors[i].filename);
    }
    free(errors);
    errors = NULL;
    error_count = 0;
    error_capacity = 0;
}

Error error_create(ErrorType type, const char* msg, uint32_t line, uint32_t col, const char* filename, bool fatal) {
    Error err;
    err.type = type;
    err.message = strdup(msg);
    err.line = line;
    err.column = col;
    err.filename = strdup(filename);
    err.is_fatal = fatal;
    return err;
}

Error error_lexer(const char* msg, uint32_t line, uint32_t col, const char* filename) {
    had_error = true;
    return error_create(ERR_LEXER, msg, line, col, filename, false);
}

Error error_parser(const char* msg, uint32_t line, uint32_t col, const char* filename) {
    had_error = true;
    return error_create(ERR_PARSER, msg, line, col, filename, false);
}

Error error_semantic(const char* msg, uint32_t line, uint32_t col, const char* filename) {
    had_error = true;
    return error_create(ERR_SEMANTIC, msg, line, col, filename, true);
}

void error_report(Error err) {
    // Hata listesine ekle
    if (error_count >= error_capacity) {
        error_capacity *= 2;
        errors = realloc(errors, sizeof(Error) * error_capacity);
    }
    errors[error_count++] = err;

    // Hata mesajını göster
    const char* type_str;
    switch (err.type) {
        case ERR_LEXER:    type_str = "Lexer"; break;
        case ERR_PARSER:   type_str = "Parser"; break;
        case ERR_SEMANTIC: type_str = "Semantic"; break;
        case ERR_CODEGEN:  type_str = "CodeGen"; break;
        case ERR_RUNTIME:  type_str = "Runtime"; break;
        default:           type_str = "Error";
    }

    fprintf(stderr, "[%s Error] %s:%d:%d: %s\n", 
            type_str, err.filename, err.line, err.column, err.message);
    
    if (err.is_fatal) {
        fprintf(stderr, "Fatal error, compilation aborted.\n");
        error_cleanup();
        exit(1);
    }
}