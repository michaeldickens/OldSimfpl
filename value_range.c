/*
 *  value_range.c
 *  Calculator
 *
 *  Created by Michael Dickens on 6/10/10.
 *  Copyright 2010 The Khabele School. All rights reserved.
 *
 */

#include "value.h"

value value_range_to(value op1, value op2)
{
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		int error_p = FALSE;
		if (value_gt(op2, value_int_max) || value_lt(op2, value_int_min)) {
			value_error(1, "Domain Error: ..() is undefined where op2 is greater than %s.", value_int_max);
			error_p = TRUE;
		}
		if (value_gt(op1, value_int_max) || value_lt(op1, value_int_min)) {
			value_error(1, "Domain Error: ..() is undefined where op1 is greater than %s.", value_int_max);
			error_p = TRUE;
		}
		value diff = value_sub(op2, op1);
		if (value_gt(diff, value_int_max) || value_lt(diff, value_int_min)) {
			value_error(1, "Domain Error: ..() is undefined where the difference between op1 and op2 is greater than %s.", value_int_max);
			error_p = TRUE;
		}
		value_clear(&diff);
		if (error_p)
			return value_init_error();
				
		value res;
		res.type = VALUE_RNG;
		res.core.u_r = value_malloc(NULL, sizeof(struct value_range));
		return_if_null(res.core.u_r);
		res.core.u_r->inclusive_p = TRUE;
		res.core.u_r->min = value_set(op1);
		res.core.u_r->max = value_set(op2);

		return res;
	}
	
	if (op1.type != VALUE_MPZ)
		value_error(1, "Type Error: ..() is undefined where op1 is %ts (integer expected).", op1);

	if (op1.type != op2.type)
		value_error(1, "Type Error: ..() is undefined for %ts and %ts.", op1, op2);
	
	return value_init_error();
}

value value_range_until(value op1, value op2)
{
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		int error_p = FALSE;
		if (value_gt(op2, value_int_max) || value_lt(op2, value_int_min)) {
			value_error(1, "Domain Error: ...() is undefined where op2 is greater than %s.", value_int_max);
			error_p = TRUE;
		}
		if (value_gt(op1, value_int_max) || value_lt(op1, value_int_min)) {
			value_error(1, "Domain Error: ...() is undefined where op1 is greater than %s.", value_int_max);
			error_p = TRUE;
		}
		value diff = value_sub(op2, op1);
		if (value_gt(diff, value_int_max) || value_lt(diff, value_int_min)) {
			value_error(1, "Domain Error: ...() is undefined where the difference between op1 and op2 is greater than %s.", value_int_max);
			error_p = TRUE;
		}
		value_clear(&diff);
		if (error_p)
			return value_init_error();
		
		value res;
		res.type = VALUE_RNG;
		res.core.u_r = value_malloc(NULL, sizeof(struct value_range));
		return_if_null(res.core.u_r);
		res.core.u_r->inclusive_p = FALSE;
		res.core.u_r->min = value_set(op1);
		res.core.u_r->max = value_set(op2);

		return res;
	}
	
	if (op1.type != VALUE_MPZ)
		value_error(1, "Type Error: ...() is undefined where op1 is %ts (integer expected).", op1);

	if (op1.type != op2.type)
		value_error(1, "Type Error: ...() is undefined for %ts and %ts.", op1, op2);
	
	return value_init_error();
}

value value_range_to_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "..()") ? value_init_error() : value_range_to(argv[0], argv[1]);
}

value value_range_until_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "...()") ? value_init_error() : value_range_until(argv[0], argv[1]);
}

