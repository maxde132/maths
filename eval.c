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

void cleanup_evaluator(void)
{
	hashmap_free(func_map);
	hashmap_free(constants);
}

#define EPSILON 1e-15

TypedValue apply_binary_op(TypedValue a, TypedValue b, TokenType op)
{
	if (a.type == Number_type && b.type == Number_type)
	{
		switch (op) {
			case OP_POW_TOK: return VAL_NUM(pow(a.v.n, b.v.n));
			case OP_MUL_TOK: return VAL_NUM(a.v.n * b.v.n);
			case OP_DIV_TOK: return VAL_NUM(a.v.n / b.v.n);
			case OP_MOD_TOK: return VAL_NUM(fmod(a.v.n, b.v.n));
			case OP_ADD_TOK: return VAL_NUM(a.v.n + b.v.n);
			case OP_SUB_TOK: return VAL_NUM(a.v.n - b.v.n);
			case OP_LESS_TOK: return VAL_NUM(a.v.n < b.v.n);
			case OP_GREATER_TOK: return VAL_NUM(a.v.n > b.v.n);
			case OP_LESSEQ_TOK: return VAL_NUM(a.v.n <= b.v.n);
			case OP_GREATEREQ_TOK: return VAL_NUM(a.v.n >= b.v.n);
			case OP_EQ_TOK: return VAL_NUM(a.v.n == b.v.n);
			case OP_NOTEQ_TOK: return VAL_NUM(a.v.n != b.v.n);
			default:
				fprintf(stderr, "invalid operator on real operands: %s\n", TOK_STRINGS[op]);
				return VAL_NUM(NAN);
		}
	} else if (a.type == Vector_type && b.type == Number_type)
	{
		size_t i = (size_t)b.v.n;
		if (fabs(i - b.v.n) > EPSILON)
		{
			fprintf(stderr, "vectors may only be indexed by an integer\n");
			return VAL_NUM(NAN);
		}
		if (i >= a.v.v.n)
		{
			fprintf(stderr, "index %zu out of range for vector of length %zu\n", i, a.v.v.n);
			return VAL_NUM(NAN);
		}
		return eval_expr(a.v.v.ptr[i]);
	}

	fprintf(stderr, "invalid operator: %s\n", TOK_STRINGS[op]);
	return VAL_NUM(NAN);
}

TypedValue eval_expr(const Expr *expr)
{
	if (!eval_is_init)
	{
		fprintf(stderr, "you must run `init_evaluator` before using any evaluator functions.\n");
		return VAL_NUM(NAN);
	}

	if (expr == NULL)
	{
		fprintf(stderr, "error: NULL expression found while evaluating\n");
		return VAL_NUM(NAN);
	}
	if (expr->type == Number_type)
		return VAL_NUM(expr->u.v.n);
	if (expr->type == String_type)
	{
		double val;
		if (hashmap_get(constants, expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&val) == 0)
		{
			fprintf(stderr, "undefined identifier: '%.*s'\n",
					(int)expr->u.v.s.len, expr->u.v.s.s);
			return VAL_NUM(NAN);
		}
		return VAL_NUM(val);
	}
	const Expr *left = expr->u.o.left;
	const Expr *right = expr->u.o.right;
	if (expr->u.o.op == OP_FUNC_CALL_TOK)
	{
		if (left == NULL
		 || right == NULL
		 || left->type != String_type)
			return VAL_NUM(NAN);
		double (*func) (double);
		if (hashmap_get(func_map, left->u.v.s.s, left->u.v.s.len, (uintptr_t *)&func) == 0)
		{
			fprintf(stderr, "undefined function in function call: '%.*s'\n",
				(int)left->u.v.s.len, left->u.v.s.s);
			return VAL_NUM(NAN);
		}

		TypedValue ret = eval_expr(right);
		if (ret.type == Number_type)
			return VAL_NUM((*func)(ret.v.n));
		fprintf(stderr, "functions with vector arguments are not yet supported\n");
		return VAL_NUM(NAN);
	}

	return apply_binary_op(
			eval_expr(left),
			eval_expr(right),
			expr->u.o.op);
}
