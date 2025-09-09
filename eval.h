#ifndef EVAL_H
#define EVAL_H

#include "config.h"
#include "expr.h"
#include "c-hashmap/map.h"

struct evaluator_state {
	hashmap *builtins;
	hashmap *variables;
	bool is_init;
};

void init_evaluator(struct evaluator_state *state);
void cleanup_evaluator(struct evaluator_state *state);
int32_t set_variable(struct evaluator_state *state, strbuf name, Expr *expr);
TypedValue eval_expr(struct evaluator_state *state, const Expr *expr);

#endif /* EVAL_H */
