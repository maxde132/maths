#include "eval.h"

#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "token.h"
#include "expr.h"
#include "config.h"
#include "c-hashmap/map.h"
#include "eval_funcs_incl.c"

/* 0 constants
 * 1 tv_tv_funcs
 * 2 d_d_funcs
 * 3 cd_cd_funcs
 * 4 cd_d_funcs
 * 5 d_cd_funcs
 */
static hashmap *eval_builtin_maps[] = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
};
static bool eval_builtins_are_initialized = false;
static size_t initialized_evaluators_count = 0;

struct evaluator_state eval_init(struct evaluator_state *restrict state_out)
{
	/* todo: finish this! */
	struct evaluator_state state = {0};

	if (eval_builtins_are_initialized)
		goto skip_builtins_init;

	/* TODO UPDATE apply_funcs TO USE THESE MAPS */
	eval_builtin_maps[1] = hashmap_create();
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("print"),			(uintptr_t)print_typedval);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("println"),		(uintptr_t)println_typedval);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("max"),			(uintptr_t)custom_max);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("min"),			(uintptr_t)custom_min);
	/*hashmap_set(eval_builtin_maps[1], hashmap_str_lit("range"), 		(uintptr_t)custom_range);*/

	eval_builtin_maps[2] = hashmap_create();
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("sin"),			(uintptr_t)sin);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("cos"),			(uintptr_t)cos);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("tan"),			(uintptr_t)tan);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("asin"),			(uintptr_t)asin);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("acos"),			(uintptr_t)acos);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("atan"),			(uintptr_t)atan);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("ln"),			(uintptr_t)log);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("log2"),			(uintptr_t)log2);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("sqrt"),			(uintptr_t)sqrt);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("floor"),			(uintptr_t)floor);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("ceil"),			(uintptr_t)ceil);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("round"),			(uintptr_t)round);

	eval_builtin_maps[3] = hashmap_create();
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_sin"),		(uintptr_t)csin);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_cos"),		(uintptr_t)ccos);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_tan"),		(uintptr_t)ctan);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_asin"),	(uintptr_t)casin);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_acos"),	(uintptr_t)cacos);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_atan"),	(uintptr_t)catan);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_ln"),		(uintptr_t)clog);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_log2"),	(uintptr_t)custom_clog2);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_sqrt"),	(uintptr_t)csqrt);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_csqrt"),	(uintptr_t)csqrt);

	eval_builtin_maps[4] = hashmap_create();
	hashmap_set(eval_builtin_maps[4], hashmap_str_lit("csqrt"),			(uintptr_t)custom_sqrt);

	eval_builtin_maps[5] = hashmap_create();
	hashmap_set(eval_builtin_maps[5], hashmap_str_lit("complex_conj"),	(uintptr_t)conj);
	hashmap_set(eval_builtin_maps[5], hashmap_str_lit("complex_phase"),	(uintptr_t)carg);
	hashmap_set(eval_builtin_maps[5], hashmap_str_lit("complex_real"),	(uintptr_t)creal);
	hashmap_set(eval_builtin_maps[5], hashmap_str_lit("complex_imag"),	(uintptr_t)cimag);

	static constexpr TypedValue TRUE_M		= VAL_BOOL(true);
	static constexpr TypedValue FALSE_M		= VAL_BOOL(false);
	static constexpr TypedValue PI_M		= VAL_NUM(3.14159265358979323846);
	static constexpr TypedValue E_M		= VAL_NUM(2.71828182845904523536);
	static constexpr TypedValue PHI_M		= VAL_NUM(1.61803398874989484820);
	static constexpr TypedValue I_M		= VAL_CNUM(I);
	static constexpr TypedValue NAN_M		= VAL_NUM(NAN);
	static constexpr TypedValue INFINITY_M	= VAL_NUM(INFINITY);

	eval_builtin_maps[0] = hashmap_create();
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("true"),	(uintptr_t)&TRUE_M);
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("false"),	(uintptr_t)&FALSE_M);
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("pi"),	(uintptr_t)&PI_M);
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("e"),	(uintptr_t)&E_M);
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("phi"),	(uintptr_t)&PHI_M);
	static_assert(I_M.v.cn == I, "how on earth does I != I? i think your computer's borked\n");
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("i"),	(uintptr_t)&I_M);
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("nan"),	(uintptr_t)&NAN_M);
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("inf"),	(uintptr_t)&INFINITY_M);


	eval_builtins_are_initialized = true;

