#ifndef MML_H
#define MML_H

#include <stdbool.h>

#define MML_BARE_USE
#include "mml/eval.h"
#include "mml/parser.h"

MML_Value MML_eval_parse(MML_state *state, const char *s);

#endif /* MML_H */
