#include <string.h>
#include <stdio.h>

#include "eval.h"
#include "parser.h"

struct evaluator_state state = {0};
void init_eval(void)
{
	init_evaluator(&state);
	printf("initialized evaluator\n");
}
void cleanup_eval(void)
{
	cleanup_evaluator(&state);
	printf("cleaned up evaluator\n");
}
void print_val(TypedValue v)
{
	print_typedval(&v);
}
void assign_var(const char *name, Expr *val)
{
	set_variable(&state, (strbuf) { (char *)name, strlen(name), false }, val);
}
TypedValue parse_eval(const char *s)
{
	const Expr *expr = parse(s);
	TypedValue ret = eval_expr(&state, expr);
	fflush(stdout);
	return ret;
}
TypedValue eval_mml(const Expr *expr)
{
	return eval_expr(&state, expr);
}
