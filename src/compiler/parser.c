#include "../../include/lexer.h"
#include "../../include/parser.h"
#include "../../include/ferror.h"
#include "../../include/ast.h"
#include "../../include/common.h"
#include "../../include/parser_concurrency.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Precedence levels
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // = += -= *= /= %= &= |= ^= <<= >>=
    PREC_CONDITIONAL, // ?:
    PREC_OR,         // ||
    PREC_AND,        // &&
    PREC_BITWISE_OR, // |
    PREC_BITWISE_XOR,// ^
    PREC_BITWISE_AND,// &
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_SHIFT,      // << >>
    PREC_TERM,       // + -
    PREC_FACTOR,     // * / %
    PREC_UNARY,      // ! ~ - + ++ -- & *
    PREC_CALL,       // . () []
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser* parser, ASTNode** node, bool can_assign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

static void parse_expression(Parser* parser, ASTNode** node, bool can_assign);
static void parse_statement(Parser* parser, ASTNode** node);
static void parse_declaration(Parser* parser, ASTNode** node);
static void parse_block(Parser* parser, ASTNode** node);

// Error handling
static void error_at(Parser* parser, Token* token, const char* message) {
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

static void error_at_current(Parser* parser, const char* message) {
    error_at(parser, &parser->current, message);
}

// Token handling
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

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

// Parsing functions
static void parse_precedence(Parser* parser, Precedence precedence, ASTNode** node, bool can_assign) {
    advance(parser);
    ParseFn prefix = get_rule(parser->previous.type)->prefix;
    if (prefix == NULL) {
        error_at_current(parser, "Expected expression");
        return;
    }

    prefix(parser, node, can_assign);

    while (precedence <= get_rule(parser->current.type)->precedence) {
        advance(parser);
        ParseFn infix = get_rule(parser->previous.type)->infix;
        infix(parser, node, can_assign);
    }

    if (can_assign && match(parser, TOKEN_EQ)) {
        error_at_current(parser, "Invalid assignment target");
    }
}

static void parse_number(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    double value = strtod(parser->previous.start, NULL);
    if (strchr(parser->previous.start, '.')) {
        *node = ast_new_float_literal(value, parser->previous.line, parser->previous.col);
    } else {
        *node = ast_new_int_literal((int64_t)value, parser->previous.line, parser->previous.col);
    }
}

static void parse_literal(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    TokenType type = parser->previous.type;
    uint32_t line = parser->previous.line;
    uint32_t col = parser->previous.col;
    
    switch (type) {
        case TOKEN_TRUE:
            *node = ast_new_bool_literal(true, line, col);
            break;
        case TOKEN_FALSE:
            *node = ast_new_bool_literal(false, line, col);
            break;
        case TOKEN_NIL:
            *node = ast_new_nil_literal(line, col);
            break;
        default:
            error_at_current(parser, "Invalid literal");
            break;
    }
}

static void parse_string(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    // Remove surrounding quotes
    int length = parser->previous.length - 2;
    char* value = malloc(length + 1);
    memcpy(value, parser->previous.start + 1, length);
    value[length] = '\0';
    *node = ast_new_string_literal(value, parser->previous.line, parser->previous.col);
}

static void parse_identifier(Parser* parser, ASTNode** node, bool can_assign) {
    Token name = parser->previous;
    *node = ast_new_identifier(name.start, name.length, name.line, name.col);
}

static void parse_grouping(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    parse_expression(parser, node, can_assign);
    consume(parser, TOKEN_RPAREN, "Expect ')' after expression");
}

static void parse_unary(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    TokenType operator_type = parser->previous.type;
    Token operator = parser->previous;

    // Parse the operand
    parse_precedence(parser, PREC_UNARY, node, can_assign);

    // Create unary expression node
    *node = ast_new_unary_expr(operator, *node);
}

static void parse_binary(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    TokenType operator_type = parser->previous.type;
    Token operator = parser->previous;
    ParseRule* rule = get_rule(operator_type);

    // Parse the right operand
    parse_precedence(parser, (Precedence)(rule->precedence + 1), node, can_assign);

    // Create binary expression node
    *node = ast_new_binary_expr(operator, *node, node);
}

static void parse_call(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    DynamicArray args = da_new(sizeof(ASTNode*), 8);

    if (!check(parser, TOKEN_RPAREN)) {
        do {
            ASTNode* arg = NULL;
            parse_expression(parser, &arg, can_assign);
            da_append(&args, &arg);
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after arguments");
    *node = ast_new_call_expr(*node, args);
}

static void parse_dot(Parser* parser, ASTNode** node, bool can_assign) {
    consume(parser, TOKEN_IDENT, "Expect property name after '.'");
    Token name = parser->previous;

    if (can_assign && match(parser, TOKEN_EQ)) {
        ASTNode* value = NULL;
        parse_expression(parser, &value, can_assign);
        *node = ast_new_set_expr(*node, name, value);
    } else {
        *node = ast_new_get_expr(*node, name);
    }
}

static void parse_and(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    Token operator = parser->previous;
    ASTNode* right = NULL;
    parse_precedence(parser, PREC_AND, &right, can_assign);
    *node = ast_new_logical_expr(operator, *node, right);
}

static void parse_or(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    Token operator = parser->previous;
    ASTNode* right = NULL;
    parse_precedence(parser, PREC_OR, &right, can_assign);
    *node = ast_new_logical_expr(operator, *node, right);
}

static void parse_var_declaration(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_IDENT, "Expect variable name");
    Token name = parser->previous;

    ASTNode* initializer = NULL;
    if (match(parser, TOKEN_EQ)) {
        parse_expression(parser, &initializer, false);
    }

    consume(parser, TOKEN_SEMI, "Expect ';' after variable declaration");
    *node = ast_new_var_decl(name, initializer);
}

static void parse_function(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_IDENT, "Expect function name");
    Token name = parser->previous;

    consume(parser, TOKEN_LPAREN, "Expect '(' after function name");
    DynamicArray params = da_new(sizeof(Token), 8);

    if (!check(parser, TOKEN_RPAREN)) {
        do {
            consume(parser, TOKEN_IDENT, "Expect parameter name");
            Token param = parser->previous;
            da_append(&params, &param);
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after parameters");
    consume(parser, TOKEN_LBRACE, "Expect '{' before function body");

    ASTNode* body = NULL;
    parse_block(parser, &body);
    *node = ast_new_function_decl(name, params, body);
}

static void parse_return_statement(Parser* parser, ASTNode** node) {
    ASTNode* value = NULL;
    if (!check(parser, TOKEN_SEMI)) {
        parse_expression(parser, &value, false);
    }

    consume(parser, TOKEN_SEMI, "Expect ';' after return value");
    *node = ast_new_return_stmt(value);
}

static void parse_if_statement(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'");
    ASTNode* condition = NULL;
    parse_expression(parser, &condition, false);
    consume(parser, TOKEN_RPAREN, "Expect ')' after condition");

    ASTNode* then_branch = NULL;
    parse_statement(parser, &then_branch);

    ASTNode* else_branch = NULL;
    if (match(parser, TOKEN_ELSE)) {
        parse_statement(parser, &else_branch);
    }

    *node = ast_new_if_stmt(condition, then_branch, else_branch);
}

static void parse_while_statement(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'");
    ASTNode* condition = NULL;
    parse_expression(parser, &condition, false);
    consume(parser, TOKEN_RPAREN, "Expect ')' after condition");

    ASTNode* body = NULL;
    parse_statement(parser, &body);

    *node = ast_new_while_stmt(condition, body);
}

static void parse_for_statement(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'for'");

    ASTNode* initializer = NULL;
    if (match(parser, TOKEN_SEMI)) {
        // No initializer
    } else if (match(parser, TOKEN_LET)) {
        parse_var_declaration(parser, &initializer);
    } else {
        parse_expression(parser, &initializer, false);
    }

    ASTNode* condition = NULL;
    if (!check(parser, TOKEN_SEMI)) {
        parse_expression(parser, &condition, false);
    }
    consume(parser, TOKEN_SEMI, "Expect ';' after loop condition");

    ASTNode* increment = NULL;
    if (!check(parser, TOKEN_RPAREN)) {
        parse_expression(parser, &increment, false);
    }
    consume(parser, TOKEN_RPAREN, "Expect ')' after for clauses");

    ASTNode* body = NULL;
    parse_statement(parser, &body);

    *node = ast_new_for_stmt(initializer, condition, increment, body);
}

static void parse_block(Parser* parser, ASTNode** node) {
    DynamicArray statements = da_new(sizeof(ASTNode*), 8);

    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        ASTNode* statement = NULL;
        parse_declaration(parser, &statement);
        if (statement) da_append(&statements, &statement);
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after block");
    *node = ast_new_block_stmt(statements);
}

static void parse_expression_statement(Parser* parser, ASTNode** node) {
    parse_expression(parser, node, false);
    consume(parser, TOKEN_SEMI, "Expect ';' after expression");
    *node = ast_new_expr_stmt(*node);
}

static void parse_try_statement(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_LBRACE, "Expect '{' after 'try'");
    
    ASTNode* try_block = NULL;
    parse_block(parser, &try_block);
    
    DynamicArray catch_blocks = da_new(sizeof(ASTNode*), 4);
    ASTNode* finally_block = NULL;
    
    while (match(parser, TOKEN_CATCH)) {
        consume(parser, TOKEN_LPAREN, "Expect '(' after 'catch'");
        consume(parser, TOKEN_IDENT, "Expect error type");
        Token error_type = parser->previous;
        consume(parser, TOKEN_IDENT, "Expect error variable name");
        Token error_var = parser->previous;
        consume(parser, TOKEN_RPAREN, "Expect ')' after catch parameters");
        
        consume(parser, TOKEN_LBRACE, "Expect '{' after catch parameters");
        ASTNode* catch_block = NULL;
        parse_block(parser, &catch_block);
        
        // Create catch block node
        ASTNode* catch_node = ast_new_catch_block(error_type, error_var, catch_block);
        da_append(&catch_blocks, &catch_node);
    }
    
    if (match(parser, TOKEN_FINALLY)) {
        consume(parser, TOKEN_LBRACE, "Expect '{' after 'finally'");
        parse_block(parser, &finally_block);
    }
    
    *node = ast_new_try_stmt(try_block, catch_blocks, finally_block);
}

static void parse_throw_statement(Parser* parser, ASTNode** node) {
    ASTNode* value = NULL;
    parse_expression(parser, &value, false);
    consume(parser, TOKEN_SEMI, "Expect ';' after throw expression");
    *node = ast_new_throw_stmt(value);
}

static void parse_go_statement(Parser* parser, ASTNode** node) {
    ASTNode* expr = NULL;
    if (match(parser, TOKEN_LBRACE)) {
        parse_block(parser, &expr);
    } else {
        parse_expression(parser, &expr, false);
    }
    *node = ast_new_go_stmt(expr);
}

static void parse_chan_declaration(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_LT, "Expect '<' after 'chan'");
    ASTNode* type = NULL;
    parse_expression(parser, &type, false);
    consume(parser, TOKEN_GT, "Expect '>' after channel type");
    
    consume(parser, TOKEN_IDENT, "Expect channel name");
    Token name = parser->previous;
    
    ASTNode* capacity = NULL;
    if (match(parser, TOKEN_LPAREN)) {
        parse_expression(parser, &capacity, false);
        consume(parser, TOKEN_RPAREN, "Expect ')' after channel capacity");
    }
    
    consume(parser, TOKEN_SEMI, "Expect ';' after channel declaration");
    *node = ast_new_chan_decl(name, type, capacity);
}

static void parse_select_statement(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_LBRACE, "Expect '{' after 'select'");
    
    DynamicArray cases = da_new(sizeof(ASTNode*), 8);
    ASTNode* default_case = NULL;
    
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        if (match(parser, TOKEN_CASE)) {
            ASTNode* channel = NULL;
            ASTNode* value = NULL;
            bool is_send = false;
            
            parse_expression(parser, &channel, false);
            
            if (match(parser, TOKEN_ARROW)) {
                // Receive case: chan <- value
                parse_expression(parser, &value, false);
            } else if (match(parser, TOKEN_LT)) {
                consume(parser, TOKEN_MINUS, "Expect '-' after '<' in send operation");
                // Send case: chan <- value
                is_send = true;
                parse_expression(parser, &value, false);
            }
            
            consume(parser, TOKEN_LBRACE, "Expect '{' after select case");
            ASTNode* body = NULL;
            parse_block(parser, &body);
            
            ASTNode* case_node = ast_new_select_case(channel, value, is_send, body);
            da_append(&cases, &case_node);
        } else if (match(parser, TOKEN_DEFAULT)) {
            consume(parser, TOKEN_LBRACE, "Expect '{' after 'default'");
            parse_block(parser, &default_case);
        } else {
            error_at_current(parser, "Expect 'case' or 'default' in select statement");
            break;
        }
    }
    
    consume(parser, TOKEN_RBRACE, "Expect '}' after select cases");
    *node = ast_new_select_stmt(cases, default_case);
}

