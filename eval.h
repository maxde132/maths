#ifndef EVAL_H
#define EVAL_H

#include "expr.h"

void init_evaluator(void);
void cleanup_evaluator(void);
int32_t set_variable(strbuf name, TypedValue *val);
TypedValue eval_expr(const Expr *expr);

#endif /* EVAL_H */
