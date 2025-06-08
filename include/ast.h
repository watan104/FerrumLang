#ifndef FERRUM_AST_H
#define FERRUM_AST_H

#include "common.h"
#include "lexer.h"

// AST node types
typedef enum {
    // Expressions
    NODE_BINARY_EXPR,      // Binary operation (e.g., a + b)
    NODE_UNARY_EXPR,       // Unary operation (e.g., -a, !b)
    NODE_CALL_EXPR,        // Function call (e.g., foo(a, b))
    NODE_GET_EXPR,         // Property access (e.g., obj.prop)
    NODE_SET_EXPR,         // Property assignment (e.g., obj.prop = val)
    NODE_LOGICAL_EXPR,     // Logical operation (e.g., a && b)
    NODE_ARRAY_EXPR,       // Array literal (e.g., [1, 2, 3])
    NODE_INDEX_EXPR,       // Array indexing (e.g., arr[i])
    NODE_CLOSURE_EXPR,     // Closure expression
    NODE_ASYNC_EXPR,       // Async expression
    NODE_AWAIT_EXPR,       // Await expression
    NODE_CHAN_SEND_EXPR,   // Channel send (e.g., ch <- value)
    NODE_CHAN_RECV_EXPR,   // Channel receive (e.g., <- ch)
    
    // Literals
    NODE_INT_LITERAL,      // Integer literal
    NODE_FLOAT_LITERAL,    // Float literal
    NODE_STRING_LITERAL,   // String literal
    NODE_BOOL_LITERAL,     // Boolean literal
    NODE_CHAR_LITERAL,     // Character literal
    NODE_NIL_LITERAL,      // Nil literal
    NODE_IDENTIFIER,       // Variable reference
    
    // Declarations
    NODE_VAR_DECL,        // Variable declaration
    NODE_FUNCTION_DECL,   // Function declaration
    NODE_CLASS_DECL,      // Class declaration
    NODE_INTERFACE_DECL,  // Interface declaration
    NODE_TRAIT_DECL,      // Trait declaration
    NODE_IMPL_DECL,       // Implementation declaration
    NODE_TYPE_DECL,       // Type declaration
    NODE_ENUM_DECL,       // Enum declaration
    NODE_IMPORT_DECL,     // Import declaration
    NODE_EXPORT_DECL,     // Export declaration
    NODE_CHAN_DECL,       // Channel declaration
    
    // Statements
    NODE_BLOCK_STMT,      // Block of statements
    NODE_IF_STMT,         // If statement
    NODE_WHILE_STMT,      // While loop
    NODE_FOR_STMT,        // For loop
    NODE_FOREACH_STMT,    // Foreach loop
    NODE_RETURN_STMT,     // Return statement
    NODE_BREAK_STMT,      // Break statement
    NODE_CONTINUE_STMT,   // Continue statement
    NODE_EXPR_STMT,       // Expression statement
    NODE_TRY_STMT,        // Try statement
    NODE_THROW_STMT,      // Throw statement
    NODE_MATCH_STMT,      // Match statement
    NODE_DEFER_STMT,      // Defer statement
    NODE_GO_STMT,         // Go statement
    NODE_SELECT_STMT,     // Select statement
    
    // Types
    NODE_TYPE_REF,        // Type reference
    NODE_TYPE_FUNC,       // Function type
    NODE_TYPE_ARRAY,      // Array type
    NODE_TYPE_MAP,        // Map type
    NODE_TYPE_TUPLE,      // Tuple type
    NODE_TYPE_GENERIC,    // Generic type
    NODE_TYPE_UNION,      // Union type
    NODE_TYPE_OPTIONAL,   // Optional type
    NODE_TYPE_CHAN,       // Channel type
    
    // Other
    NODE_ERROR           // Error node
} NodeType;

// Forward declarations
typedef struct ASTNode ASTNode;
typedef struct DynamicArray DynamicArray;

// Node structures for each type
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
    DynamicArray args;  // Array of ASTNode*
} CallExpr;

typedef struct {
    ASTNode* object;
    Token name;
} GetExpr;

typedef struct {
    ASTNode* object;
    Token name;
    ASTNode* value;
} SetExpr;

typedef struct {
    Token op;
    ASTNode* left;
    ASTNode* right;
} LogicalExpr;

typedef struct {
    DynamicArray elements;  // Array of ASTNode*
} ArrayExpr;

typedef struct {
    ASTNode* array;
    ASTNode* index;
} IndexExpr;

typedef struct {
    ASTNode* function;
    DynamicArray captures;  // Array of Token
} ClosureExpr;

typedef struct {
    ASTNode* expression;
} AsyncExpr;

