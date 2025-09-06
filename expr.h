#ifndef EXPR_H
#define EXPR_H

#include <stdint.h>

typedef enum Operator {
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
} Operator;

const uint8_t precedence[] = {
	4, 4,
	3, 3,
};

typedef struct Expr Expr;

typedef struct Operation {
	Expr *left;
	Expr *right;
	Operator op;
} Operation;

typedef enum ExprType {
	Operation_type,
	Number_type,
} ExprType;

typedef struct Expr {
	ExprType type;
	union {
		Operation o;
		double n;
	} u;
} Expr;

#endif /* EXPR_H */
