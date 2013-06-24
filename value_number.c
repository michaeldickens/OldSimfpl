/*
 *  value_number.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 6/2/10.
 *  Copyright 2010 The Khabele School. All rights reserved.
 *
 */

#include "value.h"

value value_add(value op1, value op2)
{
	value res = value_init_error();
	
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_add(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_set_z(res.core.u_mf, op1.core.u_mz, value_mpfr_round);
		mpfr_add(res.core.u_mf, res.core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_MPF && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPF);
		mpfr_set_z(res.core.u_mf, op2.core.u_mz, value_mpfr_round);
		mpfr_add(res.core.u_mf, op1.core.u_mf, res.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_MPF && op2.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_add(res.core.u_mf, op1.core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_STR && op2.type == VALUE_STR) {
		int length = strlen(op1.core.u_s) + strlen(op2.core.u_s);
		char str[length+1];
		strcpy(str, op1.core.u_s);
		strcpy(str+strlen(op1.core.u_s), op2.core.u_s);
		res = value_set_str(str);
	} else if (op1.type == VALUE_STR) {
		op2 = value_cast(op2, VALUE_STR);
		int length = strlen(op1.core.u_s) + strlen(op2.core.u_s);
		char str[length+1];
		strcpy(str, op1.core.u_s);
		strcpy(str+strlen(op1.core.u_s), op2.core.u_s);
		res = value_set_str(str);
		value_clear(&op2);
	} else if (op1.type == VALUE_RGX && op2.type == VALUE_RGX) {
		int length = strlen(op1.core.u_x) + strlen(op2.core.u_x);
		char *str = value_malloc(NULL, length+1);
		return_if_null(str);
		strcpy(str, op1.core.u_s);
		strcpy(str+strlen(op1.core.u_x), op2.core.u_x);
		res.type = VALUE_RGX;
		res.core.u_x = str;
	} else if (op1.type == VALUE_ARY && op2.type == VALUE_ARY
			|| op1.type == VALUE_PAR && op2.type == VALUE_PAR) {
		res = value_concat(op1, op2);
	} else if (op1.type == VALUE_ARY || op1.type == VALUE_PAR) {
		res = value_append(op1, op2);
	} else if (op2.type == VALUE_ARY || op2.type == VALUE_PAR) {
		res = value_cons(op1, op2);
	} else if (op2.type == VALUE_STR) {
		op1 = value_cast(op1, VALUE_STR);
		int length = strlen(op1.core.u_s) + strlen(op2.core.u_s);
		char str[length+1];
		strcpy(str, op1.core.u_s);
		strcpy(str+strlen(op1.core.u_s), op2.core.u_s);
		res = value_set_str(str);
		value_clear(&op1);
	} else {
		value_error(1, "Argument Error: Addition is undefined for %ts and %ts.", op1, op2);
	}
	
	return res;
}

value value_add_now(value *op1, value op2)
{	
	value res = value_init_nil();
	
	if (op1->type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		mpz_add(op1->core.u_mz, op1->core.u_mz, op2.core.u_mz);
	} else if (op1->type == VALUE_MPZ && op2.type == VALUE_MPF) {
		value tmp;
		tmp.type = VALUE_MPF;
		mpfr_init_set_z(tmp.core.u_mf, op1->core.u_mz, value_mpfr_round);
		mpfr_add(tmp.core.u_mf, tmp.core.u_mf, op2.core.u_mf, value_mpfr_round);
		value_clear(op1);
		*op1 = tmp;
	} else if (op1->type == VALUE_MPF && op2.type == VALUE_MPZ) {
		mpfr_add_z(op1->core.u_mf, op1->core.u_mf, op2.core.u_mz, value_mpfr_round);
	} else if (op1->type == VALUE_MPF && op2.type == VALUE_MPF) {
		mpfr_add(op1->core.u_mf, op1->core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else if (op1->type == VALUE_STR && op2.type == VALUE_STR) {
		size_t len1 = strlen(op1->core.u_s), len2 = strlen(op2.core.u_s);
		size_t length = len1 + len2;
		value_realloc(op1, length + 1);
		if (op1->type == VALUE_ERROR)
			return value_init_error();
		strcpy(op1->core.u_s + len1, op2.core.u_s);
	} else if (op1->type == VALUE_STR) {
		op2 = value_cast(op2, VALUE_STR);
		if (op2.type == VALUE_ERROR)
			return op2;
		res = value_add_now(op1, op2);
		value_clear(&op2);
	} else if (op2.type == VALUE_STR) {
		value cast = value_cast(*op1, VALUE_STR);
		if (cast.type == VALUE_ERROR)
			return cast;
		res = value_add_now(&cast, op2);
		value_clear(op1);
		*op1 = cast;
	} else if (op1->type == VALUE_ARY && op2.type == VALUE_ARY
			|| op1->type == VALUE_PAR && op2.type == VALUE_PAR) {
		res = value_concat_now(op1, op2);
	} else if (op1->type == VALUE_ARY || op1->type == VALUE_PAR) {
		res = value_append_now(op1, op2);
	} else {
		value_error(1, "Type Error: In-place addition is undefined for %ts and %ts.", *op1, op2);
		res =  value_init_error();
	}
	
	return res;
}

value value_sub(value op1, value op2)
{
	value res;
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_sub(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_set_z(res.core.u_mf, op1.core.u_mz, value_mpfr_round);
		mpfr_sub(res.core.u_mf, res.core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_MPF && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPF);
		mpfr_set_z(res.core.u_mf, op2.core.u_mz, value_mpfr_round);
		mpfr_sub(res.core.u_mf, op1.core.u_mf, res.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_MPF && op2.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_sub(res.core.u_mf, op1.core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else {
		if (op1.type != VALUE_MPZ && op1.type != VALUE_MPF)
			value_error(1, "Type Error: Subtraction is undefined where op1 is %ts (number expected).", op1);
		if (op2.type != VALUE_MPZ && op2.type != VALUE_MPF)
			value_error(1, "Type Error: Subtraction is undefined where op2 is %ts (number expected).", op2);
	}
	
	return res;
}

value value_sub_now(value *op1, value op2)
{
	value res = value_sub(*op1, op2);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op1);
	*op1 = res;
	return value_init_nil();
}

value value_mul(value op1, value op2)
{
	value res;
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_mul(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_set_z(res.core.u_mf, op1.core.u_mz, value_mpfr_round);
		mpfr_mul(res.core.u_mf, res.core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_MPF && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPF);
		mpfr_set_z(res.core.u_mf, op2.core.u_mz, value_mpfr_round);
		mpfr_mul(res.core.u_mf, op1.core.u_mf, res.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_MPF && op2.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_mul(res.core.u_mf, op1.core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_STR && op2.type == VALUE_MPZ) {
		value zero = value_set_long(0), max = value_set_ulong(LONG_MAX);
		if (value_lt(op2, zero)) {
			value_error(1, "Domain Error: Cannot multiply an array by a negative number.");
			return value_init_error(); 
		}
		if (value_gt(op2, max)) {
			value_error(1, "Domain Error: Cannot multiply an array by a number larger than LONG_MAX.");
			return value_init_error();
		}
		value_clear(&zero); value_clear(&max);
		
		unsigned long count = mpz_get_ui(op2.core.u_mz);
		int len = strlen(op1.core.u_s);
		char str[len * count + 1]; *str = '\0';
		char *ptr = str;
		int i; for (i = 0; i < count; ++i, ptr += len)
			strcpy(ptr, op1.core.u_s);
		res = value_set_str(str);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_STR) {
		if (value_lt(op1, value_zero)) {
			value_error(1, "Domain Error: Cannot multiply an array by a negative number.");
			return value_init_error(); 
		}
		if (value_gt(op1, value_int_max)) {
			value_error(1, "Domain Error: Cannot multiply an array by a number larger than LONG_MAX.");
			return value_init_error();
		}

		unsigned long count = mpz_get_ui(op1.core.u_mz);
		int len = strlen(op2.core.u_s);
		char str[len * count + 1]; *str = '\0';
		char *ptr = str;
		int i; for (i = 0; i < count; ++i, ptr += len)
			strcpy(ptr, op2.core.u_s);
		res = value_set_str(str);	
	} else if (op1.type == VALUE_ARY && op2.type == VALUE_MPZ) {
		if (value_lt(op2, value_zero)) {
			value_error(1, "Domain Error: Cannot multiply an array by a negative number.");
			return value_init_error(); 
		}
		if (value_gt(op2, value_int_max)) {
			value_error(1, "Domain Error: Cannot multiply an array by a number larger than %s.", value_int_max);
			return value_init_error();
		}
		size_t count = value_get_ulong(op2);
		size_t length = value_length(op1);
		
		value array[length * count];
				
		size_t i, j;
		for (i = 0; i < count; ++i)
			for (j = 0; j < length; ++j)
				array[i*length+j] = value_set(op1.core.u_a.a[j]);
		
		res = value_set_ary(array, length * count);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_ARY) {
		if (value_lt(op2, value_zero)) {
			value_error(1, "Domain Error: Cannot multiply an array by a negative number.");
			return value_init_error(); 
		}
		if (value_gt(op2, value_int_max)) {
			value_error(1, "Domain Error: Cannot multiply an array by a number larger than %s.", value_int_max);
			return value_init_error();
		}
		size_t count = value_get_ulong(op1);
		size_t length = value_length(op2);
		
		value array[length * count];
				
		size_t i, j;
		for (i = 0; i < count; ++i)
			for (j = 0; j < length; ++j)
				array[i*length+j] = value_set(op2.core.u_a.a[j]);
		
		res = value_set_ary(array, length * count);
	} else if (op1.type == VALUE_PAR && op2.type == VALUE_MPZ) {
		if (value_lt(op2, value_zero)) {
			value_error(1, "Domain Error: Cannot multiply a string by negative number.");
			return value_init_error(); 
		}
		if (value_gt(op2, value_int_max)) {
			value_error(1, "Domain Error: Cannot multiply a string by a number larger than %s.", value_int_max);
			return value_init_error();
		}
		size_t count = value_get_ulong(op2);
		
		res = value_set(op1);
		size_t i;
		for (i = 1; i < count; ++i)
			value_concat_now(&res, op1);
				
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_PAR) {
		if (value_lt(op2, value_zero)) {
			value_error(1, "Domain Error: Cannot multiply a string by a negative number.");
			return value_init_error(); 
		}
		if (value_gt(op2, value_int_max)) {
			value_error(1, "Domain Error: Cannot multiply a string by a number larger than %s.", value_int_max);
			return value_init_error();
		}
		size_t count = value_get_ulong(op2);
		
		res = value_set(op2);
		size_t i;
		for (i = 1; i < count; ++i)
			value_concat_now(&res, op2);
	
	} else {
		value_error(1, "Type Error: Multiplication is undefined for %ts and %ts.", op1, op2);
		res = value_init_error();
	}
	
	return res;
}

value value_mul_now(value *op1, value op2)
{
	value res = value_mul(*op1, op2);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op1);
	*op1 = res;
	return value_init_nil();
}

value value_div(value op1, value op2)
{
	if ((op1.type == VALUE_MPZ || op1.type == VALUE_MPF) && 
			value_eq(op2, value_zero)) {
		return value_init(VALUE_NAN);
	}

	value res;
		
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_div(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_set_z(res.core.u_mf, op1.core.u_mz, value_mpfr_round);
		mpfr_div(res.core.u_mf, res.core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_MPF && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPF);
		mpfr_set_z(res.core.u_mf, op2.core.u_mz, value_mpfr_round);
		mpfr_div(res.core.u_mf, op1.core.u_mf, res.core.u_mf, value_mpfr_round);
	} else if (op1.type == VALUE_MPF && op2.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_div(res.core.u_mf, op1.core.u_mf, op2.core.u_mf, value_mpfr_round);
	} else {
		if (op1.type != VALUE_MPZ && op1.type != VALUE_MPF)
			value_error(1, "Type Error: Division is undefined where op1 is %ts (number expected).", op1);
		if (op2.type != VALUE_MPZ && op2.type != VALUE_MPF)
			value_error(1, "Type Error: Division is undefined where op2 is %ts (number expected).", op2);
	}
	
	return res;
}

value value_div_now(value *op1, value op2)
{
	value res = value_div(*op1, op2);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op1);
	*op1 = res;
	return value_init_nil();
}

value value_mod(value op1, value op2)
{
	value res = value_init_error();
	value zero = value_set_long(0);
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		if (value_eq(op2, zero)) {
			value_error(1, "Arithmetic error: Modulo by zero.");
			res.type = VALUE_NAN;
		} else {
			res = value_init(VALUE_MPZ);
			mpz_tdiv_r(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
		}
	} else {
		if (op1.type != VALUE_MPZ)
			value_error(1, "Type Error: Modulo is undefined where op1 is %ts (integer expected).", op1);
		if (op2.type != VALUE_MPZ)
			value_error(1, "Type Error: Modulo is undefined where op2 is %ts (integer expected).", op2);
	}
	
	value_clear(&zero);
	return res;
}

value value_mod_now(value *op1, value op2)
{
	value res = value_mod(*op1, op2);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op1);
	*op1 = res;
	return value_init_nil();
}

value value_add_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "addition") ? value_init_error() : value_add(argv[0], argv[1]);
}

value value_sub_arg(int argc, value argv[])
{
	if (argv[0].type == VALUE_MISSING_ARG)
		argv[0] = value_set_long(0);
	else if (argv[1].type == VALUE_MISSING_ARG) {
		argv[1] = argv[0];
		argv[0] = value_set_long(0);
	}
	if (missing_arguments(argc, argv, "subtraction"))
		return value_init_error();
	return value_sub(argv[0], argv[1]);
}

value value_mul_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "multiplication") ? value_init_error() : value_mul(argv[0], argv[1]);
}

value value_div_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "division") ? value_init_error() : value_div(argv[0], argv[1]);
}

