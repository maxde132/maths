#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
	OP_ADD_TOK,
	OP_SUB_TOK,
	OP_MUL_TOK,
	OP_DIV_TOK,
	VARIABLE_TOK,
	NUMBER_TOK,
	INVALID_TOK,
	EOF_TOK,
} TokenType;

#endif /* TOKEN_H */
