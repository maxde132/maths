#ifndef EVAL_H
#define EVAL_H

#include "mml/config.h"
#include "mml/expr.h"
#include "arena/arena.h"

extern Arena *MML_global_arena;
#define mml_i(_I, _T) (arena_i(MML_global_arena, (_I), _T))
#define mml_e(_I) (mml_i((_I), MML_expr))
#define mml_ii(_I, _T, _i) (arena_i(MML_global_arena, (_I), _T) + (_i))
#define mml_vi(_v, _i) (mml_ii((_v).i, arena_index, (_i)))
#define mml_ei(_e, _i) (mml_vi((_e).v, (_i)))
#define mml_p_i(_p) ((uint8_t *)(_p) - MML_global_arena->base)

typedef struct hashmap hashmap;

typedef struct MML_state {
	struct MML_config *config;

	hashmap *variables;

	dvec_t(arena_index) exprs;

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

int32_t MML_eval_set_variable(MML_state *restrict state, strbuf name, arena_index expr);
arena_index MML_eval_get_variable(MML_state *restrict state, strbuf name);

/* evaluates EXPR using the evaluator state data in STATE */
MML_value MML_eval_expr(MML_state *restrict state, arena_index expr);
MML_value MML_eval_expr_recurse(MML_state *restrict state, arena_index expr);

MML_value MML_eval_parse(MML_state *state, const char *s);

/* push-moves EXPR onto the expression storage for STATE */
int32_t MML_eval_push_expr(MML_state *restrict state, arena_index expr);
/* evaluates the last expression in the expression storage for STATE */
MML_value MML_eval_top_expr(MML_state *restrict state);

MML_value MML_eval_last_val(MML_state *restrict state);

#endif /* EVAL_H */
