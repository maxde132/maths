#include <stdint.h>

#include "eval.h"
#include "parser.h"

int32_t main(void)
{
	struct evaluator_state __attribute__((cleanup(eval_cleanup)))
		state = eval_init(NULL);
	parse_stmts_to_evaluator("println{[5,2i].1^2}", &state);
	
	for (size_t i = 0; i < state.exprs.n; ++i)
		eval_expr(&state, state.exprs.ptr[i]);

	return 0;
}
