#include "../../include/parser.h"
#include "../../include/ferror.h"
#include "../../include/ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static ParseRule rules[] = {
    [TOKEN_PLUS]          = {NULL,    NULL,   PREC_TERM},
    [TOKEN_MINUS]         = {NULL,    NULL,   PREC_TERM},
    [TOKEN_STAR]          = {NULL,    NULL,   PREC_FACTOR},
    [TOKEN_SLASH]         = {NULL,    NULL,   PREC_FACTOR},
    [TOKEN_PERCENT]       = {NULL,    NULL,   PREC_FACTOR},
    [TOKEN_CARET]         = {NULL,    NULL,   PREC_FACTOR},
    [TOKEN_AMP]           = {NULL,    NULL,   PREC_FACTOR},
    [TOKEN_PIPE]          = {NULL,    NULL,   PREC_FACTOR},
    [TOKEN_LSHIFT]        = {NULL,    NULL,   PREC_FACTOR},
    [TOKEN_RSHIFT]        = {NULL,    NULL,   PREC_FACTOR},
    [TOKEN_EQ]            = {NULL,    NULL,   PREC_ASSIGNMENT},
    [TOKEN_EQEQ]          = {NULL,    NULL,   PREC_EQUALITY},
    [TOKEN_NEQ]           = {NULL,    NULL,   PREC_EQUALITY},
    [TOKEN_LT]            = {NULL,    NULL,   PREC_COMPARISON},
    [TOKEN_GT]            = {NULL,    NULL,   PREC_COMPARISON},
    [TOKEN_LTEQ]          = {NULL,    NULL,   PREC_COMPARISON},
    [TOKEN_GTEQ]          = {NULL,    NULL,   PREC_COMPARISON},
    [TOKEN_AMPAMP]        = {NULL,    NULL,   PREC_AND},
    [TOKEN_PIPEPIPE]      = {NULL,    NULL,   PREC_OR},
    [TOKEN_BANG]          = {NULL,    NULL,   PREC_NONE},
    [TOKEN_LPAREN]        = {NULL,    NULL,   PREC_NONE},
    [TOKEN_RPAREN]        = {NULL,    NULL,   PREC_NONE},
    [TOKEN_LBRACE]        = {NULL,    NULL,   PREC_NONE},
    [TOKEN_RBRACE]        = {NULL,    NULL,   PREC_NONE},
    [TOKEN_COMMA]         = {NULL,    NULL,   PREC_NONE},
    [TOKEN_SEMI]          = {NULL,    NULL,   PREC_NONE},
    [TOKEN_COLON]         = {NULL,    NULL,   PREC_NONE},
    [TOKEN_INT]           = {NULL,    NULL,   PREC_NONE},
    [TOKEN_FLOAT]         = {NULL,    NULL,   PREC_NONE},
    [TOKEN_STRING]        = {NULL,    NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {NULL,    NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {NULL,    NULL,   PREC_NONE},
    [TOKEN_IDENT]         = {NULL,    NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,    NULL,   PREC_NONE},
    [TOKEN_ERROR]         = {NULL,    NULL,   PREC_NONE},
};

static ParseRule* get_rule(TokenType type) {
    return &rules[type];
}

static void advance(Parser* parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = lex_next(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;

        error_at_current(parser, parser->current.start);
    }
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    error_at_current(parser, message);
}

static bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

void error_at(Parser* parser, Token* token, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;

    fprintf(stderr, "[%s:%d:%d] Error", parser->filename, token->line, token->col);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
}

void error_at_current(Parser* parser, const char* message) {
    error_at(parser, &parser->current, message);
}

static void binary(Parser* parser, ASTNode** node) {
    Token op = parser->previous;
    ParseRule* rule = get_rule(op.type);
    
    ASTNode* right = NULL;
    parse_precedence(parser, (Precedence)(rule->precedence + 1));
    
    *node = ast_new_binary_expr(op, *node, right);
}

static void literal(Parser* parser, ASTNode** node) {
    switch (parser->previous.type) {
        case TOKEN_INT:
            *node = ast_new_int_literal(strtoll(parser->previous.start, NULL, 10), 
                              parser->previous.line, 
                              parser->previous.col);
            break;
        case TOKEN_FLOAT:
            *node = ast_new_float_literal(strtod(parser->previous.start, NULL), 
                                parser->previous.line, 
                                parser->previous.col);
            break;
        case TOKEN_STRING:
            *node = ast_new_string_literal(f_strdup(parser->previous.start), 
                                 parser->previous.line, 
                                 parser->previous.col);
            break;
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            *node = ast_new_bool_literal(parser->previous.type == TOKEN_TRUE, 
                               parser->previous.line, 
                               parser->previous.col);
            break;
        default:
            error_at_current(parser, "Unexpected literal type");
    }
}

static ASTNode* parse_precedence(Parser* parser, Precedence precedence) {
    advance(parser);
    ParseFn prefix = get_rule(parser->previous.type)->prefix;
    if (prefix == NULL) {
        error_at_current(parser, "Expected expression");
        return;
    }

    ASTNode* node = NULL;
    prefix(parser, &node);

    while (precedence <= get_rule(parser->current.type)->precedence) {
        advance(parser);
        ParseFn infix = get_rule(parser->previous.type)->infix;
        infix(parser, &node);
    }
}

ASTNode* parse_expression(Parser* parser) {
    ASTNode* node = NULL;
    parse_precedence(parser, PREC_ASSIGNMENT);
    return node;
}

ASTNode* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_LET)) return parse_var_declaration(parser);
    if (match(parser, TOKEN_RETURN)) return parse_return_statement(parser);
    if (match(parser, TOKEN_LBRACE)) return parse_block(parser);
    return parse_expr_statement(parser);
}

ASTNode* parse_block(Parser* parser) {
    DynamicArray statements = da_new(sizeof(ASTNode*), 8);
    
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) da_append(&statements, &stmt);
    }
    
    consume(parser, TOKEN_RBRACE, "Expect '}' after block");
    return ast_new_block_stmt(statements);
}

ASTNode* parse_var_declaration(Parser* parser) {
    Token name;
    if (!match(parser, TOKEN_IDENT)) {
        error_at_current(parser, "Expect variable name");
        return NULL;
    }
    name = parser->previous;

    ASTNode* initializer = NULL;
    if (match(parser, TOKEN_EQ)) {
        initializer = parse_expression(parser);
    }

    consume(parser, TOKEN_SEMI, "Expect ';' after variable declaration");
    return ast_new_var_decl(name, initializer);
}

ASTNode* parse(Parser* parser) {
    DynamicArray statements = da_new(sizeof(ASTNode*), 8);
    
    while (!match(parser, TOKEN_EOF)) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) da_append(&statements, &stmt);
    }
    
    return ast_new_block_stmt(statements);
}

void parser_init(Parser* parser, Lexer* lexer, const char* filename) {
    parser->lexer = lexer;
    parser->filename = filename;
    parser->had_error = false;
    parser->panic_mode = false;
    advance(parser);
}