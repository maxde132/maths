#include <stdint.h>
#include <stdio.h>

#include "mml/eval.h"
#include "mml/expr.h"
#include "mml/mml.h"

int32_t main(void)
{
	MML_state *state = MML_init_state();

	const char *s = "5 + 9 * cos{2.3pi}";
	MML_Value ret = MML_eval_parse(state, s);
	MML_println_typedval(state, &ret);

	MML_cleanup_state(state);

	return 0;
}