value value_mod_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "modulo") ? value_init_error() : value_mod(argv[0], argv[1]);
}

value value_inc(value op)
{
	value res;
	if (op.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_add_ui(res.core.u_mz, op.core.u_mz, 1);
	} else if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_add_ui(res.core.u_mf, op.core.u_mf, 1, value_mpfr_round);
	} else {
		value_error(1, "Type Error: Increment is undefined where op is %ts (number expected).", op);
		res = value_init_error();
	}
	return res;
}

value value_inc_now(value *op)
{
	if (op->type == VALUE_MPZ)
		mpz_add_ui(op->core.u_mz, op->core.u_mz, 1);
	else if (op->type == VALUE_MPF)
		mpfr_add_ui(op->core.u_mf, op->core.u_mf, 1, value_mpfr_round);
	else {
		value_error(1, "Type Error: Increment is undefined where op is %ts (number expected).", *op);
		return value_init_error();
	}
	
	return value_init_nil();
}

value value_dec(value op)
{
	value res;
	if (op.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_sub_ui(res.core.u_mz, op.core.u_mz, 1);
	} else if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_sub_ui(res.core.u_mf, op.core.u_mf, 1, value_mpfr_round);
	} else {
		value_error(1, "Type Error: Decrement is undefined where op is %ts (number expected).", op);
		res = value_init_error();
	}
	return res;	
}

