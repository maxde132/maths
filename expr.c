#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "config.h"

const char *const TOK_STRINGS[] = {
	"OP_FUNC_CALL",
	"OP_DOT",
	"OP_AT_TOK",

	"OP_POW",

	"OP_MUL", "OP_DIV", "OP_MOD",
	"OP_ADD", "OP_SUB",

	"OP_LESS", "OP_GREATER",
	"OP_LESSEQ", "OP_GREATEREQ",
	"OP_EQ", "OP_NOTEQ",

	"OP_ASSERT_EQUAL",
	
	"OP_NOT",
	"OP_NEGATE",
	"OP_UNARY_NOTHING",
	"NOT_OP",


	"IDENT_TOK",
	"NUMBER_TOK",

	"DIGIT_TOK",
	"LETTER_TOK",
	"UNDERSCORE_TOK",

	"OPEN_PAREN_TOK",
	"CLOSE_PAREN_TOK",

	"OPEN_BRACKET_TOK",
	"CLOSE_BRACKET_TOK",

	"DQUOTE_TOK",
	"SQUOTE_TOK",
	"BACKTICK_TOK",

	"COLON_TOK",
	"SEMICOLON_TOK",
	"COMMA_TOK",

	"HASHTAG_TOK",
	"QUESTION_TOK",
	"BACKSLASH_TOK",
	"DOLLAR_TOK",
	"AMPER_TOK",
	"PIPE_TOK",
	"TILDE_TOK",

	"INVALID_TOK",
	"EOF_TOK",
	"END_OF_EXPR_TOK",
};


void print_indent(uint32_t indent)
{
	printf("%*s", indent, "");
}

void print_typedval(const TypedValue *val)
{
	if (val == nullptr)
	{
		printf("(null)");
		return;
	}
	switch (val->type) {
	case RealNumber_type:
		printf("%.*f", global_config.precision, val->v.n);
		break;
	case ComplexNumber_type:
		printf("%.*f%+.*fi",
				global_config.precision, creal(val->v.cn),
				global_config.precision, cimag(val->v.cn));
		break;
	case Boolean_type:
		if (FLAG_IS_SET(BOOLS_PRINT_NUM))
			printf("%.*f",
					global_config.precision, (val->v.b) ? 1.0 : 0.0);
		else
			printf("%s", (val->v.b) ? "true" : "false");
		break;
	case Identifier_type:
		printf("%.*s", (int)val->v.s.len, val->v.s.s);
		break;
	case Vector_type:
		print_exprh(VAL2EXPRP(*val));
		break;
	default:
		break;
	}
}
inline void println_typedval(const TypedValue *val)
{
	print_typedval(val);
	fputc('\n', stdout);
}

void print_expr(const Expr *expr, uint32_t indent)
{
	print_indent(indent);
	if (expr == nullptr)
	{
		printf("(null)\n");
		return;
	}
	switch (expr->type) {
	case Operation_type:
		printf("Expr(op=%s):\n", TOK_STRINGS[expr->u.o.op]);
		print_indent(indent+2);

		printf("Left:\n");
		print_expr(expr->u.o.left, indent+4);
		if (expr->u.o.right)
		{
			fputc('\n', stdout);
			print_indent(indent+2);
			printf("Right:\n");
			print_expr(expr->u.o.right, indent+4);
		}
		break;
	case RealNumber_type:
		printf("RealNumber(%.*f)", global_config.precision, expr->u.v.n);
		break;
	case ComplexNumber_type:
		printf("ComplexNumber(%.*f%+.*fi)",
				global_config.precision, creal(expr->u.v.cn),
				global_config.precision, cimag(expr->u.v.cn));
		break;
	case Boolean_type:
		if (FLAG_IS_SET(BOOLS_PRINT_NUM))
			printf("Boolean(%.*f)",
					global_config.precision, (expr->u.v.b) ? 1.0 : 0.0);
		else
			printf("Boolean(%s)", (expr->u.v.b) ? "true" : "false");
		break;
	case Identifier_type:
		printf("Identifier('%.*s')", (int)expr->u.v.s.len, expr->u.v.s.s);
		break;
	case Vector_type:
		printf("Vector(n=%zu):\n", expr->u.v.v.n);
		for (size_t i = 0; i < expr->u.v.v.n; ++i)
		{
			print_expr(expr->u.v.v.ptr[i], indent+2);
			if (i < expr->u.v.v.n - 1) fputc('\n', stdout);
		}
	}
}

inline void print_exprh(const Expr *expr)
{
	print_expr(expr, 0);
	fputc('\n', stdout);
}

void free_expr(Expr *e)
{
	if (e == nullptr) return;
	if (e->type == Operation_type)
	{
		free_expr(e->u.o.left);
		free_expr(e->u.o.right);
	} else if (e->type == Identifier_type)
	{
		if (e->u.v.s.allocd) free((void *)e->u.v.s.s);
	} else if (e->type == Vector_type)
	{
		for (size_t i = 0; i < e->u.v.v.n; ++i)
			free_expr(e->u.v.v.ptr[i]);
		free(e->u.v.v.ptr);
	}

	free(e);
}

TypedValue *construct_vec(size_t n, ...)
{
	TypedValue *ret = calloc(1, sizeof(TypedValue));
	ret->type = Vector_type;
	ret->v.v.n = n;
	ret->v.v.ptr = calloc(n, sizeof(Expr *));
	TypedValue cur;
	va_list args;
	va_start(args, n);
	for (size_t i = 0; i < n; ++i)
	{
		cur = va_arg(args, TypedValue);
		ret->v.v.ptr[i] = calloc(1, sizeof(Expr));
		ret->v.v.ptr[i]->type = cur.type;
		ret->v.v.ptr[i]->u.v = cur.v;
	}

	va_end(args);

	return ret;
}

inline double get_number(TypedValue *v)
{
	return (v->type == RealNumber_type)
		? v->v.n
		: ((v->v.b) ? 1.0 : 0.0);
}
inline _Complex double get_complex(TypedValue *v)
{
	return (v->type == ComplexNumber_type)
		? v->v.cn
		: get_number(v) + 0.0*I;
}
