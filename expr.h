#ifndef EXPR_H
#define EXPR_H

#include <stdint.h>

#include "token.h"

extern const uint8_t PRECEDENCE[];
extern const TokenType TOK_BY_CHAR[];

typedef struct Expr Expr;

typedef struct Operation {
	Expr *left;
	Expr *right;
	TokenType op;
} Operation;

typedef enum ExprType {
	Operation_type,
	Number_type,
	String_type,
} ExprType;

typedef struct Expr {
	ExprType type;
	union {
		Operation o;
		double n;
		strbuf s;
	} u;
} Expr;

void print_indent(uint32_t indent);
void print_exprh(Expr *expr);

void free_expr(Expr *e);

#endif /* EXPR_H */
