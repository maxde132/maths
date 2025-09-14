#ifndef EVAL_H
#define EVAL_H

#include "expr.h"
#include "c-hashmap/map.h"

struct evaluator_state {
	hashmap *variables;
	hashmap *inserted_vars;
	VecN vars_storage;
	VecN exprs;
	VecN allocd_vecs;
	bool is_init;
};

typedef TypedValue (*MML_val_func)(struct evaluator_state *state, VecN *args);

TypedValue apply_binary_op(struct evaluator_state *restrict state,
		TypedValue a, TypedValue b, TokenType op);

/* Returns a valid, initialized evaluator state, a pointer to which should be
 * passed to any function that takes `struct evaluator_state *` as an argument.
 * Additionally, if STATE_OUT is not NULL, the state is written to that pointer
 * as well.
 * `eval_cleanup` must be called on the output of this function when you are
 * done with it. */
struct evaluator_state eval_init(struct evaluator_state *state_out);
/* Cleans up allocations and such stored in an evaluator state at STATE.
 * STATE must have been obtained by a call to `eval_init`. See `eval_init` for
 * more details. */
void eval_cleanup(struct evaluator_state *state);
/* push-moves EXPR into the variable storage for STATE with the identifier in NAME.
 * IS_INSERTED determines whether the variable will be moved to the user-defined or
 * inserted variable namespace. */
int32_t eval_set_variable(struct evaluator_state *state, strbuf name, Expr *expr, bool is_inserted);

/* evaluates EXPR using the evaluator state data in STATE */
TypedValue eval_expr(struct evaluator_state *state, const Expr *expr);

/* push-moves EXPR onto the expression storage for STATE */
int32_t eval_push_expr(struct evaluator_state *state, Expr *expr);
/* evaluates the last expression in the expression storage for STATE */
TypedValue eval_top_expr(struct evaluator_state *state);

#endif /* EVAL_H */
