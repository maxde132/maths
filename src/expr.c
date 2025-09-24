#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "cvi/dvec/dvec.h"
#include "mml/parser.h"
#include "mml/eval.h"
#include "mml/expr.h"
#include "mml/config.h"

void MML_print_indent(uint32_t indent)
{
	printf("%*s", indent, "");
}

MML_Value MML_print_typedval(MML_state *state, const MML_Value *val)
{
	if (val == nullptr)
	{
		printf("(null)");
		return VAL_INVAL;
	}
	switch (val->type) {
	case Integer_type:
		printf("%" PRIi64, val->i);
		break;
	case RealNumber_type:
		printf("%.*f", MML_global_config.precision, val->n);
		break;
	case ComplexNumber_type:
		printf("%.*f%+.*fi",
				MML_global_config.precision, creal(val->cn),
				MML_global_config.precision, cimag(val->cn));
		break;
	case Boolean_type:
		if (FLAG_IS_SET(BOOLS_PRINT_NUM))
			printf("%.*f",
					MML_global_config.precision, (val->b) ? 1.0 : 0.0);
		else
			printf("%s", (val->b) ? "true" : "false");
		break;
	case Identifier_type:
		printf("%.*s", (int)val->s.len, val->s.s);
		break;
	case Vector_type:
		fputc('[', stdout);
		MML_Value cur_val;
		MML_Expr **cur;
		dv_foreach(val->v, cur)
		{
			cur_val = MML_eval_expr(state, *cur);
			MML_print_typedval(state, &cur_val);
			if ((size_t)(cur - _dv_ptr(val->v)) < dv_n(val->v)-1)
				fputs(", ", stdout);
		}
		fputc(']', stdout);
		break;
	default:
		printf("(null)");
		break;
	}

	MML_global_config.last_print_was_newline = false;
	return (MML_Value) { Invalid_type, .n = NAN };
}

inline MML_Value MML_println_typedval(MML_state *state, const MML_Value *val)
{
	MML_Value ret = MML_print_typedval(state, val);
	fputc('\n', stdout);
	MML_global_config.last_print_was_newline = true;
	return ret;
}
MML_Value MML_print_typedval_multiargs(MML_state *state, MML_ExprVec *args)
{
	for (size_t i = 0; i < dv_n(*args); ++i)
	{
		MML_Value cur_val = MML_eval_expr(state, dv_a(*args, i));
		MML_print_typedval(state, &cur_val);
		if (i < dv_n(*args)-1) fputc(' ', stdout);
	}

	return (MML_Value) { Invalid_type, .n = NAN };
}
MML_Value MML_println_typedval_multiargs(MML_state *state, MML_ExprVec *args)
{
	for (size_t i = 0; i < dv_n(*args); ++i)
	{
		MML_Value cur_val = MML_eval_expr(state, dv_a(*args, i));
		MML_println_typedval(state, &cur_val);
	}
	if (dv_n(*args) == 0)
		fputc('\n', stdout);

	return (MML_Value) { Invalid_type, .n = NAN };
}

void MML_print_expr(const MML_Expr *expr, uint32_t indent)
{
	MML_print_indent(indent);
	if (expr == nullptr)
	{
		printf("(null)\n");
		return;
	}
	switch (expr->type) {
	case Operation_type:
		printf("Operation(%s):\n", TOK_STRINGS[expr->o.op]);
		MML_print_indent(indent+2);

		printf("Left:\n");
		MML_print_expr(expr->o.left, indent+4);
		if (expr->o.right)
		{
			fputc('\n', stdout);
			MML_print_indent(indent+2);
			printf("Right:\n");
			MML_print_expr(expr->o.right, indent+4);
		}
		break;
	case Integer_type:
		printf("Integer(%" PRIi64 ")", expr->i);
		break;
	case RealNumber_type:
		printf("RealNumber(%.*f)", MML_global_config.precision, expr->n);
		break;
	case ComplexNumber_type:
		printf("ComplexNumber(%.*f%+.*fi)",
				MML_global_config.precision, creal(expr->cn),
				MML_global_config.precision, cimag(expr->cn));
		break;
	case Boolean_type:
		if (FLAG_IS_SET(BOOLS_PRINT_NUM))
			printf("Boolean(%.*f)",
					MML_global_config.precision, (expr->b) ? 1.0 : 0.0);
		else
			printf("Boolean(%s)", (expr->b) ? "true" : "false");
		break;
	case Identifier_type:
		printf("Identifier('%.*s')", (int)expr->s.len, expr->s.s);
		break;
	case Vector_type:
		printf("Vector(n=%zu):\n", dv_n(expr->v));
		for (size_t i = 0; i < dv_n(expr->v); ++i)
		{
			MML_print_expr(dv_a(expr->v, i), indent+2);
			if (i < dv_n(expr->v) - 1) fputc('\n', stdout);
		}
		break;
	default:
		printf("Invalid()");
		break;
	}

	MML_global_config.last_print_was_newline = false;
}

inline void MML_print_exprh(MML_Expr *expr)
{
	MML_print_expr(expr, 0);
	fputc('\n', stdout);
	MML_global_config.last_print_was_newline = true;
}
inline MML_Value MML_print_exprh_tv_func(MML_state *, MML_ExprVec *args)
{
	MML_print_expr(dv_a(*args, 0), 0);
	fputc('\n', stdout);
	MML_global_config.last_print_was_newline = true;

	return VAL_INVAL;
}

void MML_free_expr(MML_Expr **e)
{
	if (*e == nullptr) return;
	if (--(*e)->num_refs > 0)
	{
		*e = nullptr;
		return;
	}
	if ((*e)->type == Operation_type)
	{
		MML_free_expr(&(*e)->o.left);
		MML_free_expr(&(*e)->o.right);
	} else if ((*e)->type == Identifier_type)
	{
		if ((*e)->s.allocd)
			free((*e)->s.s);
	} else if ((*e)->type == Vector_type)
	{
		if (!(*e)->should_free_vec_block)
			MML_free_vec(&(*e)->v);
		else
		{
			free(dv_a((*e)->v, 0));
			dv_destroy((*e)->v);
			_dv_ptr((*e)->v) = nullptr;
		}
	}

	free(*e);
	*e = nullptr;
}

void MML_free_vec(MML_ExprVec *vec)
{
	for (size_t i = 0; i < dv_n(*vec); ++i)
		MML_free_expr(&dv_a(*vec, i));
	dv_destroy(*vec);
}

inline void MML_free_pp(void *p)
{
	free(*(void **)p);
}

inline double MML_get_number(MML_Value *v)
{
	if (!VAL_IS_NUM(*v) || v->type == ComplexNumber_type)
		return NAN;
	return (v->type == RealNumber_type)
		? v->n
		: ((v->b) ? 1.0 : 0.0);
}
inline _Complex double MML_get_complex(MML_Value *v)
{
	if (!VAL_IS_NUM(*v))
		return NAN;
	return (v->type == ComplexNumber_type)
		? v->cn
		: MML_get_number(v) + 0.0*I;
}
