#ifndef EVAL_H
#define EVAL_H

#include "expr.h"

typedef struct hashmap hashmap;

struct MML_state {
	hashmap *variables;
	hashmap *inserted_vars;
	MML_VecN vars_storage;
	MML_VecN exprs;
	MML_VecN allocd_vecs;
	bool is_init;
};

typedef MML_Value (*MML_val_func)(struct MML_state *state, MML_VecN *args);

#ifndef MML_BARE_USE
MML_Value MML_apply_binary_op(struct MML_state *state,
		MML_Value a, MML_Value b, MML_TokenType op);
#endif

/* Returns a valid, initialized evaluator state, a pointer to which should be
 * passed to any function that takes `struct evaluator_state *` as an argument.
 * Additionally, if STATE_OUT is not NULL, the state is written to that pointer
 * as well.
 * `eval_cleanup` must be called on the output of this function when you are
 * done with it. */
struct MML_state MML_init_state(struct MML_state *state_out);
/* Cleans up allocations and such stored in an evaluator state at STATE.
 * STATE must have been obtained by a call to `eval_init`. See `eval_init` for
 * more details. */
void MML_cleanup_state(struct MML_state *state);
/* push-moves EXPR into the variable storage for STATE with the identifier in NAME.
 * IS_INSERTED determines whether the variable will be moved to the user-defined or
 * inserted variable namespace. */
int32_t MML_eval_set_variable(struct MML_state *state, strbuf name, MML_Expr *expr, bool is_inserted);

/* evaluates EXPR using the evaluator state data in STATE */
MML_Value MML_eval_expr(struct MML_state *state, const MML_Expr *expr);

/* push-moves EXPR onto the expression storage for STATE */
int32_t MML_eval_push_expr(struct MML_state *state, MML_Expr *expr);
/* evaluates the last expression in the expression storage for STATE */
MML_Value MML_eval_top_expr(struct MML_state *state);

#endif /* EVAL_H */
