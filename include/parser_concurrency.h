#ifndef FERRUM_PARSER_CONCURRENCY_H
#define FERRUM_PARSER_CONCURRENCY_H

#include "parser.h"
#include "ast.h"

// Parse a goroutine statement (go expression or go block)
void parse_go_statement(Parser* parser, ASTNode** node);

// Parse a channel declaration (chan<T> name or chan<T> name(capacity))
void parse_chan_declaration(Parser* parser, ASTNode** node);

// Parse a select statement with multiple channel operations
void parse_select_statement(Parser* parser, ASTNode** node);

// Parse channel operations (send/receive)
void parse_chan_operation(Parser* parser, ASTNode** node, bool can_assign);

#endif // FERRUM_PARSER_CONCURRENCY_H 