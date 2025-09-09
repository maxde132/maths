#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "expr.h"

struct parser_state {
	const char *saved_s;
	Token peeked_tok;
	bool has_peeked;
	bool looking_for_int;
	bool end_parse_current_expr;
};

Token get_next_token(const char **s, struct parser_state *state);
[[nodiscard("why are you peeking if you aren't checking the return value?")]]
Token peek_token(const char **s, struct parser_state *state);
[[nodiscard("you really need to make sure you keep this `Expr *`, "
		"otherwise severe memory leaks will ensue...")]]
/* returns a VecN wrapped as Expr,
 * representing the vector of statements
 * parsed by the parser. */
const Expr *parse(const char *s);

#endif /* PARSER_H */
