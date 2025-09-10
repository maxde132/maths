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

	Expr *expr = parse(expression);
	if (FLAG_IS_SET(DEBUG)) print_exprh(expr);

	TypedValue val = eval_expr(&eval_state, expr);
	if (FLAG_IS_SET(PRINT))
		print_typedval(&val);

	free_expr(&expr);
	cleanup_evaluator(&eval_state);

	return 0;
}
