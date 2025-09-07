#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "parser.h"

#define PI_M	3.14159265358979323846
#define E_M		2.71828182845904523536

static Token cur_tok = nToken(INVALID_TOK, NULL, 0);
static uint32_t peek_n_times = 0;

Token next_token(const char **s)
{
	Token ret = nToken(INVALID_TOK, NULL, 0);
	const char **sp_proxy = s;
	static const char *s_proxy_helper = NULL;
	if (peek_n_times > 0)
	{
		if (!s_proxy_helper) s_proxy_helper = *s;
		sp_proxy = &s_proxy_helper;
		--peek_n_times;
	} else
	{
		s_proxy_helper = NULL;
	}

	while (isspace(**sp_proxy)) ++*sp_proxy;

	if (**sp_proxy == '\0') return nToken(EOF_TOK, NULL, 0);

	TokenType type = TOK_BY_CHAR[**sp_proxy - 0x21];
	switch (type) {
	case OP_DOT_TOK:
	case OP_MUL_TOK:
	case OP_POW_TOK:
	case OP_DIV_TOK:
	case OP_ADD_TOK:
	case OP_SUB_TOK:
	case OP_MOD_TOK:
	case OP_LESS_TOK:
	case OP_GREATER_TOK:
	case OP_EQ_TOK:

	case OP_NOT_TOK:

	case OPEN_PAREN_TOK:
	case CLOSE_PAREN_TOK:
	
	case OPEN_BRAC_TOK:
	case CLOSE_BRAC_TOK:
		ret = nToken(type, *sp_proxy, 1);
		++*sp_proxy;
		break;
	case DIGIT_TOK:
		ret.buf.s = *sp_proxy;
		char *end;
		strtod(ret.buf.s, &end);
		ret.buf.len = end - ret.buf.s;
		*sp_proxy = end;
		ret.type = NUMBER_TOK;
		break;
	case LETTER_TOK:
	case UNDERSCORE_TOK:
		ret.buf.s = *sp_proxy;
		do {
			++*sp_proxy;
		} while (isalnum(**sp_proxy) || **sp_proxy == '_');
		ret.buf.len = *sp_proxy - ret.buf.s;
		ret.type = IDENT_TOK;
		ret.buf.allocd = false;
		break;
	default:
		fprintf(stderr, "invalid token starts at '%.5s'\n", *sp_proxy);
		exit(1);
	}

	printf("token decoded: { %d, '%.*s' }\n",
			ret.type, (int)ret.buf.len, ret.buf.s);
	return cur_tok = ret;
}

#define PARSER_MAX_PRECED 15

Expr *parse_expr(const char **s, uint32_t max_preced)
{
	peek_n_times = 1;
	Token tok = next_token(s);

	Expr *left = calloc(1, sizeof(Expr));

	if (tok.type == IDENT_TOK)
	{
		Token ident = tok;
		peek_n_times = 1;
		tok = next_token(s);

		if (tok.type == OPEN_PAREN_TOK)
		{
			printf("calling function: '%.*s'\n", (int)ident.buf.len, ident.buf.s);
			Expr *name = calloc(1, sizeof(Expr));
			name->type = String_type;
			name->u.s.len = ident.buf.len;
			name->u.s.s = strndup(ident.buf.s, ident.buf.len);
			name->u.s.allocd = true;

			left->type = Operation_type;
			left->u.o.left = name;
			peek_n_times = 1;
			tok = next_token(s);
			left->u.o.right = parse_expr(s, PARSER_MAX_PRECED);
			left->u.o.op = OP_FUNC_CALL_TOK;

			if (cur_tok.type != CLOSE_PAREN_TOK)
			{
				fprintf(stderr, "expected close paren for function call, got %u\n", cur_tok.type);
				free_expr(left);
				return NULL;
			}
			//tok = next_token(s, false);
		} else
		{
			if (strncmp(ident.buf.s, "pi", ident.buf.len) == 0)
			{
				left->type = Number_type;
				left->u.n = PI_M;
			} else if (strncmp(ident.buf.s, "e", ident.buf.len) == 0)
			{
				left->type = Number_type;
				left->u.n = E_M;
			} else
			{
				fprintf(stderr, "unknown identifier: '%.*s'\n",
						(int)ident.buf.len, ident.buf.s);
				free_expr(left);
				return NULL;
			}
		}
	} else if (tok.type == OPEN_PAREN_TOK)
	{
		next_token(s);
		left = parse_expr(s, PARSER_MAX_PRECED);
		if (cur_tok.type != CLOSE_PAREN_TOK)
		{
			fprintf(stderr, "expected close paren for parenthesis block\n");
			return NULL;
		}
	} else if (tok.type == NUMBER_TOK)
	{
		*left = (Expr) { Number_type, .u.n = strtod(tok.buf.s, NULL) };
	} else
	{
		free_expr(left);
		return NULL;
	}

	//*s = look_ahead;

	tok = next_token(s);

	for (;;)
	{
		peek_n_times = 1;
		tok = next_token(s);
		if (tok.type > NOT_OP_TOK) break;

		uint32_t preced = PRECEDENCE[tok.type];
		if (preced > max_preced) break;

		Token op_tok = tok;
		//peek_n_times = 1;
		tok = next_token(s);

		Expr *right = parse_expr(s, preced-1);
		if (right == NULL)
		{
			fprintf(stderr, "missing right operand for operator %u\n", op_tok.type);
			free_expr(left);
			return NULL;
		}

		next_token(s);

		Expr *opnode = calloc(1, sizeof(Expr));
		opnode->type = Operation_type;
		opnode->u.o.left = left;
		opnode->u.o.right = right;
		opnode->u.o.op = op_tok.type;

		left = opnode;


		tok = cur_tok;
	}

	return left;
}

inline Expr *parse(const char *s)
{
	return parse_expr(&s, PARSER_MAX_PRECED);
}