typedef struct {
    ASTNode* expression;
} AwaitExpr;

typedef struct {
    Token name;
    ASTNode* value;
    bool is_mutable;
} VarDecl;

typedef struct {
    Token name;
    DynamicArray params;    // Array of Token
    DynamicArray type_params; // Array of Token
    ASTNode* return_type;
    ASTNode* body;
} FunctionDecl;

typedef struct {
    Token name;
    DynamicArray type_params; // Array of Token
    DynamicArray superclasses; // Array of ASTNode*
    DynamicArray members;     // Array of ASTNode*
} ClassDecl;

typedef struct {
    Token name;
    DynamicArray type_params; // Array of Token
    DynamicArray methods;     // Array of ASTNode*
} InterfaceDecl;

typedef struct {
    Token name;
    DynamicArray type_params; // Array of Token
    DynamicArray methods;     // Array of ASTNode*
} TraitDecl;

typedef struct {
    ASTNode* type;
    Token trait;
    DynamicArray methods;    // Array of ASTNode*
} ImplDecl;

typedef struct {
    Token name;
    DynamicArray type_params; // Array of Token
    ASTNode* type;
} TypeDecl;

typedef struct {
    Token name;
    DynamicArray variants;   // Array of Token
    DynamicArray values;     // Array of ASTNode*
} EnumDecl;

typedef struct {
    Token path;
    Token alias;
    bool is_all;
} ImportDecl;

typedef struct {
    ASTNode* declaration;
} ExportDecl;

typedef struct {
    DynamicArray statements; // Array of ASTNode*
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
    ASTNode* iterator;
    Token var;
    ASTNode* body;
} ForeachStmt;

typedef struct {
    ASTNode* value;
} ReturnStmt;

typedef struct {
    uint32_t _dummy; // Avoid empty struct warning
} BreakStmt;

typedef struct {
    uint32_t _dummy; // Avoid empty struct warning
} ContinueStmt;

typedef struct {
    ASTNode* expr;
} ExprStmt;

typedef struct {
    ASTNode* try_block;
    DynamicArray catch_blocks; // Array of ASTNode*
    ASTNode* finally_block;
} TryStmt;

typedef struct {
    ASTNode* value;
} ThrowStmt;

typedef struct {
    ASTNode* value;
    DynamicArray cases;     // Array of ASTNode*
    ASTNode* default_case;
} MatchStmt;

typedef struct {
    ASTNode* statement;
} DeferStmt;

// Channel send expression
typedef struct {
    ASTNode* channel;
    ASTNode* value;
} ChanSendExpr;

// Channel receive expression
typedef struct {
    ASTNode* channel;
} ChanRecvExpr;

// Channel declaration
typedef struct {
    Token name;
    ASTNode* element_type;
    ASTNode* capacity;     // Optional buffer size
} ChanDecl;

// Go statement
typedef struct {
    ASTNode* expression;
} GoStmt;

// Select case
typedef struct {
    ASTNode* channel;
    ASTNode* value;        // For send cases
    bool is_send;
    ASTNode* body;
} SelectCase;

// Select statement
typedef struct {
    DynamicArray cases;    // Array of SelectCase*
    ASTNode* default_case; // Optional default case
} SelectStmt;

// Main AST node structure
struct ASTNode {
    NodeType type;
    uint32_t line;
    uint32_t column;
    
    union {
        // Expressions
        BinaryExpr binary_expr;
        UnaryExpr unary_expr;
        CallExpr call_expr;
        GetExpr get_expr;
        SetExpr set_expr;
        LogicalExpr logical_expr;
        ArrayExpr array_expr;
        IndexExpr index_expr;
        ClosureExpr closure_expr;
        AsyncExpr async_expr;
        AwaitExpr await_expr;
        ChanSendExpr chan_send_expr;
        ChanRecvExpr chan_recv_expr;
        
        // Literals
        int64_t int_value;
        double float_value;
        char* string_value;
        bool bool_value;
        char char_value;
        char* ident_name;
        
        // Declarations
        VarDecl var_decl;
        FunctionDecl func_decl;
        ClassDecl class_decl;
        InterfaceDecl interface_decl;
        TraitDecl trait_decl;
        ImplDecl impl_decl;
        TypeDecl type_decl;
        EnumDecl enum_decl;
        ImportDecl import_decl;
        ExportDecl export_decl;
        ChanDecl chan_decl;
        
