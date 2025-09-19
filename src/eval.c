#include "mml/eval.h"

#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dvec/dvec.h"
#include "mml/parser.h"
#include "mml/expr.h"
#include "mml/config.h"
#include "c-hashmap/map.h"
#include "eval_funcs_incl.c"
#include "mml/token.h"

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

MML_state *MML_init_state(void)
{
	MML_state *state = calloc(1, sizeof(MML_state));

	if (eval_builtins_are_initialized)
		goto skip_builtins_init;

	eval_builtin_maps[1] = hashmap_create();
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("print"),			(uintptr_t)MML_print_typedval_multiargs);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("println"),		(uintptr_t)MML_println_typedval_multiargs);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("dbg"),			(uintptr_t)MML_print_exprh_tv_func);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("dbg_type"),		(uintptr_t)custom_dbg_type);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("dbg_ident"),		(uintptr_t)custom_dbg_ident);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("config_set"),		(uintptr_t)custom_config_set);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("max"),			(uintptr_t)custom_max);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("min"),			(uintptr_t)custom_min);
	/*hashmap_set(eval_builtin_maps[1], hashmap_str_lit("range"), 		(uintptr_t)custom_range);*/
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("atan2"),			(uintptr_t)custom_atan2);

	eval_builtin_maps[2] = hashmap_create();
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("sin"),			(uintptr_t)sin);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("cos"),			(uintptr_t)cos);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("tan"),			(uintptr_t)tan);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("asin"),			(uintptr_t)asin);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("acos"),			(uintptr_t)acos);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("atan"),			(uintptr_t)atan);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("sinh"),			(uintptr_t)sinh);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("cosh"),			(uintptr_t)cosh);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("tanh"),			(uintptr_t)tanh);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("asinh"),			(uintptr_t)asinh);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("acosh"),			(uintptr_t)acosh);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("atanh"),			(uintptr_t)atanh);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("ln"),			(uintptr_t)log);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("log2"),			(uintptr_t)log2);
	hashmap_set(eval_builtin_maps[2], hashmap_str_lit("log10"),			(uintptr_t)log10);
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
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_sinh"),	(uintptr_t)csinh);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_cosh"),	(uintptr_t)ccosh);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_tanh"),	(uintptr_t)ctanh);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_asinh"),	(uintptr_t)casinh);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_acosh"),	(uintptr_t)cacosh);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_atanh"),	(uintptr_t)catanh);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_ln"),		(uintptr_t)clog);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_log2"),	(uintptr_t)custom_clog2);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_log10"),	(uintptr_t)custom_clog10);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_sqrt"),	(uintptr_t)csqrt);
	hashmap_set(eval_builtin_maps[3], hashmap_str_lit("complex_csqrt"),	(uintptr_t)csqrt);

	eval_builtin_maps[4] = hashmap_create();
	hashmap_set(eval_builtin_maps[4], hashmap_str_lit("csqrt"),			(uintptr_t)custom_sqrt);

	eval_builtin_maps[5] = hashmap_create();
	hashmap_set(eval_builtin_maps[5], hashmap_str_lit("complex_conj"),	(uintptr_t)conj);
	hashmap_set(eval_builtin_maps[5], hashmap_str_lit("complex_phase"),	(uintptr_t)carg);
	hashmap_set(eval_builtin_maps[5], hashmap_str_lit("complex_real"),	(uintptr_t)creal);
	hashmap_set(eval_builtin_maps[5], hashmap_str_lit("complex_imag"),	(uintptr_t)cimag);

	static constexpr MML_Value TRUE_M		= VAL_BOOL(true);
	static constexpr MML_Value FALSE_M		= VAL_BOOL(false);
	static constexpr MML_Value PI_M		= VAL_NUM(3.14159265358979323846);
	static constexpr MML_Value E_M		= VAL_NUM(2.71828182845904523536);
	static constexpr MML_Value PHI_M		= VAL_NUM(1.61803398874989484820);
	static constexpr MML_Value I_M		= VAL_CNUM(I);
	static constexpr MML_Value NAN_M		= VAL_NUM(NAN);
	static constexpr MML_Value INFINITY_M	= VAL_NUM(INFINITY);
	static constexpr MML_Value EXIT_CMD_M	= { Invalid_type, .v.i = MML_QUIT_INVAL };
	static constexpr MML_Value CLEAR_CMD_M	= { Invalid_type, .v.i = MML_CLEAR_INVAL };

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
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("exit"),	(uintptr_t)&EXIT_CMD_M);
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("clear"),	(uintptr_t)&CLEAR_CMD_M);


	eval_builtins_are_initialized = true;

