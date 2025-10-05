#include "mml/eval.h"

#include <complex.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mml/expr.h"
#include "mml/config.h"
#include "mml/token.h"
#include "mml/parser.h"
#include "arena/arena.h"
#include "cvi/dvec/dvec.h"
#include "c-hashmap/map.h"

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
Arena *MML_global_arena = NULL;

void math__register_functions(hashmap *maps[6]);
void stdmml__register_functions(hashmap *maps[6]);

MML_state *MML_init_state(void)
{
	MML_state *state = calloc(1, sizeof(MML_state));
	state->config = &MML_global_config;

	if (eval_builtins_are_initialized)
		goto skip_builtins_init;

	MML_global_arena = arena_make(1<<13);

	eval_builtin_maps[1] = hashmap_create();
	eval_builtin_maps[2] = hashmap_create();
	eval_builtin_maps[3] = hashmap_create();
	eval_builtin_maps[4] = hashmap_create();
	eval_builtin_maps[5] = hashmap_create();

	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("print"),			(uintptr_t)MML_print_typedval_multiargs);
	hashmap_set(eval_builtin_maps[1], hashmap_str_lit("println"),		(uintptr_t)MML_println_typedval_multiargs);


	eval_builtin_maps[0] = hashmap_create();
	static constexpr MML_value EXIT_CMD_M	= { Invalid_type, .i = MML_QUIT_INVAL };
	static constexpr MML_value CLEAR_CMD_M	= { Invalid_type, .i = MML_CLEAR_INVAL };

	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("exit"),	(uintptr_t)&EXIT_CMD_M);
	hashmap_set(eval_builtin_maps[0], hashmap_str_lit("clear"),	(uintptr_t)&CLEAR_CMD_M);

	math__register_functions(eval_builtin_maps);
	stdmml__register_functions(eval_builtin_maps);

	eval_builtins_are_initialized = true;

skip_builtins_init:

	state->variables = nullptr;
	dv_init(state->exprs);


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

	arena_index *cur;
	dv_foreach(state->exprs, cur)
		MML_free_expr(*cur);
	dv_destroy(state->exprs);

	state->is_init = false;
	if (--initialized_evaluators_count == 0)
	{
		for (uint8_t i = 0; i < 6; ++i)
		{
			hashmap_free(eval_builtin_maps[i]);
			eval_builtin_maps[i] = nullptr;
		}

		arena_free(MML_global_arena);
	}

	free(state);
}

int32_t MML_eval_set_variable(MML_state *restrict state,
		strbuf name, arena_index expr)
{
	if (state->variables == nullptr)
		state->variables = hashmap_create();
	return hashmap_set(state->variables,
			name.s, name.len, (uintptr_t)expr);
}
arena_index MML_eval_get_variable(MML_state *restrict state,
		strbuf name)
{
	arena_index out;
	if (state->variables != nullptr && 
			hashmap_get(state->variables, name.s, name.len, (uintptr_t *)&out))
		return out;

	return SIZE_MAX;
}

#define EPSILON 1e-14

static MML_value apply_func(MML_state *restrict state,
		strbuf ident, MML_value right_vec)
{
	MML_val_func vec_args_func;
	if (hashmap_get(eval_builtin_maps[1], ident.s, ident.len, (uintptr_t *)&vec_args_func))
		return ((*vec_args_func)(state, &right_vec.v));

	double (*d_d_func) (double);
	_Complex double (*cd_cd_func) (_Complex double);
	_Complex double (*cd_d_func) (double);
	double (*d_cd_func) (_Complex double);

	if (right_vec.v.n == 0)
	{
		MML_log_err("undefined function for empty argument list in call to function: '%.*s'\n",
				(int)ident.len, ident.s);
		return VAL_INVAL;
	}
	arena_index *right_vec_p = mml_i(right_vec.v.i, arena_index);
	const MML_value first_arg_val = MML_eval_expr(state, right_vec_p[0]);
	if (first_arg_val.type == RealNumber_type)
	{
		if (hashmap_get(eval_builtin_maps[4], ident.s, ident.len, (uintptr_t *)&cd_d_func))
			if (first_arg_val.type == RealNumber_type)
				return VAL_CNUM((*cd_d_func)(first_arg_val.n));
		if (hashmap_get(eval_builtin_maps[2], ident.s, ident.len, (uintptr_t *)&d_d_func))
			if (first_arg_val.type == RealNumber_type)
				return VAL_NUM((*d_d_func)(first_arg_val.n));
	} else if (first_arg_val.type == ComplexNumber_type)
	{
		char *temp __attribute__((cleanup(MML_free_pp)))
			= calloc(ident.len + sizeof("complex_")-1, sizeof(char));
		memcpy(temp, "complex_", sizeof("complex_")-1);
		memcpy(temp+8, ident.s, ident.len);

		if (hashmap_get(eval_builtin_maps[5], temp, ident.len + sizeof("complex_")-1, (uintptr_t *)&d_cd_func))
			if (first_arg_val.type == ComplexNumber_type)
				return VAL_NUM((*d_cd_func)(first_arg_val.cn));

		if (hashmap_get(eval_builtin_maps[3], temp, ident.len + sizeof("complex_")-1, (uintptr_t *)&cd_cd_func))
			if (first_arg_val.type == ComplexNumber_type)
				return VAL_CNUM((*cd_cd_func)(first_arg_val.cn));
	}


	MML_log_err("undefined function '%.*s' for %s argument in function call\n",
			(int)ident.len, ident.s,
			EXPR_TYPE_STRINGS[first_arg_val.type]);
	return VAL_INVAL;
}

