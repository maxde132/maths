#ifndef EXPR_H
#define EXPR_H

#include <complex.h>
#include <stdint.h>

#include "token.h"
#include "cvi/dvec/dvec.h"

typedef struct MML_Expr MML_Expr;

typedef struct MML_Operation {
	MML_Expr *left;
	MML_Expr *right;
	MML_TokenType op;
} MML_Operation;

typedef enum MML_ExprType {
	Invalid_type,
	Operation_type,
	Integer_type,
	RealNumber_type,
	ComplexNumber_type,
	Boolean_type,
	Identifier_type,
	Vector_type,
} MML_ExprType;

typedef dvec_t(MML_Expr *) MML_ExprVec;

#define VALTYPE_IS_ORDERED(v) \
	((v).type != ComplexNumber_type && \
	 (v).type != Vector_type && \
	 (v).type != Invalid_type)

struct value_union_size {
	uint8_t b[24];
};

typedef struct MML_Value {
	MML_ExprType type;
	union {
		double n;
		_Complex double cn;
		bool b;
		strbuf s;
		MML_ExprVec v;
		int64_t i;
		struct value_union_size w;
	};
} MML_Value;

typedef struct MML_Expr {
	MML_ExprType type;
	uint16_t num_refs;
	bool should_free_vec_block;
	union {
		MML_Operation o;
		double n;
		_Complex double cn;
		bool b;
		strbuf s;
		MML_ExprVec v;
		int64_t i;
		struct value_union_size w;
	};
} MML_Expr;

#define EXPR_NUM(num) ((MML_Expr) { RealNumber_type, .n = (num) })
#define VAL_NUM(num) ((MML_Value) { RealNumber_type, .n = (num) })
#define VAL_CNUM(num) ((MML_Value) { ComplexNumber_type, .cn = (num) })
#define VAL_BOOL(bl) ((MML_Value) { Boolean_type, .b = (bl) })

enum INVAL_TYPES {
	MML_ERROR_INVAL,
	MML_QUIT_INVAL,
	MML_CLEAR_INVAL,
};

constexpr MML_Value VAL_INVAL = { Invalid_type, .i = MML_ERROR_INVAL };

#define VAL_IS_NUM(v) (\
    (v).type == RealNumber_type \
 || (v).type == ComplexNumber_type \
 || (v).type == Boolean_type)

typedef struct MML_state MML_state;
void MML_print_indent(uint32_t indent);
MML_Value MML_print_typedval(MML_state *, const MML_Value *val);
MML_Value MML_println_typedval(MML_state *state, const MML_Value *val);
MML_Value MML_print_typedval_multiargs(MML_state *state, MML_ExprVec *args);
MML_Value MML_println_typedval_multiargs(MML_state *state, MML_ExprVec *args);
void MML_print_exprh(MML_Expr *expr);
MML_Value MML_print_exprh_tv_func(MML_state *, MML_ExprVec *args);

void MML_free_expr(MML_Expr **e);

void MML_free_vec(MML_ExprVec *vec);

void MML_free_pp(void *p);


double MML_get_number(const MML_Value *v);
_Complex double MML_get_complex(const MML_Value *v);

#endif /* EXPR_H */