value value_dec_now(value *op)
{
	if (op->type == VALUE_MPZ)
		mpz_sub_ui(op->core.u_mz, op->core.u_mz, 1);
	else if (op->type == VALUE_MPF)
		mpfr_sub_ui(op->core.u_mf, op->core.u_mf, 1, value_mpfr_round);
	else {
		value_error(1, "Type Error: Decrement is undefined where op is %ts (number expected).", *op);
		return value_init_error();
	}
	
	return value_init_nil();
}

value value_inc_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "increment") ? value_init_error() : value_inc(argv[0]);
}

value value_dec_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "decrement") ? value_init_error() : value_dec(argv[0]);
}

value value_uminus(value op)
{
	if (op.type == VALUE_MPZ) {
		value res = value_init(VALUE_MPZ);
		mpz_neg(res.core.u_mz, op.core.u_mz);
		return res;
	} else if (op.type == VALUE_MPF) {
		value res = value_init(VALUE_MPF);
		mpfr_neg(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: Unary minus is undefined where op is %ts (number expected).", op);
		return value_init_error();
	}

}

value value_uplus(value op)
{
	if (op.type == VALUE_MPZ || op.type == VALUE_MPF) {
		value res = value_set(op);
		return res;
	} else {
		value_error(1, "Type Error: Unary plus is undefined where op is %ts (number expected).", op);
		return value_init_error();
	}
}

value value_abs(value op)
{
	if (op.type == VALUE_MPZ) {
		value res = value_init(VALUE_MPZ);
		mpz_abs(res.core.u_mz, op.core.u_mz);
		return res;
	} else if (op.type == VALUE_MPF) {
		value res = value_init(VALUE_MPF);
		mpfr_abs(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: abs is undefined where op is %ts (number expected).", op);
		return value_init_error();
	}
}

value value_uminus_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "unary minus") ? value_init_error() : value_uminus(argv[0]);
}

