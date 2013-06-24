/*
 *  sexp_to_c.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 8/22/10.
 *
 */

#include "sexp_to_c.h"

int init_sexp_to_c()
{
	return 0;
}

value create_associated_numbers(value sexp)
{
	return create_associated_numbers_recursive(sexp, value_zero);
}

value create_associated_numbers_recursive(value sexp, value start)
{
	if (sexp.type != VALUE_BLK) {
		return value_inc(start);
	}
	
	value associated = value_init(VALUE_BLK);
	size_t i;
	for (i = 0; i < sexp.core.u_blk.length; ++i) {
		value num = create_associated_numbers_recursive(sexp.core.u_blk.a[i], start);
		value_inc_now(&start);
		value_append_now2(&associated, &num);
	}
	
	return associated;
}

value get_count(value counts)
{
	if (counts.type == VALUE_MPZ) {
		return counts;
	}
	
	return get_count(counts.core.u_blk.a[0]);
}

int sexp_to_c(FILE *stream, value sexp)
{
	value counts = create_associated_numbers(sexp);
	
	int res = sexp_to_c_recursive(stream, sexp, counts);
	value_clear(&counts);
	return res;
}

/* 
 * Convert the given s-expression into C code.
 */
int sexp_to_c_recursive(FILE *stream, value sexp, value counts)
{
	if (sexp.type == VALUE_MPZ || sexp.type == VALUE_MPF) {
		value_fprintf(stream, "var%v = value_set_str_smart(\"%v\", 0);\n", counts, sexp);
	}
	
	if (sexp.type != VALUE_BLK) {
		return 0;
//		value_error(1, "Type Error: sexp_to_c() is undefined where sexp is %ts (block expected).", sexp);
//		return VALUE_ERROR;
	}
	
	int error_p = 0;
	
	size_t i;
	for (i = 0; i < sexp.core.u_blk.length; ++i) {
		if (sexp.core.u_blk.a[i].type == VALUE_BIF && sexp.core.u_blk.a[i].core.u_bif->spec.delay_eval_p)
			;
		else if (error_p = sexp_to_c_recursive(stream, sexp.core.u_blk.a[i], counts.core.u_blk.a[i]))
			return error_p;
	}
	
	char *name;
	
	if (sexp.core.u_blk.length > 0 && sexp.core.u_blk.a[0].type == VALUE_BIF) {
		value (*f)(int argc, value argv[]) = sexp.core.u_blk.a[0].core.u_bif->f;
		int type = 0;
		if (f == &value_add_arg) {
			name = "value_add";
		} else if (f == &value_sub_arg) {
			name = "value_sub";
		} else if (f == &value_mul_arg) {
			name = "value_mul";
		} else if (f == &value_div_arg) {
			name = "value_div";
		} else if (f == &value_eq_arg) {
			name = "value_eq";
		} else if (f == &value_if_arg) {
			type = S2C_IF;
		} else if (f == &value_times_arg) {
			type = S2C_TIMES;
		} else {
			name = "UNDEFINED_FUNCTION";
		}

		if (type == 0) {
			value str = value_set_str(name);
			value_fprintf(stream, "var%v = %v(var%v, var%v);\n", get_count(counts), str, get_count(counts.core.u_blk.a[1]), get_count(counts.core.u_blk.a[2]));
			value_clear(&str);
		} else if (type == S2C_IF) {
			// Do some different stuff.
			
		} else if (type == S2C_TIMES) {
			value iter;
			for (iter = value_set_long(0); value_lt(iter, sexp.core.u_blk.a[1]); value_inc_now(&iter)) {
				sexp_to_c_recursive(stream, sexp.core.u_blk.a[2], counts.core.u_blk.a[2]);
			}
		}
	}
	
	return 0;
}

int run_me()
{

return 0;
}