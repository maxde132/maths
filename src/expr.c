#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "mml/parser.h"
#include "mml/eval.h"
#include "mml/expr.h"
#include "mml/config.h"

void MML_print_indent(uint32_t indent)
{
	printf("%*s", indent, "");
}

MML_Value MML_print_typedval(MML_state *, MML_Value *val)
{
	if (val == nullptr)
	{
		printf("(null)");
		return VAL_INVAL;
	}
	switch (val->type) {
	case RealNumber_type:
		printf("%.*f", MML_global_config.precision, val->v.n);
		break;
	case ComplexNumber_type:
		printf("%.*f%+.*fi",
				MML_global_config.precision, creal(val->v.cn),
				MML_global_config.precision, cimag(val->v.cn));
		break;
	case Boolean_type:
		if (FLAG_IS_SET(BOOLS_PRINT_NUM))
			printf("%.*f",
					MML_global_config.precision, (val->v.b) ? 1.0 : 0.0);
		else
			printf("%s", (val->v.b) ? "true" : "false");
		break;
	case Identifier_type:
	case InsertedIdentifier_type:
		printf("%.*s", (int)val->v.s.len, val->v.s.s);
		break;
	case Vector_type:
		MML_print_vec(&val->v.v);
		break;
	default:
		printf("(null)");
		break;
	}

	MML_global_config.last_print_was_newline = false;
	return (MML_Value) { Invalid_type, .v.n = NAN };
}

inline MML_Value MML_println_typedval(MML_state *state, MML_Value *val)
{
	MML_Value ret = MML_print_typedval(state, val);
	fputc('\n', stdout);
	MML_global_config.last_print_was_newline = true;
	return ret;
}
MML_Value MML_print_typedval_multiargs(MML_state *state, MML_VecN *args)
{
	for (size_t i = 0; i < args->n; ++i)
	{
		MML_Value cur_val = MML_eval_expr(state, args->ptr[i]);
		MML_print_typedval(state, &cur_val);
		if (i < args->n-1) fputc(' ', stdout);
	}

	return (MML_Value) { Invalid_type, .v.n = NAN };
}
MML_Value MML_println_typedval_multiargs(MML_state *state, MML_VecN *args)
{
	for (size_t i = 0; i < args->n; ++i)
	{
		MML_Value cur_val = MML_eval_expr(state, args->ptr[i]);
		MML_println_typedval(state, &cur_val);
	}
	if (args->n == 0)
		fputc('\n', stdout);

	return (MML_Value) { Invalid_type, .v.n = NAN };
}

