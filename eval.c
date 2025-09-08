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
	hashmap_set(func_map, hashmap_str_lit("sin"),		(uintptr_t)sin);
	hashmap_set(func_map, hashmap_str_lit("cos"),		(uintptr_t)cos);
	hashmap_set(func_map, hashmap_str_lit("tan"),		(uintptr_t)tan);
	hashmap_set(func_map, hashmap_str_lit("asin"),		(uintptr_t)asin);
	hashmap_set(func_map, hashmap_str_lit("acos"),		(uintptr_t)acos);
	hashmap_set(func_map, hashmap_str_lit("atan"),		(uintptr_t)atan);
	hashmap_set(func_map, hashmap_str_lit("log"),		(uintptr_t)log);
	hashmap_set(func_map, hashmap_str_lit("log2"),		(uintptr_t)log2);
	hashmap_set(func_map, hashmap_str_lit("sqrt"),		(uintptr_t)sqrt);
	hashmap_set(func_map, hashmap_str_lit("print"),		(uintptr_t)print_double);
	hashmap_set(func_map, hashmap_str_lit("println"),	(uintptr_t)println_double);

	constants = hashmap_create();
	static TypedValue constant_structs[] = {
		VAL_NUM(PI_M),
		VAL_NUM(E_M),
		VAL_NUM(PHI_M),
	};
	hashmap_set(constants, hashmap_str_lit("pi"),	(uintptr_t)&constant_structs[0]);
	hashmap_set(constants, hashmap_str_lit("e"),	(uintptr_t)&constant_structs[1]);
	hashmap_set(constants, hashmap_str_lit("phi"),	(uintptr_t)&constant_structs[2]);

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
			case PIPE_TOK: return VAL_NUM(fabs(a.v.n));
			default:
				fprintf(stderr, "invalid operator on real operands: %s\n", TOK_STRINGS[op]);
				return VAL_NUM(NAN);
		}
	} else if (a.type == Vector_type
		  && b.type == Number_type
		  && op == OP_DOT_TOK)
	{
		size_t i = (size_t)b.v.n;
		if (fabs(i - b.v.n) > EPSILON || b.v.n < 0)
		{
			fprintf(stderr, "vectors may only be indexed by a positive integer\n");
			return VAL_NUM(NAN);
		}
		if (i >= a.v.v.n)
		{
			fprintf(stderr, "index %zu out of range for vector of length %zu\n", i, a.v.v.n);
			return VAL_NUM(NAN);
		}
		return eval_expr(a.v.v.ptr[i]);
	} else if (a.type == Vector_type && b.type == Vector_type
		  && op == OP_MUL_TOK
		  && a.v.v.n == 2 && b.v.v.n == 2)
	{
		TypedValue first_product = apply_binary_op(
				eval_expr(a.v.v.ptr[0]),
				eval_expr(b.v.v.ptr[0]),
				OP_MUL_TOK);
		TypedValue second_product = apply_binary_op(
				eval_expr(a.v.v.ptr[1]),
				eval_expr(b.v.v.ptr[1]),
				OP_MUL_TOK);
		return apply_binary_op(
				first_product,
				second_product,
				OP_ADD_TOK);
	} else if (a.type == Vector_type && op == PIPE_TOK)
	{
		double sum = 0.0;
		TypedValue cur_elem;
		for (size_t i = 0; i < a.v.v.n; ++i)
		{
			cur_elem = eval_expr(a.v.v.ptr[i]);
			sum += apply_binary_op(cur_elem, cur_elem, OP_MUL_TOK).v.n;
		}
		return VAL_NUM(sqrt(sum));
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
	if (expr->type == Vector_type)
		return (TypedValue) { Vector_type, .v.v = expr->u.v.v };
	if (expr->type == Number_type)
		return VAL_NUM(expr->u.v.n);
	if (expr->type == String_type)
	{
		TypedValue *val;
		if (hashmap_get(constants, expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&val) == 0)
		{
			fprintf(stderr, "undefined identifier: '%.*s'\n",
					(int)expr->u.v.s.len, expr->u.v.s.s);
			return VAL_NUM(NAN);
		}
		return *val;
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
			(right) ? eval_expr(right) : VAL_NUM(NAN),
			expr->u.o.op);
}
