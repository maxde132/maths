#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "token.h"

struct config global_config = {
	.PROG_NAME = NULL,
	.precision = 6,
};

char *expression = NULL;

uint32_t runtime_flags = 0;

void print_usage(void)
{
	fprintf(stderr, "usage: %s [options] <EXPR>\n\n"

                    "options:\n"
                    "  -d, --debug                        Enable debug output\n"
                    "  -P, --print                        Print the result value even if `print` was not called\n"
                    "  -p PREC, --precision=PREC          Set the number of decimal digits to be printed when printing numbers\n"
			  "  -h, --help                         Display this help message\n"
			  "  -V, --version                      Display program information\n"
			  "  -                                  Read expression string from stdin\n"
			, global_config.PROG_NAME);
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
			else
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
