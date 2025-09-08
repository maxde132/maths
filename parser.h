#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "expr.h"

struct parser_state {
	const char *saved_s;
	Token peeked_tok;
	bool has_peeked;
	bool looking_for_int;
};

Token get_next_token(const char **s, struct parser_state *state);
Token peek_token(const char **s, struct parser_state *state);
Expr *parse(const char *s);

#endif /* PARSER_H */
