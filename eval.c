#include "eval.h"

#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "token.h"
#include "expr.h"
#include "c-hashmap/map.h"
#include "eval_funcs_incl.c"

void init_evaluator(struct evaluator_state *state)
{
	if (state->is_init) return;
	state->builtins = hashmap_create();
	hashmap_set(state->builtins, hashmap_str_lit("print"),		(uintptr_t)print_typedval);
	hashmap_set(state->builtins, hashmap_str_lit("println"),		(uintptr_t)println_typedval);

	hashmap_set(state->builtins, hashmap_str_lit("sin"),			(uintptr_t)sin);
	hashmap_set(state->builtins, hashmap_str_lit("cos"),			(uintptr_t)cos);
	hashmap_set(state->builtins, hashmap_str_lit("tan"),			(uintptr_t)tan);
	hashmap_set(state->builtins, hashmap_str_lit("asin"),			(uintptr_t)asin);
	hashmap_set(state->builtins, hashmap_str_lit("acos"),			(uintptr_t)acos);
	hashmap_set(state->builtins, hashmap_str_lit("atan"),			(uintptr_t)atan);
	hashmap_set(state->builtins, hashmap_str_lit("log"),			(uintptr_t)log);
	hashmap_set(state->builtins, hashmap_str_lit("log2"),			(uintptr_t)log2);
	hashmap_set(state->builtins, hashmap_str_lit("sqrt"),			(uintptr_t)sqrt);

	hashmap_set(state->builtins, hashmap_str_lit("complex_sin"),	(uintptr_t)csin);
	hashmap_set(state->builtins, hashmap_str_lit("complex_cos"),	(uintptr_t)ccos);
	hashmap_set(state->builtins, hashmap_str_lit("complex_tan"),	(uintptr_t)ctan);
	hashmap_set(state->builtins, hashmap_str_lit("complex_asin"),	(uintptr_t)casin);
	hashmap_set(state->builtins, hashmap_str_lit("complex_acos"),	(uintptr_t)cacos);
	hashmap_set(state->builtins, hashmap_str_lit("complex_atan"),	(uintptr_t)catan);
	hashmap_set(state->builtins, hashmap_str_lit("complex_log"),	(uintptr_t)clog);
	hashmap_set(state->builtins, hashmap_str_lit("complex_log2"),	(uintptr_t)custom_clog2);
	hashmap_set(state->builtins, hashmap_str_lit("complex_sqrt"),	(uintptr_t)csqrt);
	hashmap_set(state->builtins, hashmap_str_lit("csqrt"),		(uintptr_t)custom_sqrt);
	hashmap_set(state->builtins, hashmap_str_lit("complex_conj"),	(uintptr_t)conj);
	hashmap_set(state->builtins, hashmap_str_lit("complex_phase"),	(uintptr_t)carg);
	hashmap_set(state->builtins, hashmap_str_lit("complex_real"),	(uintptr_t)creal);
	hashmap_set(state->builtins, hashmap_str_lit("complex_imag"),	(uintptr_t)cimag);

	static TypedValue constant_structs[] = {
		VAL_BOOL(true),
		VAL_BOOL(false),
		VAL_NUM(PI_M),
		VAL_NUM(E_M),
		VAL_NUM(PHI_M),
		VAL_CNUM(I),
		VAL_NUM(NAN),
		VAL_NUM(INFINITY),
	};
	hashmap_set(state->builtins, hashmap_str_lit("true"),	(uintptr_t)&constant_structs[0]);
	hashmap_set(state->builtins, hashmap_str_lit("false"),(uintptr_t)&constant_structs[1]);
	hashmap_set(state->builtins, hashmap_str_lit("pi"),	(uintptr_t)&constant_structs[2]);
	hashmap_set(state->builtins, hashmap_str_lit("e"),	(uintptr_t)&constant_structs[3]);
	hashmap_set(state->builtins, hashmap_str_lit("phi"),	(uintptr_t)&constant_structs[4]);
	hashmap_set(state->builtins, hashmap_str_lit("i"),	(uintptr_t)&constant_structs[5]);
	hashmap_set(state->builtins, hashmap_str_lit("nan"),	(uintptr_t)&constant_structs[6]);
	hashmap_set(state->builtins, hashmap_str_lit("inf"),	(uintptr_t)&constant_structs[7]);

	state->variables = hashmap_create();

	state->is_init = true;
}

void cleanup_evaluator(struct evaluator_state *state)
{
	hashmap_free(state->builtins);
	hashmap_free(state->variables);

	state->is_init = false;
}

int32_t set_variable(struct evaluator_state *state, strbuf name, size_t index)
{
	return hashmap_set(state->variables, name.s, name.len, (uintptr_t)index);
}

