#ifndef FERRUM_LEXER_H
#define FERRUM_LEXER_H

#include "common.h"

typedef enum {
    // Keywords
    TOKEN_FN, TOKEN_LET, TOKEN_IF, TOKEN_ELSE, TOKEN_FOR,
    TOKEN_WHILE, TOKEN_RETURN, TOKEN_IMPORT, TOKEN_TYPE,
    TOKEN_STRUCT, TOKEN_ENUM, TOKEN_MATCH, TOKEN_GO,
    TOKEN_CHAN, TOKEN_SELECT, TOKEN_DEFER, TOKEN_INTERFACE,
    
    // Literals
    TOKEN_INT, TOKEN_FLOAT, TOKEN_STRING, TOKEN_CHAR,
    TOKEN_TRUE, TOKEN_FALSE, TOKEN_NIL,
    
    // Identifiers
    TOKEN_IDENT,
    
    // Operators
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_PERCENT, TOKEN_CARET, TOKEN_AMP, TOKEN_PIPE,
    TOKEN_LSHIFT, TOKEN_RSHIFT, TOKEN_EQ, TOKEN_EQEQ,
    TOKEN_NEQ, TOKEN_LT, TOKEN_GT, TOKEN_LTEQ, TOKEN_GTEQ,
    TOKEN_PLUSEQ, TOKEN_MINUSEQ, TOKEN_STAREQ, TOKEN_SLASHEQ,
    TOKEN_AMPAMP, TOKEN_PIPEPIPE, TOKEN_BANG,
    
    // Symbols
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_LBRACKET, TOKEN_RBRACKET, TOKEN_COMMA, TOKEN_SEMI,
    TOKEN_COLON, TOKEN_DOT, TOKEN_DOTDOT, TOKEN_ARROW,
    TOKEN_AT, TOKEN_HASH, TOKEN_QUESTION, TOKEN_DOLLAR,
    
    // Special
    TOKEN_EOF, TOKEN_ERROR, TOKEN_COMMENT
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    uint32_t length;
    uint32_t line;
    uint32_t col;
} Token;

typedef struct {
    const char* source;
    const char* current;
    uint32_t line;
    uint32_t col;
    bool in_comment;
} Lexer;

// API Fonksiyonları
void lexer_init(Lexer* lexer, const char* source);
Token lex_next(Lexer* lexer);
const char* token_type_to_str(TokenType type);
Token make_token(Lexer* lexer, TokenType type, int length);
Token make_ident_or_keyword(Lexer* lexer);

#endif // FERRUM_LEXER_H