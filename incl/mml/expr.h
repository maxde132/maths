#ifndef EXPR_H
#define EXPR_H

#include <complex.h>
#include <stdint.h>

#include "token.h"

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

typedef struct MML_VecN {
	MML_Expr **ptr;
	size_t n;
	size_t allocd_size;
} MML_VecN;

typedef union MML_EvalValue {
	double n;
	_Complex double cn;
	bool b;
	strbuf s;
	MML_VecN v;
	int64_t i;
} MML_EvalValue;

#define VALTYPE_IS_ORDERED(v) \
	((v).type != ComplexNumber_type && \
	 (v).type != Vector_type && \
	 (v).type != Invalid_type)

typedef struct MML_Value {
	MML_ExprType type;
	MML_EvalValue v;
} MML_Value;

typedef struct MML_Expr {
	MML_ExprType type;
	uint16_t num_refs;
	bool should_free_vec_block;
	union {
		MML_Operation o;
		MML_EvalValue v;
	} u;
} MML_Expr;

#define EXPR_NUM(num) ((MML_Expr) { RealNumber_type, .u.v.n = (num) })
#define VAL_NUM(num) ((MML_Value) { RealNumber_type, .v.n = (num) })
#define VAL_CNUM(num) ((MML_Value) { ComplexNumber_type, .v.cn = (num) })
#define VAL_BOOL(bl) ((MML_Value) { Boolean_type, .v.b = (bl) })
#define VAL2EXPRP(val) (&(MML_Expr) { .type = (val).type, .u.v = (val).v })

enum INVAL_TYPES {
	MML_ERROR_INVAL,
	MML_QUIT_INVAL,
	MML_CLEAR_INVAL,
};

constexpr MML_Value VAL_INVAL = { Invalid_type, .v.i = MML_ERROR_INVAL };

#define VAL_IS_NUM(v) (\
    (v).type == RealNumber_type \
 || (v).type == ComplexNumber_type \
 || (v).type == Boolean_type)

typedef struct MML_state MML_state;
void MML_print_indent(uint32_t indent);
MML_Value MML_print_typedval(MML_state *, const MML_Value *val);
MML_Value MML_println_typedval(MML_state *state, const MML_Value *val);
MML_Value MML_print_typedval_multiargs(MML_state *state, MML_VecN *args);
MML_Value MML_println_typedval_multiargs(MML_state *state, MML_VecN *args);
void MML_print_exprh(MML_Expr *expr);
MML_Value MML_print_exprh_tv_func(MML_state *, MML_VecN *args);

void MML_free_expr(MML_Expr **e);
void MML_free_expr_not_parent(MML_Expr **e);

#ifndef NDEBUG
struct call_info {
	size_t line_n;
	const char *filename;
};

MML_VecN MML_new_vec_debug(size_t n, struct call_info call);
void MML_free_vec_debug(MML_VecN *vec, struct call_info call);
#define MML_new_vec(n) (MML_new_vec_debug(n, (struct call_info) {__LINE__, __FILE_NAME__}))
#define MML_free_vec(vec) (MML_free_vec_debug(vec, (struct call_info) {__LINE__, __FILE_NAME__}))
#else
MML_VecN MML_new_vec(size_t n);
void MML_free_vec(MML_VecN *vec);
#endif
MML_VecN MML_construct_vec(size_t n, ...);
/* does a push-move (pushes VAL to vector, "moves" VAL to vector).
 * "move" means it doesn't increment the reference count, so you might get
 * memory leaks if you try to use/free VAL after calling this. */
int32_t MML_push_to_vec(MML_VecN *vec, MML_Expr *val);
MML_Expr **MML_peek_top_vec(MML_VecN *vec);
MML_Expr *MML_pop_from_vec(MML_VecN *vec);
void MML_print_vec(const MML_VecN *vec);
void MML_println_vec(const MML_VecN *vec);

void MML_free_pp(void *p);


double MML_get_number(MML_Value *v);
_Complex double MML_get_complex(MML_Value *v);

#endif /* EXPR_H */
