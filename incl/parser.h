#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "expr.h"
#include "eval.h"

struct parser_state {
	const char *saved_s;
	Token peeked_tok;
	bool has_peeked;
	bool looking_for_int;
};

Token get_next_token(const char **s, struct parser_state *state);
[[nodiscard("why are you peeking if you aren't checking the return value?")]]
Token peek_token(const char **s, struct parser_state *state);
[[nodiscard("you really need to make sure you keep this `Expr *`, "
		"otherwise severe memory leaks will ensue...")]]
Expr *parse(const char *s);

VecN parse_stmts(const char *s);
void parse_stmts_to_evaluator(const char *s, struct evaluator_state *eval_state);

#endif /* PARSER_H */
