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
	NO_EVAL	= BIT(4),
	RUN_PROMPT	= BIT(5),
};

#define SET_FLAG(f) (MML_global_config.runtime_flags |= (f))
#define FLAG_IS_SET(f) ((MML_global_config.runtime_flags & (f)) != 0)

struct MML_config {
	char *PROG_NAME;
	uint32_t precision;
	uint32_t runtime_flags;
	MML_state *eval_state;
	bool last_print_was_newline;
};
extern struct MML_config MML_global_config;

void MML_term_set_raw_mode(void);
void MML_term_restore(void);
void MML_print_usage(void);
void MML_arg_parse(int32_t argc, char **argv);

strbuf MML_read_string_from_stream(FILE *stream);

#endif /* CONFIG_H */