value value_uplus_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "unary plus") ? value_init_error() : value_uplus(argv[0]);
}

value value_abs_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "abs()") ? value_init_error() : value_abs(argv[0]);
}

int compare_to_zero(int x)
{
	if (x < 0) return -1;
	if (x == 0) return 0;
	return 1;
}

/* 
 * Return Codes
 * 
 * -2: error
 * -1: op1 < op2
 *  0: op1 == op2
 *  1: op1 > op2
 * 
 * Other comparison functions may return values that are greater or less than 1 or 
 * -1. value_cmp() is guaranteed to return -1, 0, or 1. In order to convert values 
 * to -1, 0, or 1, the expression (v > 0) - (v < 0) is used. From 
 * http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign
 */
int value_cmp(value op1, value op2)
{
	int v;
	if (op1.type == VALUE_MPZ) {
		if (op2.type == VALUE_MPZ) {
			return ((v = mpz_cmp(op1.core.u_mz, op2.core.u_mz)) > 0) - (v < 0);
			
		} else if (op2.type == VALUE_MPF) {
			return ((v = mpfr_cmp_z(op2.core.u_mf, op1.core.u_mz)) < 0) - (v > 0);
		}
	}
	
	if (op1.type == VALUE_MPF) {
		if (op2.type == VALUE_MPZ) {
			return ((v = mpfr_cmp_z(op1.core.u_mf, op2.core.u_mz)) > 0) - (v < 0);
		} else if (op2.type == VALUE_MPF) {
			return ((mpfr_cmp(op1.core.u_mf, op2.core.u_mf)) > 0) - (v < 0);
		}
	}
	
	if (op1.type != op2.type)
		return -2;
	
	size_t i, length;
	
	switch (op1.type) {
	case VALUE_NIL:
	case VALUE_INF:
	case VALUE_NAN:
		return 0;
		
	case VALUE_BOO:
		// Since 0 is false and 1 is true, this will return -1, 0, or 1.
		return op2.core.u_b - op1.core.u_b;

	case VALUE_STR:
	case VALUE_ID:
	case VALUE_VAR:
	case VALUE_RGX:
	case VALUE_SYM:
		return ((v = strcmp(op1.core.u_s, op2.core.u_s)) > 0) - (v < 0);
	
	case VALUE_ARY:
		length = op1.core.u_a.length < op2.core.u_a.length ? op1.core.u_a.length : op2.core.u_a.length;
		int cmp;
		for (i = 0; i < length; ++i)
			if (cmp = value_cmp_any(op1.core.u_a.a[i], op2.core.u_a.a[i]))
				return cmp;
		
		if (op1.core.u_a.length < op2.core.u_a.length)
			return -1;
		if (op1.core.u_a.length == op2.core.u_a.length)
			return 0;
		return 1;
	
	case VALUE_LST:
		while (op1.type != VALUE_NIL) {
			if (op2.type == VALUE_NIL)
				return 1;
			if ((v = value_cmp_any(op1, op2)) != 0)
				return v;
			op1 = op1.core.u_l[1];
			op2 = op2.core.u_l[1];
		}
		
		if (op2.type == VALUE_NIL)
			return 0;
		return -1;
		
	case VALUE_PTR:
		while (op1.type == VALUE_PAR) {
			if (op2.type != VALUE_PAR)
				return 1;
			if ((v = value_cmp_any(op1.core.u_p->head, op2.core.u_p->head)) != 0)
				return v;
			op1 = op1.core.u_p->tail;
			op2 = op2.core.u_p->tail;
		}
		
		if (op2.type != VALUE_PAR)
			return value_cmp_any(op1, op2);
		return -1;
		
		
		
	default:
		value_error(1, "Type Error: Comparison is undefined for %ts and %ts.", op1, op2);
		return -2;
	}
	
	return -2;
}

int value_cmp_any(value op1, value op2)
{
	int v;
	if (op1.type == VALUE_MPZ) {
		if (op2.type == VALUE_MPZ) {
			return ((v = mpz_cmp(op1.core.u_mz, op2.core.u_mz)) > 0) - (v < 0);
			
		} else if (op2.type == VALUE_MPF) {
			return ((v = mpfr_cmp_z(op2.core.u_mf, op1.core.u_mz)) < 0) - (v > 0);
		}
	}
	
	if (op1.type == VALUE_MPF) {
		if (op2.type == VALUE_MPZ) {
			return ((v = mpfr_cmp_z(op1.core.u_mf, op2.core.u_mz)) > 0) - (v < 0);
		} else if (op2.type == VALUE_MPF) {
			return ((mpfr_cmp(op1.core.u_mf, op2.core.u_mf)) > 0) - (v < 0);
		}
	}
	
	if (op1.type != op2.type)
		return op1.type < op2.type ? -1 : op1.type == op2.type ? 0 : 1;
		
	return value_cmp(op1, op2);
}

value value_cmp_std(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return value_init_error();
	return value_set_long(res);	
}

int value_lt(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return FALSE;
	return res < 0;
}

value value_lt_std(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return value_init_error();
	return value_set_bool(res < 0);
}

int value_le(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return FALSE;
	return res <= 0;
}

value value_le_std(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return value_init_error();
	return value_set_bool(res <= 0);
}

