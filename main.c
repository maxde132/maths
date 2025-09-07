#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "eval.h"
#include "config.h"

extern char *expression;

int32_t main(int32_t argc, char **argv)
{
	parse_args(argc, argv);

	if (FLAG_IS_SET(READ_STDIN))
		expression = read_string_from_stream(stdin).s;

	Expr *expr = parse(expression);
	if (FLAG_IS_SET(DEBUG)) print_exprh(expr);

	init_evaluator();
	double val = eval_expr(expr);
	if (FLAG_IS_SET(PRINT))
		printf("%.*f", global_config.precision, val);

	free_expr(expr);

	return 0;
}