#define EPSILON 1e-15

inline bool doubles_are_equal_func(double a, double b)
{
	if (FLAG_IS_SET(ESTIMATE_EQUALITY))
		return fabs(a-b) < EPSILON;
	return a == b;
}
bool doubles_are_equal_func(double, double);

TypedValue apply_binary_op(struct evaluator_state *state, TypedValue a, TypedValue b, TokenType op)
{
	if (VAL_IS_NUM(a) && VAL_IS_NUM(b)
		&& a.type != ComplexNumber_type && b.type != ComplexNumber_type)
	{
		switch (op) {
			case OP_POW_TOK: return VAL_NUM(pow(get_number(&a), get_number(&b)));
			case OP_MUL_TOK: return VAL_NUM(get_number(&a) * get_number(&b));
			case OP_DIV_TOK: return VAL_NUM(get_number(&a) / get_number(&b));
			case OP_MOD_TOK: return VAL_NUM(fmod(get_number(&a), get_number(&b)));
			case OP_ADD_TOK: return VAL_NUM(get_number(&a) + get_number(&b));
			case OP_SUB_TOK: return VAL_NUM(get_number(&a) - get_number(&b));
			case OP_LESS_TOK: return VAL_BOOL(get_number(&a) < get_number(&b));
			case OP_GREATER_TOK: return VAL_BOOL(get_number(&a) > get_number(&b));
			case OP_LESSEQ_TOK: return VAL_BOOL(get_number(&a) <= get_number(&b));
			case OP_GREATEREQ_TOK: return VAL_BOOL(get_number(&a) >= get_number(&b));
			case OP_EQ_TOK: return VAL_BOOL(doubles_are_equal_func(get_number(&a), get_number(&b)));
			case OP_NOTEQ_TOK: return VAL_BOOL(!doubles_are_equal_func(get_number(&a), get_number(&b)));
			case OP_NOT_TOK: return VAL_BOOL(!((bool) get_number(&a)));
			case OP_NEGATE: return VAL_NUM(-1*get_number(&a));
			case PIPE_TOK: return VAL_NUM(fabs(get_number(&a)));
			case OP_UNARY_NOTHING: return a;
			default:
				fprintf(stderr, "invalid operator on real operands: %s\n", TOK_STRINGS[op]);
				return VAL_NUM(NAN);
		}
	} else if (VAL_IS_NUM(a) && VAL_IS_NUM(b))
	{
		switch (op) {
			case OP_POW_TOK: return VAL_CNUM(cpow(get_complex(&a), get_complex(&b)));
			case OP_MUL_TOK: return VAL_CNUM(get_complex(&a) * get_complex(&b));
			case OP_DIV_TOK: return VAL_CNUM(get_complex(&a) / get_complex(&b));
			case OP_ADD_TOK: return VAL_CNUM(get_complex(&a) + get_complex(&b));
			case OP_SUB_TOK: return VAL_CNUM(get_complex(&a) - get_complex(&b));
			case OP_EQ_TOK: return VAL_BOOL(get_complex(&a) == get_complex(&b));
			case OP_NOTEQ_TOK: return VAL_BOOL(get_complex(&a) != get_complex(&b));
			case OP_NEGATE: return VAL_CNUM((-1.0 + 0.0*I) * get_complex(&a));
			case PIPE_TOK: return VAL_CNUM(cabs(get_complex(&a)));
			case OP_UNARY_NOTHING: return a;
			default:
				fprintf(stderr, "invalid operator on complex operands: %s\n", TOK_STRINGS[op]);
				return VAL_CNUM(NAN);
		}
	} else if (a.type == Vector_type
		  && b.type == RealNumber_type
		  && op == OP_DOT_TOK)
	{
		// vector index
		size_t i = (size_t)get_number(&b);
		if (fabs(i - get_number(&b)) > EPSILON || get_number(&b) < 0)
		{
			fprintf(stderr, "vectors may only be indexed by a positive integer\n");
			return VAL_NUM(NAN);
		}
		if (i >= a.v.v.n)
		{
			fprintf(stderr, "index %zu out of range for vector of length %zu\n", i, a.v.v.n);
			return VAL_NUM(NAN);
		}
		return eval_expr(state, a.v.v.ptr[i]);
	} else if (a.type == Vector_type && b.type == Vector_type
		  && op == OP_MUL_TOK
		  && a.v.v.n == b.v.v.n)
	{
		// n-dimensional dot product
		//
		// if vector contains nested vectors, performs a 'distributed dot product'
		// where the dot product of two vectors is calculated using the dot products
		// of the corresponding nested vectors in each, along with the regular
		// multiplication. (not intentionally, that's just what happens)
		double sum = 0.0;
		for (size_t i = 0; i < a.v.v.n; ++i)
		{
			sum += apply_binary_op(state,
					eval_expr(state, a.v.v.ptr[i]),
					eval_expr(state, b.v.v.ptr[i]),
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
			cur_elem = eval_expr(state, a.v.v.ptr[i]);
			sum += apply_binary_op(state, cur_elem, cur_elem, OP_MUL_TOK).v.n;
		}
		return VAL_NUM(sqrt(sum));
	}

	fprintf(stderr, "invalid operator: %s\n", TOK_STRINGS[op]);
	return VAL_NUM(NAN);
}

TypedValue eval_expr(struct evaluator_state *state, const Expr *expr)
{
	if (!state->is_init)
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
	if (expr->type == RealNumber_type)
		return VAL_NUM(expr->u.v.n);
	if (expr->type == ComplexNumber_type)
		return VAL_CNUM(expr->u.v.cn);
	if (expr->type == Boolean_type)
		return VAL_BOOL(expr->u.v.b);
	if (expr->type == String_type)
	{
		TypedValue *val;
		size_t out;
		if (hashmap_get(state->builtins, expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&val))
			return *val;
		else if (hashmap_get(state->variables, expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&out))
			return eval_expr(state, state->user_vars.ptr[out]);

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
		TypedValue right_val = eval_expr(state, right);
		double (*d_d_func) (double);
		_Complex double (*cd_cd_func) (_Complex double);
		double (*d_cd_func) (_Complex double);
		_Complex double (*cd_d_func) (double);
		void (*v_tv_func) (TypedValue *);
		if (strncmp(left->u.v.s.s, "print", 5) == 0)
		{
			if (hashmap_get(state->builtins, left->u.v.s.s, left->u.v.s.len, (uintptr_t *)&v_tv_func))
			{
				(*v_tv_func)(&right_val);
				return VAL_NUM(NAN);
			}
		}
		if (right_val.type == RealNumber_type)
		{
			if (strncmp(left->u.v.s.s, "csqrt", left->u.v.s.len) == 0)
			{
				if (hashmap_get(state->builtins, left->u.v.s.s, left->u.v.s.len, (uintptr_t *)&cd_d_func))
				{
					if (right_val.type == RealNumber_type)
						return VAL_CNUM((*cd_d_func)(right_val.v.n));
				}
				goto undefined_func;
			}
			if (hashmap_get(state->builtins, left->u.v.s.s, left->u.v.s.len, (uintptr_t *)&d_d_func))
			{
				if (right_val.type == RealNumber_type)
					return VAL_NUM((*d_d_func)(right_val.v.n));
			}
		} else if (right_val.type == ComplexNumber_type)
		{
			char *temp = calloc(left->u.v.s.len + sizeof("complex_")-1, sizeof(char));
			strncpy(temp, "complex_", sizeof("complex_")-1);
			strncpy(temp+8, left->u.v.s.s, left->u.v.s.len);

			if (strncmp(left->u.v.s.s, "phase", left->u.v.s.len) == 0
			 || strncmp(left->u.v.s.s, "real", left->u.v.s.len) == 0
			 || strncmp(left->u.v.s.s, "imag", left->u.v.s.len) == 0)
			{
				if (hashmap_get(state->builtins, temp, left->u.v.s.len + sizeof("complex_")-1, (uintptr_t *)&d_cd_func))
				{
					free(temp);
					if (right_val.type == ComplexNumber_type)
						return VAL_NUM((*d_cd_func)(right_val.v.cn));
				}
				goto undefined_func;
			}
			if (hashmap_get(state->builtins, temp, left->u.v.s.len + sizeof("complex_")-1, (uintptr_t *)&cd_cd_func))
			{
				free(temp);
				if (right_val.type == ComplexNumber_type)
					return VAL_CNUM((*cd_cd_func)(right_val.v.cn));
			}
			
			free(temp);
		}

undefined_func:
		fprintf(stderr, "undefined function for %s argument in function call: '%.*s'\n",
			(right_val.type == RealNumber_type) ? "real" :
				(right_val.type == ComplexNumber_type) ? "complex" :
					"vector",
			(int)left->u.v.s.len, left->u.v.s.s);
		return VAL_NUM(NAN);

		//fprintf(stderr, "functions with vector arguments are not yet supported\n");
		//return VAL_NUM(NAN);
	}

	return apply_binary_op(state,
			eval_expr(state, left),
			(right) ? eval_expr(state, right) : VAL_NUM(NAN),
			expr->u.o.op);
}
