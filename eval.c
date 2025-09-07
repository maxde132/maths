#include "eval.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "token.h"
#include "expr.h"
#include "c-hashmap/map.h"
#include "eval_funcs_incl.c"

static hashmap *func_map;
static hashmap *constants;
static bool eval_is_init = false;

void init_evaluator(void)
{
	func_map = hashmap_create();
	hashmap_set(func_map, hashmap_str_lit("cos"),		(uintptr_t)cos);
	hashmap_set(func_map, hashmap_str_lit("sin"),		(uintptr_t)sin);
	hashmap_set(func_map, hashmap_str_lit("log"),		(uintptr_t)log);
	hashmap_set(func_map, hashmap_str_lit("log2"),		(uintptr_t)log2);
	hashmap_set(func_map, hashmap_str_lit("sqrt"),		(uintptr_t)sqrt);
	hashmap_set(func_map, hashmap_str_lit("print"),		(uintptr_t)print_double);
	hashmap_set(func_map, hashmap_str_lit("println"),	(uintptr_t)println_double);

	constants = hashmap_create();
	union {
		uintptr_t u;
		double d;
	} d2u;
	d2u.d = PI_M;	hashmap_set(constants, hashmap_str_lit("pi"),	d2u.u);
	d2u.d = E_M;	hashmap_set(constants, hashmap_str_lit("e"),	d2u.u);
	d2u.d = PHI_M;	hashmap_set(constants, hashmap_str_lit("phi"),	d2u.u);

	eval_is_init = true;
}

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
		case OP_LESSEQ_TOK: return a <= b;
		case OP_GREATEREQ_TOK: return a >= b;
		case OP_EQ_TOK: return a == b;
		case OP_NOTEQ_TOK: return a != b;
		default:
			fprintf(stderr, "invalid operator: %u\n", op);
			return NAN;
	}
}

double eval_expr(const Expr *expr)
{
	if (!eval_is_init)
	{
		fprintf(stderr, "you must run `init_evaluator` before using any evaluator functions.\n");
		return NAN;
	}

	if (expr == NULL)
	{
		fprintf(stderr, "error: NULL expression found while evaluating\n");
		return NAN;
	}
	if (expr->type == Number_type)
		return expr->u.n;
	if (expr->type == String_type)
	{
		double val;
		if (hashmap_get(constants, expr->u.s.s, expr->u.s.len, (uintptr_t *)&val) == 0)
		{
			fprintf(stderr, "undefined identifier: '%.*s'\n",
					(int)expr->u.s.len, expr->u.s.s);
			return NAN;
		}
		return val;
	}
	const Expr *left = expr->u.o.left;
	const Expr *right = expr->u.o.right;
	if (expr->u.o.op == OP_FUNC_CALL_TOK)
	{
		if (left == NULL
		 || right == NULL
		 || left->type != String_type)
			return NAN;
		double (*func) (double);
		if (hashmap_get(func_map, left->u.s.s, left->u.s.len, (uintptr_t *)&func) == 0)
		{
			fprintf(stderr, "undefined function in function call: '%.*s'\n",
				(int)left->u.s.len, left->u.s.s);
			return NAN;
		}

		return (*func)(eval_expr(right));
		//if (strncmp(left->u.s.s, "cos", left->u.s.len) == 0)
		//	return cos(eval_expr(right));
		//if (strncmp(left->u.s.s, "sin", left->u.s.len) == 0)
		//	return sin(eval_expr(right));

	}
	return apply_binary_op(
			eval_expr(left),
			eval_expr(right),
			expr->u.o.op);
}
