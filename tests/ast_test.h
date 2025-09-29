#ifndef MML_AST_TEST_H
#define MML_AST_TEST_H

#include <stdarg.h>
#include <stdlib.h>

#include "mml/expr.h"

#define _create_leaf(t, f, val) ({ \
	MML_expr *e__ = calloc(1, sizeof(MML_expr)); \
	e__->type = (t); \
	e__->f = (val); \
	e__->num_refs = 1; \
	e__; \
})

static inline MML_expr *_create_vec_leaf(size_t n, ...) { 
	MML_expr *e = calloc(1, sizeof(MML_expr));
	e->type = Vector_type;
	dv_init(e->v);
	dv_resize(e->v, n);

	va_list ap;
	va_start(ap, n);
	for (size_t i = 0; i < n; ++i)
		dv_a(e->v, i) = va_arg(ap, MML_expr *);
	va_end(ap);

	e->num_refs = 1;

	return e;
}


/* whichever is named `_create_oper` is the one used */
#define _create_oper(oper, a, b) ({ \
	MML_expr *__e = calloc(1, sizeof(MML_expr)); \
	__e->type = Operation_type; \
	__e->o.op = (oper); \
	__e->o.left = (a); \
	__e->o.right = (b); \
	__e->num_refs = 1; \
	__e; \
})
static inline MML_expr *__create_oper(MML_token_type op, MML_expr *a, MML_expr *b) {
	MML_expr *e = calloc(1, sizeof(MML_expr));
	e->type = Operation_type;
	e->o.op = op;
	e->o.left = a;
	e->o.right = b;
	e->num_refs = 1;
	return e;
}

#define CallFunction(a, b) \
	(_create_oper(MML_OP_FUNC_CALL_TOK, (a), (b)))

#define Add(a, b) \
	(_create_oper(MML_OP_ADD_TOK, (a), (b)))
#define Subtract(a, b) \
	(_create_oper(MML_OP_SUB_TOK, (a), (b)))
#define Multiply(a, b) \
	(_create_oper(MML_OP_MUL_TOK, (a), (b)))
#define Divide(a, b) \
	(_create_oper(MML_OP_DIV_TOK, (a), (b)))

#define Modulo(a, b) \
	(_create_oper(MML_OP_MOD_TOK, (a), (b)))
#define Power(a, b) \
	(_create_oper(MML_OP_POW_TOK, (a), (b)))
#define Root(a, b) \
	(_create_oper(MML_OP_ROOT, (a), (b)))

#define LessThan(a, b) \
	(_create_oper(MML_OP_LESS_TOK, (a), (b)))
#define GreaterThan(a, b) \
	(_create_oper(MML_OP_GREATER_TOK, (a), (b)))
#define LessThanOrEqual(a, b) \
	(_create_oper(MML_OP_LESSEQ_TOK, (a), (b)))
#define GreaterThanOrEqual(a, b) \
	(_create_oper(MML_OP_GREATEREQ_TOK, (a), (b)))
#define Equal(a, b) \
	(_create_oper(MML_OP_EQ_TOK, (a), (b)))
#define NotEqual(a, b) \
	(_create_oper(MML_OP_NOTEQ_TOK, (a), (b)))
#define ExactlyEqual(a, b) \
	(_create_oper(MML_OP_EXACT_EQ_TOK, (a), (b)))
#define ExactlyNotEqual(a, b) \
	(_create_oper(MML_OP_EXACT_NOTEQ_TOK, (a), (b)))

#define AssertEqual(a, b) \
	(_create_oper(MML_OP_ASSERT_EQUAL, (a), (b)))

#define Not(a, b) \
	(_create_oper(MML_OP_NOT_TOK, (a), (b)))
#define Negate(a, b) \
	(_create_oper(MML_OP_NEGATE, (a), (b)))


#define Real(r) \
	(_create_leaf(RealNumber_type, n, (r)))
#define Complex(c) \
	(_create_leaf(ComplexNumber_type, cn, (c)))
#define Boolean(bl) \
	(_create_leaf(Boolean_type, b, (bl)))
#define Vector(n, ...) \
	(_create_vec_leaf(n, ##__VA_ARGS__))

#define Identifier(sl) \
	(_create_leaf(Identifier_type, s, ((strbuf) { (sl), sizeof(sl)-1, false })))
#define IdentifierS(sll) \
	(_create_leaf(Identifier_type, s, ((strbuf) { ( #sll ), sizeof(#sll)-1, false })))
#define IdentifierL(s_, l) \
	(_create_leaf(Identifier_type, s, ((strbuf) { (s_), (l), false })))

#define Function(sl) \
	(_create_leaf(Identifier_type, s, ((strbuf) { (sl), sizeof(sl)-1, false })))
#define FunctionS(sll) \
	(_create_leaf(Identifier_type, s, ((strbuf) { ( #sll ), sizeof(#sll)-1, false })))
#define FunctionL(s_, l) \
	(_create_leaf(Identifier_type, s, ((strbuf) { (s_), (l), false })))

#define Variable(sl) \
	(_create_leaf(Identifier_type, s, ((strbuf) { (sl), sizeof(sl)-1, false })))
#define VariableS(sll) \
	(_create_leaf(Identifier_type, s, ((strbuf) { ( #sll ), sizeof(#sll)-1, false })))
#define VariableL(s_, l) \
	(_create_leaf(Identifier_type, s, ((strbuf) { (s_), (l), false })))



#endif /* MML_AST_TEST_H */
