#ifndef EVAL_H
#define EVAL_H

#include "expr.h"
#include "c-hashmap/map.h"

typedef struct UserVarStack {
	const Expr **ptr;
	size_t in_use;
	size_t allocd_size;
} UserVarStack;

struct evaluator_state {
	hashmap *builtins;
	hashmap *variables;
	UserVarStack user_vars;
	bool is_init;
};

struct evaluator_state init_evaluator(struct evaluator_state *state_out);
void cleanup_evaluator(struct evaluator_state *state);
int32_t set_variable(struct evaluator_state *state,
		strbuf name, const Expr *val);
TypedValue eval_expr(struct evaluator_state *state,
		const Expr *expr);

#endif /* EVAL_H */
