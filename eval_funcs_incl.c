#include <complex.h>
#include <stdio.h>

#include "expr.h"

_Complex double custom_clog2(_Complex double a)
{
	return clog(a)/clog(2.0);
}

_Complex double custom_sqrt(double a)
{
	return csqrt(a + 0.0*I);
}

TypedValue custom_summation(VecN args)
{
	/* argument types: [
	 * 	init: Expr,
	 * 	cond: Expr,
	 * 	accum: Expr]
	 */
	TypedValue sum = VAL_NUM(0.0);

	return sum;
}
