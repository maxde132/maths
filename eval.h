#ifndef EVAL_H
#define EVAL_H

#include "config.h"
#include "expr.h"
#include "c-hashmap/map.h"

typedef struct UserVarStack {
	Expr **ptr;
	size_t in_use;
	size_t allocd_size;
} UserVarStack;

struct evaluator_state {
	hashmap *builtins;
	hashmap *variables;
	UserVarStack user_vars;
	bool is_init;
};

void init_evaluator(struct evaluator_state *state);
void cleanup_evaluator(struct evaluator_state *state);
int32_t set_variable(struct evaluator_state *state, strbuf name, size_t index);
TypedValue eval_expr(struct evaluator_state *state, const Expr *expr);

#endif /* EVAL_H */
