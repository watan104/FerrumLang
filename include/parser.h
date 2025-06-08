#ifndef FERRUM_PARSER_H
#define FERRUM_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "common.h"

// Parser structure
typedef struct {
    Lexer* lexer;           // Lexer instance
    const char* filename;    // Source file name
    Token current;          // Current token
    Token previous;         // Previous token
    bool had_error;         // Error flag
    bool panic_mode;        // Error recovery mode
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

typedef void (*ParseFn)(Parser* parser, ASTNode** node, bool can_assign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

// Parser functions
void parser_init(Parser* parser, Lexer* lexer, const char* filename);
ASTNode* parse(Parser* parser);

// Helper functions
void advance(Parser* parser);
void consume(Parser* parser, TokenType type, const char* message);
bool match(Parser* parser, TokenType type);
bool check(Parser* parser, TokenType type);

// Expression parsing
void parse_expression(Parser* parser, ASTNode** node, bool can_assign);
void parse_precedence(Parser* parser, Precedence precedence, ASTNode** node, bool can_assign);

// Statement parsing
void parse_statement(Parser* parser, ASTNode** node);
void parse_block(Parser* parser, ASTNode** node);
void parse_var_declaration(Parser* parser, ASTNode** node);
void parse_function(Parser* parser, ASTNode** node);

// Error handling
void error_at(Parser* parser, Token* token, const char* message);
void error_at_current(Parser* parser, const char* message);

#endif // FERRUM_PARSER_H