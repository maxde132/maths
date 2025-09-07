#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "eval.h"

int32_t main(int32_t argc, char **argv)
{
	const char *str = "0.5 + 10 % 4";
	if (argc > 1)
		str = argv[1];

	Expr *expr = parse(str);
	print_exprh(expr);
	printf("ret = %.6f\n", eval_expr(expr));
	free_expr(expr);

	return 0;
}
