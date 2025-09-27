#ifndef EXPR_H
#define EXPR_H

#include <complex.h>
#include <stdint.h>

#include "token.h"
#include "cvi/dvec/dvec.h"

typedef struct MML_expr MML_expr;

typedef struct MML_Operation {
	MML_expr *left;
	MML_expr *right;
	MML_token_type op;
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
} MML_expr_type;

typedef dvec_t(MML_expr *) MML_expr_vec;

#define VALTYPE_IS_ORDERED(v) \
	((v).type != ComplexNumber_type && \
	 (v).type != Vector_type && \
	 (v).type != Invalid_type)

struct value_union_size {
	uint8_t b[24];
};

typedef struct MML_value {
	MML_expr_type type;
	union {
		double n;
		_Complex double cn;
		bool b;
		strbuf s;
		MML_expr_vec v;
		int64_t i;
		struct value_union_size w;
	};
} MML_value;

typedef struct MML_expr {
	MML_expr_type type;
	uint16_t num_refs;
	bool should_free_vec_block;
	union {
		MML_Operation o;
		double n;
		_Complex double cn;
		bool b;
		strbuf s;
		MML_expr_vec v;
		int64_t i;
		struct value_union_size w;
	};
} MML_expr;

#define EXPR_NUM(num) ((MML_expr) { RealNumber_type, .n = (num) })
#define VAL_NUM(num) ((MML_value) { RealNumber_type, .n = (num) })
#define VAL_CNUM(num) ((MML_value) { ComplexNumber_type, .cn = (num) })
#define VAL_BOOL(bl) ((MML_value) { Boolean_type, .b = (bl) })

enum INVAL_TYPES {
	MML_ERROR_INVAL,
	MML_QUIT_INVAL,
	MML_CLEAR_INVAL,
};

constexpr MML_value VAL_INVAL = { Invalid_type, .i = MML_ERROR_INVAL };

#define VAL_IS_NUM(v) (\
    (v).type == RealNumber_type \
 || (v).type == ComplexNumber_type \
 || (v).type == Boolean_type)

typedef struct MML_state MML_state;
void MML_print_indent(uint32_t indent);
MML_value MML_print_typedval(MML_state *, const MML_value *val);
MML_value MML_println_typedval(MML_state *state, const MML_value *val);
MML_value MML_print_typedval_multiargs(MML_state *state, MML_expr_vec *args);
MML_value MML_println_typedval_multiargs(MML_state *state, MML_expr_vec *args);
void MML_print_exprh(MML_expr *expr);
MML_value MML_print_exprh_tv_func(MML_state *, MML_expr_vec *args);

void MML_free_expr(MML_expr **e);

void MML_free_vec(MML_expr_vec *vec);

void MML_free_pp(void *p);


double MML_get_number(const MML_value *v);
_Complex double MML_get_complex(const MML_value *v);

#endif /* EXPR_H */