        // Statements
        BlockStmt block_stmt;
        IfStmt if_stmt;
        WhileStmt while_stmt;
        ForStmt for_stmt;
        ForeachStmt foreach_stmt;
        ReturnStmt return_stmt;
        BreakStmt break_stmt;
        ContinueStmt continue_stmt;
        ExprStmt expr_stmt;
        TryStmt try_stmt;
        ThrowStmt throw_stmt;
        MatchStmt match_stmt;
        DeferStmt defer_stmt;
        GoStmt go_stmt;
        SelectStmt select_stmt;
    };
};

// AST node creation functions
ASTNode* ast_new_node(NodeType type, uint32_t line, uint32_t column);
void ast_free_node(ASTNode* node);

// Expression nodes
ASTNode* ast_new_binary_expr(Token op, ASTNode* left, ASTNode* right);
ASTNode* ast_new_unary_expr(Token op, ASTNode* operand);
ASTNode* ast_new_call_expr(ASTNode* callee, DynamicArray args);
ASTNode* ast_new_get_expr(ASTNode* object, Token name);
ASTNode* ast_new_set_expr(ASTNode* object, Token name, ASTNode* value);
ASTNode* ast_new_logical_expr(Token op, ASTNode* left, ASTNode* right);
ASTNode* ast_new_array_expr(DynamicArray elements);
ASTNode* ast_new_index_expr(ASTNode* array, ASTNode* index);
ASTNode* ast_new_closure_expr(ASTNode* function, DynamicArray captures);
ASTNode* ast_new_async_expr(ASTNode* expression);
ASTNode* ast_new_await_expr(ASTNode* expression);

// Literal nodes
ASTNode* ast_new_int_literal(int64_t value, uint32_t line, uint32_t column);
ASTNode* ast_new_float_literal(double value, uint32_t line, uint32_t column);
ASTNode* ast_new_string_literal(char* value, uint32_t line, uint32_t column);
ASTNode* ast_new_bool_literal(bool value, uint32_t line, uint32_t column);
ASTNode* ast_new_char_literal(char value, uint32_t line, uint32_t column);
ASTNode* ast_new_nil_literal(uint32_t line, uint32_t column);
ASTNode* ast_new_identifier(const char* name, int length, uint32_t line, uint32_t column);

// Declaration nodes
ASTNode* ast_new_var_decl(Token name, ASTNode* value);
ASTNode* ast_new_function_decl(Token name, DynamicArray params, ASTNode* body);
ASTNode* ast_new_class_decl(Token name, DynamicArray type_params, DynamicArray superclasses, DynamicArray members);
ASTNode* ast_new_interface_decl(Token name, DynamicArray type_params, DynamicArray methods);
ASTNode* ast_new_trait_decl(Token name, DynamicArray type_params, DynamicArray methods);
ASTNode* ast_new_impl_decl(ASTNode* type, Token trait, DynamicArray methods);
ASTNode* ast_new_type_decl(Token name, DynamicArray type_params, ASTNode* type);
ASTNode* ast_new_enum_decl(Token name, DynamicArray variants, DynamicArray values);
ASTNode* ast_new_import_decl(Token path, Token alias, bool is_all);
ASTNode* ast_new_export_decl(ASTNode* declaration);

// Statement nodes
ASTNode* ast_new_block_stmt(DynamicArray statements);
ASTNode* ast_new_if_stmt(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);
ASTNode* ast_new_while_stmt(ASTNode* condition, ASTNode* body);
ASTNode* ast_new_for_stmt(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* ast_new_foreach_stmt(ASTNode* iterator, Token var, ASTNode* body);
ASTNode* ast_new_return_stmt(ASTNode* value);
ASTNode* ast_new_break_stmt(uint32_t line, uint32_t column);
ASTNode* ast_new_continue_stmt(uint32_t line, uint32_t column);
ASTNode* ast_new_expr_stmt(ASTNode* expr);
ASTNode* ast_new_try_stmt(ASTNode* try_block, DynamicArray catch_blocks, ASTNode* finally_block);
ASTNode* ast_new_throw_stmt(ASTNode* value);
ASTNode* ast_new_match_stmt(ASTNode* value, DynamicArray cases, ASTNode* default_case);
ASTNode* ast_new_defer_stmt(ASTNode* statement);

// New AST node creation functions
ASTNode* ast_new_chan_send_expr(ASTNode* channel, ASTNode* value);
ASTNode* ast_new_chan_recv_expr(ASTNode* channel);
ASTNode* ast_new_chan_decl(Token name, ASTNode* element_type, ASTNode* capacity);
ASTNode* ast_new_go_stmt(ASTNode* expression);
ASTNode* ast_new_select_case(ASTNode* channel, ASTNode* value, bool is_send, ASTNode* body);
ASTNode* ast_new_select_stmt(DynamicArray cases, ASTNode* default_case);

#endif // FERRUM_AST_H