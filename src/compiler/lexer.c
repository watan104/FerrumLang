// src/compiler/lexer.c
#include <ctype.h>
#include <string.h>
#include "../../include/lexer.h"
#include "../../include/ferror.h"

static const struct {
    const char* keyword;
    TokenType token;
} keywords[] = {
    // Control flow
    {"fn", TOKEN_FN}, {"let", TOKEN_LET}, {"if", TOKEN_IF},
    {"else", TOKEN_ELSE}, {"for", TOKEN_FOR}, {"while", TOKEN_WHILE},
    {"return", TOKEN_RETURN}, {"break", TOKEN_BREAK}, {"continue", TOKEN_CONTINUE},
    {"match", TOKEN_MATCH}, {"case", TOKEN_CASE}, {"default", TOKEN_DEFAULT},
    
    // Type system
    {"type", TOKEN_TYPE}, {"struct", TOKEN_STRUCT}, {"enum", TOKEN_ENUM},
    {"interface", TOKEN_INTERFACE}, {"impl", TOKEN_IMPL}, {"trait", TOKEN_TRAIT},
    {"pub", TOKEN_PUB}, {"priv", TOKEN_PRIV}, {"static", TOKEN_STATIC},
    
    // Module system
    {"import", TOKEN_IMPORT}, {"from", TOKEN_FROM}, {"as", TOKEN_AS},
    {"mod", TOKEN_MOD}, {"use", TOKEN_USE},
    
    // Error handling
    {"try", TOKEN_TRY}, {"catch", TOKEN_CATCH}, {"throw", TOKEN_THROW},
    {"finally", TOKEN_FINALLY},
    
    // Concurrency
    {"go", TOKEN_GO}, {"chan", TOKEN_CHAN}, {"select", TOKEN_SELECT},
    {"defer", TOKEN_DEFER}, {"async", TOKEN_ASYNC}, {"await", TOKEN_AWAIT},
    
    // Memory management
    {"alloc", TOKEN_ALLOC}, {"free", TOKEN_FREE}, {"ref", TOKEN_REF},
    {"deref", TOKEN_DEREF}, {"move", TOKEN_MOVE}, {"copy", TOKEN_COPY},
    
    // Constants
    {"true", TOKEN_TRUE}, {"false", TOKEN_FALSE}, {"nil", TOKEN_NIL},
    {"self", TOKEN_SELF}, {"super", TOKEN_SUPER}
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

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_hex_digit(char c) {
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool is_binary_digit(char c) {
    return c == '0' || c == '1';
}

static bool is_octal_digit(char c) {
    return c >= '0' && c <= '7';
}

static Token make_ident_or_keyword(Lexer* lexer) {
    const char* start = lexer->current - 1;
    int length = 1;
    
    while (is_alpha(*lexer->current) || is_digit(*lexer->current) || *lexer->current == '_') {
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

static Token make_number(Lexer* lexer) {
    const char* start = lexer->current - 1;
    int length = 1;
    bool is_float = false;
    int base = 10;
    
    // Check for hex, binary, or octal prefix
    if (*start == '0') {
        if (*lexer->current == 'x' || *lexer->current == 'X') {
            base = 16;
            lexer->current++;
            lexer->col++;
            length++;
        } else if (*lexer->current == 'b' || *lexer->current == 'B') {
            base = 2;
            lexer->current++;
            lexer->col++;
            length++;
        } else if (*lexer->current >= '0' && *lexer->current <= '7') {
            base = 8;
        }
    }
    
    // Parse digits
    for (;;) {
        char c = *lexer->current;
        bool valid = false;
        
        switch (base) {
            case 16: valid = is_hex_digit(c); break;
            case 2: valid = is_binary_digit(c); break;
            case 8: valid = is_octal_digit(c); break;
            case 10: valid = is_digit(c); break;
        }
        
        if (!valid && c != '.' && c != 'e' && c != 'E') break;
        
        if (c == '.') {
            if (is_float || base != 10) break;
            is_float = true;
        } else if ((c == 'e' || c == 'E') && base == 10) {
            is_float = true;
            lexer->current++;
            lexer->col++;
            length++;
            
            if (*lexer->current == '+' || *lexer->current == '-') {
                lexer->current++;
                lexer->col++;
                length++;
            }
            
            if (!is_digit(*lexer->current)) {
                return make_token(lexer, TOKEN_ERROR, length);
            }
        }
        
        lexer->current++;
        lexer->col++;
        length++;
    }
    
    return make_token(lexer, is_float ? TOKEN_FLOAT : TOKEN_INT, length);
}

static Token make_string(Lexer* lexer, char quote) {
    int length = 1;
    bool escaped = false;
    
    for (;;) {
        char c = *lexer->current;
        if (c == '\0' || c == '\n') {
            return make_token(lexer, TOKEN_ERROR, length);
        }
        
        lexer->current++;
        lexer->col++;
        length++;
        
        if (escaped) {
            escaped = false;
            continue;
        }
        
        if (c == '\\') {
            escaped = true;
        } else if (c == quote) {
            break;
        }
    }
    
    return make_token(lexer, TOKEN_STRING, length);
}

static Token make_char(Lexer* lexer) {
    int length = 1;
    bool escaped = false;
    
    for (;;) {
        char c = *lexer->current;
        if (c == '\0' || c == '\n') {
            return make_token(lexer, TOKEN_ERROR, length);
        }
        
        lexer->current++;
        lexer->col++;
        length++;
        
        if (escaped) {
            escaped = false;
            continue;
        }
        
        if (c == '\\') {
            escaped = true;
        } else if (c == '\'') {
            break;
        }
    }
    
    return make_token(lexer, TOKEN_CHAR, length);
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = *lexer->current;
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                lexer->current++;
                lexer->col++;
                break;
            case '\n':
                lexer->line++;
                lexer->col = 1;
                lexer->current++;
                break;
            case '/':
                if (lexer->current[1] == '/') {
                    // Line comment
                    while (*lexer->current != '\n' && *lexer->current != '\0') {
                        lexer->current++;
                        lexer->col++;
                    }
                } else if (lexer->current[1] == '*') {
                    // Block comment
                    lexer->current += 2;
                    lexer->col += 2;
                    int nesting = 1;
                    
                    while (nesting > 0 && *lexer->current != '\0') {
                        if (lexer->current[0] == '/' && lexer->current[1] == '*') {
                            nesting++;
                            lexer->current += 2;
                            lexer->col += 2;
                        } else if (lexer->current[0] == '*' && lexer->current[1] == '/') {
                            nesting--;
                            lexer->current += 2;
                            lexer->col += 2;
                        } else {
                            if (*lexer->current == '\n') {
                                lexer->line++;
                                lexer->col = 1;
                            } else {
                                lexer->col++;
                            }
                            lexer->current++;
                        }
                    }
                    
                    if (nesting > 0) {
                        // Unterminated block comment
                        return;
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

Token lex_next(Lexer* lexer) {
    skip_whitespace(lexer);
    
    lexer->start = lexer->current;
    
    if (*lexer->current == '\0') {
        return make_token(lexer, TOKEN_EOF, 0);
    }
    
    char c = *lexer->current++;
    lexer->col++;
    
    if (is_alpha(c)) return make_ident_or_keyword(lexer);
    if (is_digit(c)) return make_number(lexer);
    
    switch (c) {
        // Single-character tokens
        case '(': return make_token(lexer, TOKEN_LPAREN, 1);
        case ')': return make_token(lexer, TOKEN_RPAREN, 1);
        case '{': return make_token(lexer, TOKEN_LBRACE, 1);
        case '}': return make_token(lexer, TOKEN_RBRACE, 1);
        case '[': return make_token(lexer, TOKEN_LBRACKET, 1);
        case ']': return make_token(lexer, TOKEN_RBRACKET, 1);
        case ',': return make_token(lexer, TOKEN_COMMA, 1);
        case ';': return make_token(lexer, TOKEN_SEMI, 1);
        case ':': return make_token(lexer, TOKEN_COLON, 1);
        case '?': return make_token(lexer, TOKEN_QUESTION, 1);
        
        // One or two character tokens
        case '!':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_BANG_EQ, 2);
            }
            return make_token(lexer, TOKEN_BANG, 1);
        case '=':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_EQEQ, 2);
            } else if (*lexer->current == '>') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_ARROW, 2);
            }
            return make_token(lexer, TOKEN_EQ, 1);
        case '<':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_LTEQ, 2);
            } else if (*lexer->current == '<') {
                lexer->current++;
                lexer->col++;
                if (*lexer->current == '=') {
                    lexer->current++;
                    lexer->col++;
                    return make_token(lexer, TOKEN_LSHIFT_EQ, 3);
                }
                return make_token(lexer, TOKEN_LSHIFT, 2);
            }
            return make_token(lexer, TOKEN_LT, 1);
        case '>':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_GTEQ, 2);
            } else if (*lexer->current == '>') {
                lexer->current++;
                lexer->col++;
                if (*lexer->current == '=') {
                    lexer->current++;
                    lexer->col++;
                    return make_token(lexer, TOKEN_RSHIFT_EQ, 3);
                }
                return make_token(lexer, TOKEN_RSHIFT, 2);
            }
            return make_token(lexer, TOKEN_GT, 1);
        case '+':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_PLUS_EQ, 2);
            } else if (*lexer->current == '+') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_PLUS_PLUS, 2);
            }
            return make_token(lexer, TOKEN_PLUS, 1);
        case '-':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_MINUS_EQ, 2);
            } else if (*lexer->current == '-') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_MINUS_MINUS, 2);
            } else if (*lexer->current == '>') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_ARROW, 2);
            }
            return make_token(lexer, TOKEN_MINUS, 1);
        case '*':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_STAR_EQ, 2);
            }
            return make_token(lexer, TOKEN_STAR, 1);
        case '/':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_SLASH_EQ, 2);
            }
            return make_token(lexer, TOKEN_SLASH, 1);
        case '%':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_PERCENT_EQ, 2);
            }
            return make_token(lexer, TOKEN_PERCENT, 1);
        case '&':
            if (*lexer->current == '&') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_AMPAMP, 2);
            } else if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_AMP_EQ, 2);
            }
            return make_token(lexer, TOKEN_AMP, 1);
        case '|':
            if (*lexer->current == '|') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_PIPEPIPE, 2);
            } else if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_PIPE_EQ, 2);
            }
            return make_token(lexer, TOKEN_PIPE, 1);
        case '^':
            if (*lexer->current == '=') {
                lexer->current++;
                lexer->col++;
                return make_token(lexer, TOKEN_CARET_EQ, 2);
            }
            return make_token(lexer, TOKEN_CARET, 1);
        
        // String and character literals
        case '"': return make_string(lexer, '"');
        case '\'': return make_char(lexer);
        
        default:
            return make_token(lexer, TOKEN_ERROR, 1);
    }
}