skip_builtins_init:

	state->variables = nullptr;
	dv_init(state->vars_storage);
	dv_init(state->exprs);
	dv_init(state->allocd_vecs);


	state->is_init = true;
	++initialized_evaluators_count;

	return state;
}

void MML_cleanup_state(MML_state *restrict state)
{
	if (state->variables != nullptr)
	{
		hashmap_free(state->variables);
		state->variables = nullptr;
	}

	MML_free_vec(&state->vars_storage);
	state->vars_storage = (MML_ExprVec) {0};

	MML_free_vec(&state->exprs);
	state->exprs = (MML_ExprVec) {0};

	MML_free_vec(&state->allocd_vecs);
	state->allocd_vecs = (MML_ExprVec) {0};

	state->is_init = false;
	if (--initialized_evaluators_count == 0)
	{
		for (uint8_t i = 0; i < 6; ++i)
		{
			hashmap_free(eval_builtin_maps[i]);
			eval_builtin_maps[i] = nullptr;
		}
	}

	free(state);
}

int32_t MML_eval_set_variable(MML_state *restrict state,
		strbuf name, MML_Expr *expr)
{
	dv_push(state->vars_storage, expr);
	if (state->variables == nullptr)
		state->variables = hashmap_create();
	return hashmap_set(state->variables,
			name.s, name.len, (uintptr_t)dv_n(state->vars_storage)-1);
}
MML_Expr *MML_eval_get_variable(MML_state *restrict state,
		strbuf name)
{
	size_t out;
	if (state->variables != nullptr && 
			hashmap_get(state->variables, name.s, name.len, (uintptr_t *)&out))
		return dv_a(state->vars_storage, out);

	return NULL;
}

#define EPSILON 1e-15

static MML_Value apply_func(MML_state *state,
		strbuf ident, MML_Value right_vec)
{
	MML_val_func vec_args_func;
	if (hashmap_get(eval_builtin_maps[1], ident.s, ident.len, (uintptr_t *)&vec_args_func))
		return ((*vec_args_func)(state, &right_vec.v.v));

	double (*d_d_func) (double);
	_Complex double (*cd_cd_func) (_Complex double);
	_Complex double (*cd_d_func) (double);
	double (*d_cd_func) (_Complex double);

	const MML_Value first_arg_val = MML_eval_expr(state, dv_a(right_vec.v.v, 0));
	if (first_arg_val.type == RealNumber_type)
	{
		if (hashmap_get(eval_builtin_maps[4], ident.s, ident.len, (uintptr_t *)&cd_d_func))
			if (first_arg_val.type == RealNumber_type)
				return VAL_CNUM((*cd_d_func)(first_arg_val.v.n));
		if (hashmap_get(eval_builtin_maps[2], ident.s, ident.len, (uintptr_t *)&d_d_func))
			if (first_arg_val.type == RealNumber_type)
				return VAL_NUM((*d_d_func)(first_arg_val.v.n));
	} else if (first_arg_val.type == ComplexNumber_type)
	{
		char *temp __attribute__((cleanup(MML_free_pp)))
			= calloc(ident.len + sizeof("complex_")-1, sizeof(char));
		memcpy(temp, "complex_", sizeof("complex_")-1);
		memcpy(temp+8, ident.s, ident.len);

		if (hashmap_get(eval_builtin_maps[5], temp, ident.len + sizeof("complex_")-1, (uintptr_t *)&d_cd_func))
			if (first_arg_val.type == ComplexNumber_type)
				return VAL_NUM((*d_cd_func)(first_arg_val.v.cn));

		if (hashmap_get(eval_builtin_maps[3], temp, ident.len + sizeof("complex_")-1, (uintptr_t *)&cd_cd_func))
			if (first_arg_val.type == ComplexNumber_type)
				return VAL_CNUM((*cd_cd_func)(first_arg_val.v.cn));
	}


	fprintf(stderr, "undefined function for %s argument in function call: '%.*s'\n",
			EXPR_TYPE_STRINGS[first_arg_val.type],
			(int)ident.len, ident.s);
	return VAL_INVAL;
}

