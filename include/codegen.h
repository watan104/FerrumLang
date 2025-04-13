#ifndef FERRUM_CODEGEN_H
#define FERRUM_CODEGEN_H

#include "ast.h"
#include "common.h"

typedef enum {
    TARGET_X86_64,
    TARGET_ARM64,
    TARGET_WASM
} TargetArch;

typedef struct {
    TargetArch arch;
    bool optimize;
    bool debug_info;
    ByteBuffer output;
} CodeGenContext;

// Code generation API
void codegen_init(CodeGenContext* ctx, TargetArch arch);
void codegen_free(CodeGenContext* ctx);
bool codegen_generate(CodeGenContext* ctx, ASTNode* ast, const char* output_path);

// Target-specific functions
void codegen_x86_64(CodeGenContext* ctx, ASTNode* ast);
void codegen_arm64(CodeGenContext* ctx, ASTNode* ast);
void codegen_wasm(CodeGenContext* ctx, ASTNode* ast);

// Helper functions
void emit_instruction(CodeGenContext* ctx, const char* fmt, ...);
void emit_data_section(CodeGenContext* ctx, const char* label, const void* data, usize size);
void emit_text_section(CodeGenContext* ctx);
void emit_label(CodeGenContext* ctx, const char* label);
void emit_function_prologue(CodeGenContext* ctx, const char* func_name);
void emit_function_epilogue(CodeGenContext* ctx);

#endif // FERRUM_CODEGEN_H