#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#include "expr.h"
#include "token.h"

void print_indent(uint32_t indent)
{
	printf("%*s", indent, "");
}

void print_expr(Expr *expr, uint32_t indent)
{
	print_indent(indent);
	switch (expr->type) {
	case Operation_type:
		printf("Expr(op=%d):\n", expr->u.o.op);
		print_indent(indent+2);

		printf("Left:\n");
		print_expr(expr->u.o.left, indent+4);
		print_indent(indent+2);
		printf("Right:\n");
		print_expr(expr->u.o.right, indent+4);
		break;
	case Number_type:
		printf("Number(%.6f)", expr->u.n);
		break;
	}

	fputc('\n', stdout);
}

void free_expr(Expr *expr)
{
	if (expr->type == Number_type)
		return;
	free_expr(expr->u.o.left);
	free_expr(expr->u.o.right);
}

size_t last_token_length = 0;

TokenType next_token(const char **s)
{
	*s += last_token_length;
	last_token_length = 0;
	TokenType ret = INVALID_TOK;

	while (isspace(**s))
		++*s;

	switch ((*s)[last_token_length]) {
	case '+':
		++last_token_length;
		ret = OP_ADD_TOK;
		break;
	case '-':
		++last_token_length;
		ret = OP_SUB_TOK;
		break;
	case '*':
		++last_token_length;
		ret = OP_MUL_TOK;
		break;
	case '/':
		++last_token_length;
		ret = OP_DIV_TOK;
		break;
	default:
		if (isdigit((*s)[0]))
		{
			do {
				++last_token_length;
			} while (isdigit((*s)[last_token_length]));
			ret = NUMBER_TOK;
		} else if (isalnum((*s)[0]))
		{
			do {
				++last_token_length;
			} while (isalnum((*s)[last_token_length]) ||
					(*s)[last_token_length] == '_');
			ret = VARIABLE_TOK;
		} else if ((*s)[0] == '\0' || (*s)[0] == (char)EOF)
			ret = EOF_TOK;
		else
			ret = INVALID_TOK;
	}

	printf("token decoded: %d\n", ret);
	return ret;
}

Expr *parse(const char **s, uint32_t max_preced, int level)
{
	Expr *left = calloc(1, sizeof(Expr));

	TokenType tok = next_token(s);

	if (tok == NUMBER_TOK)
		memcpy(left,
			&(Expr) { Number_type, { .n = strtod(*s, NULL) } },
			sizeof(Expr));

	for (;;)
	{
		printf("%*stok == %d\n", level, "", tok);
		tok = next_token(s);
		if (tok > OP_DIV_TOK)
			break;

		uint32_t preced = precedence[tok];
		if (preced > max_preced)
			break;

		//tok = next_token(&s);
		// we don't need this bit until I want right-associative operators
		//static char buf[512] = {0};
		//strncpy(buf, s, last_token_length);
		next_token(s);

		Expr *right = parse(s, preced+1, level+1);

		Expr *tmp = calloc(1, sizeof(Expr));
		memcpy(tmp, left, sizeof(Expr));

		memcpy(left, &(Expr) { Operation_type, {
			.o = {
				tmp, right,
				(Operator)tok
			}}
		}, sizeof(Expr));
	}

	return left;
}

int32_t main(void)
{
	Expr *expr = parse(&(const char *) {"5 + 9"}, 15, 0);
	print_expr(expr, 0);
	free_expr(expr);

	return 0;
}
