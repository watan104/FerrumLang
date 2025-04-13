#include "../../include/ast.h"
#include "../../include/common.h"
#include <stdlib.h>
#include <string.h>

ASTNode* ast_new_node(NodeType type, uint32_t line, uint32_t column) {
    ASTNode* node = (ASTNode*)f_malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->line = line;
    node->column = column;
    return node;
}

void ast_free_node(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_INT_LITERAL:
        case NODE_FLOAT_LITERAL:
            break;
            
        case NODE_STRING_LITERAL:
            if (node->string_value) f_free(node->string_value);
            break;
            
        case NODE_BOOL_LITERAL:
            break;
            
        case NODE_IDENTIFIER:
            if (node->ident_name) f_free(node->ident_name);
            break;
            
        case NODE_BINARY_EXPR:
            ast_free_node(node->binary_expr.left);
            ast_free_node(node->binary_expr.right);
            break;
            
        case NODE_UNARY_EXPR:
            ast_free_node(node->unary_expr.operand);
            break;
            
        case NODE_CALL_EXPR:
            ast_free_node(node->call_expr.callee);
            for (usize i = 0; i < node->call_expr.args.count; i++) {
                ASTNode* arg = *(ASTNode**)da_get(&node->call_expr.args, i);
                ast_free_node(arg);
            }
            da_free(&node->call_expr.args);
            break;
            
        case NODE_VAR_DECL:
            ast_free_node(node->var_decl.value);
            break;
            
        case NODE_FUNCTION_DECL:
            for (usize i = 0; i < node->func_decl.params.count; i++) {
                Token* param = (Token*)da_get(&node->func_decl.params, i);
                // Token'lar için ekstra temizleme gerekmez
            }
            da_free(&node->func_decl.params);
            ast_free_node(node->func_decl.body);
            break;
            
        case NODE_BLOCK_STMT:
            for (usize i = 0; i < node->block_stmt.statements.count; i++) {
                ASTNode* stmt = *(ASTNode**)da_get(&node->block_stmt.statements, i);
                ast_free_node(stmt);
            }
            da_free(&node->block_stmt.statements);
            break;
            
        case NODE_IF_STMT:
            ast_free_node(node->if_stmt.condition);
            ast_free_node(node->if_stmt.then_branch);
            if (node->if_stmt.else_branch) {
                ast_free_node(node->if_stmt.else_branch);
            }
            break;
            
        case NODE_WHILE_STMT:
            ast_free_node(node->while_stmt.condition);
            ast_free_node(node->while_stmt.body);
            break;
            
        case NODE_FOR_STMT:
            if (node->for_stmt.initializer) {
                ast_free_node(node->for_stmt.initializer);
            }
            if (node->for_stmt.condition) {
                ast_free_node(node->for_stmt.condition);
            }
            if (node->for_stmt.increment) {
                ast_free_node(node->for_stmt.increment);
            }
            ast_free_node(node->for_stmt.body);
            break;
            
        case NODE_RETURN_STMT:
            if (node->return_stmt.value) {
                ast_free_node(node->return_stmt.value);
            }
            break;
            
        case NODE_EXPR_STMT:
            ast_free_node(node->expr_stmt.expr);
            break;
    }
    
    f_free(node);
}

ASTNode* ast_new_int_literal(int64_t value, uint32_t line, uint32_t column) {
    ASTNode* node = ast_new_node(NODE_INT_LITERAL, line, column);
    node->int_value = value;
    return node;
}

ASTNode* ast_new_float_literal(double value, uint32_t line, uint32_t column) {
    ASTNode* node = ast_new_node(NODE_FLOAT_LITERAL, line, column);
    node->float_value = value;
    return node;
}

ASTNode* ast_new_string_literal(char* value, uint32_t line, uint32_t column) {
    ASTNode* node = ast_new_node(NODE_STRING_LITERAL, line, column);
    node->string_value = value;
    return node;
}

ASTNode* ast_new_bool_literal(bool value, uint32_t line, uint32_t column) {
    ASTNode* node = ast_new_node(NODE_BOOL_LITERAL, line, column);
    node->bool_value = value;
    return node;
}

ASTNode* ast_new_identifier(char* name, uint32_t line, uint32_t column) {
    ASTNode* node = ast_new_node(NODE_IDENTIFIER, line, column);
    node->ident_name = name;
    return node;
}

ASTNode* ast_new_binary_expr(Token op, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_new_node(NODE_BINARY_EXPR, op.line, op.col);
    node->binary_expr.op = op;
    node->binary_expr.left = left;
    node->binary_expr.right = right;
    return node;
}

ASTNode* ast_new_unary_expr(Token op, ASTNode* operand) {
    ASTNode* node = ast_new_node(NODE_UNARY_EXPR, op.line, op.col);
    node->unary_expr.op = op;
    node->unary_expr.operand = operand;
    return node;
}

ASTNode* ast_new_call_expr(ASTNode* callee, DynamicArray args) {
    ASTNode* node = ast_new_node(NODE_CALL_EXPR, callee->line, callee->column);
    node->call_expr.callee = callee;
    node->call_expr.args = args;
    return node;
}

ASTNode* ast_new_var_decl(Token name, ASTNode* value) {
    ASTNode* node = ast_new_node(NODE_VAR_DECL, name.line, name.col);
    node->var_decl.name = name;
    node->var_decl.value = value;
    return node;
}

ASTNode* ast_new_function_decl(Token name, DynamicArray params, ASTNode* body) {
    ASTNode* node = ast_new_node(NODE_FUNCTION_DECL, name.line, name.col);
    node->func_decl.name = name;
    node->func_decl.params = params;
    node->func_decl.body = body;
    return node;
}

ASTNode* ast_new_block_stmt(DynamicArray statements) {
    ASTNode* node = ast_new_node(NODE_BLOCK_STMT, 0, 0);
    node->block_stmt.statements = statements;
    return node;
}

ASTNode* ast_new_if_stmt(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    ASTNode* node = ast_new_node(NODE_IF_STMT, condition->line, condition->column);
    node->if_stmt.condition = condition;
    node->if_stmt.then_branch = then_branch;
    node->if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* ast_new_while_stmt(ASTNode* condition, ASTNode* body) {
    ASTNode* node = ast_new_node(NODE_WHILE_STMT, condition->line, condition->column);
    node->while_stmt.condition = condition;
    node->while_stmt.body = body;
    return node;
}

ASTNode* ast_new_for_stmt(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ASTNode* node = ast_new_node(NODE_FOR_STMT, 
        initializer ? initializer->line : (condition ? condition->line : increment ? increment->line : 0),
        initializer ? initializer->column : (condition ? condition->column : increment ? increment->column : 0));
    node->for_stmt.initializer = initializer;
    node->for_stmt.condition = condition;
    node->for_stmt.increment = increment;
    node->for_stmt.body = body;
    return node;
}

ASTNode* ast_new_return_stmt(ASTNode* value) {
    ASTNode* node = ast_new_node(NODE_RETURN_STMT, value ? value->line : 0, value ? value->column : 0);
    node->return_stmt.value = value;
    return node;
}

ASTNode* ast_new_expr_stmt(ASTNode* expr) {
    ASTNode* node = ast_new_node(NODE_EXPR_STMT, expr->line, expr->column);
    node->expr_stmt.expr = expr;
    return node;
}