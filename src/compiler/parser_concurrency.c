#include "parser_concurrency.h"
#include "common.h"
#include "ast.h"
#include "lexer.h"

// Parse a goroutine statement (go expression or go block)
void parse_go_statement(Parser* parser, ASTNode** node) {
    ASTNode* expr = NULL;
    if (match(parser, TOKEN_LBRACE)) {
        parse_block(parser, &expr);
    } else {
        parse_expression(parser, &expr, false);
    }
    *node = ast_new_go_stmt(expr);
}

// Parse a channel declaration (chan<T> name or chan<T> name(capacity))
void parse_chan_declaration(Parser* parser, ASTNode** node) {
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

// Parse a select statement with multiple channel operations
void parse_select_statement(Parser* parser, ASTNode** node) {
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

// Parse channel operations (send/receive)
void parse_chan_operation(Parser* parser, ASTNode** node, bool can_assign) {
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