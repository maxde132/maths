#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
	// operator tokens
	OP_FUNC_CALL_TOK,
	OP_DOT_TOK,

	OP_POW_TOK,

	OP_MUL_TOK, OP_DIV_TOK, OP_MOD_TOK,
	OP_ADD_TOK, OP_SUB_TOK,

	OP_LESS_TOK, OP_GREATER_TOK,
	OP_LESSEQ_TOK, OP_GREATEREQ_TOK,
	OP_EQ_TOK, OP_NOTEQ_TOK,

	OP_NOT_TOK, // this is boolean not
	NOT_OP_TOK,

	// non-operator tokens
	IDENT_TOK,
	NUMBER_TOK,

	DIGIT_TOK, // not really used
	LETTER_TOK,
	UNDERSCORE_TOK,

	OPEN_PAREN_TOK,
	CLOSE_PAREN_TOK,

	OPEN_BRAC_TOK,
	CLOSE_BRAC_TOK,

	OPEN_BRACKET_TOK,
	CLOSE_BRACKET_TOK,

	DQUOTE_TOK,
	SQUOTE_TOK,
	BACKTICK_TOK,

	COLON_TOK,
	SEMICOLON_TOK,
	COMMA_TOK,

	HASHTAG_TOK,
	QUESTION_TOK,
	BACKSLASH_TOK,
	DOLLAR_TOK,
	AMPER_TOK,
	PIPE_TOK,
	TILDE_TOK,
	AT_SYMB_TOK,

	INVALID_TOK,
	EOF_TOK,
} TokenType;

typedef struct strbuf {
	char *s;
	size_t len;
	bool allocd;
} strbuf;

typedef struct Token {
	strbuf buf;
	TokenType type;
} Token;

#define nToken(type, buf, len) ((Token) { { (char *)(buf), (len), false }, (type) })

#endif /* TOKEN_H */