MML_Value MML_apply_binary_op(MML_state *restrict state, MML_Value a, MML_Value b, MML_TokenType op)
{
	if (a.type == Invalid_type)
		return VAL_INVAL;

	if (VAL_IS_NUM(a) && b.type == Invalid_type)
	{
		// unary operators
		switch (op) {
		case MML_OP_NOT_TOK:
			if (a.type != ComplexNumber_type)
				return VAL_BOOL(MML_get_number(&a) == 0);

			fprintf(stderr, "invalid unary operator on complex operand: %s\n", TOK_STRINGS[MML_OP_NOT_TOK]);	
			return VAL_INVAL;
		case MML_OP_NEGATE: return (a.type == ComplexNumber_type)
				? VAL_CNUM(-MML_get_complex(&a))
				: VAL_NUM(-MML_get_number(&a));
		case MML_PIPE_TOK: return (a.type == ComplexNumber_type)
				? VAL_CNUM(cabs(MML_get_complex(&a)))
				: VAL_NUM(fabs(MML_get_number(&a)));
		case MML_TILDE_TOK:
			MML_Expr *ret = calloc(1, sizeof(MML_Expr));
			ret->type = Vector_type;
			ret->should_free_vec_block = true;
			dv_init(ret->u.v.v);
			dv_resize(ret->u.v.v, 2);

			MML_Expr *data = calloc(2, sizeof(MML_Expr));
			const MML_Value negated_a = MML_apply_binary_op(state,
					a,
					VAL_INVAL,
					MML_OP_NEGATE);
			data[0] = (MML_Expr) { a.type, .num_refs=1, .u.v=a.v };
			data[1] = (MML_Expr) { negated_a.type, .num_refs=1, .u.v=negated_a.v };
			dv_a(ret->u.v.v, 0) = &data[0];
			dv_a(ret->u.v.v, 1) = &data[1];

			dv_push(state->allocd_vecs, ret);
			return (MML_Value) { Vector_type, .v = ret->u.v };
		case MML_OP_UNARY_NOTHING: return a;
		default:
			//fprintf(stderr, "invalid unary operator on %s operand: %s\n",
			//		(a.type == ComplexNumber_type) ? "complex" : "real",
			//		TOK_STRINGS[op]);
			return VAL_INVAL;
		}
	} else if (VAL_IS_NUM(a) && VAL_IS_NUM(b)
		&& a.type != ComplexNumber_type && b.type != ComplexNumber_type)
	{
		switch (op) {
			case MML_OP_POW_TOK: return VAL_NUM(pow(MML_get_number(&a), MML_get_number(&b)));
			case MML_OP_MUL_TOK: return VAL_NUM(MML_get_number(&a) * MML_get_number(&b));
			case MML_OP_DIV_TOK: return VAL_NUM(MML_get_number(&a) / MML_get_number(&b));
			case MML_OP_MOD_TOK: return VAL_NUM(fmod(MML_get_number(&a), MML_get_number(&b)));
			case MML_OP_ADD_TOK: return VAL_NUM(MML_get_number(&a) + MML_get_number(&b));
			case MML_OP_SUB_TOK: return VAL_NUM(MML_get_number(&a) - MML_get_number(&b));
			case MML_OP_LESS_TOK: return VAL_BOOL(MML_get_number(&a) < MML_get_number(&b));
			case MML_OP_GREATER_TOK: return VAL_BOOL(MML_get_number(&a) > MML_get_number(&b));
			case MML_OP_LESSEQ_TOK: return VAL_BOOL(MML_get_number(&a) <= MML_get_number(&b));
			case MML_OP_GREATEREQ_TOK: return VAL_BOOL(MML_get_number(&a) >= MML_get_number(&b));
			case MML_OP_EQ_TOK: return VAL_BOOL(fabs(MML_get_number(&a) - MML_get_number(&b)) < EPSILON);
			case MML_OP_NOTEQ_TOK: return VAL_BOOL(fabs(MML_get_number(&a) - MML_get_number(&b)) >= EPSILON);
			case MML_OP_EXACT_EQ: return VAL_BOOL(MML_get_number(&a) == MML_get_number(&b));
			case MML_OP_EXACT_NOTEQ: return VAL_BOOL(MML_get_number(&a) != MML_get_number(&b));
			default:
				fprintf(stderr, "invalid binary operator on real operands: %s\n", TOK_STRINGS[op]);
				return VAL_INVAL;
		}
	} else if (VAL_IS_NUM(a) && VAL_IS_NUM(b))
	{
		switch (op) {
			case MML_OP_POW_TOK: return VAL_CNUM(cpow(MML_get_complex(&a), MML_get_complex(&b)));
			case MML_OP_MUL_TOK: return VAL_CNUM(MML_get_complex(&a) * MML_get_complex(&b));
			case MML_OP_DIV_TOK: return VAL_CNUM(MML_get_complex(&a) / MML_get_complex(&b));
			case MML_OP_ADD_TOK: return VAL_CNUM(MML_get_complex(&a) + MML_get_complex(&b));
			case MML_OP_SUB_TOK: return VAL_CNUM(MML_get_complex(&a) - MML_get_complex(&b));
			case MML_OP_EQ_TOK:
			case MML_OP_EXACT_EQ:
				return VAL_BOOL(MML_get_complex(&a) == MML_get_complex(&b));
			case MML_OP_NOTEQ_TOK:
			case MML_OP_EXACT_NOTEQ:
				return VAL_BOOL(MML_get_complex(&a) != MML_get_complex(&b));
			default:
				fprintf(stderr, "invalid binary operator on complex operands: %s\n", TOK_STRINGS[op]);
				return VAL_INVAL;
		}
	} else if (a.type == Vector_type && b.type == RealNumber_type
			&& op == MML_OP_DOT_TOK)
	{
		if (a.type == RealNumber_type)
		{
			fprintf(stderr, "cannot index real number with vector\n");
			return VAL_INVAL;
		}
		// vector index
		size_t i = (size_t)MML_get_number(&b);
		if (fabs(i - MML_get_number(&b)) > EPSILON || MML_get_number(&b) < 0)
		{
			fprintf(stderr, "vectors may only be indexed by a positive integer\n");
			return VAL_INVAL;
		}
		if (i >= dv_n(a.v.v))
		{
			fprintf(stderr, "index %zu out of range for vector of length %zu\n", i, dv_n(a.v.v));
			return VAL_INVAL;
		}
		return MML_eval_expr(state, dv_a(a.v.v, i));
	} else if (a.type == Vector_type && b.type == Vector_type
		  && op == MML_OP_MUL_TOK
		  && dv_n(a.v.v) == dv_n(b.v.v))
	{
		// n-dimensional dot product
		//
		// if vector contains nested vectors, performs a 'distributed dot product'
		// where the dot product of two vectors is calculated using the dot products
		// of the corresponding nested vectors in each, along with the regular
		// multiplication. (not intentionally, that's just what happens)
		double sum = 0.0;
		for (size_t i = 0; i < dv_n(a.v.v); ++i)
		{
			sum += MML_apply_binary_op(state,
					MML_eval_expr(state, dv_a(a.v.v, i)),
					MML_eval_expr(state, dv_a(b.v.v, i)),
					MML_OP_MUL_TOK).v.n;
		}
		return VAL_NUM(sum);
	} else if (a.type == Vector_type && op == MML_PIPE_TOK)
	{
		// compute vector magnitude
		_Complex double sum = 0.0;
		MML_Value cur_elem;
		for (size_t i = 0; i < dv_n(a.v.v); ++i)
		{
			cur_elem = MML_eval_expr(state, dv_a(a.v.v, i));
			sum += MML_apply_binary_op(state, cur_elem, cur_elem, MML_OP_MUL_TOK).v.n;
		}
		_Complex double ret = csqrt(sum);
		return (cimag(ret) == 0.0) ? VAL_NUM(creal(ret)) : VAL_CNUM(ret);
	} else if ((a.type == Vector_type && VAL_IS_NUM(b)) ||
		     (VAL_IS_NUM(a) && b.type == Vector_type))
	{
		switch (op) {
		case MML_OP_ADD_TOK:
		case MML_OP_SUB_TOK:
		case MML_OP_MUL_TOK:
		case MML_OP_DIV_TOK:
			const MML_ExprVec *src_vec = (a.type == Vector_type)
				? &a.v.v
				: &b.v.v;
			const MML_Value *scalar = (VAL_IS_NUM(a)) ? &a : &b;

			MML_Expr *ret = calloc(1, sizeof(MML_Expr));
			ret->type = Vector_type;
			ret->should_free_vec_block = true;
			dv_init(ret->u.v.v);
			dv_resize(ret->u.v.v, dv_n(*src_vec));

			MML_Expr *data = calloc(dv_n(*src_vec), sizeof(MML_Expr));
			for (size_t i = 0; i < dv_n(*src_vec); ++i)
			{
				MML_Value cur = MML_apply_binary_op(state,
						MML_eval_expr(state, dv_a(*src_vec, i)),
						*scalar,
						op);
				data[i].type = cur.type;
				data[i].num_refs = 1;
				data[i].u.v = cur.v;
				dv_a(ret->u.v.v, i) = &data[i];
			}
			dv_push(state->allocd_vecs, ret);
			return (MML_Value) { Vector_type, .v = ret->u.v };
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

MML_Value MML_eval_expr_recurse(MML_state *state, const MML_Expr *expr)
{
	if (!state->is_init)
	{
		fprintf(stderr, "you must run `MML_init_state` before using any evaluator functions.\n");
		return VAL_INVAL;
	}

	if (expr == nullptr)
		return VAL_INVAL;
	switch (expr->type) {
	case Invalid_type:
		return VAL_INVAL;
	case Vector_type:
		return (MML_Value) { Vector_type, .v.v = expr->u.v.v };
	case RealNumber_type:
		return VAL_NUM(expr->u.v.n);
	case ComplexNumber_type:
		return VAL_CNUM(expr->u.v.cn);
	case Boolean_type:
		return VAL_BOOL(expr->u.v.b);
	case Identifier_type: {
		MML_Value *val;
		if (strncmp(expr->u.v.s.s, "ans", expr->u.v.s.len) == 0)
			return state->last_val;
		if (hashmap_get(eval_builtin_maps[0], expr->u.v.s.s, expr->u.v.s.len, (uintptr_t *)&val))
			return *val;
		
		MML_Expr *e = MML_eval_get_variable(state, expr->u.v.s);
		if (e != NULL)
			return MML_eval_expr_recurse(state, e);

		fprintf(stderr, "warning: undefined identifier: '%.*s'\n",
				(int)expr->u.v.s.len, expr->u.v.s.s);
		return VAL_INVAL;
	}
	default:
		break;
	}

	const MML_Expr *left = expr->u.o.left;
	const MML_Expr *right = expr->u.o.right;

	if (expr->u.o.op == MML_OP_ASSERT_EQUAL && left->type == Identifier_type)
	{
		MML_eval_set_variable(state, left->u.v.s, (MML_Expr *)right);
		return MML_eval_expr_recurse(state, right);
	} else if (expr->u.o.op == MML_OP_FUNC_CALL_TOK)
	{
		if (left == NULL
		 || right == NULL
		 || left->type != Identifier_type)
			return VAL_INVAL;
		MML_Value right_val_vec = MML_eval_expr_recurse(state, right);
		if (right_val_vec.type == Invalid_type)
			return VAL_INVAL;
		return apply_func(state, left->u.v.s, right_val_vec);
	}

	return MML_apply_binary_op(state,
			MML_eval_expr_recurse(state, left),
			(right) ? MML_eval_expr_recurse(state, right) : VAL_INVAL,
			expr->u.o.op);
}
inline MML_Value MML_eval_expr(MML_state *state, const MML_Expr *expr)
{
	return state->last_val = MML_eval_expr_recurse(state, expr);
}

inline int32_t MML_eval_push_expr(MML_state *state, MML_Expr *expr)
{
	--expr->num_refs;
	return dv_push(state->exprs, expr);
}
inline MML_Value MML_eval_top_expr(MML_state *state)
{
	return MML_eval_expr_recurse(state, *(const MML_Expr **)dv_peek(state->exprs));
}
