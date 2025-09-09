#ifndef EXPR_H
#define EXPR_H

#include <complex.h>
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
	RealNumber_type,
	ComplexNumber_type,
	Boolean_type,
	String_type,
	Vector_type,
} ExprType;

typedef struct VecN {
	Expr **ptr;
	size_t n;
} VecN;

typedef union EvalValue {
	double n;
	_Complex double cn;
	bool b;
	strbuf s;
	VecN v;
} EvalValue;

typedef struct TypedValue {
	ExprType type;
	EvalValue v;
} TypedValue;

typedef struct Expr {
	ExprType type;
	union {
		Operation o;
		EvalValue v;
	} u;
} Expr;

#define EXPR_NUM(num) ((Expr) { RealNumber_type, .u.v.n = (num) })
#define VAL_NUM(num) ((TypedValue) { RealNumber_type, .v.n = (num) })
#define VAL_CNUM(num) ((TypedValue) { ComplexNumber_type, .v.cn = (num) })
#define VAL_BOOL(bl) ((TypedValue) { Boolean_type, .v.b = (bl) })
#define VAL2EXPRP(val) (&(Expr) { .type = (val).type, .u.v = (val).v })

#define VAL_IS_NUM(v) (\
    (v).type == RealNumber_type \
 || (v).type == ComplexNumber_type \
 || (v).type == Boolean_type)

#define PI_M	3.14159265358979323846
#define E_M		2.71828182845904523536
#define PHI_M	1.61803398874989484820

void print_indent(uint32_t indent);
void print_typedval(TypedValue *val);
void println_typedval(TypedValue *val);
void print_exprh(Expr *expr);

void free_expr(Expr *e);

TypedValue *construct_vec(size_t n, ...);

double get_number(TypedValue *v);
_Complex double get_complex(TypedValue *v);

#endif /* EXPR_H */
