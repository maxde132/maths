#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>

#include "parser.h"
#include "eval.h"
#include "config.h"

extern strbuf expression;

void sig_handler(int32_t signum)
{
	eval_cleanup(&global_config.eval_state);
	fprintf(stderr, "terminated with signal %d\n", signum);
	exit(2);
}

int32_t main(int32_t argc, char **argv)
{
	signal(SIGINT, sig_handler);
	eval_init(&global_config.eval_state);
	parse_args(argc, argv);

	if (FLAG_IS_SET(READ_STDIN))
		expression = read_string_from_stream(stdin);

	//Expr *expr = parse(expression.s);
	//eval_push_expr(&eval_state, expr);
	parse_stmts_to_evaluator(expression.s, &global_config.eval_state);
	if (FLAG_IS_SET(DEBUG)) println_vec(&global_config.eval_state.exprs);

	if (!FLAG_IS_SET(NO_EVAL))
	{
		for (size_t i = 0; i < global_config.eval_state.exprs.n; ++i)
		{
			TypedValue val = eval_expr(
					&global_config.eval_state,
					global_config.eval_state.exprs.ptr[i]);
			if (i == global_config.eval_state.exprs.n-1 && FLAG_IS_SET(PRINT))
				print_typedval(&global_config.eval_state, &val);
		}
	}

	if (expression.allocd)
		free(expression.s);
	eval_cleanup(&global_config.eval_state);

	return 0;
}
