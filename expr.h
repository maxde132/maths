#ifndef EXPR_H
#define EXPR_H

#include <stdint.h>

#include "token.h"

extern const uint8_t PRECEDENCE[];
extern const TokenType TOK_BY_CHAR[];
extern const char *TOK_STRINGS[];

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

#define PI_M	3.14159265358979323846
#define E_M		2.71828182845904523536
#define PHI_M	1.61803398874989484820

void print_indent(uint32_t indent);
void print_exprh(Expr *expr);

void free_expr(Expr *e);

#endif /* EXPR_H */
