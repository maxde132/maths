#include "mml/prompt.h"

#include <stdint.h>

#include "mml/mml.h"

void MML_run_prompt(struct MML_state *state)
{
	MML_parse_stmts("println{[5,2i].1^2}", state);
	
	for (size_t i = 0; i < state->exprs.n; ++i)
		MML_eval_expr(state, state->exprs.ptr[i]);
}