skip_builtins_init:

	state.variables = nullptr;
	state.inserted_vars = nullptr;
	state.vars_storage = new_vec(1);
	state.exprs = new_vec(1);
	state.allocd_vecs = new_vec(1);


	state.is_init = true;
	++initialized_evaluators_count;

	if (state_out != nullptr)
		*state_out = state;

	return state;
}

void eval_cleanup(struct evaluator_state *restrict state)
{
	if (state->variables != nullptr)
	{
		hashmap_free(state->variables);
		state->variables = nullptr;
	}

	if (state->inserted_vars != nullptr)
	{
		hashmap_free(state->inserted_vars);
		state->inserted_vars = nullptr;
	}

	free_vec(&state->vars_storage);
	state->vars_storage = (VecN) { nullptr, 0, 0 };

	free_vec(&state->exprs);
	state->exprs = (VecN) { nullptr, 0, 0 };

	free_vec(&state->allocd_vecs);
	state->allocd_vecs = (VecN) { nullptr, 0, 0 };

	state->is_init = false;
	if (--initialized_evaluators_count == 0)
	{
		for (uint8_t i = 0; i < 6; ++i)
		{
			hashmap_free(eval_builtin_maps[i]);
			eval_builtin_maps[i] = nullptr;
		}
	}
}

int32_t eval_set_variable(struct evaluator_state *restrict state,
		strbuf name, Expr *expr, bool is_inserted)
{
	push_to_vec(&state->vars_storage, expr);
	if (is_inserted)
	{
		if (state->inserted_vars == nullptr)
			state->inserted_vars = hashmap_create();
		return hashmap_set(state->inserted_vars,
				name.s, name.len, (uintptr_t)state->vars_storage.n-1);
	} else
	{
		if (state->variables == nullptr)
			state->variables = hashmap_create();
		return hashmap_set(state->variables,
				name.s, name.len, (uintptr_t)state->vars_storage.n-1);
	}
}

#define EPSILON 1e-15

inline bool doubles_are_equal_func(double a, double b)
{
	if (FLAG_IS_SET(ESTIMATE_EQUALITY))
		return fabs(a-b) < EPSILON;
	return a == b;
}
bool doubles_are_equal_func(double, double);

TypedValue apply_func(struct evaluator_state *state,
		strbuf ident, TypedValue right)
{
	TypedValue (*vec_args_func) (struct evaluator_state *, TypedValue *);
	if (hashmap_get(eval_builtin_maps[1], ident.s, ident.len, (uintptr_t *)&vec_args_func))
		return ((*vec_args_func)(state, &right));

	double (*d_d_func) (double);
	_Complex double (*cd_cd_func) (_Complex double);
	_Complex double (*cd_d_func) (double);
	double (*d_cd_func) (_Complex double);
	if (right.type == RealNumber_type)
	{
		if (hashmap_get(eval_builtin_maps[4], ident.s, ident.len, (uintptr_t *)&cd_d_func))
			if (right.type == RealNumber_type)
				return VAL_CNUM((*cd_d_func)(right.v.n));
		if (hashmap_get(eval_builtin_maps[2], ident.s, ident.len, (uintptr_t *)&d_d_func))
			if (right.type == RealNumber_type)
				return VAL_NUM((*d_d_func)(right.v.n));
	} else if (right.type == ComplexNumber_type)
	{
		char *temp __attribute__((cleanup(free_pp)))
			= calloc(ident.len + sizeof("complex_")-1, sizeof(char));
		memcpy(temp, "complex_", sizeof("complex_")-1);
		memcpy(temp+8, ident.s, ident.len);

		if (hashmap_get(eval_builtin_maps[5], temp, ident.len + sizeof("complex_")-1, (uintptr_t *)&d_cd_func))
			if (right.type == ComplexNumber_type)
				return VAL_NUM((*d_cd_func)(right.v.cn));

		if (hashmap_get(eval_builtin_maps[3], temp, ident.len + sizeof("complex_")-1, (uintptr_t *)&cd_cd_func))
			if (right.type == ComplexNumber_type)
				return VAL_CNUM((*cd_cd_func)(right.v.cn));
	}

	fprintf(stderr, "undefined function for %s argument in function call: '%.*s'\n",
		(right.type == RealNumber_type) ? "real" :
			(right.type == ComplexNumber_type) ? "complex" :
				"vector",
		(int)ident.len, ident.s);

	return VAL_INVAL;
}

