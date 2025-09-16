#include "mml/prompt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

#include "mml/config.h"
#include "mml/expr.h"
#include "mml/eval.h"
#include "mml/parser.h"

enum KEY_CODES {
	KC_UP_ARROW		= 0x0000'0000'0041'5b1b,
	KC_DOWN_ARROW	= 0x0000'0000'0042'5b1b,
	KC_LEFT_ARROW	= 0x0000'0000'0044'5b1b,
	KC_RIGHT_ARROW	= 0x0000'0000'0043'5b1b,
	KC_ALT_UP_ARROW	= 0x0000'4133'3b31'5b1b,
	KC_ALT_DOWN_ARROW	= 0x0000'4233'3b31'5b1b,
	KC_ALT_LEFT_ARROW	= 0x0000'0000'0000'621b,
	KC_ALT_RIGHT_ARROW= 0x0000'0000'0000'661b,
	KC_ALT_D		= 0x0000'0000'0000'641b,
	KC_BACKSPACE	= 0x0000'0000'0000'0008,
	KC_DELETE		= 0x0000'0000'0000'007f,
	KC_EOF		= 0x0000'0000'0000'0004,
	KC_ESC		= 0x0000'0000'0000'001b,
};

/*constexpr size_t HIST_MAX_LEN = 40;
char *hist_storage[HIST_MAX_LEN] = {nullptr};
size_t hist_in_use = 0;*/


ssize_t get_prompt_line(char *out, size_t len)
{
	char *cursor = out;

	uint64_t c = 0;
	bool needs_update = false;
	bool is_block_cursor = false;
	//size_t cur_hist_index = hist_in_use;

	while ((size_t)(cursor - out) < len)
	{
		if (read(STDIN_FILENO, &c, sizeof(c)) > 0)
		{
			switch (c) {
			case KC_ESC:
				is_block_cursor = true;
				needs_update = true;
				break;
			case KC_EOF:
				if (cursor == out)
					return -1;
				return -2;
			case '\n':
				goto break_prompt_line_loop;
				break;
			case KC_ALT_UP_ARROW:
			case KC_ALT_LEFT_ARROW:
				cursor = out;
				is_block_cursor = true;
				needs_update = true;
				break;
			case KC_ALT_DOWN_ARROW:
			case KC_ALT_RIGHT_ARROW:
				cursor = strchr(out, '\0');
				is_block_cursor = true;
				needs_update = true;
				break;
			case KC_UP_ARROW:
			case KC_DOWN_ARROW:
				break;
			case KC_LEFT_ARROW:
				if (cursor == out)
					break;
				--cursor;
				is_block_cursor = true;
				needs_update = true;
				break;
			case KC_BACKSPACE:
			case KC_DELETE:
				if (cursor == out)
						break;
				memmove(cursor-1, cursor, len - (cursor - out));
				--cursor;
				is_block_cursor = false;
				needs_update = true;
				break;
			case KC_ALT_D:
				memset(out, '\0', len);
				cursor = out;
				needs_update = true;
				break;
			case KC_RIGHT_ARROW:
				if (!cursor[0] && !cursor[1])
					break;
				is_block_cursor = true;
				++cursor;
				needs_update = true;
				break;
			default:
				memmove(cursor+1, cursor, len - (cursor - out));
				*cursor++ = c;
				is_block_cursor = false;
				needs_update = true;
				break;
			}
			c = 0;
		}

		if (needs_update)
		{
			fprintf(stdout, "\r\x1b[4C\x1b[K%s\x1b[%dG%s",
					out, (int)(5+(cursor-out)), (is_block_cursor)
						? "\x1b[1 q"
						: "\x1b[5 q");
			fflush(stdout);
			needs_update = false;
		}
	}
break_prompt_line_loop:
	*cursor++ = '\0';

	return cursor - out;
}

constexpr size_t LINE_MAX_LEN = 4096;

void MML_run_prompt(MML_state *state)
{
	MML_term_set_raw_mode();

	char line_in[LINE_MAX_LEN+1] = {0};

	MML_Value cur_val = VAL_INVAL;
	size_t expr_n_offset = 0;
	puts("-- MML interactive prompt --");
	puts("run `exit` or press SUPER+D to quit prompt.");
	ssize_t n_read = 0;
	while (!(cur_val.type == Invalid_type && cur_val.v.i == MML_QUIT_INVAL))
	{
		printf("==> ");
		fflush(stdout);

		memset(line_in, '\0', (n_read>0) ? (size_t)n_read : LINE_MAX_LEN);
		n_read = get_prompt_line(line_in, LINE_MAX_LEN);
		puts("");
		if (n_read == -1)
			break;
		else if (n_read == -2)
			continue;

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

		fflush(stdout);
		fflush(stderr);
	}

	/*for (size_t i = 0; i < hist_in_use; ++i)
		free(hist_storage[i]);*/

	MML_term_restore();
}
