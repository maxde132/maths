#include <complex.h>
#include <math.h>
#include <stdio.h>

#include "mml/eval.h"
#include "mml/expr.h"

_Complex double custom_clog2(_Complex double a)
{
	return clog(a)/clog(2.0);
}

_Complex double custom_clog10(_Complex double a)
{
	return clog(a)/clog(10.0);
}

_Complex double custom_sqrt(double a)
{
	return csqrt(a + 0.0*I);
}

MML_Value custom_atan2(MML_state *state, MML_VecN *args)
{
	if (args->n != 2)
	{
		fprintf(stderr, "`atan2` takes exactly 2 real number arguments\n");
		return VAL_INVAL;
	}

	const MML_Value y = MML_eval_expr(state, args->ptr[0]);
	const MML_Value x = MML_eval_expr(state, args->ptr[1]);
	if (y.type != RealNumber_type || x.type != RealNumber_type)
	{
		fprintf(stderr, "`atan2` takes exactly 2 real number arguments\n");
		return VAL_INVAL;
	}
	return VAL_NUM(atan2(y.v.n, x.v.n));
}

MML_Value custom_max(MML_state *state, MML_VecN *args)
{
	MML_Value max = VAL_INVAL;

	for (size_t i = 0; i < args->n; ++i)
	{
		if (max.type == Invalid_type)
			max = MML_eval_expr(state, args->ptr[i]);
		if (!VALTYPE_IS_ORDERED(max))
			return VAL_INVAL;
		MML_Value cur = MML_eval_expr(state, args->ptr[i]);
		MML_Value tmp = MML_apply_binary_op(state, cur, max, MML_OP_GREATER_TOK);
		if (tmp.type == Boolean_type && tmp.v.b)
			max = cur;
	}

	return max;
}

MML_Value custom_min(MML_state *state, MML_VecN *args)
{
	MML_Value min = VAL_INVAL;

	for (size_t i = 0; i < args->n; ++i)
	{
		if (min.type == Invalid_type)
			min = MML_eval_expr(state, args->ptr[i]);
		if (!VALTYPE_IS_ORDERED(min))
			return VAL_INVAL;
		MML_Value cur = MML_eval_expr(state, args->ptr[i]);
		MML_Value tmp = MML_apply_binary_op(state, cur, min, MML_OP_LESS_TOK);
		if (tmp.type == Boolean_type && tmp.v.b)
			min = cur;
	}

	return min;
}
