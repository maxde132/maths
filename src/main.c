#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>

#include "mml/mml.h"
#include "mml/config.h"
#include "mml/prompt.h"
#include "dvec/dvec.h"

extern strbuf expression;

void sig_handler(int32_t signum)
{
	MML_cleanup_state(MML_global_config.eval_state);
	MML_term_restore();
	fprintf(stderr, "\n\nterminated with signal %d\n", signum);
	exit(2);
}

int32_t main(int32_t argc, char **argv)
{
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGQUIT, sig_handler);

	MML_global_config.eval_state = MML_init_state();
	MML_arg_parse(argc, argv);

	if (FLAG_IS_SET(RUN_PROMPT))
	{
		MML_run_prompt(MML_global_config.eval_state);
		MML_cleanup_state(MML_global_config.eval_state);
		return 0;
	}

	if (FLAG_IS_SET(READ_STDIN))
		expression = MML_read_string_from_stream(stdin);

	//Expr *expr = parse(expression.s);
	//eval_push_expr(&eval_state, expr);
	MML_parse_stmts(expression.s, MML_global_config.eval_state);
	if (FLAG_IS_SET(DEBUG)) MML_println_vec(&MML_global_config.eval_state->exprs);

	if (!FLAG_IS_SET(NO_EVAL))
	{
		for (size_t i = 0; i < dv_n(MML_global_config.eval_state->exprs); ++i)
		{
			MML_Value val = MML_eval_expr(
					MML_global_config.eval_state,
					dv_a(MML_global_config.eval_state->exprs, i));

			if (i == dv_n(MML_global_config.eval_state->exprs)-1 && FLAG_IS_SET(PRINT))
				MML_print_typedval(MML_global_config.eval_state, &val);
		}
	}

	if (expression.allocd)
		free(expression.s);
	MML_cleanup_state(MML_global_config.eval_state);

	return 0;
}
