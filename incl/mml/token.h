#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

typedef enum {
	// operator tokens
	MML_OP_FUNC_CALL_TOK,
	MML_OP_DOT_TOK,
	MML_OP_AT_TOK,

	MML_OP_POW_TOK,

	MML_OP_MUL_TOK, MML_OP_DIV_TOK, MML_OP_MOD_TOK,
	MML_OP_ADD_TOK, MML_OP_SUB_TOK,

	MML_OP_LESS_TOK, MML_OP_GREATER_TOK,
	MML_OP_LESSEQ_TOK, MML_OP_GREATEREQ_TOK,
	MML_OP_EQ_TOK, MML_OP_NOTEQ_TOK,

	MML_OP_ASSERT_EQUAL,

	MML_OP_NOT_TOK, // this is boolean not
	MML_OP_NEGATE,
	MML_OP_UNARY_NOTHING,
	MML_NOT_OP_TOK,

	// non-operator tokens
	MML_IDENT_TOK,
	MML_NUMBER_TOK,

	MML_DIGIT_TOK, // not really used
	MML_LETTER_TOK,
	MML_UNDERSCORE_TOK,

	MML_OPEN_PAREN_TOK,
	MML_CLOSE_PAREN_TOK,

	MML_OPEN_BRAC_TOK,
	MML_CLOSE_BRAC_TOK,

	MML_OPEN_BRACKET_TOK,
	MML_CLOSE_BRACKET_TOK,

	MML_DQUOTE_TOK,
	MML_SQUOTE_TOK,
	MML_BACKTICK_TOK,

	MML_COLON_TOK,
	MML_SEMICOLON_TOK,
	MML_COMMA_TOK,

	MML_HASHTAG_TOK,
	MML_QUESTION_TOK,
	MML_BACKSLASH_TOK,
	MML_DOLLAR_TOK,
	MML_AMPER_TOK,
	MML_PIPE_TOK,
	MML_TILDE_TOK,

	MML_INVALID_TOK,
	MML_EOF_TOK,
} MML_TokenType;

typedef struct strbuf {
	char *s;
	size_t len;
	bool allocd;
} strbuf;

#define str_lit(s) ((strbuf) { (s), sizeof(s)-1, false })

typedef struct MML_Token {
	strbuf buf;
	MML_TokenType type;
} MML_Token;

#define nToken(type, buf, len) ((MML_Token) { { (char *)(buf), (len), false }, (type) })

#endif /* TOKEN_H */