TypedValue apply_binary_op(struct evaluator_state *restrict state, TypedValue a, TypedValue b, TokenType op)
{
	if (VAL_IS_NUM(a) && b.type == Invalid_type)
	{
		switch (op) {
		case OP_NOT_TOK:
			if (a.type != ComplexNumber_type)
				return VAL_BOOL(get_number(&a) == 0);

			fprintf(stderr, "invalid unary operator on complex operand: %s\n", TOK_STRINGS[OP_NOT_TOK]);	
			return VAL_INVAL;
		case OP_NEGATE: return (a.type == ComplexNumber_type)
				? VAL_CNUM((-1.0 + 0.0*I) * get_complex(&a))
				: VAL_NUM(-1*get_number(&a));
		case PIPE_TOK: return (a.type == ComplexNumber_type)
				? VAL_CNUM(cabs(get_complex(&a)))
				: VAL_NUM(fabs(get_number(&a)));
		case TILDE_TOK:
			Expr *ret = calloc(1, sizeof(Expr));
			ret->type = Vector_type;
			ret->should_free_vec_block = true;
			ret->u.v.v = new_vec(2);
			ret->u.v.v.n = 2;

			Expr *data = calloc(2, sizeof(Expr));
			const TypedValue not_negated = a;
			const TypedValue negated = apply_binary_op(state,
					a,
					VAL_INVAL,
					OP_NEGATE);
			data[0] = (Expr) {
				not_negated.type, .num_refs=1, .u.v=not_negated.v
			};
			data[1] = (Expr) {
				negated.type, .num_refs=1, .u.v=negated.v
			};
			ret->u.v.v.ptr[0] = &data[0];
			ret->u.v.v.ptr[1] = &data[1];
			push_to_vec(&state->allocd_vecs, ret);
			return (TypedValue) { Vector_type, .v = ret->u.v };
		case OP_UNARY_NOTHING: return a;
		default:
			fprintf(stderr, "invalid unary operator on %s operand: %s\n",
					(a.type == ComplexNumber_type) ? "complex" : "real",
					TOK_STRINGS[op]);
			return VAL_INVAL;
		}
	} else if (VAL_IS_NUM(a) && VAL_IS_NUM(b)
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
			default:
				fprintf(stderr, "invalid binary operator on real operands: %s\n", TOK_STRINGS[op]);
				return VAL_INVAL;
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
			default:
				fprintf(stderr, "invalid binary operator on complex operands: %s\n", TOK_STRINGS[op]);
				return VAL_INVAL;
		}
	} else if (a.type == Vector_type && b.type == RealNumber_type
			&& op == OP_DOT_TOK)
	{
		if (a.type == RealNumber_type)
		{
			fprintf(stderr, "cannot index real number with vector\n");
			return VAL_INVAL;
		}
		// vector index
		size_t i = (size_t)get_number(&b);
		if (fabs(i - get_number(&b)) > EPSILON || get_number(&b) < 0)
		{
			fprintf(stderr, "vectors may only be indexed by a positive integer\n");
			return VAL_INVAL;
		}
		if (i >= a.v.v.n)
		{
			fprintf(stderr, "index %zu out of range for vector of length %zu\n", i, a.v.v.n);
			return VAL_INVAL;
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
		_Complex double sum = 0.0;
		TypedValue cur_elem;
		for (size_t i = 0; i < a.v.v.n; ++i)
		{
			cur_elem = eval_expr(state, a.v.v.ptr[i]);
			sum += apply_binary_op(state, cur_elem, cur_elem, OP_MUL_TOK).v.n;
		}
		_Complex double ret = csqrt(sum);
		return (cimag(ret) == 0.0) ? VAL_NUM(creal(ret)) : VAL_CNUM(ret);
	} else if ((a.type == Vector_type && VAL_IS_NUM(b)) ||
		     (VAL_IS_NUM(a) && b.type == Vector_type))
	{
		switch (op) {
		case OP_ADD_TOK:
		case OP_SUB_TOK:
		case OP_MUL_TOK:
		case OP_DIV_TOK:
			const VecN *src_vec = (a.type == Vector_type)
				? &a.v.v
				: &b.v.v;
			const TypedValue *scalar = (VAL_IS_NUM(a)) ? &a : &b;

			Expr *ret = calloc(1, sizeof(Expr));
			ret->type = Vector_type;
			ret->should_free_vec_block = true;
			ret->u.v.v = new_vec(src_vec->n);
			ret->u.v.v.n = src_vec->n;

			Expr *data = calloc(src_vec->n, sizeof(Expr));
			for (size_t i = 0; i < src_vec->n; ++i)
			{
				TypedValue cur = apply_binary_op(state,
						eval_expr(state, src_vec->ptr[i]),
						*scalar,
						op);
				data[i].type = cur.type;
				data[i].num_refs = 1;
				data[i].u.v = cur.v;
				ret->u.v.v.ptr[i] = &data[i];
			}
			push_to_vec(&state->allocd_vecs, ret);
			return (TypedValue) { Vector_type, .v = ret->u.v };
		default:
			fprintf(stderr,
					"invalid binary operator on Vector and arithmetic operands: %s\n",
					TOK_STRINGS[op]);
			return VAL_INVAL;
		}

	}

	fprintf(stderr, "invalid operator: %s\n", TOK_STRINGS[op]);
	return VAL_INVAL;
}

