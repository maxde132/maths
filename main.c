#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>

#include "parser.h"
#include "eval.h"
#include "config.h"

extern char *expression;
struct evaluator_state eval_state;

int32_t main(int32_t argc, char **argv)
{
	init_evaluator(&eval_state);
	parse_args(argc, argv);

	if (FLAG_IS_SET(READ_STDIN))
		expression = read_string_from_stream(stdin).s;

	parse_into(expression, &eval_state);
	if (FLAG_IS_SET(DEBUG)) print_exprh(eval_state.expr_to_eval);

	TypedValue val;
	for (size_t i = 0; i < eval_state.expr_to_eval->u.v.v.n; ++i)
	{
		val = eval_stmt_n(&eval_state, i);
		if (i == eval_state.expr_to_eval->u.v.v.n - 1)
			if (FLAG_IS_SET(PRINT))
				print_typedval(&val);
	}
	cleanup_evaluator(&eval_state);

	return 0;
}