static void parse_async_expression(Parser* parser, ASTNode** node) {
    ASTNode* expr = NULL;
    if (match(parser, TOKEN_LBRACE)) {
        parse_block(parser, &expr);
    } else {
        parse_expression(parser, &expr, false);
    }
    *node = ast_new_async_expr(expr);
}

static void parse_await_expression(Parser* parser, ASTNode** node) {
    ASTNode* expr = NULL;
    parse_expression(parser, &expr, false);
    *node = ast_new_await_expr(expr);
}

static void parse_chan_operation(Parser* parser, ASTNode** node, bool can_assign) {
    (void)can_assign;
    ASTNode* channel = NULL;
    ASTNode* value = NULL;
    bool is_send = false;
    
    if (match(parser, TOKEN_LT)) {
        consume(parser, TOKEN_MINUS, "Expect '-' after '<' in channel operation");
        if (parser->previous.type == TOKEN_IDENT) {
            // Send operation: ch <- value
            channel = *node;
            is_send = true;
            parse_expression(parser, &value, false);
            *node = ast_new_chan_send_expr(channel, value);
        } else {
            // Receive operation: <- ch
            parse_expression(parser, &channel, false);
            *node = ast_new_chan_recv_expr(channel);
        }
    }
}

static void parse_statement(Parser* parser, ASTNode** node) {
    if (match(parser, TOKEN_IF)) {
        parse_if_statement(parser, node);
    } else if (match(parser, TOKEN_WHILE)) {
        parse_while_statement(parser, node);
    } else if (match(parser, TOKEN_FOR)) {
        parse_for_statement(parser, node);
    } else if (match(parser, TOKEN_RETURN)) {
        parse_return_statement(parser, node);
    } else if (match(parser, TOKEN_TRY)) {
        parse_try_statement(parser, node);
    } else if (match(parser, TOKEN_THROW)) {
        parse_throw_statement(parser, node);
    } else if (match(parser, TOKEN_GO)) {
        parse_go_statement(parser, node);
    } else if (match(parser, TOKEN_SELECT)) {
        parse_select_statement(parser, node);
    } else if (match(parser, TOKEN_CHAN)) {
        parse_chan_declaration(parser, node);
    } else if (match(parser, TOKEN_LBRACE)) {
        parse_block(parser, node);
    } else {
        parse_expression_statement(parser, node);
    }
}

