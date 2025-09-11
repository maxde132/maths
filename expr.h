#ifndef EXPR_H
#define EXPR_H

#include <complex.h>
#include <stdint.h>

#include "token.h"

constexpr const uint8_t PRECEDENCE[] = {
	1,
	1,
	1,

	2,

	3, 3, 3,
	4, 4,

	6, 6, 6, 6,
	7, 7,

	2, 2, 2,
};
constexpr const TokenType TOK_BY_CHAR[] = { // starts at 0x21
	OP_NOT_TOK,		//'!'
	DQUOTE_TOK,		//'"'
	HASHTAG_TOK,	//'#'
	DOLLAR_TOK,		//'$'
	OP_MOD_TOK,		//'%'
	AMPER_TOK,		//'&'
	SQUOTE_TOK,		//'\''
	OPEN_PAREN_TOK,	//'('
	CLOSE_PAREN_TOK,	//')'
	OP_MUL_TOK,		//'*'
	OP_ADD_TOK,		//'+'
	COMMA_TOK,		//','
	OP_SUB_TOK,		//'-'
	OP_DOT_TOK,		//'.'
	OP_DIV_TOK,		//'/'
	DIGIT_TOK,		//'0'
	DIGIT_TOK,		//'1'
	DIGIT_TOK,		//'2'
	DIGIT_TOK,		//'3'
	DIGIT_TOK,		//'4'
	DIGIT_TOK,		//'5'
	DIGIT_TOK,		//'6'
	DIGIT_TOK,		//'7'
	DIGIT_TOK,		//'8'
	DIGIT_TOK,		//'9'
	COLON_TOK,		//':'
	SEMICOLON_TOK,	//';'
	OP_LESS_TOK,	//'<'
	OP_EQ_TOK,		//'='
	OP_GREATER_TOK,	//'>'
	QUESTION_TOK,	//'?'
	OP_AT_TOK,		//'@'
	LETTER_TOK,		//'A'
	LETTER_TOK,		//'B'
	LETTER_TOK,		//'C'
	LETTER_TOK,		//'D'
	LETTER_TOK,		//'E'
	LETTER_TOK,		//'F'
	LETTER_TOK,		//'G'
	LETTER_TOK,		//'H'
	LETTER_TOK,		//'I'
	LETTER_TOK,		//'J'
	LETTER_TOK,		//'K'
	LETTER_TOK,		//'L'
	LETTER_TOK,		//'M'
	LETTER_TOK,		//'N'
	LETTER_TOK,		//'O'
	LETTER_TOK,		//'P'
	LETTER_TOK,		//'Q'
	LETTER_TOK,		//'R'
	LETTER_TOK,		//'S'
	LETTER_TOK,		//'T'
	LETTER_TOK,		//'U'
	LETTER_TOK,		//'V'
	LETTER_TOK,		//'W'
	LETTER_TOK,		//'X'
	LETTER_TOK,		//'Y'
	LETTER_TOK,		//'Z'
	OPEN_BRACKET_TOK,	//'['
	BACKSLASH_TOK,	//'\\'
	CLOSE_BRACKET_TOK,//']'
	OP_POW_TOK,		//'^'
	UNDERSCORE_TOK,	//'_'
	BACKTICK_TOK,	//'`'
	LETTER_TOK,		//'a'
	LETTER_TOK,		//'b'
	LETTER_TOK,		//'c'
	LETTER_TOK,		//'d'
	LETTER_TOK,		//'e'
	LETTER_TOK,		//'f'
	LETTER_TOK,		//'g'
	LETTER_TOK,		//'h'
	LETTER_TOK,		//'i'
	LETTER_TOK,		//'j'
	LETTER_TOK,		//'k'
	LETTER_TOK,		//'l'
	LETTER_TOK,		//'m'
	LETTER_TOK,		//'n'
	LETTER_TOK,		//'o'
	LETTER_TOK,		//'p'
	LETTER_TOK,		//'q'
	LETTER_TOK,		//'r'
	LETTER_TOK,		//'s'
	LETTER_TOK,		//'t'
	LETTER_TOK,		//'u'
	LETTER_TOK,		//'v'
	LETTER_TOK,		//'w'
	LETTER_TOK,		//'x'
	LETTER_TOK,		//'y'
	LETTER_TOK,		//'z'
	OPEN_BRAC_TOK,	//'{'
	PIPE_TOK,		//'|'
	CLOSE_BRAC_TOK,	//'}'
	TILDE_TOK,		//'~'
};

extern const char *const TOK_STRINGS[];

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
	Identifier_type,
	InsertedIdentifier_type,
	Vector_type,
} ExprType;

typedef struct VecN {
	Expr **ptr;
	size_t n;
	size_t allocd_size;
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
	uint32_t num_refs;
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

void print_indent(uint32_t indent);
void print_typedval(TypedValue *val);
void println_typedval(TypedValue *val);
void print_exprh(Expr *expr);

void free_expr(Expr **e);

struct call_info {
	size_t line_n;
	const char *filename;
};

VecN new_vec_debug(size_t n, struct call_info call);
void free_vec_debug(VecN *vec, struct call_info call);
#ifndef NDEBUG
#define new_vec(n) (new_vec_debug(n, (struct call_info) {__LINE__, __FILE_NAME__}))
#define free_vec(vec) (free_vec_debug(vec, (struct call_info) {__LINE__, __FILE_NAME__}))
#else
#define new_vec(n) (new_vec_debug(n, (struct call_info) {0, nullptr}))
#define free_vec(n) (free_vec_debug(n, (struct call_info) {0, nullptr}))
#endif
VecN construct_vec(size_t n, ...);
int32_t push_to_vec(VecN *vec, Expr *val);
Expr **peek_top_vec(VecN *vec);
Expr *pop_from_vec(VecN *vec);

double get_number(TypedValue *v);
_Complex double get_complex(TypedValue *v);

#endif /* EXPR_H */