int value_eq(value op1, value op2)
{
	// If the types are not equal, op1 and op2 cannot be equal unless they are MPZ or MPF.
	if (op1.type != op2.type && !(op1.type == VALUE_MPZ && op2.type == VALUE_MPF || op1.type == VALUE_MPF && op2.type == VALUE_MPZ))
		return FALSE;
	
	size_t i, length1, length2;

	switch (op1.type) {
	case VALUE_NIL:
	case VALUE_NAN:
	case VALUE_INF: 
	case VALUE_ERROR:
		return TRUE;
	case VALUE_MPZ:
		if (op2.type == VALUE_MPZ)
			return mpz_cmp(op1.core.u_mz, op2.core.u_mz) == 0;
		if (op2.type == VALUE_MPF) {
			mpfr_t opf;
			mpfr_init_set_z(opf, op1.core.u_mz, value_mpfr_round);
			int res = mpfr_equal_p(opf, op2.core.u_mf);
			mpfr_clear(opf);
			return res;
		}
	case VALUE_MPF:
		if (op2.type == VALUE_MPZ) {
			mpfr_t opf;
			mpfr_init_set_z(opf, op2.core.u_mz, value_mpfr_round);
			int res = mpfr_equal_p(op1.core.u_mf, opf);
			mpfr_clear(opf);
			return res;
		}
		if (op2.type == VALUE_MPF)
			return mpfr_equal_p(op1.core.u_mf, op2.core.u_mf);
	case VALUE_BOO:
		return (op1.core.u_b && op2.core.u_b) || (!op1.core.u_b && !op2.core.u_b);
	case VALUE_STR:
	case VALUE_RGX:
	case VALUE_SYM:
	case VALUE_ID:
	case VALUE_VAR:
		return strcmp(op1.core.u_s, op2.core.u_s) == 0;
	case VALUE_ARY:
		if (op1.core.u_a.length != op2.core.u_a.length)
			return FALSE;
		
		for (i = 0; i < op1.core.u_a.length; ++i)
			if (value_ne(op1.core.u_a.a[i], op2.core.u_a.a[i]))
				return FALSE;
		return TRUE;
	
	case VALUE_LST:
		while (op1.type != VALUE_NIL) {
			if (op2.type == VALUE_NIL)
				return FALSE;
			if (value_ne(op1.core.u_l[0], op2.core.u_l[0]))
				return FALSE;
			op1 = op1.core.u_l[1];
			op2 = op2.core.u_l[1];
		}
		return TRUE;

	case VALUE_PAR:
		while (op1.type == VALUE_PAR) {
			if (op2.type != VALUE_PAR)
				return FALSE;
			if (value_ne(op1.core.u_p->head, op2.core.u_p->head))
				return FALSE;
			op1 = op1.core.u_p->tail;
			op2 = op2.core.u_p->tail;
		}
		return value_eq(op1, op2);

	case VALUE_HSH:
		length1 = value_hash_length(op1);
		length2 = value_hash_length(op2);
		if (length1 != length2)
			return FALSE;
		for (i = 0; i < length1; ++i)
			if (value_ne(op1.core.u_h.a[i], op2.core.u_h.a[i]))
				return FALSE;
		return TRUE;
	
	case VALUE_BLK:
		if (op1.core.u_blk.length != op2.core.u_blk.length)
			return FALSE;
		
		for (i = 0; i < op1.core.u_blk.length; ++i)
			if (value_ne(op1.core.u_blk.a[i], op2.core.u_blk.a[i]))
				return FALSE;
		return TRUE;
		
	case VALUE_BIF:
		return op1.core.u_bif->f == op2.core.u_bif->f;
	
	case VALUE_TYP:
		return op1.core.u_type == op2.core.u_type;

	}

	return FALSE;
}

int value_eq_with_if_statements(value op1, value op2)
{
	// If the types are not equal, op1 and op2 cannot be equal unless they are MPZ or MPF.
	if (op1.type != op2.type && !(op1.type == VALUE_MPZ && op2.type == VALUE_MPF || op1.type == VALUE_MPF && op2.type == VALUE_MPZ))
		return FALSE;
	
	if (op1.type == VALUE_NIL)
		return TRUE;
	if (op1.type == VALUE_NAN)
		return TRUE;
	if (op1.type == VALUE_INF)
		return TRUE;
	if (op1.type == VALUE_ERROR)
		return TRUE;
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ)
		return mpz_cmp(op1.core.u_mz, op2.core.u_mz) == 0;
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPF) {
		mpfr_t opf;
		mpfr_init_set_z(opf, op1.core.u_mz, value_mpfr_round);
		int res = mpfr_cmp(opf, op2.core.u_mf) == 0;
		mpfr_clear(opf);
		return res;
	}
	if (op1.type == VALUE_MPF && op2.type == VALUE_MPZ) {
		mpfr_t opf;
		mpfr_init_set_z(opf, op2.core.u_mz, value_mpfr_round);
		int res = mpfr_cmp(op1.core.u_mf, opf) == 0;
		mpfr_clear(opf);
		return res;
	}
	if (op1.type == VALUE_MPF && op2.type == VALUE_MPF)
		return mpfr_cmp(op1.core.u_mf, op2.core.u_mf) == 0;
	if (op1.type == VALUE_BOO)
		return (op1.core.u_b && op2.core.u_b) || (!op1.core.u_b && !op2.core.u_b);
	if (op1.type == VALUE_STR || op1.type == VALUE_RGX || op1.type == VALUE_SYM)
		return strcmp(op1.core.u_s, op2.core.u_s) == 0;
	if (op1.type == VALUE_ARY) {
		if (op1.core.u_a.length != op2.core.u_a.length)
			return FALSE;
		
		size_t i;
		for (i = 0; i < op1.core.u_a.length; ++i)
			if (value_ne(op1.core.u_a.a[i], op2.core.u_a.a[i]))
				return FALSE;
		return TRUE;
	}
	
	if (op1.type == VALUE_LST) {
		while (op1.type != VALUE_NIL) {
			if (op2.type == VALUE_NIL)
				return FALSE;
			if (value_ne(op1.core.u_l[0], op2.core.u_l[0]))
				return FALSE;
			op1 = op1.core.u_l[1];
			op2 = op2.core.u_l[1];
		}
		return TRUE;
	}

	if (op1.type == VALUE_HSH) {
		size_t i, length1 = value_hash_length(op1), length2 = value_hash_length(op2);
		if (length1 != length2)
			return FALSE;
		for (i = 0; i < length1; ++i)
			if (value_ne(op1.core.u_h.a[i], op2.core.u_h.a[i]))
				return FALSE;
		return TRUE;
	}
	
	if (op1.type == VALUE_BLK) {
		if (op1.core.u_blk.length != op2.core.u_blk.length)
			return FALSE;
		
		size_t i;
		for (i = 0; i < op1.core.u_blk.length; ++i)
			if (value_ne(op1.core.u_blk.a[i], op2.core.u_blk.a[i]))
				return FALSE;
		return TRUE;
	}
		
	if (op1.type == VALUE_ID)
		return strcmp(op1.core.u_id, op2.core.u_id) == 0;
	if (op1.type == VALUE_VAR)
		return strcmp(op1.core.u_var, op2.core.u_var) == 0;
	
	if (op1.type == VALUE_BIF)
		return op1.core.u_bif->f == op2.core.u_bif->f;
	
	if (op1.type == VALUE_TYP)
		return op1.core.u_type == op2.core.u_type;

	return FALSE;
}

