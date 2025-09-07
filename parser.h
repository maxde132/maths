#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "expr.h"

Token get_next_token(const char **s);
Token peek_token(const char **s);
Expr *parse(const char *s);

#endif /* PARSER_H */
