#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>

#include "parser.h"
#include "eval.h"
#include "config.h"

extern strbuf expression;
struct evaluator_state eval_state = {0};

void sig_handler(int32_t signum)
{
	eval_cleanup(&eval_state);
	fprintf(stderr, "terminated with signal %d\n", signum);
	exit(2);
}

int32_t main(int32_t argc, char **argv)
{
	signal(SIGINT, sig_handler);
	eval_init(&eval_state);
	parse_args(argc, argv);

	if (FLAG_IS_SET(READ_STDIN))
		expression = read_string_from_stream(stdin);

	Expr *expr = parse(expression.s);
	eval_push_expr(&eval_state, expr);
	if (FLAG_IS_SET(DEBUG)) print_exprh(expr);

	if (!FLAG_IS_SET(NO_EVAL))
	{
		TypedValue val = eval_top_expr(&eval_state);
		if (FLAG_IS_SET(PRINT))
			print_typedval(&val);
	}

	if (expression.allocd)
		free(expression.s);
	eval_cleanup(&eval_state);

	return 0;
}
