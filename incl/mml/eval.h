#ifndef EVAL_H
#define EVAL_H

#include "expr.h"

typedef struct hashmap hashmap;

typedef struct MML_state {
	hashmap *variables;
	MML_VecN vars_storage;
	MML_VecN exprs;
	MML_VecN allocd_vecs;
	bool is_init;
} MML_state;

typedef MML_Value (*MML_val_func)(MML_state *restrict state, MML_VecN *args);

#ifndef MML_BARE_USE
MML_Value MML_apply_binary_op(MML_state *restrict state,
		MML_Value a, MML_Value b, MML_TokenType op);
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
int32_t MML_eval_set_variable(MML_state *restrict state, strbuf name, MML_Expr *expr);

/* evaluates EXPR using the evaluator state data in STATE */
MML_Value MML_eval_expr(MML_state *restrict state, const MML_Expr *expr);

/* push-moves EXPR onto the expression storage for STATE */
int32_t MML_eval_push_expr(MML_state *restrict state, MML_Expr *expr);
/* evaluates the last expression in the expression storage for STATE */
MML_Value MML_eval_top_expr(MML_state *restrict state);

#endif /* EVAL_H */
