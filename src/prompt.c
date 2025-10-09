#include "mml/prompt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

#include "mml/config.h"
#include "mml/expr.h"
#include "mml/eval.h"
#include "mml/parser.h"

#define NSEC_IN_SEC 1000000000

#define time_blck(elapsed_p, ...) { \
	const clock_t start = clock(); \
	{ __VA_ARGS__; }; \
	*(elapsed_p) = (uint64_t)((double)(clock() - start) / CLOCKS_PER_SEC * NSEC_IN_SEC); \
}

#define PROMPT_STR "==> "

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

/*static constexpr size_t HIST_MAX_LEN = 40;
static char *hist_storage[HIST_MAX_LEN] = {nullptr};
static size_t hist_in_use = 0;*/


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
				// somehow truncates the string to end where the deleted character was (maybe misplaced null terminator?)
				if (cursor == out)
						break;
				memmove(cursor-1, cursor, len - (cursor - out));
				--cursor;
				is_block_cursor = false;
				needs_update = true;
				break;
			case KC_ALT_D:
				*out = '\0';
				cursor = out;
				is_block_cursor = false;
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
				// somehow truncates the string to end just after the inserted character (maybe misplaced null terminator?)
				memmove(cursor+1, cursor, len - (cursor - out) - 1);
				*cursor++ = c;
				is_block_cursor = false;
				needs_update = true;
				break;
			}
			c = 0;
		}

		if (needs_update)
		{
			//fprintf(stderr, "\n[debug] text from cursor: '%s'\n", cursor);
			//fprintf(stderr, "[debug] text from out: '%s'\n", out);
			fprintf(stdout, "\r%s\x1b[K%s\x1b[%dG%s",
					PROMPT_STR,
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

	MML_value cur_val = VAL_INVAL;
	puts("-- MML interactive prompt --");
	puts("run `exit` or press CTRL+D to quit the prompt.");
	ssize_t n_read = 0;
	while (!(cur_val.type == Invalid_type && cur_val.i == MML_QUIT_INVAL))
	{
		printf("%s", PROMPT_STR);
		fflush(stdout);

		memset(line_in, '\0', (n_read>0) ? (size_t)n_read : LINE_MAX_LEN);
		n_read = get_prompt_line(line_in, LINE_MAX_LEN);
		puts("");
		if (n_read == -1)
			break;
		else if (n_read == -2)
			continue;
	#ifndef NDEBUG
		//printf("buf: '%s'\n", line_in);
	#endif

		uint64_t nsecs;
		MML_expr_dvec exprs;
		if (!FLAG_IS_SET(DBG_TIME))
			exprs = MML_parse_stmts(line_in);
		else {
			time_blck(&nsecs, exprs = MML_parse_stmts(line_in));
			MML_log_dbg("parsed in %.6fs\n", (double)nsecs/NSEC_IN_SEC);
		}

		MML_expr **cur;
		if (!FLAG_IS_SET(DBG_TIME))
		{
			dv_foreach(exprs, cur)
				if (*cur != NULL)
					cur_val = MML_eval_expr(state, *cur);
		} else
		{
			time_blck(&nsecs, 
			dv_foreach(exprs, cur)
			{
				if (*cur != NULL)
					cur_val = MML_eval_expr(state, *cur);
			});
			MML_log_dbg("evaluated in %.6fs\n", (double)nsecs/NSEC_IN_SEC);
		}

		if (!state->config->last_print_was_newline)
			puts("\033[7m%\033[0m");
		state->config->last_print_was_newline = true;

		if (cur_val.type != Invalid_type)
		{
			MML_println_typedval(state, &cur_val);
			state->last_val = cur_val;
		} else
		{
			switch (cur_val.i) {
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
