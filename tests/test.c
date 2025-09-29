#include <stdint.h>

#include "mml/eval.h"
#include "mml/expr.h"
#include "ast_test.h"
#include "mml/token.h"
#include "mml/parser.h"

constexpr MML_token_type OPPOSITE_OPERATION[] = {
	MML_NOT_OP_TOK,
	MML_NOT_OP_TOK,
	MML_NOT_OP_TOK,

	MML_OP_ROOT,
	MML_OP_POW_TOK,

	MML_OP_DIV_TOK,
	MML_OP_MUL_TOK,
	MML_NOT_OP_TOK,
	MML_OP_SUB_TOK,
	MML_OP_ADD_TOK,
};

enum where_x_is {
	NOWHERE,
	LEFT,
	LEFT_LEFT,
	LEFT_RIGHT,
	RIGHT,
	RIGHT_LEFT,
	RIGHT_RIGHT,
};
const char *WHERE_STRS[] = {
	"NOWHERE",
	"LEFT",
	"LEFT_LEFT",
	"LEFT_RIGHT",
	"RIGHT",
	"RIGHT_LEFT",
	"RIGHT_RIGHT",
};

#define IS_ON_ONE_SIDE(w) ((w)%3==1)

bool strbuf_cmp(strbuf s1, strbuf s2)
{
	return s1.len == s2.len && strncmp(s1.s, s2.s, s1.len) == 0;
}

enum where_x_is find_x_in_ast(MML_expr *ast, strbuf s)
{
	if (ast->type != Operation_type)
		return NOWHERE; 
	if (ast->o.left->type == Identifier_type && strbuf_cmp(ast->o.left->s, s))
		return LEFT;
	if (ast->o.left->type == Operation_type)
	{
		if (ast->o.left->o.left->type == Identifier_type && strbuf_cmp(ast->o.left->o.left->s, s))
			return LEFT_LEFT;
		if (ast->o.left->o.right->type == Identifier_type && strbuf_cmp(ast->o.left->o.right->s, s))
			return LEFT_RIGHT;
	}
	if (ast->o.right->type == Identifier_type && strbuf_cmp(ast->o.right->s, s))
		return RIGHT;
	if (ast->o.right->type == Operation_type)
	{
		if (ast->o.right->o.left->type == Identifier_type && strbuf_cmp(ast->o.right->o.left->s, s))
			return RIGHT_LEFT;
		if (ast->o.right->o.right->type == Identifier_type && strbuf_cmp(ast->o.right->o.right->s, s))
			return RIGHT_RIGHT;
	}

	return NOWHERE;
}

int32_t main(int32_t argc, char **argv)
{
	MML_state *state = MML_init_state();

	const strbuf var = str_lit("x");
	const char *s = "3 = x/19.3";
	if (argc > 1)
		s = argv[1];
	MML_expr *e = MML_parse(s);
	MML_print_exprh(e);

	enum where_x_is where = find_x_in_ast(e, var);
	printf("x is %s\n", WHERE_STRS[where]);
	printf("x is alone on one side: %s\n", (IS_ON_ONE_SIDE(where)) ? "true" : "false");

	printf("%s\n", s);
	

	while (!IS_ON_ONE_SIDE(where))
	{
		switch (where) {
		case LEFT_LEFT:
			MML_expr *ll_new_left = e->o.left->o.left; // set variable
			MML_expr *ll_new_right = _create_oper(
				OPPOSITE_OPERATION[e->o.left->o.op],
				e->o.right,
				e->o.left->o.right
			);
			free(e->o.left);
			e->o.left = ll_new_left;
			e->o.right = ll_new_right;
			break;
		case RIGHT_LEFT:
			MML_expr *rl_new_left = _create_oper(
				OPPOSITE_OPERATION[e->o.right->o.op],
				e->o.left,
				e->o.right->o.right
			);
			MML_expr *rl_new_right = e->o.right->o.left;
			e->o.left = rl_new_left;
			free(e->o.right);
			e->o.right = rl_new_right;
			break;
		case LEFT_RIGHT:
			break;
		case RIGHT_RIGHT:
			break;
		}
		where = find_x_in_ast(e, var);
	}

	MML_print_exprh(e);

	MML_value val;

	if (where == LEFT) {
		printf("%.*s = ", (int)var.len, var.s);
		val = MML_eval_expr(state, e->o.right);
		MML_println_typedval(state, &val);
	} else {
		val = MML_eval_expr(state, e->o.left);
		MML_print_typedval(state, &val);
		printf(" = %.*s\n", (int)var.len, var.s);
	}

	MML_free_expr(&e);

	MML_cleanup_state(state);
	return 0;
}
