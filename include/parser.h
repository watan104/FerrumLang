#ifndef FERRUM_PARSER_H
#define FERRUM_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "common.h"

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
    const char* filename;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser* parser, ASTNode* node);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

// Parser API
void parser_init(Parser* parser, Lexer* lexer, const char* filename);
ASTNode* parse(Parser* parser);

// Yardımcı fonksiyonlar
void advance(Parser* parser);
void consume(Parser* parser, TokenType type, const char* message);
bool match(Parser* parser, TokenType type);
bool check(Parser* parser, TokenType type);

// Expression parsing
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_precedence(Parser* parser, Precedence precedence);

// Statement parsing
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_block(Parser* parser);
ASTNode* parse_var_declaration(Parser* parser);
ASTNode* parse_function(Parser* parser);

// Error handling
void error_at(Parser* parser, Token* token, const char* message);
void error_at_current(Parser* parser, const char* message);

#endif // FERRUM_PARSER_H