value value_eq_std(value op1, value op2)
{
	return value_set_bool(value_eq(op1, op2));
}

int value_ne(value op1, value op2)
{
	return !value_eq(op1, op2);
}

value value_ne_std(value op1, value op2)
{
	return value_set_bool(value_ne(op1, op2));
}

int value_ge(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return FALSE;
	return res >= 0;
}

value value_ge_std(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return value_init_error();
	return value_set_bool(res >= 0);
}


int value_gt(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return FALSE;
	return res > 0;
}

value value_gt_std(value op1, value op2)
{
	int res = value_cmp(op1, op2);
	if (res == -2)
		return value_init_error();
	return value_set_bool(res > 0);
}

/* 
 * Not yet implemented. This is currently useless because nothing is pass by 
 * reference.
 */
int value_equal_p(value op1, value op2)
{
	return VALUE_ERROR;
}

value value_equal_p_std(value op1, value op2)
{
	int res = value_equal_p(op1, op2);
	if (res == VALUE_ERROR)
		return value_init_error();
	return value_set_bool(res);
	
}

value value_cmp_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "comparison") ? value_init_error() : value_cmp_std(argv[0], argv[1]);
}

value value_lt_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "less than comparison") ? value_init_error() : value_lt_std(argv[0], argv[1]);
}

value value_le_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "less than or equal to comparison") ? value_init_error() : value_le_std(argv[0], argv[1]);
}

value value_eq_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "equals comparison") ? value_init_error() : value_eq_std(argv[0], argv[1]);
}

value value_ne_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "not equal comparison") ? value_init_error() : value_ne_std(argv[0], argv[1]);
}

value value_ge_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "greater than or equal to comparison") ? value_init_error() : value_ge_std(argv[0], argv[1]);
}

value value_gt_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "greater than comparison") ? value_init_error() : value_gt_std(argv[0], argv[1]);
}


int value_not_p(value op)
{
	return !value_true_p(op);
}

value value_not_p_std(value op)
{
	return value_set_bool(value_not_p(op));
}

int value_and_p(value op1, value op2)
{
	return value_true_p(op1) && value_true_p(op2);
}

value value_and_p_std(value op1, value op2)
{
	if (value_true_p(op1))
		return value_set(op2);
	return value_set(op1);
}

int value_or_p(value op1, value op2)
{
	return value_true_p(op1) || value_true_p(op2);
}

value value_or_p_std(value op1, value op2)
{
	if (value_true_p(op1))
		return value_set(op1);
	return value_set(op2);
}

value value_not_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "logical NOT") ? value_init_error() : value_not_p_std(argv[0]);
}

value value_and_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "logical AND") ? value_init_error() : value_and_p_std(argv[0], argv[1]);
}

value value_or_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "logical OR") ? value_init_error() : value_or_p_std(argv[0], argv[1]);
}


value value_2exp(value op1, long op2)
{
	if (op2 < 0)
		return value_shr(op1, (unsigned long) (-op2));
	else if (op2 == 0)
		return op1;
	return value_shl(op1, (unsigned long) op2);
}

value value_shl(value op1, unsigned long op2)
{
	value res = value_set(op1);
	if (res.type == VALUE_MPZ)
		mpz_mul_2exp(res.core.u_mz, op1.core.u_mz, op2);
	else if (res.type == VALUE_MPF)
		mpfr_mul_2exp(res.core.u_mf, op1.core.u_mf, op2, value_mpfr_round);
	else
		value_error(1, "Type Error: Bit shift left is undefined where op1 is %ts (number expected).", op1);
	return res;
}

value value_shr(value op1, unsigned long op2)
{
	value res = value_set(op1);
	if (res.type == VALUE_MPZ)
		mpz_div_2exp(res.core.u_mz, op1.core.u_mz, op2);
	else if (res.type == VALUE_MPF)
		mpfr_div_2exp(res.core.u_mf, op1.core.u_mf, op2, value_mpfr_round);
	else
		value_error(1, "Type Error: Bit shift right is undefined where op1 is %ts (number expected).", op1);
	return res;
}

