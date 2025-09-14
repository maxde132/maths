#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "mml/parser.h"
#include "mml/eval.h"
#include "mml/token.h"

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
	"INSERTED_IDENT_TOK",
	"NUMBER_TOK",

	"DIGIT_TOK",
	"LETTER_TOK",
	"UNDERSCORE_TOK",

	"OPEN_PAREN_TOK",
	"CLOSE_PAREN_TOK",

	"OPEN_BRAC_TOK",
	"CLOSE_BRAC_TOK",

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

const char *const EXPR_TYPE_STRINGS[] = {
	"operation",
	"real number",
	"complex number",
	"boolean",
	"identifier",
	"inserted identifier",
	"vector",
	"invalid",
};


static MML_Token current_tok = nToken(MML_INVALID_TOK, NULL, 0);

// Gets the next token and advances the string pointer.
static MML_Token get_next_token(const char **s, struct parser_state *state)
{
	MML_Token ret = nToken(MML_INVALID_TOK, NULL, 0);

	if (state->has_peeked) {
		state->has_peeked = false;
		*s = state->saved_s;
		return state->peeked_tok;
	}

	while (isspace(**s)) ++*s;

	if (**s == '\0') return nToken(MML_EOF_TOK, NULL, 0);

	MML_TokenType type = TOK_BY_CHAR[**s - 0x21];
	switch (type) {
	case MML_OP_DOT_TOK:
	case MML_OP_AT_TOK:
	case MML_OP_POW_TOK:
	case MML_OP_MUL_TOK:
	case MML_OP_DIV_TOK:
	case MML_OP_MOD_TOK:
	case MML_OP_ADD_TOK:
	case MML_OP_SUB_TOK:
	case MML_OPEN_PAREN_TOK:
	case MML_CLOSE_PAREN_TOK:
	case MML_OPEN_BRAC_TOK:
	case MML_CLOSE_BRAC_TOK:
	case MML_OPEN_BRACKET_TOK:
	case MML_CLOSE_BRACKET_TOK:
	case MML_COMMA_TOK:
	case MML_PIPE_TOK:
	case MML_SEMICOLON_TOK:
	case MML_TILDE_TOK:
		ret = nToken(type, *s, 1);
		++*s;
		break;
	case MML_OP_LESS_TOK:
		if ((*s)[1] == '=')
		{
			ret = nToken(MML_OP_LESSEQ_TOK, *s, 2);
			*s += 2;
			break;
		}
		ret = nToken(MML_OP_LESS_TOK, *s, 1);
		++*s;
		break;
	case MML_OP_GREATER_TOK:
		if ((*s)[1] == '=')
		{
			ret = nToken(MML_OP_GREATEREQ_TOK, *s, 2);
			*s += 2;
			break;
		}
		ret = nToken(MML_OP_GREATER_TOK, *s, 1);
		++*s;
		break;
	case MML_OP_EQ_TOK:
		if ((*s)[1] != '=')
		{
			ret = nToken(MML_OP_ASSERT_EQUAL, *s, 1);
			++*s;
			break;
		}
		ret = nToken(MML_OP_EQ_TOK, *s, 2);
		*s += 2;
		break;
	case MML_OP_NOT_TOK:
		if ((*s)[1] == '=')
		{
			ret = nToken(MML_OP_NOTEQ_TOK, *s, 2);
			*s += 2;
			break;
		}
		ret = nToken(MML_OP_NOT_TOK, *s, 1);
		++*s;
		break;
	case MML_DIGIT_TOK:
		ret.buf.s = (char *)*s;
		char *end;
		if (state->looking_for_int)
			strtoll(ret.buf.s, &end, 10);
		else
			strtod(ret.buf.s, &end);
		ret.buf.len = end - ret.buf.s;
		*s = end;
		ret.type = MML_NUMBER_TOK;
		break;
	case MML_DOLLAR_TOK:
		++*s;
		[[fallthrough]];
	case MML_LETTER_TOK:
	case MML_UNDERSCORE_TOK:
		ret.buf.s = (char *)*s;
		do {
			++*s;
		} while (isalnum(**s) || **s == '_');
		ret.buf.len = *s - ret.buf.s;
		ret.type = (type == MML_DOLLAR_TOK) ? MML_INSERTED_IDENT_TOK : MML_IDENT_TOK;
		ret.buf.allocd = false;
		break;
	default:
		fprintf(stderr, "invalid token starts at '%.5s'\n", *s);
		break;
	}


	current_tok = ret;
	return ret;
}