static void parse_type_params(Parser* parser, DynamicArray* type_params) {
    if (match(parser, TOKEN_LT)) {
        do {
            consume(parser, TOKEN_IDENT, "Expect type parameter name");
            Token param = parser->previous;
            da_append(type_params, &param);
        } while (match(parser, TOKEN_COMMA));
        consume(parser, TOKEN_GT, "Expect '>' after type parameters");
    }
}

static void parse_type_declaration(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_IDENT, "Expect type name");
    Token name = parser->previous;
    
    DynamicArray type_params = da_new(sizeof(Token), 4);
    parse_type_params(parser, &type_params);
    
    consume(parser, TOKEN_EQ, "Expect '=' after type name");
    
    ASTNode* type = NULL;
    parse_expression(parser, &type, false);
    
    consume(parser, TOKEN_SEMI, "Expect ';' after type declaration");
    
    *node = ast_new_type_decl(name, type_params, type);
}

static void parse_interface_declaration(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_IDENT, "Expect interface name");
    Token name = parser->previous;
    
    DynamicArray type_params = da_new(sizeof(Token), 4);
    parse_type_params(parser, &type_params);
    
    consume(parser, TOKEN_LBRACE, "Expect '{' before interface body");
    
    DynamicArray methods = da_new(sizeof(ASTNode*), 8);
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        ASTNode* method = NULL;
        parse_function(parser, &method);
        da_append(&methods, &method);
    }
    
    consume(parser, TOKEN_RBRACE, "Expect '}' after interface body");
    
    *node = ast_new_interface_decl(name, type_params, methods);
}