MML_value MML_apply_binary_op(MML_state *restrict state, MML_value a, MML_value b, MML_token_type op)
{
	if (a.type == Invalid_type)
		return VAL_INVAL;

	if (b.type == Invalid_type)
	{
		// unary operators
		switch (op) {
		case MML_OP_NOT_TOK:
			if (a.type != ComplexNumber_type)
				return VAL_BOOL(MML_get_number(&a) == 0);

			MML_log_warn("invalid unary operator on complex operand: %s\n", TOK_STRINGS[MML_OP_NOT_TOK]);
			return VAL_INVAL;
		case MML_OP_NEGATE:
			switch (a.type) {
			case ComplexNumber_type:
				return VAL_CNUM(-MML_get_complex(&a));
			case Boolean_type:
			case RealNumber_type:
				return VAL_NUM(-MML_get_number(&a));
			case Vector_type:
				return MML_apply_binary_op(state, a, VAL_NUM(-1), MML_OP_MUL_TOK);
			default:
				MML_log_warn("failed to apply %s operator on %s operand\n", TOK_STRINGS[op], EXPR_TYPE_STRINGS[a.type]);
				return VAL_INVAL;
			}
		case MML_PIPE_TOK:
			switch (a.type) {
			case ComplexNumber_type:
				return VAL_CNUM(cabs(MML_get_complex(&a)));
			case Boolean_type:
			case RealNumber_type:
				return VAL_NUM(fabs(MML_get_number(&a)));
			case Vector_type:
				// compute vector magnitude
				_Complex double sum = 0.0;
				MML_value cur_elem;
				arena_index *av_p = mml_i(a.v.i, arena_index);
				for (size_t i = 0; i < a.v.n; ++i)
				{
					cur_elem = MML_eval_expr(state, av_p[i]);
					sum += MML_apply_binary_op(state, cur_elem, cur_elem, MML_OP_MUL_TOK).n;
				}
				_Complex double ret = csqrt(sum);
				return (cimag(ret) == 0.0) ? VAL_NUM(creal(ret)) : VAL_CNUM(ret);
			default:
				MML_log_warn("failed to apply %s operator on %s operand\n", TOK_STRINGS[op], EXPR_TYPE_STRINGS[a.type]);
				return VAL_INVAL;
			}
		case MML_TILDE_TOK:
			MML_expr_vec ret;
			ret.i = arena_alloc(MML_global_arena, 2*sizeof(arena_index));
			ret.n = 2;

			arena_index data = arena_alloc(MML_global_arena, 2*sizeof(MML_expr));
			const MML_value negated_a = MML_apply_binary_op(state,
					a,
					VAL_INVAL,
					MML_OP_NEGATE);
			mml_ii(data, MML_expr, 0)->type = a.type;
			memcpy(&mml_ii(data, MML_expr, 0)->w, &a.w, sizeof(a.w));

			mml_ii(data, MML_expr, 1)->type = negated_a.type;
			memcpy(&mml_ii(data, MML_expr, 1)->w, &negated_a.w, sizeof(negated_a.w));

			*mml_vi(ret, 0) = data;
			*mml_vi(ret, 1) = data+sizeof(MML_expr);

			return (MML_value) { Vector_type, .v = ret };
		case MML_OP_UNARY_NOTHING: return a;
		case MML_OP_ROOT:
			switch (a.type) {
			case ComplexNumber_type:
				return VAL_CNUM(csqrt(MML_get_complex(&a)));
			case Boolean_type:
			case RealNumber_type:
				return VAL_NUM(sqrt(MML_get_number(&a)));
			default:
				MML_log_warn("failed to apply %s operator on %s operand\n", TOK_STRINGS[op], EXPR_TYPE_STRINGS[a.type]);
				return VAL_INVAL;
			}
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
			case MML_OP_ROOT: return VAL_NUM(pow(MML_get_number(&a), 1.0/MML_get_number(&b)));
			default:
				MML_log_warn("invalid binary operator on real operands: %s\n", TOK_STRINGS[op]);
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
			case MML_OP_ROOT: return VAL_NUM(cpow(MML_get_complex(&a), 1.0/MML_get_complex(&b)));
			default:
				MML_log_warn("invalid binary operator on complex operands: %s\n", TOK_STRINGS[op]);
				return VAL_INVAL;
		}
	} else if (a.type == Vector_type && b.type == RealNumber_type
			&& op == MML_OP_DOT_TOK)
	{
		// vector index
		size_t i = (size_t)MML_get_number(&b);
		if (fabs(i - MML_get_number(&b)) > EPSILON || MML_get_number(&b) < 0)
		{
			MML_log_err("vectors may only be indexed by a positive integer\n");
			return VAL_INVAL;
		}
		if (i >= a.v.n)
		{
			MML_log_err("index %zu out of range for vector of length %zu\n", i, a.v.n);
			return VAL_INVAL;
		}
		return MML_eval_expr(state, *mml_ei(a, i));
	} else if (a.type == Vector_type && b.type == Vector_type
		  && a.v.n == b.v.n)
	{
		switch (op) {
			case MML_OP_MUL_TOK:
			{
				// n-dimensional dot product
				//
				// if vector contains nested vectors, performs a 'distributed dot product'
				// where the dot product of two vectors is calculated using the dot products
				// of the corresponding nested vectors in each, along with the regular
				// multiplication. (not intentionally, that's just what happens)
				double sum = 0.0;
				for (size_t i = 0; i < a.v.n; ++i)
				{
					sum += MML_apply_binary_op(state,
							MML_eval_expr(state, *mml_ei(a, i)), 
							MML_eval_expr(state, *mml_ei(b, i)), 
							MML_OP_MUL_TOK).n;
				}
				return VAL_NUM(sum);
			}
			case MML_OP_EQ_TOK:
			{
				for (size_t i = 0; i < a.v.n; ++i)
				{
					if (!MML_apply_binary_op(state,
							MML_eval_expr(state, *mml_ei(a, i)), 
							MML_eval_expr(state, *mml_ei(b, i)), 
							MML_OP_EQ_TOK).b)
						return VAL_BOOL(false);
				}
				return VAL_BOOL(true);
			}
			default:
				MML_log_err("invalid binary operator on two equal-length vector operands: %s\n",
						TOK_STRINGS[op]);
				return VAL_INVAL;
		}
	} else if ((a.type == Vector_type && VAL_IS_NUM(b)) ||
		     (VAL_IS_NUM(a) && b.type == Vector_type))
	{
		switch (op) {
		case MML_OP_ADD_TOK:
		case MML_OP_SUB_TOK:
		case MML_OP_MUL_TOK:
		case MML_OP_DIV_TOK:
			const MML_expr_vec *src_vec = (a.type == Vector_type)
				? &a.v
				: &b.v;
			MML_expr_vec ret;
			ret.i = arena_alloc(MML_global_arena, src_vec->n * sizeof(arena_index));
			ret.n = src_vec->n;

			arena_index data = arena_alloc(MML_global_arena, src_vec->n * sizeof(MML_expr));
			for (size_t i = 0; i < src_vec->n; ++i)
			{
				MML_value cur;
				if (a.type == Vector_type)
					cur = MML_apply_binary_op(state,
							MML_eval_expr(state, *mml_ei(a, i)),
							b,
							op);
				else
					cur = MML_apply_binary_op(state,
							a,
							MML_eval_expr(state, *mml_ei(b, i)),
							op);
				mml_ii(data, MML_expr, i)->type = cur.type;
				memcpy(&mml_ii(data, MML_expr, i)->w, &cur.w, sizeof(cur.w));
				*mml_vi(ret, i) = data + i*sizeof(MML_expr);
			}
			return (MML_value) { Vector_type, .v = ret };
		default:
			MML_log_warn("invalid binary operator on %s and %s operands: %s\n",
					EXPR_TYPE_STRINGS[a.type], EXPR_TYPE_STRINGS[b.type],
					TOK_STRINGS[op]);
			return VAL_INVAL;
		}

	}

	MML_log_warn("invalid binary operator on %s and %s operands: %s\n",
			EXPR_TYPE_STRINGS[a.type], EXPR_TYPE_STRINGS[b.type],
			TOK_STRINGS[op]);
	return VAL_INVAL;
}

