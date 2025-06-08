/*enough iam done i cannot code anymore :(*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"
#include "ferror.h"
#include "runtime/memory.h"
#include "runtime/io.h"

static void print_usage(const char* program_name) {
    printf("Usage: %s [options] <source_file>\n", program_name);
    printf("Options:\n");
    printf("  -o <file>    Specify output file (default: a.out)\n");
    printf("  -v           Print version information\n");
    printf("  -h           Print this help message\n");
    printf("  -d           Enable debug output\n");
}

static void print_version(void) {
    printf("Ferrum Compiler version 0.1.0\n");
    printf("Copyright (c) 2024 Ferrum Team\n");
}

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        ferror_set("Could not open file '%s'", path);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        ferror_set("Not enough memory to read '%s'", path);
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        ferror_set("Could not read file '%s'", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char* argv[]) {
    char* output_file = "a.out";
    bool debug_mode = false;
    char* source_file = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[i], "-d") == 0) {
            debug_mode = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: -o requires an argument\n");
                return 1;
            }
            output_file = argv[i];
        } else if (source_file == NULL) {
            source_file = argv[i];
        } else {
            fprintf(stderr, "Error: Multiple source files not supported\n");
            return 1;
        }
    }

    if (source_file == NULL) {
        fprintf(stderr, "Error: No source file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    // Initialize the memory system
    memory_init();

    // Read the source file
    char* source = read_file(source_file);
    if (source == NULL) {
        fprintf(stderr, "Error: %s\n", ferror_get());
        return 1;
    }

    // Initialize lexer
    Lexer lexer;
    lexer_init(&lexer, source);

    // Initialize parser
    Parser parser;
    parser_init(&parser, &lexer, source_file);

    // Parse the program
    ASTNode* ast = parse(&parser);
    if (ast == NULL || parser.had_error) {
        fprintf(stderr, "Error: Parsing failed\n");
        free(source);
        return 1;
    }

    if (debug_mode) {
        printf("AST Structure:\n");
        // Note: ast_print function not found in headers, using debug print instead
        printf("Debug: AST root node type = %d\n", ast->type);
    }

    // Initialize code generation context
    CodeGenContext codegen_ctx;
    codegen_init(&codegen_ctx, TARGET_X86_64);  // Default to x86_64

    // Generate code
    if (!codegen_generate(&codegen_ctx, ast, output_file)) {
        fprintf(stderr, "Error: Code generation failed - %s\n", ferror_get());
        ast_free_node(ast);
        codegen_free(&codegen_ctx);
        free(source);
        return 1;
    }

    // Cleanup
    ast_free_node(ast);
    codegen_free(&codegen_ctx);
    free(source);
    memory_cleanup();

    printf("Successfully compiled %s to %s\n", source_file, output_file);
    return 0;
}
