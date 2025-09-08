#ifndef CONFIG_H
#define CONFIG_H

#include <inttypes.h>
#include <stdio.h>

#include "token.h"
#include "expr.h"

#define BIT(n) (1<<n)
enum {
	DEBUG		= BIT(0),
	PRINT		= BIT(1),
	READ_STDIN	= BIT(2),
};

#define SET_FLAG(f) (runtime_flags |= (f))
#define FLAG_IS_SET(f) ((runtime_flags & (f)) != 0)

extern uint32_t runtime_flags;

struct config {
	char *PROG_NAME;
	uint32_t precision;
};
extern struct config global_config;
typedef struct UserVars {
	strbuf name;
	Expr *e;
} UserVar;

extern UserVar *user_vars;
extern UserVar *user_vars_top;

void print_usage(void);
void parse_args(int32_t argc, char **argv);

strbuf read_string_from_stream(FILE *stream);

#endif /* CONFIG_H */