value value_shl_std(value op1, value op2)
{
	if (value_lt(op2, value_int_max) && value_ge(op2, value_zero))
		return value_shl(op1, value_get_long(op2));
	else {
		value_error(1, "Type Error: left shift is undefined where op2 is greater than LONG_MAX.");
		return value_init_error();
	}
}

value value_shr_std(value op1, value op2)
{
	if (value_lt(op2, value_int_max) && value_ge(op2, value_zero))
		return value_shr(op1, value_get_long(op2));
	else {
		value_error(1, "Type Error: right shift is undefined where op2 is greater than LONG_MAX.");
		return value_init_error();
	}	
}

value value_and(value op1, value op2)
{
	value res;
	if (op1.type == VALUE_BOO && op2.type == VALUE_BOO) {
		res = value_set_bool(op1.core.u_b && op2.core.u_b);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_and(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
	} else if (op1.type == VALUE_ARY && op2.type == VALUE_ARY) {
		op1 = value_uniq(op1);
		res = value_init(VALUE_ARY);
		size_t i, length = value_length(op1);
		for (i = 0; i < length; ++i)
			if (value_contains_p(op2, op1.core.u_a.a[i]))
				if (value_append_now(&res, op1.core.u_a.a[i]).type == VALUE_ERROR)
					return value_init_error();
	} else {
		value_error(1, "Type Error: Bitwise AND is undefined for %ts and %ts.", op1, op2);
		res = value_init_error();
	}
	
	return res;
}

value value_or(value op1, value op2)
{
	value res;
	if (op1.type == VALUE_BOO && op2.type == VALUE_BOO) {
		res = value_set_bool(op1.core.u_b || op2.core.u_b);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_ior(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
	} else if (op1.type == VALUE_ARY && op2.type == VALUE_ARY) {
		res = value_concat(op1, op2);
		if (value_uniq_now(&res).type == VALUE_ERROR)
			return value_init_error();
	} else {
		value_error(1, "Type Error: Bitwise OR is undefined for %ts and %ts.", op1, op2);
		res = value_init_error();
	}
	
	return res;
}

value value_xor(value op1, value op2)
{
	value res;
	if (op1.type == VALUE_BOO && op2.type == VALUE_BOO) {
		res = value_set_bool(op1.core.u_b & op2.core.u_b);
	} else if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_xor(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
	} else if (op1.type == VALUE_ARY && op2.type == VALUE_ARY) {
		res = value_init(VALUE_ARY);
		value hash = value_cast(op2, VALUE_HSH);
		size_t i, length = value_length(op1);
		for (i = 0; i < length; ++i)
			if (!value_hash_exists(hash, op1.core.u_a.a[i])) {
				if (value_append_now(&res, op1.core.u_a.a[i]).type == VALUE_ERROR)
					return value_init_error();
			}
		
		value_clear(&hash);
		hash = value_cast(op1, VALUE_HSH);
		length = value_length(op2);
		for (i = 0; i < length; ++i)
			if (!value_hash_exists(hash, op2.core.u_a.a[i])) {
				if (value_append_now(&res, op2.core.u_a.a[i]).type == VALUE_ERROR)
					return value_init_error();
			}
		
		value_clear(&hash);
		return res;
		
	} else {
		value_error(1, "Type Error: Bitwise XOR is undefined for %ts and %ts.", op1, op2);
		res = value_init_error();
	}
	
	return res;
}

value value_not(value op)
{
	value res = value_init_error();
	if (op.type == VALUE_MPZ) {
		res = value_init(VALUE_MPZ);
		mpz_com(res.core.u_mz, op.core.u_mz);
	} else
		value_error(1, "Type Error: Bitwise NOT is undefined where op1 is %ts (integer expected).", op);
	
	return res;
}

value value_2exp_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "2exp()"))
		return value_init_error();
	else if (value_lt(argv[1], value_int_max) && value_ge(argv[1], value_zero))
		return value_2exp(argv[0], value_get_long(argv[1]));
	else {
		value_error(1, "Type Error: 2exp() is undefined where op2 is greater than ULONG_MAX.");
		return value_init_error();
	}
}

value value_shl_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "left shift") ? value_init_error() : value_shl_std(argv[0], argv[1]);
}

value value_shr_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "right shift") ? value_init_error() : value_shr_std(argv[0], argv[1]);
}

value value_and_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "bitwise AND") ? value_init_error() : value_and(argv[0], argv[1]);
}

value value_or_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "bitwise OR") ? value_init_error() : value_or(argv[0], argv[1]);
}

value value_xor_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "bitwise XOR") ? value_init_error() : value_xor(argv[0], argv[1]);
}

value value_not_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "bitwise NOT") ? value_init_error() : value_not(argv[0]);
}




value value_srand(value seed)
{
	if (seed.type == VALUE_MPZ) {
		value max = value_set_ulong(ULONG_MAX);
		if (value_le(seed, max)) {
			init_genrand(value_get_ulong(seed));
		}
		
		value_clear(&max);
	}
	
	value_error(1, "Implementation error: srand() has not yet been implemented.");
	return value_init_error();
}