static void parse_trait_declaration(Parser* parser, ASTNode** node) {
    consume(parser, TOKEN_IDENT, "Expect trait name");
    Token name = parser->previous;
    
    DynamicArray type_params = da_new(sizeof(Token), 4);
    parse_type_params(parser, &type_params);
    
    consume(parser, TOKEN_LBRACE, "Expect '{' before trait body");
    
    DynamicArray methods = da_new(sizeof(ASTNode*), 8);
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        ASTNode* method = NULL;
        parse_function(parser, &method);
        da_append(&methods, &method);
    }
    
    consume(parser, TOKEN_RBRACE, "Expect '}' after trait body");
    
    *node = ast_new_trait_decl(name, type_params, methods);
}

static void parse_impl_declaration(Parser* parser, ASTNode** node) {
    ASTNode* type = NULL;
    parse_expression(parser, &type, false);
    
    consume(parser, TOKEN_FOR, "Expect 'for' after type in impl");
    consume(parser, TOKEN_IDENT, "Expect trait name");
    Token trait = parser->previous;
    
    consume(parser, TOKEN_LBRACE, "Expect '{' before impl body");
    
    DynamicArray methods = da_new(sizeof(ASTNode*), 8);
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        ASTNode* method = NULL;
        parse_function(parser, &method);
        da_append(&methods, &method);
    }
    
    consume(parser, TOKEN_RBRACE, "Expect '}' after impl body");
    
    *node = ast_new_impl_decl(type, trait, methods);
}

