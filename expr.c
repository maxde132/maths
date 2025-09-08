#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "config.h"

const uint8_t PRECEDENCE[] = {
	1,
	1,
	1,

	2,

	3, 3, 3,
	4, 4,

	6, 6, 6, 6,
	7, 7,
};

const TokenType TOK_BY_CHAR[] = { // starts at 0x21
	OP_NOT_TOK,		//'!'
	DQUOTE_TOK,		//'"'
	HASHTAG_TOK,	//'#'
	DOLLAR_TOK,		//'$'
	OP_MOD_TOK,		//'%'
	AMPER_TOK,		//'&'
	SQUOTE_TOK,		//'\''
	OPEN_PAREN_TOK,	//'('
	CLOSE_PAREN_TOK,	//')'
	OP_MUL_TOK,		//'*'
	OP_ADD_TOK,		//'+'
	COMMA_TOK,		//','
	OP_SUB_TOK,		//'-'
	OP_DOT_TOK,		//'.'
	OP_DIV_TOK,		//'/'
	DIGIT_TOK,		//'0'
	DIGIT_TOK,		//'1'
	DIGIT_TOK,		//'2'
	DIGIT_TOK,		//'3'
	DIGIT_TOK,		//'4'
	DIGIT_TOK,		//'5'
	DIGIT_TOK,		//'6'
	DIGIT_TOK,		//'7'
	DIGIT_TOK,		//'8'
	DIGIT_TOK,		//'9'
	COLON_TOK,		//':'
	SEMICOLON_TOK,	//';'
	OP_LESS_TOK,	//'<'
	OP_EQ_TOK,		//'='
	OP_GREATER_TOK,	//'>'
	QUESTION_TOK,	//'?'
	OP_AT_TOK,		//'@'
	LETTER_TOK,		//'A'
	LETTER_TOK,		//'B'
	LETTER_TOK,		//'C'
	LETTER_TOK,		//'D'
	LETTER_TOK,		//'E'
	LETTER_TOK,		//'F'
	LETTER_TOK,		//'G'
	LETTER_TOK,		//'H'
	LETTER_TOK,		//'I'
	LETTER_TOK,		//'J'
	LETTER_TOK,		//'K'
	LETTER_TOK,		//'L'
	LETTER_TOK,		//'M'
	LETTER_TOK,		//'N'
	LETTER_TOK,		//'O'
	LETTER_TOK,		//'P'
	LETTER_TOK,		//'Q'
	LETTER_TOK,		//'R'
	LETTER_TOK,		//'S'
	LETTER_TOK,		//'T'
	LETTER_TOK,		//'U'
	LETTER_TOK,		//'V'
	LETTER_TOK,		//'W'
	LETTER_TOK,		//'X'
	LETTER_TOK,		//'Y'
	LETTER_TOK,		//'Z'
	OPEN_BRACKET_TOK,	//'['
	BACKSLASH_TOK,	//'\\'
	CLOSE_BRACKET_TOK,//']'
	OP_POW_TOK,		//'^'
	UNDERSCORE_TOK,	//'_'
	BACKTICK_TOK,	//'`'
	LETTER_TOK,		//'a'
	LETTER_TOK,		//'b'
	LETTER_TOK,		//'c'
	LETTER_TOK,		//'d'
	LETTER_TOK,		//'e'
	LETTER_TOK,		//'f'
	LETTER_TOK,		//'g'
	LETTER_TOK,		//'h'
	LETTER_TOK,		//'i'
	LETTER_TOK,		//'j'
	LETTER_TOK,		//'k'
	LETTER_TOK,		//'l'
	LETTER_TOK,		//'m'
	LETTER_TOK,		//'n'
	LETTER_TOK,		//'o'
	LETTER_TOK,		//'p'
	LETTER_TOK,		//'q'
	LETTER_TOK,		//'r'
	LETTER_TOK,		//'s'
	LETTER_TOK,		//'t'
	LETTER_TOK,		//'u'
	LETTER_TOK,		//'v'
	LETTER_TOK,		//'w'
	LETTER_TOK,		//'x'
	LETTER_TOK,		//'y'
	LETTER_TOK,		//'z'
	OPEN_BRAC_TOK,	//'{'
	PIPE_TOK,		//'|'
	CLOSE_BRAC_TOK,	//'}'
	TILDE_TOK,		//'~'
};

//#define n16_NULLS NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL


const char *TOK_STRINGS[] = {
	"OP_FUNC_CALL",
	"OP_DOT",
	"OP_AT_TOK",

	"OP_POW",

	"OP_MUL", "OP_DIV", "OP_MOD",
	"OP_ADD", "OP_SUB",

	"OP_LESS", "OP_GREATER",
	"OP_LESSEQ", "OP_GREATEREQ",
	"OP_EQ", "OP_NOTEQ",
	
	"OP_NOT",
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
};

void print_indent(uint32_t indent)
{
	printf("%*s", indent, "");
}

void print_expr(Expr *expr, uint32_t indent)
{
	print_indent(indent);
	if (expr == NULL)
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
	case Number_type:
		printf("Number(%.*f)", global_config.precision, expr->u.v.n);
		break;
	case String_type:
		printf("String('%.*s')", (int)expr->u.v.s.len, expr->u.v.s.s);
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

inline void print_exprh(Expr *expr)
{
	print_expr(expr, 0);
	fputc('\n', stdout);
}

void free_expr(Expr *e)
{
	if (!e) return;
	if (e->type == Operation_type)
	{
		free_expr(e->u.o.left);
		free_expr(e->u.o.right);
	} else if (e->type == String_type)
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
