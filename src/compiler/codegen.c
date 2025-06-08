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

// Runtime support functions for concurrency
static void emit_runtime_support(CodeGenContext* ctx) {
    // Channel operations
    emit_function_prologue(ctx, "rt_chan_create");
    emit_instruction(ctx, "  mov rdi, [rsp + 16]");  // element size
    emit_instruction(ctx, "  mov rsi, [rsp + 24]");  // buffer capacity
    emit_instruction(ctx, "  call malloc");
    emit_instruction(ctx, "  mov [rax], rdi");       // store element size
    emit_instruction(ctx, "  mov [rax + 8], rsi");   // store capacity
    emit_instruction(ctx, "  mov qword [rax + 16], 0"); // head = 0
    emit_instruction(ctx, "  mov qword [rax + 24], 0"); // tail = 0
    emit_instruction(ctx, "  mov qword [rax + 32], 0"); // count = 0
    emit_function_epilogue(ctx);

    emit_function_prologue(ctx, "rt_chan_send");
    emit_instruction(ctx, "  mov rdi, [rsp + 16]");  // channel
    emit_instruction(ctx, "  mov rsi, [rsp + 24]");  // value ptr
    emit_instruction(ctx, "  mov rdx, [rdi]");       // element size
    emit_instruction(ctx, "  mov rcx, [rdi + 8]");   // capacity
    emit_instruction(ctx, "  mov r8, [rdi + 32]");   // count
    emit_instruction(ctx, "  cmp r8, rcx");
    emit_instruction(ctx, "  je .wait");             // if full, wait
    emit_instruction(ctx, "  mov r9, [rdi + 24]");   // tail
    emit_instruction(ctx, "  imul r9, rdx");         // tail * element_size
    emit_instruction(ctx, "  add r9, rdi");
    emit_instruction(ctx, "  add r9, 40");           // data starts at offset 40
    emit_instruction(ctx, "  push rcx");
    emit_instruction(ctx, "  mov rcx, rdx");
    emit_instruction(ctx, "  rep movsb");            // copy value
    emit_instruction(ctx, "  pop rcx");
    emit_instruction(ctx, "  inc qword [rdi + 24]");  // tail++
    emit_instruction(ctx, "  inc qword [rdi + 32]");  // count++
    emit_instruction(ctx, ".done:");
    emit_function_epilogue(ctx);
    emit_instruction(ctx, ".wait:");
    emit_instruction(ctx, "  pause");
    emit_instruction(ctx, "  jmp rt_chan_send");

    emit_function_prologue(ctx, "rt_chan_recv");
    emit_instruction(ctx, "  mov rdi, [rsp + 16]");  // channel
    emit_instruction(ctx, "  mov rsi, [rsp + 24]");  // dest ptr
    emit_instruction(ctx, "  mov rdx, [rdi]");       // element size
    emit_instruction(ctx, "  mov r8, [rdi + 32]");   // count
    emit_instruction(ctx, "  test r8, r8");
    emit_instruction(ctx, "  jz .wait");             // if empty, wait
    emit_instruction(ctx, "  mov r9, [rdi + 16]");   // head
    emit_instruction(ctx, "  imul r9, rdx");         // head * element_size
    emit_instruction(ctx, "  add r9, rdi");
    emit_instruction(ctx, "  add r9, 40");           // data starts at offset 40
    emit_instruction(ctx, "  push rcx");
    emit_instruction(ctx, "  mov rcx, rdx");
    emit_instruction(ctx, "  rep movsb");            // copy value
    emit_instruction(ctx, "  pop rcx");
    emit_instruction(ctx, "  inc qword [rdi + 16]");  // head++
    emit_instruction(ctx, "  dec qword [rdi + 32]");  // count--
    emit_instruction(ctx, ".done:");
    emit_function_epilogue(ctx);
    emit_instruction(ctx, ".wait:");
    emit_instruction(ctx, "  pause");
    emit_instruction(ctx, "  jmp rt_chan_recv");

    // Goroutine support
    emit_function_prologue(ctx, "rt_go");
    emit_instruction(ctx, "  mov rdi, [rsp + 16]");  // function ptr
    emit_instruction(ctx, "  mov rsi, [rsp + 24]");  // argument ptr
    emit_instruction(ctx, "  push rdi");
    emit_instruction(ctx, "  push rsi");
    emit_instruction(ctx, "  call CreateThread");     // Windows-specific, need to adapt for other platforms
    emit_function_epilogue(ctx);
}

static void codegen_x86_64(CodeGenContext* ctx, ASTNode* ast) {
    if (!ast) return;
    
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

        case NODE_CHAN_DECL:
            // Allocate channel
            emit_instruction(ctx, "  mov rdi, %d", sizeof(void*));  // element size
            emit_instruction(ctx, "  mov rsi, %d", ast->chan_decl.capacity ? 16 : 0);  // default capacity
            emit_instruction(ctx, "  call rt_chan_create");
            break;

        case NODE_CHAN_SEND_EXPR:
            // Evaluate value
            codegen_x86_64(ctx, ast->chan_send_expr.value);
            emit_instruction(ctx, "  push rax");  // save value
            // Get channel
            codegen_x86_64(ctx, ast->chan_send_expr.channel);
            emit_instruction(ctx, "  mov rdi, rax");  // channel ptr
            emit_instruction(ctx, "  pop rsi");       // value
            emit_instruction(ctx, "  call rt_chan_send");
            break;

        case NODE_CHAN_RECV_EXPR:
            // Get channel
            codegen_x86_64(ctx, ast->chan_recv_expr.channel);
            emit_instruction(ctx, "  mov rdi, rax");  // channel ptr
            emit_instruction(ctx, "  sub rsp, 8");    // space for result
            emit_instruction(ctx, "  mov rsi, rsp");  // result ptr
            emit_instruction(ctx, "  call rt_chan_recv");
            emit_instruction(ctx, "  pop rax");       // get result
            break;

        case NODE_GO_STMT:
            // Get function address
            emit_instruction(ctx, "  lea rdi, [rip + %s]", "function_label");  // TODO: proper function labels
            emit_instruction(ctx, "  mov rsi, 0");    // no arguments for now
            emit_instruction(ctx, "  call rt_go");
            break;

        case NODE_SELECT_STMT:
            // TODO: Implement select statement
            panic("Select statement not yet implemented");
            break;
            
        default:
            panic("Unsupported AST node type for codegen");
    }
}

bool codegen_generate(CodeGenContext* ctx, ASTNode* ast, const char* output_path) {
    if (!ast) return false;
    
    emit_text_section(ctx);
    emit_runtime_support(ctx);
    
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
    
    // Write output to file
    FILE* out = fopen(output_path, "wb");
    if (!out) {
        panic("Cannot open output file: %s", output_path);
        return false;
    }
    
    fwrite(ctx->output.data, 1, ctx->output.length, out);
    fclose(out);
    
    return true;
}