value value_rand(value max)
{
	value res, tmp, 
			adder = value_init_nil(), 
			max32 = value_set_ulong(0xffffffff);
	size_t shift = 0;
	
	if (value_eq(max, value_zero)) {
		res = value_set_double(genrand_real2());
		
	} else if (max.type == VALUE_MPZ) {
		
		res = value_set_ulong(0);
		max = value_set(max);
		
		while (value_gt(max, max32)) {
			value_clear(&adder);
			adder = value_set_ulong(genrand_int32());
			tmp = adder;
			adder = value_shl(adder, shift);
			value_clear(&tmp);
			
			value_add_now(&res, adder);
			tmp = max;
			max = value_shr(max, 32);
			value_clear(&tmp);
			shift += 32;
		}
		
		value_clear(&adder);
		tmp = value_rand(value_zero);
		adder = value_mul(tmp, max);
		value_clear(&tmp);
		tmp = value_cast(adder, VALUE_MPZ);
		value_clear(&adder);
		adder = value_shl(tmp, shift);
		value_clear(&tmp);
		
		value_add_now(&res, adder);
		value_clear(&max);
				
	} else if (max.type == VALUE_MPF) {
		
		// This is a bit slower than I'd like it to be. It has to not only 
		// calculate a random MPZ, but has to calculate a logarithm and 
		// perform several initializations, sets, and clears. This won't 
		// matter if (rand) isn't being called much, but if the user relies 
		// heavily on (rand), this will be important.
		
		// (prec) holds the precision, to ensure that the precision of the 
		// result is at least as great as (max) requires.
		mpfr_prec_t prec;
		value log = value_log2(max);
		prec = mpfr_get_d(log.core.u_mf, value_mpfr_round);
		value_clear(&log);
		
		value zmax = value_cast(max, VALUE_MPZ);
		value zres = value_rand(zmax);
		value randf = value_set_double(genrand_real2());
		prec += mpfr_get_prec(randf.core.u_mf);
		
		res.type = VALUE_MPF;
		mpfr_init2(res.core.u_mf, prec + 1);
		mpfr_add_z(res.core.u_mf, randf.core.u_mf, zres.core.u_mz, value_mpfr_round);
		
		value_clear(&zmax);
		value_clear(&zres);
		value_clear(&randf);
				
	} else {
		value_error(1, "Type Error: rand() is undefined where op is %ts (number expected).", max);
		res = value_init_error();
	}
	
	value_clear(&adder);
	value_clear(&max32);
	return res;
}

value value_srand_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "srand()") ? value_init_error() : value_srand(argv[0]);
}

value value_rand_arg(int argc, value argv[])
{
	if (argv[0].type == VALUE_NIL)
		argv[0] = value_set_long(0);
	
	return missing_arguments(argc, argv, "rand()") ? value_init_error() : value_rand(argv[0]);
}


value value_times(value *variables, value op, value func)
{	
	value res = value_init_nil();
	if (op.type == VALUE_MPZ) {
		value iter;
		for (iter = value_set_long(0); value_lt(iter, op); value_inc_now(&iter)) {
			value tmp = value_call(variables, func, 1, &iter);
			if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				res = tmp;
				break;
			} else if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_BREAK) {
				break;
			} else if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_YIELD) {
				if (res.type == VALUE_NIL) res = value_init(VALUE_ARY);
				value_append_now(&res, *tmp.core.u_stop.core);
			}			
			value_clear(&tmp);
		}
		
		value_clear(&iter);
		
	} else {
		value_error(1, "Type Error: times() is undefined where op is %ts (integer expected).", op);
		res = value_init_error();
	}
	
	return res;
}

value value_summation(value *variables, value op, value func)
{
	value res = value_init_nil();
	
	if (op.type == VALUE_ARY) {
		size_t i;
		value tmp = value_init_nil();
		for (i = 0; i < op.core.u_a.length; ++i) {
			tmp = value_call(variables, func, 1, &op.core.u_a.a[i]);

			if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_BREAK) {
				value_clear(&tmp);
				break;
			} else if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				res = tmp;
				break;
			}
			
			if (i == 0)
				res = tmp;
			else {
				value_add_now(&res, tmp);
				value_clear(&tmp);
			}
		}
	

	} else if (op.type == VALUE_PAR) {
		value optr = op;
		
		int first_time_p = TRUE;
		value tmp = value_init_nil();
		while (optr.type == VALUE_PAR) {
			tmp = value_call(variables, func, 1, optr.core.u_l + 0);
			if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_BREAK) {
				value_clear(&tmp);
				break;
			} else if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				res = tmp;
				break;
			}
			
			if (first_time_p) {
				res = tmp;
				first_time_p = FALSE;
			} else {
				value_add_now(&res, tmp);
				value_clear(&tmp);
			}
			
			optr = optr.core.u_p->tail;
		}
			
	} else if (op.type == VALUE_RNG) {
		if (value_eq(op.core.u_r->min, op.core.u_r->max))
			return res;
		value min = value_set(op.core.u_r->min);
		value max = value_set(op.core.u_r->max);
		int reversed_p = value_gt(min, max);
		if (op.core.u_r->inclusive_p)
			if (reversed_p)
				value_dec_now(&max);
			else value_inc_now(&max);
		
		int first_time_p = TRUE;
		value tmp = value_init_nil();
		for (; reversed_p ? value_gt(min, max) : value_lt(min, max); reversed_p ? value_dec_now(&min) : value_inc_now(&min)) {
			tmp = value_call(variables, func, 1, &min);
			if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_BREAK) {
				value_clear(&tmp);
				break;
			} else if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				res = tmp;
				break;
			}
			
			if (first_time_p) {
				value_clear(&res);
				res = tmp;
				first_time_p = FALSE;
			} else {
				value_add_now(&res, tmp);
				value_clear(&tmp);
			}
		}
				
		value_clear(&min);
		value_clear(&max);
				
	
	} else {
		value_error(1, "Type Error: summation() is undefined where op is %ts (iterable expected).", op);
		res = value_init_error();
	}
	
	return res;
}

value value_times_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "times()") ? value_init_error() : value_times(tmp, argv[1], argv[2]);
}

value value_summation_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "summation()") ? value_init_error() : value_summation(tmp, argv[1], argv[2]);
}