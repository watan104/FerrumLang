#ifndef FERRUM_AST_H
#define FERRUM_AST_H

#include "common.h"
#include "lexer.h"

typedef enum {
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_STRING_LITERAL,
    NODE_BOOL_LITERAL,
    NODE_IDENTIFIER,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_CALL_EXPR,
    NODE_VAR_DECL,
    NODE_FUNCTION_DECL,
    NODE_BLOCK_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_FOR_STMT,
    NODE_RETURN_STMT,
    NODE_EXPR_STMT
} NodeType;

typedef struct ASTNode ASTNode;

typedef struct {
    Token op;
    ASTNode* left;
    ASTNode* right;
} BinaryExpr;

typedef struct {
    Token op;
    ASTNode* operand;
} UnaryExpr;

typedef struct {
    ASTNode* callee;
    DynamicArray args;  // ASTNode* array
} CallExpr;

typedef struct {
    Token name;
    ASTNode* value;
} VarDecl;

typedef struct {
    Token name;
    DynamicArray params;  // Token array
    ASTNode* body;
} FunctionDecl;

typedef struct {
    DynamicArray statements;  // ASTNode* array
} BlockStmt;

typedef struct {
    ASTNode* condition;
    ASTNode* then_branch;
    ASTNode* else_branch;
} IfStmt;

typedef struct {
    ASTNode* condition;
    ASTNode* body;
} WhileStmt;

typedef struct {
    ASTNode* initializer;
    ASTNode* condition;
    ASTNode* increment;
    ASTNode* body;
} ForStmt;

typedef struct {
    ASTNode* value;
} ReturnStmt;

typedef struct {
    ASTNode* expr;
} ExprStmt;

struct ASTNode {
    NodeType type;
    uint32_t line;
    uint32_t column;
    union {
        // Literals
        int64_t int_value;
        double float_value;
        char* string_value;
        bool bool_value;
        char* ident_name;
        
        // Expressions
        BinaryExpr binary_expr;
        UnaryExpr unary_expr;
        CallExpr call_expr;
        
        // Statements
        VarDecl var_decl;
        FunctionDecl func_decl;
        BlockStmt block_stmt;
        IfStmt if_stmt;
        WhileStmt while_stmt;
        ForStmt for_stmt;
        ReturnStmt return_stmt;
        ExprStmt expr_stmt;
    };
};

// AST API
ASTNode* ast_new_node(NodeType type, uint32_t line, uint32_t column);
void ast_free_node(ASTNode* node);

// Node creation helpers
ASTNode* ast_new_int_literal(int64_t value, uint32_t line, uint32_t column);
ASTNode* ast_new_float_literal(double value, uint32_t line, uint32_t column);
ASTNode* ast_new_string_literal(char* value, uint32_t line, uint32_t column);
ASTNode* ast_new_bool_literal(bool value, uint32_t line, uint32_t column);
ASTNode* ast_new_identifier(char* name, uint32_t line, uint32_t column);
ASTNode* ast_new_binary_expr(Token op, ASTNode* left, ASTNode* right);
ASTNode* ast_new_unary_expr(Token op, ASTNode* operand);
ASTNode* ast_new_var_decl(Token name, ASTNode* value);
ASTNode* ast_new_block_stmt(DynamicArray statements);
ASTNode* ast_new_if_stmt(ASTNode* cond, ASTNode* then_branch, ASTNode* else_branch);

#endif // FERRUM_AST_H