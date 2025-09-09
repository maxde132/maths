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

	const Expr *exprs = parse(expression);
	if (FLAG_IS_SET(DEBUG)) print_exprh(exprs);

	TypedValue val;
	for (size_t i = 0; i < exprs->u.v.v.n; ++i)
	{
		val = eval_expr(&eval_state, exprs->u.v.v.ptr[i]);
		if (i == exprs->u.v.v.n - 1)
			if (FLAG_IS_SET(PRINT))
				print_typedval(&val);
	}
	free_expr((Expr *)exprs);
	cleanup_evaluator(&eval_state);

	return 0;
}
