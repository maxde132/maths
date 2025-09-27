#ifndef EVAL_H
#define EVAL_H

#include "config.h"
#include "expr.h"

typedef struct hashmap hashmap;

typedef struct MML_state {
	struct MML_config *config;

	hashmap *variables;

	MML_expr_vec exprs;

	MML_expr_vec vars_storage;
	MML_expr_vec allocd_vecs;

	MML_value last_val;
	bool is_init;
} MML_state;

typedef MML_value (*MML_val_func)(MML_state *restrict state, MML_expr_vec *args);

#ifndef MML_BARE_USE
MML_value MML_apply_binary_op(MML_state *restrict state,
		MML_value a, MML_value b, MML_token_type op);
#endif


/* Returns a pointer to a valid, initialized evaluator state, which should be
 * passed to any function that takes `MML_state *` as an argument.
 * Additionally, if STATE_OUT is not NULL, the state is written to that pointer
 * as well.
 * `MML_cleanup_state` must be called on the output of this function when you are
 * done with it. */
MML_state *MML_init_state(void);
/* Cleans up allocations and such stored in an evaluator state at STATE.
 * STATE must have been obtained by a call to `MML_init_state`. See `MML_init_state` for
 * more details. */
void MML_cleanup_state(MML_state *restrict state);
/* push-moves EXPR into the variable storage for STATE with the identifier in NAME.
 * IS_INSERTED determines whether the variable will be moved to the user-defined or
 * inserted variable namespace. */
int32_t MML_eval_set_variable(MML_state *restrict state, strbuf name, MML_expr *expr);
MML_expr *MML_eval_get_variable(MML_state *restrict state, strbuf name);

/* evaluates EXPR using the evaluator state data in STATE */
MML_value MML_eval_expr(MML_state *restrict state, const MML_expr *expr);
MML_value MML_eval_expr_recurse(MML_state *restrict state, const MML_expr *expr);

MML_value MML_eval_parse(MML_state *state, const char *s);

/* push-moves EXPR onto the expression storage for STATE */
int32_t MML_eval_push_expr(MML_state *restrict state, MML_expr *expr);
/* evaluates the last expression in the expression storage for STATE */
MML_value MML_eval_top_expr(MML_state *restrict state);

MML_value MML_eval_last_val(MML_state *restrict state);

#endif /* EVAL_H */