// Peeks at the next token without advancing the string pointer.
static MML_Token peek_token(const char **s, struct parser_state *state)
{
	if (!state->has_peeked)
	{
		const char *s_copy = *s;
		state->peeked_tok = get_next_token(&s_copy, state);
		state->saved_s = s_copy;
		state->has_peeked = true;
	}
	return state->peeked_tok;
}

#define PARSER_MAX_PRECED 15

static bool op_is_unary(MML_TokenType op)
{
	return (op == MML_TILDE_TOK || (op >= MML_OP_NOT_TOK && op <= MML_OP_UNARY_NOTHING));
}

static bool op_is_right_associative(MML_TokenType op)
{
	return op == MML_OP_POW_TOK || op_is_unary(op);
}

static MML_Expr *parse_expr(const char **s, uint32_t max_preced, struct parser_state *state)
{
	MML_Token tok = get_next_token(s, state);

	MML_Expr *left = calloc(1, sizeof(MML_Expr));
	left->num_refs = 1;

	if (tok.type == MML_OP_SUB_TOK || tok.type == MML_OP_ADD_TOK
			|| op_is_unary(tok.type))
	{
		MML_TokenType new_token_type = tok.type;
		if (new_token_type == MML_OP_ADD_TOK)
			new_token_type = MML_OP_UNARY_NOTHING;
		else if (new_token_type == MML_OP_SUB_TOK)
			new_token_type = MML_OP_NEGATE;
		
		MML_Expr *operand = parse_expr(s, PRECEDENCE[new_token_type], state);

		left->type = Operation_type;
		left->u.o.left = operand;
		left->u.o.right = NULL;
		left->u.o.op = new_token_type;
	} else if (tok.type == MML_IDENT_TOK || tok.type == MML_INSERTED_IDENT_TOK)
	{
		MML_Token ident = tok;
		MML_Token next_tok = peek_token(s, state);

		if (tok.type == MML_IDENT_TOK && next_tok.type == MML_OPEN_BRAC_TOK)
		{
			MML_Expr *name = calloc(1, sizeof(MML_Expr));
			name->type = Identifier_type;
			name->num_refs = 1;
			name->u.v.s = ident.buf;

			left->type = Operation_type;
			left->u.o.left = name;
			left->u.o.op = MML_OP_FUNC_CALL_TOK;

			get_next_token(s, state);

			left->u.o.right = calloc(1, sizeof(MML_Expr));
			left->u.o.right->type = Vector_type;
			left->u.o.right->num_refs = 1;
			left->u.o.right->should_free_vec_block = false;
			left->u.o.right->u.v.v = MML_new_vec(1);
			do
			{
				if (peek_token(s, state).type == MML_CLOSE_BRAC_TOK)
					break;
				MML_Expr *next_expr = parse_expr(s, PARSER_MAX_PRECED, state);
				MML_push_to_vec(&left->u.o.right->u.v.v,
						next_expr);
				--next_expr->num_refs;
			} while (get_next_token(s, state).type == MML_COMMA_TOK);

			if (current_tok.type != MML_CLOSE_BRAC_TOK)
			{
				fprintf(stderr, "expected closing brace for function call, got %s\n",
						TOK_STRINGS[next_tok.type]);
				MML_free_expr(&left);
				return nullptr;
			}
		} else
		{
			left->type = (tok.type == MML_IDENT_TOK)
				? Identifier_type
				: InsertedIdentifier_type;
			left->u.v.s = ident.buf;
		}
	} else if (tok.type == MML_OPEN_PAREN_TOK)
	{
		free(left); // have to free the memory block held in `left` before overwriting the pointer
		left = parse_expr(s, PARSER_MAX_PRECED, state);
		MML_Token close_paren_tok = get_next_token(s, state);
		if (close_paren_tok.type != MML_CLOSE_PAREN_TOK)
		{
			fprintf(stderr, "expected close paren for parenthesis block, got %s\n", TOK_STRINGS[close_paren_tok.type]);
			MML_free_expr(&left);
			return nullptr;
		}
	} else if (tok.type == MML_OPEN_BRACKET_TOK)
	{
		MML_VecN vec = MML_new_vec(1);
		while (tok.type != MML_CLOSE_BRACKET_TOK)
		{
			tok = peek_token(s, state);
			if (tok.type == MML_CLOSE_BRACKET_TOK)
				break;

			MML_Expr *e = parse_expr(s, PARSER_MAX_PRECED, state);
			MML_push_to_vec(&vec, e);
			--e->num_refs;

			tok = get_next_token(s, state);
			if (tok.type != MML_CLOSE_BRACKET_TOK
			 && tok.type != MML_COMMA_TOK)
			{
				fprintf(stderr, "unexpected token %s found after element"
						" in vector literal (expected CLOSE_BRACKET_TOK or COMMA_TOK)\n",
					 TOK_STRINGS[tok.type]);
				MML_free_expr(&left);
				MML_free_vec(&vec);
				return nullptr;
			}
		}
		*left = (MML_Expr) { Vector_type, 1, .u.v.v = vec };
	} else if (tok.type == MML_PIPE_TOK)
	{
		free(left);
		left = nullptr;
		tok = peek_token(s, state);
		if (tok.type == MML_PIPE_TOK)
		{
			fprintf(stderr, "expected expression in pipe block\n");
			return nullptr;
		}
		left = parse_expr(s, PARSER_MAX_PRECED, state);
		MML_Token close_pipe_tok = get_next_token(s, state);
		if (close_pipe_tok.type != MML_PIPE_TOK)
		{
			fprintf(stderr, "expected closing pipe for pipe block, got %s\n",
					TOK_STRINGS[close_pipe_tok.type]);
			MML_free_expr(&left);
			return nullptr;
		}
		MML_Expr *opnode = calloc(1, sizeof(MML_Expr));
		opnode->type = Operation_type;
		opnode->num_refs = 1;
		opnode->u.o.left = left;
		opnode->u.o.right = nullptr;
		opnode->u.o.op = MML_PIPE_TOK;

		left = opnode;
	} else if (tok.type == MML_NUMBER_TOK)
	{
		if (state->looking_for_int)
			*left = EXPR_NUM((double)strtoll(tok.buf.s, NULL, 10));
		else
			*left = EXPR_NUM(strtod(tok.buf.s, NULL)); 
		left->num_refs = 1;
		state->looking_for_int = false;
	} else
	{
		//fprintf(stderr, "found no valid operations/literals, returning (null)\n");
		MML_free_expr(&left);
		return nullptr;
	}

	for (;;)
	{
		MML_Token op_tok = peek_token(s, state);
		if (op_tok.type == MML_INVALID_TOK)
		{
			fprintf(stderr, "invalid token found\n");
			MML_free_expr(&left);
			return nullptr;
		}
		bool do_advance = true;
		if (op_tok.type == MML_INSERTED_IDENT_TOK
		 || op_tok.type == MML_IDENT_TOK || op_tok.type == MML_NUMBER_TOK)
		{
			op_tok.type = MML_OP_MUL_TOK;
			do_advance = false;
		}
		if (op_tok.type > MML_NOT_OP_TOK)
			break;

		uint32_t preced = PRECEDENCE[op_tok.type];
		if (preced > max_preced)
			break;

		if (do_advance) get_next_token(s, state);

		if (op_tok.type == MML_OP_DOT_TOK)
			state->looking_for_int = true;

		MML_Expr *right = parse_expr(s,
				op_is_right_associative(op_tok.type)
					? preced
					: preced-1, state);

		if (right == nullptr)
		{
			fprintf(stderr, "expected expression after operator %s\n",
					TOK_STRINGS[op_tok.type]);
			MML_free_expr(&left);
			return nullptr;
		}

		MML_Expr *opnode = calloc(1, sizeof(MML_Expr));
		opnode->type = Operation_type;
		opnode->num_refs = 1;
		opnode->u.o.left = left;
		opnode->u.o.right = right;
		opnode->u.o.op = op_tok.type;

		left = opnode;
	}

	return left;
}

inline MML_Expr *MML_parse(const char *s)
{
	struct parser_state state = {0};
	return parse_expr(&s, PARSER_MAX_PRECED, &state);
}
MML_VecN MML_parse_stmts_to_ret(const char *s)
{
	MML_VecN ret = MML_new_vec(1);
	struct parser_state state = {0};
	do
	{
		MML_push_to_vec(&ret, parse_expr(&s, PARSER_MAX_PRECED, &state));
	} while (get_next_token(&s, &state).type == MML_SEMICOLON_TOK);

	return ret;
}

void MML_parse_stmts(const char *s, struct MML_state *state)
{
	struct parser_state parser_state = {0};
	do
	{
		MML_Expr *cur_expr = parse_expr(&s, PARSER_MAX_PRECED, &parser_state);
		if (cur_expr != nullptr)
			MML_eval_push_expr(state, cur_expr);
	} while (get_next_token(&s, &parser_state).type == MML_SEMICOLON_TOK);
}
