#include "eval.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "token.h"
#include "expr.h"
double apply_binary_op(double a, double b, TokenType op)
{
	switch (op) {
		case OP_POW_TOK: return pow(a, b);
		case OP_MUL_TOK: return a * b;
		case OP_DIV_TOK: return a / b;
		case OP_MOD_TOK: return fmod(a, b);
		case OP_ADD_TOK: return a + b;
		case OP_SUB_TOK: return a - b;
		case OP_LESS_TOK: return a < b;
		case OP_GREATER_TOK: return a > b;
		case OP_EQ_TOK: return a == b;
		default:
			fprintf(stderr, "invalid operator: %u\n", op);
			return NAN;
	}
}

double eval_expr(const Expr *expr)
{
	if (expr == NULL)
	{
		fprintf(stderr, "error: NULL expression found while evaluating\n");
		exit(1);
	}
	if (expr->type == Number_type)
		return expr->u.n;
	const Expr *left = expr->u.o.left;
	const Expr *right = expr->u.o.right;
	if (expr->u.o.op == OP_FUNC_CALL_TOK)
	{
		if (left == NULL
		 || right == NULL
		 || left->type != String_type)
			return NAN;
		if (strncmp(left->u.s.s, "cos", left->u.s.len) == 0)
			return cos(eval_expr(right));
		if (strncmp(left->u.s.s, "sin", left->u.s.len) == 0)
			return sin(eval_expr(right));

		fprintf(stderr, "invalid function in function call: '%.*s'\n",
				(int)left->u.s.len, left->u.s.s);
	}
	return apply_binary_op(
			eval_expr(left),
			eval_expr(right),
			expr->u.o.op);
}

