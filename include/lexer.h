#ifndef FERRUM_LEXER_H
#define FERRUM_LEXER_H

#include <stdint.h>

// Token types
typedef enum {
    // Single-character tokens
    TOKEN_LPAREN, TOKEN_RPAREN,     // ( )
    TOKEN_LBRACE, TOKEN_RBRACE,     // { }
    TOKEN_LBRACKET, TOKEN_RBRACKET, // [ ]
    TOKEN_COMMA, TOKEN_SEMI,        // , ;
    TOKEN_COLON, TOKEN_QUESTION,    // : ?
    TOKEN_DOT,                      // .
    
    // One or two character tokens
    TOKEN_BANG, TOKEN_BANG_EQ,      // ! !=
    TOKEN_EQ, TOKEN_EQEQ,          // = ==
    TOKEN_GT, TOKEN_GTEQ,          // > >=
    TOKEN_LT, TOKEN_LTEQ,          // < <=
    TOKEN_PLUS, TOKEN_PLUS_EQ,     // + +=
    TOKEN_MINUS, TOKEN_MINUS_EQ,   // - -=
    TOKEN_STAR, TOKEN_STAR_EQ,     // * *=
    TOKEN_SLASH, TOKEN_SLASH_EQ,   // / /=
    TOKEN_PERCENT, TOKEN_PERCENT_EQ, // % %=
    TOKEN_AMP, TOKEN_AMP_EQ,       // & &=
    TOKEN_PIPE, TOKEN_PIPE_EQ,     // | |=
    TOKEN_CARET, TOKEN_CARET_EQ,   // ^ ^=
    
    // Three character tokens
    TOKEN_LSHIFT, TOKEN_LSHIFT_EQ,  // << <<=
    TOKEN_RSHIFT, TOKEN_RSHIFT_EQ,  // >> >>=
    TOKEN_ARROW,                    // ->
    
    // Increment/Decrement
    TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS, // ++ --
    
    // Logical operators
    TOKEN_AMPAMP, TOKEN_PIPEPIPE,   // && ||
    
    // Literals
    TOKEN_IDENT,    // Identifiers
    TOKEN_STRING,   // String literals
    TOKEN_CHAR,     // Character literals
    TOKEN_INT,      // Integer literals
    TOKEN_FLOAT,    // Float literals
    
    // Keywords - Control Flow
    TOKEN_IF, TOKEN_ELSE,
    TOKEN_FOR, TOKEN_WHILE,
    TOKEN_BREAK, TOKEN_CONTINUE,
    TOKEN_RETURN,
    TOKEN_MATCH, TOKEN_CASE, TOKEN_DEFAULT,
    
    // Keywords - Functions and Variables
    TOKEN_FN, TOKEN_LET,
    TOKEN_CONST, TOKEN_STATIC,
    
    // Keywords - Types
    TOKEN_TYPE, TOKEN_STRUCT,
    TOKEN_ENUM, TOKEN_INTERFACE,
    TOKEN_IMPL, TOKEN_TRAIT,
    TOKEN_PUB, TOKEN_PRIV,
    
    // Keywords - Modules
    TOKEN_MOD, TOKEN_USE,
    TOKEN_IMPORT, TOKEN_FROM,
    TOKEN_AS,
    
    // Keywords - Error Handling
    TOKEN_TRY, TOKEN_CATCH,
    TOKEN_THROW, TOKEN_FINALLY,
    
    // Keywords - Concurrency
    TOKEN_GO, TOKEN_CHAN,
    TOKEN_SELECT, TOKEN_DEFER,
    TOKEN_ASYNC, TOKEN_AWAIT,
    
    // Keywords - Memory Management
    TOKEN_ALLOC, TOKEN_FREE,
    TOKEN_REF, TOKEN_DEREF,
    TOKEN_MOVE, TOKEN_COPY,
    
    // Keywords - Constants
    TOKEN_TRUE, TOKEN_FALSE,
    TOKEN_NIL, TOKEN_SELF,
    TOKEN_SUPER,
    
    // Special tokens
    TOKEN_ERROR,    // Error token
    TOKEN_EOF,      // End of file
    
    TOKEN_COUNT     // Number of token types
} TokenType;

// Token structure
typedef struct {
    TokenType type;     // Type of token
    const char* start;  // Start of token text in source
    int length;        // Length of token text
    uint32_t line;     // Line number in source
    uint32_t col;      // Column number in source
} Token;

// Lexer structure
typedef struct {
    const char* start;    // Start of current token
    const char* current;  // Current position in source
    uint32_t line;       // Current line number
    uint32_t col;        // Current column number
} Lexer;

// Lexer functions
void lexer_init(Lexer* lexer, const char* source);
Token lex_next(Lexer* lexer);

#endif // FERRUM_LEXER_H