#include "../../include/codegen.h"
#include "../../include/ast.h"
#include "../../include/common.h"
#include <stdio.h>
#include <stdarg.h>

void codegen_init(CodeGenContext* ctx, TargetArch arch) {
    ctx->arch = arch;
    ctx->optimize = false;
    ctx->debug_info = true;
    ctx->output = byte_buffer_new(1024);
}

void codegen_free(CodeGenContext* ctx) {
    byte_buffer_free(&ctx->output);
}

static void emit_instruction(CodeGenContext* ctx, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    char buffer[256];
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    if (len > 0) {
        byte_buffer_append(&ctx->output, buffer, len);
        byte_buffer_append_byte(&ctx->output, '\n');
    }
    
    va_end(args);
}

static void emit_data_section(CodeGenContext* ctx, const char* label, const void* data, usize size) {
    emit_instruction(ctx, "section .data");
    emit_instruction(ctx, "%s:", label);
    emit_instruction(ctx, "  db ");
    
    const u8* bytes = (const u8*)data;
    for (usize i = 0; i < size; i++) {
        if (i > 0) emit_instruction(ctx, ", ");
        emit_instruction(ctx, "0x%02x", bytes[i]);
    }
}

static void emit_text_section(CodeGenContext* ctx) {
    emit_instruction(ctx, "section .text");
}

static void emit_label(CodeGenContext* ctx, const char* label) {
    emit_instruction(ctx, "%s:", label);
}

static void emit_function_prologue(CodeGenContext* ctx, const char* func_name) {
    emit_instruction(ctx, "global %s", func_name);
    emit_label(ctx, func_name);
    emit_instruction(ctx, "  push rbp");
    emit_instruction(ctx, "  mov rbp, rsp");
}

static void emit_function_epilogue(CodeGenContext* ctx) {
    emit_instruction(ctx, "  mov rsp, rbp");
    emit_instruction(ctx, "  pop rbp");
    emit_instruction(ctx, "  ret");
}

static void codegen_x86_64(CodeGenContext* ctx, ASTNode* ast) {
    emit_text_section(ctx);
    
    // Runtime başlatma
    emit_instruction(ctx, "global _start");
    emit_label(ctx, "_start");
    
    // AST traversal
    switch (ast->type) {
        case NODE_BLOCK_STMT:
            for (usize i = 0; i < ast->block_stmt.statements.count; i++) {
                ASTNode* stmt = *(ASTNode**)da_get(&ast->block_stmt.statements, i);
                codegen_x86_64(ctx, stmt);
            }
            break;
            
        case NODE_INT_LITERAL:
            emit_instruction(ctx, "  mov rax, %ld", ast->int_value);
            break;
            
        case NODE_BINARY_EXPR:
            codegen_x86_64(ctx, ast->binary_expr.left);
            emit_instruction(ctx, "  push rax");
            codegen_x86_64(ctx, ast->binary_expr.right);
            emit_instruction(ctx, "  pop rbx");
            
            switch (ast->binary_expr.op.type) {
                case TOKEN_PLUS:
                    emit_instruction(ctx, "  add rax, rbx");
                    break;
                case TOKEN_MINUS:
                    emit_instruction(ctx, "  sub rax, rbx");
                    break;
                case TOKEN_STAR:
                    emit_instruction(ctx, "  imul rax, rbx");
                    break;
                case TOKEN_SLASH:
                    emit_instruction(ctx, "  cqo");
                    emit_instruction(ctx, "  idiv rbx");
                    break;
                default:
                    panic("Unsupported binary operator");
            }
            break;
            
        default:
            panic("Unsupported AST node type for codegen");
    }
    
    // Sistem çağrısı ile çıkış
    emit_instruction(ctx, "  mov rax, 60");  // exit syscall
    emit_instruction(ctx, "  mov rdi, 0");   // exit code
    emit_instruction(ctx, "  syscall");
}

bool codegen_generate(CodeGenContext* ctx, ASTNode* ast, const char* output_path) {
    if (!ast) return false;
    
    switch (ctx->arch) {
        case TARGET_X86_64:
            codegen_x86_64(ctx, ast);
            break;
        case TARGET_ARM64:
        case TARGET_WASM:
            panic("Target architecture not yet implemented");
            return false;
        default:
            return false;
    }
    
    // Çıktıyı dosyaya yaz
    FILE* out = fopen(output_path, "wb");
    if (!out) {
        panic("Cannot open output file: %s", output_path);
        return false;
    }
    
    fwrite(ctx->output.data, 1, ctx->output.length, out);
    fclose(out);
    
    return true;
}