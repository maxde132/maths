#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "expr.h"

Token next_token(const char **s);
Expr *parse(const char *s);

#endif /* PARSER_H */
