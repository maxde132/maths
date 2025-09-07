#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "parser.h"

static const char *saved_s = NULL;
static Token peeked_tok;
static bool has_peeked = false;

// Gets the next token and advances the string pointer.
Token get_next_token(const char **s)
{
	Token ret = nToken(INVALID_TOK, NULL, 0);

	if (has_peeked) {
		has_peeked = false;
		*s = saved_s;
		return peeked_tok;
	}

	while (isspace(**s)) ++*s;

	if (**s == '\0') return nToken(EOF_TOK, NULL, 0);

	TokenType type = TOK_BY_CHAR[**s - 0x21];
	switch (type) {
	case OP_DOT_TOK:
	case OP_MUL_TOK:
	case OP_POW_TOK:
	case OP_DIV_TOK:
	case OP_ADD_TOK:
	case OP_SUB_TOK:
	case OP_MOD_TOK:
	case OPEN_PAREN_TOK:
	case CLOSE_PAREN_TOK:
	case OPEN_BRAC_TOK:
	case CLOSE_BRAC_TOK:
	case OPEN_BRACKET_TOK:
	case CLOSE_BRACKET_TOK:
		ret = nToken(type, *s, 1);
		++*s;
		break;
	case OP_LESS_TOK:
		if ((*s)[1] == '=')
		{
			ret = nToken(OP_LESSEQ_TOK, *s, 2);
			*s += 2;
		}
		ret = nToken(OP_LESS_TOK, *s, 1);
		++*s;
		break;
	case OP_GREATER_TOK:
		if ((*s)[1] == '=')
		{
			ret = nToken(OP_GREATEREQ_TOK, *s, 2);
			*s += 2;
		}
		ret = nToken(OP_GREATER_TOK, *s, 1);
		++*s;
		break;
	case OP_EQ_TOK:
		if ((*s)[1] != '=')
		{
			fprintf(stderr, "assignment operator is not yet supported. "
					"'==' is the equality operator\n");
			break;
		}
		ret = nToken(OP_EQ_TOK, *s, 2);
		*s += 2;
		break;
	case OP_NOT_TOK:
		if ((*s)[1] != '=')
		{
			fprintf(stderr, "boolean NOT operator is not yet supported. "
					"'!=' is the inequality operator\n");
			break;
		}
		ret = nToken(OP_NOTEQ_TOK, *s, 2);
		*s += 2;
		break;
	case DIGIT_TOK:
		ret.buf.s = (char *)*s;
		char *end;
		strtod(ret.buf.s, &end);
		ret.buf.len = end - ret.buf.s;
		*s = end;
		ret.type = NUMBER_TOK;
		break;
	case LETTER_TOK:
	case UNDERSCORE_TOK:
		ret.buf.s = (char *)*s;
		do {
			++*s;
		} while (isalnum(**s) || **s == '_');
		ret.buf.len = *s - ret.buf.s;
		ret.type = IDENT_TOK;
		ret.buf.allocd = false;
		break;
	default:
		fprintf(stderr, "invalid token starts at '%.5s'\n", *s);
		exit(1);
	}

	/*printf("token decoded: { %d, '%.*s' }\n",
			ret.type, (int)ret.buf.len, ret.buf.s);*/
	return ret;
}

// Peeks at the next token without advancing the string pointer.
Token peek_token(const char **s)
{
	if (!has_peeked)
	{
		const char *s_copy = *s;
		peeked_tok = get_next_token(&s_copy);
		saved_s = s_copy;
		has_peeked = true;
	}
	return peeked_tok;
}

#define PARSER_MAX_PRECED 15

bool op_is_right_associative(TokenType op)
{
	return op == OP_POW_TOK;
}

Expr *parse_expr(const char **s, uint32_t max_preced)
{
	Token tok = get_next_token(s);

	Expr *left = calloc(1, sizeof(Expr));

	if (tok.type == IDENT_TOK)
	{
		Token ident = tok;
		Token next_tok = peek_token(s);

		if (next_tok.type == OPEN_PAREN_TOK)
		{
			//printf("calling function: '%.*s'\n", (int)ident.buf.len, ident.buf.s);
			Expr *name = calloc(1, sizeof(Expr));
			name->type = String_type;
			name->u.s = ident.buf;

			left->type = Operation_type;
			left->u.o.left = name;
			left->u.o.op = OP_FUNC_CALL_TOK;

			get_next_token(s);
			left->u.o.right = parse_expr(s, PARSER_MAX_PRECED);

			Token close_paren_tok = get_next_token(s);
			if (close_paren_tok.type != CLOSE_PAREN_TOK)
			{
				fprintf(stderr, "expected close paren for function call, got %u\n", close_paren_tok.type);
				free_expr(left);
				return NULL;
			}
		} else
		{
			left->type = String_type;
			left->u.s = ident.buf;
			/*if (strncmp(ident.buf.s, "pi", ident.buf.len) == 0)
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
			}*/
		}
	} else if (tok.type == OPEN_PAREN_TOK)
	{
		left = parse_expr(s, PARSER_MAX_PRECED);
		Token close_paren_tok = get_next_token(s);
		if (close_paren_tok.type != CLOSE_PAREN_TOK)
		{
			fprintf(stderr, "expected close paren for parenthesis block, got %u\n", close_paren_tok.type);
			free_expr(left);
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

	for (;;)
	{
		Token op_tok = peek_token(s);
		if (op_tok.type > NOT_OP_TOK)
			break;

		uint32_t preced = PRECEDENCE[op_tok.type];
		if (preced > max_preced)
			break;

		get_next_token(s);

		Expr *right = parse_expr(s,
				op_is_right_associative(op_tok.type)
					? preced
					: preced-1);

		if (right == NULL)
		{
			fprintf(stderr, "missing right operand for operator %u\n", op_tok.type);
			free_expr(left);
			return NULL;
		}

		Expr *opnode = calloc(1, sizeof(Expr));
		opnode->type = Operation_type;
		opnode->u.o.left = left;
		opnode->u.o.right = right;
		opnode->u.o.op = op_tok.type;

		left = opnode;
	}

	return left;
}

inline Expr *parse(const char *s)
{
	return parse_expr(&s, PARSER_MAX_PRECED);
}
