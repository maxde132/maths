#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "token.h"
#include "parser.h"
#include "eval.h"

struct config global_config = {
	.PROG_NAME = NULL,
	.precision = 6,
	.runtime_flags = 0,
};

char *expression = NULL;

extern struct evaluator_state eval_state;

void print_usage(void)
{
	fprintf(stderr, "usage: %s [options] <EXPR>\n\n"

                    "options:\n"
                    "  -d, --debug                        Enable debug output\n"
                    "  -P, --print                        Print the result value even if `print` was not called (default OFF)\n"
                    "  -p PREC, --precision=PREC          Set the number of decimal digits to be printed when printing numbers (default 6)\n"
                    "  --bools-are-nums                   Write the number 1 or 0 to represent boolean values (default OFF)\n"
			  "  --estimate-equality                Consider two reals equal if they are equal to 14 digits of accuracy (default OFF)\n"
			  "  -h, --help                         Display this help message\n"
			  "  -V, --version                      Display program information\n"
			  "  -                                  Read expression string from stdin\n"
			, global_config.PROG_NAME);
	if (eval_state.user_vars.ptr != NULL)
	{
		for (size_t i = 0; i < eval_state.user_vars.in_use; ++i)
			free_expr(eval_state.user_vars.ptr[i]);
		free(eval_state.user_vars.ptr);
	}
	cleanup_evaluator(&eval_state);
	exit(1);
}

void print_info(void)
{
	printf("Compiled on " __DATE__ " at " __TIME__ " with "
#ifdef __GNUC__
	"GCC"
#elif defined(__clang__)
	"Clang"
#elif defined(__INTEL_COMPILER__)
	"an Intel compiler of"
#endif
		" version " __VERSION__ " and C standard version %ld.\n",
		__STDC_VERSION__);
	exit(1);
}

void parse_args(int32_t argc, char **argv)
{
	global_config.PROG_NAME = argv[0];
	int32_t arg_n = 1;
	for (; arg_n < argc; ++arg_n)
	{
		if (strncmp(argv[arg_n], "--", 2) == 0)
		{
			if (strcmp(argv[arg_n]+2, "debug") == 0)
				SET_FLAG(DEBUG);
			else if (strcmp(argv[arg_n]+2, "print") == 0)
				SET_FLAG(PRINT);
			else if (strcmp(argv[arg_n]+2, "help") == 0)
				print_usage();
			else if (strcmp(argv[arg_n]+2, "version") == 0)
				print_info();
			else if (strncmp(argv[arg_n]+2, "precision=", 10) == 0)
				global_config.precision = strtoul(argv[arg_n]+2+10, NULL, 10);
			else if (strcmp(argv[arg_n]+2, "bools-are-nums") == 0)
				SET_FLAG(BOOLS_PRINT_NUM);
			else if (strcmp(argv[arg_n]+2, "estimate-equality") == 0)
				SET_FLAG(ESTIMATE_EQUALITY);
			else if (strncmp(argv[arg_n]+2, "set_var:", 8) == 0)
			{
				const char *cur = argv[arg_n]+2+8;
				cur = strchr(argv[arg_n]+2+8, '=');
				if (cur++ == NULL)
				{
					fprintf(stderr, "argument error: expected expression following command-line variable definition\n");
					cleanup_evaluator(&eval_state);
					exit(1);
				}
				strbuf name = { argv[arg_n]+2+8, cur - (argv[arg_n]+2+8) - 1, false };
				Expr *expr = parse(cur);
				set_variable(&eval_state, name, expr);
			} else
			{
				fprintf(stderr, "argument error: unknown option '%s'\n", argv[arg_n]);
				print_usage();
			}
		} else if (argv[arg_n][0] == '-')
		{
			char *cur = argv[arg_n];
			while (*++cur)
			{
				switch (*cur) {
				case 'h':
					print_usage();
					break;
				case 'V':
					print_info();
					break;
				case 'd':
					SET_FLAG(DEBUG);
					break;
				case 'P':
					SET_FLAG(PRINT);
					break;
				case 'p':
					if (cur[1] != '\0')
					{
						global_config.precision = strtoul(cur+1, &cur, 10);
						--cur;
						break;
					}
					if (argv[arg_n+1] == NULL)
					{
						fprintf(stderr, "argument error: integer is required following "
								"'-p' argument to specify precision.\n");
						print_usage();
					}
					global_config.precision = strtoul(argv[++arg_n], NULL, 10);
					break;
				default:
					fprintf(stderr, "argument error: unknown option '-%c'\n", *cur);
					print_usage();
				}
			}
			if (cur == argv[arg_n]+1)
				SET_FLAG(READ_STDIN);
		} else if (expression == NULL)
		{
			expression = argv[arg_n];
		}
	}

	if (expression == NULL)
		SET_FLAG(READ_STDIN);
}

strbuf read_string_from_stream(FILE *stream)
{
	size_t buf_size = 2048;
	strbuf ret_buf = {0};
	ret_buf.s = malloc(buf_size);
	if (ret_buf.s == NULL)
	{
		fprintf(stderr, "failed to allocate the initial buffer size\n");
		return (strbuf) { NULL, 0, 0 };
	}

	size_t cur_chunk_read;
	
	while ((cur_chunk_read = fread(ret_buf.s + ret_buf.len, 1, buf_size - ret_buf.len- 1, stream)) > 0)
	{
		ret_buf.len += cur_chunk_read;
		if (ret_buf.len >= buf_size - 1)
		{
			buf_size *= 2;
			char *temp_buf = realloc(ret_buf.s, buf_size);
			if (temp_buf == NULL)
			{
				free(ret_buf.s);
				fprintf(stderr, "failed to reallocate buffer with size %zu\n", buf_size);
				return (strbuf) { NULL, 0, 0 };
			}
			ret_buf.s = temp_buf;
		}
	}

	ret_buf.s[ret_buf.len++] = '\0';

	return ret_buf;
}
