#include <complex.h>
#include <math.h>
#include <stdio.h>

#include "eval.h"
#include "expr.h"

_Complex double custom_clog2(_Complex double a)
{
	return clog(a)/clog(2.0);
}

_Complex double custom_sqrt(double a)
{
	return csqrt(a + 0.0*I);
}

TypedValue custom_max(struct evaluator_state *state, TypedValue v)
{
	if (v.type != Vector_type)
		return VAL_INVAL;
	const VecN *vec = &v.v.v;
	TypedValue max = VAL_INVAL;

	for (size_t i = 0; i < vec->n; ++i)
	{
		if (max.type == Invalid_type)
			max = eval_expr(state, vec->ptr[i]);
		if (!VALTYPE_IS_ORDERED(max))
			return VAL_INVAL;
		TypedValue cur = eval_expr(state, vec->ptr[i]);
		TypedValue tmp = apply_binary_op(state, cur, max, OP_GREATER_TOK);
		if (tmp.type == Boolean_type && tmp.v.b)
			max = cur;
	}

	return max;
}

TypedValue custom_min(struct evaluator_state *state, TypedValue v)
{
	if (v.type != Vector_type)
		return VAL_INVAL;
	const VecN *vec = &v.v.v;
	TypedValue min = VAL_INVAL;

	for (size_t i = 0; i < vec->n; ++i)
	{
		if (min.type == Invalid_type)
			min = eval_expr(state, vec->ptr[i]);
		if (!VALTYPE_IS_ORDERED(min))
			return VAL_INVAL;
		TypedValue cur = eval_expr(state, vec->ptr[i]);
		TypedValue tmp = apply_binary_op(state, cur, min, OP_LESS_TOK);
		if (tmp.type == Boolean_type && tmp.v.b)
			min = cur;
	}

	return min;
}