TypedValue eval_expr(struct evaluator_state *state, const Expr *expr)
{
	if (!state->is_init)
	{
		fprintf(stderr, "you must run `init_evaluator` before using any evaluator functions.\n");
		return VAL_INVAL;
	}

	if (expr == nullptr)
		return VAL_INVAL;
	else if (expr->type == Vector_type)
		return (TypedValue) { Vector_type, .v.v = expr->u.v.v };
	else if (expr->type == RealNumber_type)
		return VAL_NUM(expr->u.v.n);
	else if (expr->type == ComplexNumber_type)
		return VAL_CNUM(expr->u.v.cn);
	else if (expr->type == Boolean_type)
		return VAL_BOOL(expr->u.v.b);
	else if (expr->type == Identifier_type)
	{
		TypedValue *val;
		size_t out;
		if (hashmap_get(eval_builtin_maps[0], expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&val))
			return *val;
		else if (hashmap_get(state->variables, expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&out))
			return eval_expr(state, state->vars_storage.ptr[out]);

		fprintf(stderr, "undefined identifier in user-defined variable namespace: '%.*s'\n",
				(int)expr->u.v.s.len, expr->u.v.s.s);
		return VAL_INVAL;
	} else if (expr->type == InsertedIdentifier_type)
	{
		size_t out;
		if (state->inserted_vars != nullptr
				&& hashmap_get(state->inserted_vars, expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&out))
			return eval_expr(state, state->vars_storage.ptr[out]);

		fprintf(stderr, "undefined identifier in inserted variable namespace: '$%.*s'\n",
				(int)expr->u.v.s.len, expr->u.v.s.s);
		return VAL_INVAL;
	}

	const Expr *left = expr->u.o.left;
	const Expr *right = expr->u.o.right;

	if (expr->u.o.op == OP_ASSERT_EQUAL
			&& (left->type == Identifier_type || left->type == InsertedIdentifier_type))
	{
		eval_set_variable(state, left->u.v.s,
				(Expr *)right,
				left->type == InsertedIdentifier_type);
		return eval_expr(state, right);
	} else if (expr->u.o.op == OP_FUNC_CALL_TOK)
	{
		if (left == NULL
		 || right == NULL
		 || left->type != Identifier_type)
			return VAL_INVAL;
		TypedValue right_val = eval_expr(state, right);
		return apply_func(state, left->u.v.s, right_val);
	}

	return apply_binary_op(state,
			eval_expr(state, left),
			(right) ? eval_expr(state, right) : VAL_INVAL,
			expr->u.o.op);
}

inline int32_t eval_push_expr(struct evaluator_state *state, Expr *expr)
{
	--expr->num_refs;
	return push_to_vec(&state->exprs, expr);
}
inline TypedValue eval_top_expr(struct evaluator_state *state)
{
	return eval_expr(state, *(const Expr **)peek_top_vec(&state->exprs));
}
