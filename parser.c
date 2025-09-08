#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "parser.h"

static struct {
	const char *saved_s;
	Token peeked_tok;
	bool has_peeked;
	bool looking_for_int;
} parser_config = {0};

// Gets the next token and advances the string pointer.
Token get_next_token(const char **s)
{
	Token ret = nToken(INVALID_TOK, NULL, 0);

	if (parser_config.has_peeked) {
		parser_config.has_peeked = false;
		*s = parser_config.saved_s;
		return parser_config.peeked_tok;
	}

	while (isspace(**s)) ++*s;

	if (**s == '\0') return nToken(EOF_TOK, NULL, 0);

	TokenType type = TOK_BY_CHAR[**s - 0x21];
	switch (type) {
	case OP_DOT_TOK:
	case OP_AT_TOK:
	case OP_POW_TOK:
	case OP_MUL_TOK:
	case OP_DIV_TOK:
	case OP_MOD_TOK:
	case OP_ADD_TOK:
	case OP_SUB_TOK:
	case OPEN_PAREN_TOK:
	case CLOSE_PAREN_TOK:
	case OPEN_BRAC_TOK:
	case CLOSE_BRAC_TOK:
	case OPEN_BRACKET_TOK:
	case CLOSE_BRACKET_TOK:
	case COMMA_TOK:
	case PIPE_TOK:
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
		if (parser_config.looking_for_int)
			strtoll(ret.buf.s, &end, 10);
		else
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
		break;
	}

	return ret;
}

// Peeks at the next token without advancing the string pointer.
Token peek_token(const char **s)
{
	if (!parser_config.has_peeked)
	{
		const char *s_copy = *s;
		parser_config.peeked_tok = get_next_token(&s_copy);
		parser_config.saved_s = s_copy;
		parser_config.has_peeked = true;
	}
	return parser_config.peeked_tok;
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

		if (next_tok.type == OPEN_BRAC_TOK)
		{
			//printf("calling function: '%.*s'\n", (int)ident.buf.len, ident.buf.s);
			Expr *name = calloc(1, sizeof(Expr));
			name->type = String_type;
			name->u.v.s = ident.buf;

			left->type = Operation_type;
			left->u.o.left = name;
			left->u.o.op = OP_FUNC_CALL_TOK;

			get_next_token(s);
			left->u.o.right = parse_expr(s, PARSER_MAX_PRECED);

			Token close_paren_tok = get_next_token(s);
			if (close_paren_tok.type != CLOSE_BRAC_TOK)
			{
				fprintf(stderr, "expected closing brace for function call, got %s\n", TOK_STRINGS[close_paren_tok.type]);
				free_expr(left);
				return NULL;
			}
		} else
		{
			left->type = String_type;
			left->u.v.s = ident.buf;
		}
	} else if (tok.type == OPEN_PAREN_TOK)
	{
		free(left); // have to free the memory block held in `left` before overwriting the pointer
		left = parse_expr(s, PARSER_MAX_PRECED);
		Token close_paren_tok = get_next_token(s);
		if (close_paren_tok.type != CLOSE_PAREN_TOK)
		{
			fprintf(stderr, "expected close paren for parenthesis block, got %s\n", TOK_STRINGS[close_paren_tok.type]);
			free_expr(left);
			return NULL;
		}
	} else if (tok.type == OPEN_BRACKET_TOK)
	{
		VecN vec = { calloc(1, sizeof(Expr *)), 0 };
		while (tok.type != CLOSE_BRACKET_TOK)
		{
			vec.ptr[vec.n++] = parse_expr(s, PARSER_MAX_PRECED);
			if ((tok = get_next_token(s)).type == COMMA_TOK)
			{
				Expr **tmp = realloc(vec.ptr, 2*vec.n * sizeof(Expr *));
				if (tmp == NULL)
				{
					fprintf(stderr, "unable to expand memory for vector\n");
					for (size_t i = 0; i < vec.n; ++i)
						free_expr(vec.ptr[i]);
					free(vec.ptr);
					free_expr(left);
					return NULL;
				}
				vec.ptr = tmp;
			} else break;
		}
		*left = (Expr) { Vector_type, .u.v.v = vec };
	} else if (tok.type == PIPE_TOK)
	{
		left = parse_expr(s, PARSER_MAX_PRECED);
		Token close_pipe_tok = get_next_token(s);
		if (close_pipe_tok.type != PIPE_TOK)
		{
			fprintf(stderr, "expected closing pipe for pipe block, got %s\n", TOK_STRINGS[close_pipe_tok.type]);
			free_expr(left);
			return NULL;
		}
		Expr *opnode = calloc(1, sizeof(Expr));
		opnode->type = Operation_type;
		opnode->u.o.left = left;
		opnode->u.o.right = NULL;
		opnode->u.o.op = PIPE_TOK;

		left = opnode;
	} else if (tok.type == NUMBER_TOK)
	{
		if (parser_config.looking_for_int)
			*left = EXPR_NUM((double)strtoll(tok.buf.s, NULL, 10));
		else
			*left = EXPR_NUM(strtod(tok.buf.s, NULL)); 
		parser_config.looking_for_int = false;
	} else
	{
		fprintf(stderr, "found no valid operations/literals, returning (null)\n");
		free_expr(left);
		return NULL;
	}

	for (;;)
	{
		Token op_tok = peek_token(s);
		if (op_tok.type == INVALID_TOK)
		{
			fprintf(stderr, "invalid token found\n");
			free_expr(left);
			return NULL;
		}
		if (op_tok.type > NOT_OP_TOK)
			break;

		uint32_t preced = PRECEDENCE[op_tok.type];
		if (preced > max_preced)
			break;

		get_next_token(s);

		if (op_tok.type == OP_DOT_TOK)
			parser_config.looking_for_int = true;

		Expr *right = parse_expr(s,
				op_is_right_associative(op_tok.type)
					? preced
					: preced-1);

		if (right == NULL)
		{
			fprintf(stderr, "missing right operand for operator %s\n",
					TOK_STRINGS[op_tok.type]);
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

Expr *parse(const char *s)
{
	memset(&parser_config, 0, sizeof(parser_config));
	return parse_expr(&s, PARSER_MAX_PRECED);
}
