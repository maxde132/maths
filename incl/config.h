#ifndef CONFIG_H
#define CONFIG_H

#include <inttypes.h>
#include <stdio.h>

#include "eval.h"
#include "token.h"

#define BIT(n) (1<<n)
enum {
	DEBUG		= BIT(0),
	PRINT		= BIT(1),
	READ_STDIN	= BIT(2),
	BOOLS_PRINT_NUM = BIT(3),
	NO_ESTIMATE_EQUALITY = BIT(4),
	NO_EVAL	= BIT(5),
};

#define SET_FLAG(f) (global_config.runtime_flags |= (f))
#define FLAG_IS_SET(f) ((global_config.runtime_flags & (f)) != 0)

struct config {
	char *PROG_NAME;
	uint32_t precision;
	uint32_t runtime_flags;
	struct evaluator_state eval_state;
};
extern struct config global_config;

void print_usage(void);
void parse_args(int32_t argc, char **argv);

strbuf read_string_from_stream(FILE *stream);

#endif /* CONFIG_H */