static void parse_declaration(Parser* parser, ASTNode** node) {
    if (match(parser, TOKEN_LET)) {
        parse_var_declaration(parser, node);
    } else if (match(parser, TOKEN_FN)) {
        parse_function(parser, node);
    } else if (match(parser, TOKEN_TYPE)) {
        parse_type_declaration(parser, node);
    } else if (match(parser, TOKEN_INTERFACE)) {
        parse_interface_declaration(parser, node);
    } else if (match(parser, TOKEN_TRAIT)) {
        parse_trait_declaration(parser, node);
    } else if (match(parser, TOKEN_IMPL)) {
        parse_impl_declaration(parser, node);
    } else {
        parse_statement(parser, node);
    }

    if (parser->panic_mode) synchronize(parser);
}

static void parse_expression(Parser* parser, ASTNode** node, bool can_assign) {
    parse_precedence(parser, PREC_ASSIGNMENT, node, can_assign);
}

static void synchronize(Parser* parser) {
    parser->panic_mode = false;

    while (parser->current.type != TOKEN_EOF) {
        if (parser->previous.type == TOKEN_SEMI) return;

        switch (parser->current.type) {
            case TOKEN_FN:
            case TOKEN_LET:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_FOR:
            case TOKEN_RETURN:
                return;
            default:
                ; // Do nothing
        }

        advance(parser);
    }
}

