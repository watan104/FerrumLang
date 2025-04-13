// src/compiler/lexer.c
#include <ctype.h>
#include <string.h>
#include "lexer.h"
#include "ferror.h"

static const struct {
    const char* keyword;
    TokenType token;
} keywords[] = {
    {"fn", TOKEN_FN}, {"let", TOKEN_LET}, {"if", TOKEN_IF},
    {"else", TOKEN_ELSE}, {"for", TOKEN_FOR}, {"while", TOKEN_WHILE},
    {"return", TOKEN_RETURN}, {"import", TOKEN_IMPORT}, {"type", TOKEN_TYPE},
    {"struct", TOKEN_STRUCT}, {"enum", TOKEN_ENUM}, {"match", TOKEN_MATCH},
    {"go", TOKEN_GO}, {"chan", TOKEN_CHAN}, {"select", TOKEN_SELECT},
    {"defer", TOKEN_DEFER}, {"interface", TOKEN_INTERFACE},
    {"true", TOKEN_TRUE}, {"false", TOKEN_FALSE}, {"nil", TOKEN_NIL}
};

static Token make_token(Lexer* lexer, TokenType type, int length) {
    Token token;
    token.type = type;
    token.start = lexer->current - length;
    token.length = length;
    token.line = lexer->line;
    token.col = lexer->col - length;
    return token;
}

static Token make_ident_or_keyword(Lexer* lexer) {
    const char* start = lexer->current - 1;
    int length = 1;
    
    while (isalnum(*lexer->current) || *lexer->current == '_') {
        lexer->current++;
        lexer->col++;
        length++;
    }
    
    // Check if identifier is a keyword
    for (size_t i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        if (strlen(keywords[i].keyword) == length && 
            memcmp(start, keywords[i].keyword, length) == 0) {
            return make_token(lexer, keywords[i].token, length);
        }
    }
    
    // Regular identifier
    return make_token(lexer, TOKEN_IDENT, length);
}

Token lex_next(Lexer* lexer) {
    // Skip whitespace
    while (isspace(*lexer->current)) {
        if (*lexer->current == '\n') {
            lexer->line++;
            lexer->col = 1;
        } else {
            lexer->col++;
        }
        lexer->current++;
    }
    
    // Handle EOF
    if (*lexer->current == '\0') {
        return make_token(lexer, TOKEN_EOF, 0);
    }
    
    char c = *lexer->current++;
    lexer->col++;
    
    // Single character tokens
    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN, 1);
        case ')': return make_token(lexer, TOKEN_RPAREN, 1);
        case '{': return make_token(lexer, TOKEN_LBRACE, 1);
        case '}': return make_token(lexer, TOKEN_RBRACE, 1);
        case ',': return make_token(lexer, TOKEN_COMMA, 1);
        case ';': return make_token(lexer, TOKEN_SEMI, 1);
        case ':': return make_token(lexer, TOKEN_COLON, 1);
        case '+': return make_token(lexer, TOKEN_PLUS, 1);
        case '-': 
            if (*lexer->current == '>') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_ARROW, 2);
            }
            return make_token(lexer, TOKEN_MINUS, 1);
        case '*': return make_token(lexer, TOKEN_STAR, 1);
        case '/': 
            // Handle comments
            if (*lexer->current == '/') {
                while (*lexer->current != '\n' && *lexer->current != '\0') {
                    lexer->current++;
                    lexer->col++;
                }
                return lex_next(lexer); // Skip comment
            }
            return make_token(lexer, TOKEN_SLASH, 1);
    }
    
    // Identifiers and keywords
    if (isalpha(c) || c == '_') {
        return make_ident_or_keyword(lexer);
    }
    
    // Numbers
    if (isdigit(c)) {
        const char* start = lexer->current - 1;
        int length = 1;
        bool is_float = false;
        bool is_hex = false;
        
        if (c == '0' && (*lexer->current == 'x' || *lexer->current == 'X')) {
            is_hex = true;
            lexer->current++;
            lexer->col++;
            length++;
        }
        
        while (isdigit(*lexer->current) || 
               (is_hex && isxdigit(*lexer->current)) ||
               (*lexer->current == '.' && !is_float)) {
            if (*lexer->current == '.') {
                is_float = true;
            }
            lexer->current++;
            lexer->col++;
            length++;
        }
        
        return make_token(lexer, is_float ? TOKEN_FLOAT : TOKEN_INT, length);
    }
    
    // Strings
    if (c == '"') {
        const char* start = lexer->current;
        int length = 0;
        
        while (*lexer->current != '"' && *lexer->current != '\0') {
            if (*lexer->current == '\\') {
                lexer->current++; // Skip escape char
                length++;
            }
            lexer->current++;
            lexer->col++;
            length++;
        }
        
        if (*lexer->current != '"') {
            return make_token(lexer, TOKEN_ERROR, 0); // Unterminated string
        }
        
        lexer->current++; // Skip closing quote
        lexer->col++;
        return make_token(lexer, TOKEN_STRING, length + 2); // Include quotes
    }
    
    return make_token(lexer, TOKEN_ERROR, 1);
}