#include "mml/prompt.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "mml/config.h"
#include "mml/expr.h"
#include "mml/eval.h"
#include "mml/parser.h"

size_t fgetsn(char *restrict out, size_t size, FILE *stream)
{
	if (out == NULL || size == 0) return 0;

	char *p = out;
	int c;
	while ((size_t)(p - out) < size-1 && (c = fgetc(stream)) != EOF && c != '\n')
		*p++ = c;

	if (c == '\n' && (size_t)(p - out) < size-1)
		*p++ = '\n';

	*p++ = '\0';

	return p - out;
}

constexpr size_t LINE_MAX = 4096;

void MML_run_prompt(MML_state *state)
{
	char line_in[LINE_MAX+1] = {0};

	MML_Value cur_val = VAL_INVAL;
	size_t expr_n_offset = 0;
	printf("-- MML interactive prompt --\n");
	printf("run `exit` to quit prompt.\n");
	while (!(cur_val.type == Invalid_type && cur_val.v.i == MML_QUIT_INVAL))
	{
		printf("==> ");
		size_t n = fgetsn(line_in, LINE_MAX+1, stdin);
		if (line_in[n-2] != '\n')
			fprintf(stderr, "warning: input longer than %zu "
					"characters was truncated\n", LINE_MAX);
		line_in[n-2] = '\0';

		MML_parse_stmts(line_in, state);

		while (expr_n_offset < state->exprs.n)
		{
			MML_Expr *cur = state->exprs.ptr[expr_n_offset++];
			if (cur != nullptr)
				cur_val = MML_eval_expr(state, cur);
		}

		if (!MML_global_config.last_print_was_newline)
			puts("\033[7m%\033[0m");
		MML_global_config.last_print_was_newline = true;

		if (cur_val.type != Invalid_type)
			MML_println_typedval(state, &cur_val);
		else
		{
			switch (cur_val.v.i) {
			case MML_CLEAR_INVAL:
				printf("\033[2J\033[;;f");
				break;
			default:
				break;
			}
		}
	}
}