// Rule table
static ParseRule rules[] = {
    [TOKEN_LPAREN]    = {parse_grouping, parse_call,   PREC_CALL},
    [TOKEN_RPAREN]    = {NULL,          NULL,          PREC_NONE},
    [TOKEN_LBRACE]    = {NULL,          NULL,          PREC_NONE},
    [TOKEN_RBRACE]    = {NULL,          NULL,          PREC_NONE},
    [TOKEN_COMMA]     = {NULL,          NULL,          PREC_NONE},
    [TOKEN_DOT]       = {NULL,          parse_dot,     PREC_CALL},
    [TOKEN_MINUS]     = {parse_unary,   parse_binary,  PREC_TERM},
    [TOKEN_PLUS]      = {NULL,          parse_binary,  PREC_TERM},
    [TOKEN_SEMI]      = {NULL,          NULL,          PREC_NONE},
    [TOKEN_SLASH]     = {NULL,          parse_binary,  PREC_FACTOR},
    [TOKEN_STAR]      = {NULL,          parse_binary,  PREC_FACTOR},
    [TOKEN_BANG]      = {parse_unary,   NULL,          PREC_NONE},
    [TOKEN_BANG_EQ]   = {NULL,          parse_binary,  PREC_EQUALITY},
    [TOKEN_EQ]        = {NULL,          NULL,          PREC_NONE},
    [TOKEN_EQEQ]      = {NULL,          parse_binary,  PREC_EQUALITY},
    [TOKEN_GT]        = {NULL,          parse_binary,  PREC_COMPARISON},
    [TOKEN_GTEQ]      = {NULL,          parse_binary,  PREC_COMPARISON},
    [TOKEN_LT]        = {NULL,          parse_chan_operation, PREC_CALL},
    [TOKEN_LTEQ]      = {NULL,          parse_binary,  PREC_COMPARISON},
    [TOKEN_IDENT]     = {parse_identifier, NULL,          PREC_NONE},
    [TOKEN_STRING]    = {parse_string,  NULL,          PREC_NONE},
    [TOKEN_INT]       = {parse_number,  NULL,          PREC_NONE},
    [TOKEN_AMPAMP]    = {NULL,          parse_and,     PREC_AND},
    [TOKEN_PIPEPIPE]  = {NULL,          parse_or,      PREC_OR},
    [TOKEN_IF]        = {NULL,          NULL,          PREC_NONE},
    [TOKEN_ELSE]      = {NULL,          NULL,          PREC_NONE},
    [TOKEN_TRUE]      = {parse_literal, NULL,          PREC_NONE},
    [TOKEN_FALSE]     = {parse_literal, NULL,          PREC_NONE},
    [TOKEN_NIL]       = {parse_literal, NULL,          PREC_NONE},
    [TOKEN_RETURN]    = {NULL,          NULL,          PREC_NONE},
    [TOKEN_ERROR]     = {NULL,          NULL,          PREC_NONE},
    [TOKEN_EOF]       = {NULL,          NULL,          PREC_NONE},
    [TOKEN_DEFAULT]   = {NULL,          NULL,          PREC_NONE},
    [TOKEN_MATCH]     = {NULL,          NULL,          PREC_NONE},
    [TOKEN_CASE]      = {NULL,          NULL,          PREC_NONE},
    [TOKEN_GO]        = {NULL,          NULL,          PREC_NONE},
    [TOKEN_CHAN]      = {NULL,          NULL,          PREC_NONE},
    [TOKEN_SELECT]    = {NULL,          NULL,          PREC_NONE},
    [TOKEN_DEFER]     = {NULL,          NULL,          PREC_NONE},
    [TOKEN_ASYNC]     = {NULL,          NULL,          PREC_NONE},
    [TOKEN_AWAIT]     = {NULL,          NULL,          PREC_NONE},
    [TOKEN_ALLOC]     = {NULL,          NULL,          PREC_NONE},
    [TOKEN_FREE]      = {NULL,          NULL,          PREC_NONE},
    [TOKEN_REF]       = {NULL,          NULL,          PREC_NONE},
    [TOKEN_DEREF]     = {NULL,          NULL,          PREC_NONE},
    [TOKEN_MOVE]      = {NULL,          NULL,          PREC_NONE},
    [TOKEN_COPY]      = {NULL,          NULL,          PREC_NONE},
    [TOKEN_SELF]      = {NULL,          NULL,          PREC_NONE},
    [TOKEN_SUPER]     = {NULL,          NULL,          PREC_NONE},
};

static ParseRule* get_rule(TokenType type) {
    return &rules[type];
}

void parser_init(Parser* parser, Lexer* lexer, const char* filename) {
    parser->lexer = lexer;
    parser->filename = filename;
    parser->had_error = false;
    parser->panic_mode = false;
    advance(parser);
}

ASTNode* parse(Parser* parser) {
    ASTNode* node = NULL;
    parse_declaration(parser, &node);
    return node;
}