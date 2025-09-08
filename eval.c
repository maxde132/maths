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

static hashmap *builtins;
static hashmap *variables;
static bool eval_is_init = false;

void init_evaluator(void)
{
	builtins = hashmap_create();
	hashmap_set(builtins, hashmap_str_lit("sin"),		(uintptr_t)sin);
	hashmap_set(builtins, hashmap_str_lit("cos"),		(uintptr_t)cos);
	hashmap_set(builtins, hashmap_str_lit("tan"),		(uintptr_t)tan);
	hashmap_set(builtins, hashmap_str_lit("asin"),		(uintptr_t)asin);
	hashmap_set(builtins, hashmap_str_lit("acos"),		(uintptr_t)acos);
	hashmap_set(builtins, hashmap_str_lit("atan"),		(uintptr_t)atan);
	hashmap_set(builtins, hashmap_str_lit("log"),		(uintptr_t)log);
	hashmap_set(builtins, hashmap_str_lit("log2"),		(uintptr_t)log2);
	hashmap_set(builtins, hashmap_str_lit("sqrt"),		(uintptr_t)sqrt);
	hashmap_set(builtins, hashmap_str_lit("print"),		(uintptr_t)print_double);
	hashmap_set(builtins, hashmap_str_lit("println"),	(uintptr_t)println_double);

	static TypedValue constant_structs[] = {
		VAL_NUM(PI_M),
		VAL_NUM(E_M),
		VAL_NUM(PHI_M),
	};
	hashmap_set(builtins, hashmap_str_lit("pi"),	(uintptr_t)&constant_structs[0]);
	hashmap_set(builtins, hashmap_str_lit("e"),	(uintptr_t)&constant_structs[1]);
	hashmap_set(builtins, hashmap_str_lit("phi"),	(uintptr_t)&constant_structs[2]);

	variables = hashmap_create();

	eval_is_init = true;
}

void cleanup_evaluator(void)
{
	hashmap_free(builtins);
	hashmap_free(variables);

	eval_is_init = false;
}

int32_t set_variable(strbuf name, Expr *expr)
{
	return hashmap_set(variables, name.s, name.len, (uintptr_t)expr);
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
		// vector index
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
		  && a.v.v.n == b.v.v.n)
	{
		// n-dimensional dot product
		double sum = 0.0;
		for (size_t i = 0; i < a.v.v.n; ++i)
		{
			sum += apply_binary_op(
					eval_expr(a.v.v.ptr[i]),
					eval_expr(b.v.v.ptr[i]),
					OP_MUL_TOK).v.n;
		}
		return VAL_NUM(sum);
	} else if (a.type == Vector_type && op == PIPE_TOK)
	{
		// compute vector magnitude
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
		Expr *out;
		if (hashmap_get(builtins, expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&val))
			return *val;
		else if (hashmap_get(variables, expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&out))
			return eval_expr(out);

		fprintf(stderr, "undefined identifier: '%.*s'\n",
				(int)expr->u.v.s.len, expr->u.v.s.s);
		return VAL_NUM(NAN);
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
		if (hashmap_get(builtins, left->u.v.s.s, left->u.v.s.len, (uintptr_t *)&func) == 0)
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