MML_value MML_eval_expr_recurse(MML_state *restrict state, arena_index expr)
{
	if (!state->is_init)
	{
		MML_log_err("you must run `MML_init_state` before using any evaluator functions.\n");
		return VAL_INVAL;
	}

	if (expr == SIZE_MAX)
		return VAL_INVAL;
	switch (mml_e(expr)->type) {
	case Invalid_type:
		return VAL_INVAL;
	case Vector_type:
		return (MML_value) { Vector_type, .v = mml_e(expr)->v };
	case RealNumber_type:
		return VAL_NUM(mml_e(expr)->n);
	case ComplexNumber_type:
		return VAL_CNUM(mml_e(expr)->cn);
	case Boolean_type:
		return VAL_BOOL(mml_e(expr)->b);
	case Identifier_type: {
		MML_value *val;
		if (mml_e(expr)->s.len == 3 && strncmp(mml_e(expr)->s.s, "ans", 3) == 0)
			return state->last_val;
		if (hashmap_get(eval_builtin_maps[0], mml_e(expr)->s.s, mml_e(expr)->s.len, (uintptr_t *)&val))
			return *val;
		
		arena_index e = MML_eval_get_variable(state, mml_e(expr)->s);
		if (e != SIZE_MAX)
			return MML_eval_expr_recurse(state, e);

		MML_log_warn("undefined identifier: '%.*s'\n",
				(int)mml_e(expr)->s.len, mml_e(expr)->s.s);
		return VAL_INVAL;
	}
	default:
		break;
	}

	arena_index left = mml_e(expr)->o.left;
	arena_index right = mml_e(expr)->o.right;

	if (mml_e(expr)->o.op == MML_OP_ASSERT_EQUAL && left != SIZE_MAX && mml_e(left)->type == Identifier_type)
	{
		MML_eval_set_variable(state, mml_e(left)->s, right);
		return MML_eval_expr_recurse(state, right);
	} else if (mml_e(expr)->o.op == MML_OP_FUNC_CALL_TOK)
	{
		MML_log_dbg("arena base = %p\n", MML_global_arena->base);
		MML_log_dbg("left = %zu\n", left);
		MML_log_dbg("SIZE_MAX - left == %zu\n", SIZE_MAX-left);
		MML_log_dbg("mml_e(left) = %p\n", mml_e(left));
		if (left == SIZE_MAX
		 || right == SIZE_MAX
		 || mml_e(left)->type != Identifier_type)
			return VAL_INVAL;

		MML_value right_val_vec = MML_eval_expr_recurse(state, right);
		if (right_val_vec.type == Invalid_type)
			return VAL_INVAL;

		return apply_func(state, mml_e(left)->s, right_val_vec);
	}

	return MML_apply_binary_op(state,
			MML_eval_expr_recurse(state, left),
			(right != SIZE_MAX) ? MML_eval_expr_recurse(state, right) : VAL_INVAL,
			mml_e(expr)->o.op);
}
inline MML_value MML_eval_expr(MML_state *restrict state, arena_index expr)
{
	return state->last_val = MML_eval_expr_recurse(state, expr);
}

inline int32_t MML_eval_push_expr(MML_state *restrict state, arena_index expr)
{
	return dv_push(state->exprs, expr) != NULL;
}
inline MML_value MML_eval_top_expr(MML_state *restrict state)
{
	return MML_eval_expr_recurse(state, *dv_peek(state->exprs));
}

inline MML_value MML_eval_last_val(MML_state *restrict state)
{
	return state->last_val;
}

MML_value MML_eval_parse(MML_state *restrict state, const char *s)
{
	arena_index_vec exprs = MML_parse_stmts_to_ret(s);
	MML_value cur;
	arena_index *cur_i;
	dv_foreach(exprs, cur_i)
		cur = MML_eval_expr(state, *cur_i);

	return cur;
}