void MML_print_expr(MML_Expr *expr, uint32_t indent)
{
	MML_print_indent(indent);
	if (expr == nullptr)
	{
		printf("(null)\n");
		return;
	}
	switch (expr->type) {
	case Operation_type:
		printf("Operation(%s):\n", TOK_STRINGS[expr->u.o.op]);
		MML_print_indent(indent+2);

		printf("Left:\n");
		MML_print_expr(expr->u.o.left, indent+4);
		if (expr->u.o.right)
		{
			fputc('\n', stdout);
			MML_print_indent(indent+2);
			printf("Right:\n");
			MML_print_expr(expr->u.o.right, indent+4);
		}
		break;
	case RealNumber_type:
		printf("RealNumber(%.*f)", MML_global_config.precision, expr->u.v.n);
		break;
	case ComplexNumber_type:
		printf("ComplexNumber(%.*f%+.*fi)",
				MML_global_config.precision, creal(expr->u.v.cn),
				MML_global_config.precision, cimag(expr->u.v.cn));
		break;
	case Boolean_type:
		if (FLAG_IS_SET(BOOLS_PRINT_NUM))
			printf("Boolean(%.*f)",
					MML_global_config.precision, (expr->u.v.b) ? 1.0 : 0.0);
		else
			printf("Boolean(%s)", (expr->u.v.b) ? "true" : "false");
		break;
	case Identifier_type:
		printf("Identifier('%.*s')", (int)expr->u.v.s.len, expr->u.v.s.s);
		break;
	case InsertedIdentifier_type:
		printf("InsertedIdentifier('%.*s')", (int)expr->u.v.s.len, expr->u.v.s.s);
		break;
	case Vector_type:
		printf("Vector(n=%zu):\n", expr->u.v.v.n);
		for (size_t i = 0; i < expr->u.v.v.n; ++i)
		{
			MML_print_expr(expr->u.v.v.ptr[i], indent+2);
			if (i < expr->u.v.v.n - 1) fputc('\n', stdout);
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
		MML_free_expr(&(*e)->u.o.left);
		MML_free_expr(&(*e)->u.o.right);
	} else if ((*e)->type == Identifier_type)
	{
		if ((*e)->u.v.s.allocd)
			free((*e)->u.v.s.s);
	} else if ((*e)->type == Vector_type)
	{
		if (!(*e)->should_free_vec_block)
			MML_free_vec(&(*e)->u.v.v);
		else
		{
			free((*e)->u.v.v.ptr[0]);
			free((*e)->u.v.v.ptr);
			(*e)->u.v.v.ptr = nullptr;
		}
	}

	free(*e);
	*e = nullptr;
}

#ifndef NDEBUG
VecN MML_new_vec_debug(size_t n, struct call_info call)
#else
MML_VecN MML_new_vec(size_t n)
#endif
{
#ifndef NDEBUG
	static size_t n_vecs_allocd = 0;
	if (call.filename != nullptr)
		printf("alloc'd vector #%zu (%s:%zu)\n",
			++n_vecs_allocd,
			call.filename, call.line_n);
#endif
	return (MML_VecN) {
		.ptr = calloc(n, sizeof(MML_Expr *)),
		.n = 0,
		.allocd_size = n,
	};
}

#ifndef NDEBUG
void MML_free_vec_debug(VecN *vec, struct call_info call)
#else
void MML_free_vec(MML_VecN *vec)
#endif
{
	for (size_t i = 0; i < vec->n; ++i)
		MML_free_expr(&vec->ptr[i]);
	free(vec->ptr);
#ifndef NDEBUG
	static size_t vector_free_count = 0;
	if (call.filename != nullptr)
		printf("freed vector #%zu (%s:%zu)\n",
			++vector_free_count,
			call.filename, call.line_n);
#endif
}

MML_VecN MML_construct_vec(size_t n, ...)
{
	MML_VecN ret = {
		.ptr = calloc(n, sizeof(MML_Expr *)),
		.n = n,
		.allocd_size = n
	};
	MML_Value cur;
	va_list args;
	va_start(args, n);
	for (size_t i = 0; i < n; ++i)
	{
		cur = va_arg(args, MML_Value);
		ret.ptr[i] = calloc(1, sizeof(MML_Expr));
		ret.ptr[i]->type = cur.type;
		ret.ptr[i]->u.v = cur.v;
	}

	va_end(args);

	return ret;
}

int32_t MML_push_to_vec(MML_VecN *vec, MML_Expr *elem)
{
	if (vec->allocd_size < (vec->n + 1))
	{
		MML_Expr **tmp = realloc(vec->ptr,
				(vec->allocd_size*=2) * sizeof(MML_Expr *));
		if (tmp == NULL)
		{
			fprintf(stderr, "could not resize vector memory\n");
			MML_free_vec(vec);
			return 1;
		}
		vec->ptr = tmp;
	}
	vec->ptr[vec->n++] = elem;
	++elem->num_refs;
	return 0;
}

inline MML_Expr **MML_peek_top_vec(MML_VecN *vec)
{
	MML_Expr **ret = &vec->ptr[vec->n-1];
	++(*ret)->num_refs;
	return ret;
}

inline MML_Expr *MML_pop_from_vec(MML_VecN *vec)
{
	return vec->ptr[--vec->n];
}

inline void MML_print_vec(MML_VecN *vec)
{
	MML_print_expr(&(MML_Expr) { Vector_type, 0, .u.v.v = *vec }, 0);
}
inline void MML_println_vec(MML_VecN *vec)
{
	MML_print_expr(&(MML_Expr) { Vector_type, 0, .u.v.v = *vec }, 0);
	fputc('\n', stdout);
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
		? v->v.n
		: ((v->v.b) ? 1.0 : 0.0);
}
inline _Complex double MML_get_complex(MML_Value *v)
{
	if (!VAL_IS_NUM(*v))
		return NAN;
	return (v->type == ComplexNumber_type)
		? v->v.cn
		: MML_get_number(v) + 0.0*I;
}
