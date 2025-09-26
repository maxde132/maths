#include <math.h>
#include <stdio.h>

#include "mml/eval.h"
#include "mml/expr.h"
#include "mml/config.h"
#include "mml/parser.h"
#include "c-hashmap/map.h"

static MML_Value custom_dbg_type(MML_state *state, MML_ExprVec *args)
{
	printf("%s", EXPR_TYPE_STRINGS[MML_eval_expr(state, dv_a(*args, 0)).type]);
	MML_global_config.last_print_was_newline = false;

	return VAL_INVAL;
}

static MML_Value custom_dbg_ident(MML_state *state, MML_ExprVec *args)
{
	MML_print_exprh(MML_eval_get_variable(state, dv_a(*args, 0)->s));

	return VAL_INVAL;
}

static MML_Value custom_config_set(MML_state *state, MML_ExprVec *args)
{
	if (dv_n(*args) != 2
	 || dv_a(*args, 0)->type != Identifier_type)
	{
		fprintf(stderr, "`config_set` takes two arguments; "
				"an identifier and some other value\n");
		return VAL_INVAL;
	}

	const strbuf config_ident = dv_a(*args, 0)->s;

	if (strncmp(config_ident.s, "precision", sizeof("precision")-1) == 0)
	{
		MML_Value val = MML_eval_expr(state, dv_a(*args, 1));
		if (val.type != RealNumber_type)
		{
			MML_log_err("`config_set`: the `precision` config setting "
					"must be of type RealNumber\n");
			return VAL_INVAL;
		}
		MML_global_config.precision = (uint32_t)floor(val.n);
	} else if (strncmp(config_ident.s, "full_prec_floats", sizeof("full_prec_floats")-1) == 0)
	{
		MML_Value val = MML_eval_expr(state, dv_a(*args, 1));
		if (val.type != Boolean_type)
		{
			MML_log_err("`config_set`: the `full_prec_floats` config setting "
					"must be of type Boolean\n");
			return VAL_INVAL;
		}
		MML_global_config.full_prec_floats = val.b;
	} else
	{
		fprintf(stderr, "`config_set`: unknown config setting `%.*s`\n",
				(int)config_ident.len, config_ident.s);
		return VAL_INVAL;
	}

	return VAL_INVAL;
}

static void register_functions(hashmap *maps[6])
{
	hashmap_set(maps[1], hashmap_str_lit("dbg"),		(uintptr_t)MML_print_exprh_tv_func);
	hashmap_set(maps[1], hashmap_str_lit("dbg_type"),	(uintptr_t)custom_dbg_type);
	hashmap_set(maps[1], hashmap_str_lit("dbg_ident"),	(uintptr_t)custom_dbg_ident);
	hashmap_set(maps[1], hashmap_str_lit("config_set"),	(uintptr_t)custom_config_set);
}

void stdmml__register_functions(hashmap *maps[6])
{
	register_functions(maps);
}
