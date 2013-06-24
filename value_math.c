/*
 *  value_math.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 9/6/10.
 *  
 *  Advanced mathematical functions.
 */

#include "value.h"

int value_probab_prime_p(value op)
{
	if (op.type == VALUE_MPZ)
		return mpz_probab_prime_p(op.core.u_mz, 10);
	else {
		value_error(1, "Type Error: probab_prime() is undefined where op is %ts (integer expected).", op);
		return FALSE;
	}
}

value value_probab_prime_p_std(value op)
{
	if (op.type == VALUE_MPZ)
		return value_set_bool(mpz_probab_prime_p(op.core.u_mz, 10));
	else {
		value_error(1, "Type Error: probab_prime() is undefined where op is %ts (integer expected).", op);
		return value_init_error();
	}
}

value value_nextprime(value op)
{
	if (op.type == VALUE_MPZ) {
		value res = value_init(VALUE_MPZ);
		mpz_nextprime(res.core.u_mz, op.core.u_mz);
		return res;
	} else {
		value_error(1, "Type Error: nextprime() is undefined where op is %ts (integer expected).", op);
		return value_init_error();
	}

}

value value_gcd(value op1, value op2)
{
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ) {
		value res = value_init(VALUE_MPZ);
		mpz_gcd(res.core.u_mz, op1.core.u_mz, op2.core.u_mz);
		return res;
	} else {
		if (op1.type != VALUE_MPZ)
			value_error(1, "Type Error: nextprime() is undefined where op1 is %ts (integer expected).", op1);
		if (op2.type != VALUE_MPZ)
			value_error(1, "Type Error: nextprime() is undefined where op2 is %ts (integer expected).", op2);
		return value_init_error();
	}

}

value value_probab_prime_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "probab_prime?()") ? value_init_error() : value_probab_prime_p_std(argv[0]);
}

value value_nextprime_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "nextprime()") ? value_init_error() : value_nextprime(argv[0]);
}

value value_gcd_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "gcd()") ? value_init_error() : value_gcd(argv[0], argv[1]);
}

value value_seconds()
{
	return value_set_ulong((unsigned long) time(NULL));
}

value value_seconds_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "seconds()") ? value_init_error() : value_seconds();
}

value value_pow(value op1, value op2)
{
	int error_p = FALSE;
	
	if (op1.type != VALUE_MPZ && op1.type != VALUE_MPF) {
		value_error(1, "Type Error: Exponentiation is undefined where op1 is %ts (number expected).", op1);
		error_p = TRUE;
	}
	
	if (op2.type != VALUE_MPZ && op2.type != VALUE_MPF) {
		value_error(1, "Type Error: Exponentiation is undefined where op2 is %ts (number expected).", op2);
		error_p = TRUE;
	}
	
	if (error_p)
		return value_init_error();
	
	value res, x, y;
	
	res = value_init(VALUE_MPF);
	
	// op1 and op2 must be casted to MPFs because MPZ does not have good enough support for 
	// exponentiation.
	
	if (op1.type == VALUE_MPF)
		x = op1;
	else x = value_cast(op1, VALUE_MPF);
	
	if (op2.type == VALUE_MPF)
		y = op2;
	else y = value_cast(op2, VALUE_MPF);
	
	mpfr_pow(res.core.u_mf, x.core.u_mf, y.core.u_mf, value_mpfr_round);
	
	if (op1.type != VALUE_MPF)
		value_clear(&x);
	if (op2.type != VALUE_MPF)
		value_clear(&y);
	
	// If both numbers are integers and the exponent is non-negative, convert the result from 
	// a float to an integer.
	if (op1.type == VALUE_MPZ && op2.type == VALUE_MPZ && mpz_cmp_si(op2.core.u_mz, 0) >= 0) {
		y = res;
		res = value_cast(y, VALUE_MPZ);
		value_clear(&y);
	}
	
	return res;
}

value value_choose(value op1, value op2)
{
	value res;
	
	int error_p = FALSE;
	if (op1.type != VALUE_MPZ) {
		value_error(1, "Type Error: Binomial coefficient (n choose k) is undefined where op1 is %ts (integer expected).", op1);
		error_p = TRUE;
	}
	if (op2.type != VALUE_MPZ) {
		value_error(1, "Type Error: Binomial coefficient (n choose k) is undefined where op2 is %ts (integer expected).", op2);
		error_p = TRUE;
	}
	if (error_p)
		return value_init_error();
		
	if (value_gt(op2, op1)) {
		return value_set_long(0);
	}
	
	if (value_gt(op2, value_int_max)) {
		value_error(1, "Domain Error: Combinatoin (op1 choose op2) is undefined where op2 is greater than %ts.", value_int_max);
		return value_init_error();
	}
	unsigned long ui = mpz_get_ui(op2.core.u_mz);
	
	res.type = VALUE_MPZ;
	mpz_init(res.core.u_mz);
	mpz_bin_ui(res.core.u_mz, op1.core.u_mz, ui);
	return res;
}

value value_exp(value op)
{
	if (op.type != VALUE_MPZ && op.type != VALUE_MPF) {
		value_error(1, "Argument Error: Logarithms are undefined when op is %ts (number expected).", op);
		return value_init_error();
	}
	
	value x;
	if (op.type == VALUE_MPF)
		x = op;
	else x = value_cast(op, VALUE_MPF);
	
	value res = value_init(VALUE_MPF);
	mpfr_exp(res.core.u_mf, x.core.u_mf, value_mpfr_round);
	if (op.type != VALUE_MPF)
		value_clear(&x);
	return res;
}

value value_log(value op)
{
	if (op.type != VALUE_MPZ && op.type != VALUE_MPF) {
		value_error(1, "Argument Error: Logarithms are undefined when op is %ts (number expected).", op);
		return value_init_error();
	}
	
	if (value_le(op, value_zero))
		return value_init(VALUE_NAN);
	
	value x;
	if (op.type == VALUE_MPF)
		x = op;
	else x = value_cast(op, VALUE_MPF);
	
	value res = value_init(VALUE_MPF);
	mpfr_log(res.core.u_mf, x.core.u_mf, value_mpfr_round);
	if (op.type != VALUE_MPF)
		value_clear(&x);
	return res;
}

value value_log2(value op)
{
	if (op.type != VALUE_MPZ && op.type != VALUE_MPF) {
		value_error(1, "Argument Error: Logarithms are undefined when op is %ts (number expected).", op);
		return value_init_error();
	}
	
	if (value_le(op, value_zero))
		return value_init(VALUE_NAN);
	
	value x;
	if (op.type == VALUE_MPF)
		x = op;
	else x = value_cast(op, VALUE_MPF);
	
	value res = value_init(VALUE_MPF);
	mpfr_log2(res.core.u_mf, x.core.u_mf, value_mpfr_round);
	if (op.type != VALUE_MPF)
		value_clear(&x);
	return res;
}

value value_log10(value op)
{
	if (op.type != VALUE_MPZ && op.type != VALUE_MPF) {
		value_error(1, "Argument Error: Logarithms are undefined when op is %ts (number expected).", op);
		return value_init_error();
	}
	
	if (value_le(op, value_zero))
		return value_init(VALUE_NAN);
	
	value x;
	if (op.type == VALUE_MPF)
		x = op;
	else x = value_cast(op, VALUE_MPF);
	
	value res = value_init(VALUE_MPF);
	mpfr_log10(res.core.u_mf, x.core.u_mf, value_mpfr_round);
	if (op.type != VALUE_MPF)
		value_clear(&x);
	return res;
}

value value_sqrt(value op)
{	
	if (op.type != VALUE_MPZ && op.type != VALUE_MPF) {
		value_error(1, "Argument Error: Square root is undefined when op is %ts (number expected).", op);
		return value_init_error();
	}
	
	if (value_lt(op, value_zero))
		return value_init(VALUE_NAN);
	
	value x;
	if (op.type == VALUE_MPF)
		x = op;
	else x = value_cast(op, VALUE_MPF);
	
	value res = value_init(VALUE_MPF);
	mpfr_sqrt(res.core.u_mf, x.core.u_mf, value_mpfr_round);
	if (x.type != VALUE_MPF)
		value_clear(&x);
	
	return res;
}

value value_factorial(value op)
{
	value res;
	
	if (op.type != VALUE_MPZ) {
		value_error(1, "Type Error: Factorial is undefined where op is %ts (integer expected).", op);
		return value_init_error();
	}
	
	if (mpz_sgn(op.core.u_mz) == -1) {
		value_error(1, "Type Error: Factorial is undefined where op is %ts (non-negative integer expected).", op);
		return value_init_error();
	}
	
	unsigned long ulop = value_get_ulong(op);
	if (mpz_cmp_ui(op.core.u_mz, ulop)) {
		value_error(1, "Type Error: Factorial is undefined where op is %ts (must be less than ULONG_MAX).", op);
		return value_init_error();
	}
	
	res = value_init(VALUE_MPZ);
	mpz_fac_ui(res.core.u_mz, ulop);	
	
	return res;
}

value value_pow_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "exponentiation") ? value_init_error() : value_pow(argv[0], argv[1]);
}

value value_choose_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "combination (op1 choose op2)") ? value_init_error() : value_choose(argv[0], argv[1]);
}

value value_exp_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "natural exponentiation") ? value_init_error() : value_exp(argv[0]);
}

value value_log_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "log") ? value_init_error() : value_log(argv[0]);
}

value value_log10_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "log10") ? value_init_error() : value_log10(argv[0]);
}

value value_log2_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "log2") ? value_init_error() : value_log2(argv[0]);
}

value value_sqrt_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "sqrt") ? value_init_error() : value_sqrt(argv[0]);
}

value value_factorial_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "factorial") ? value_init_error() : value_factorial(argv[0]);
}

value value_trig(value op, int func)
{
	if (op.type != VALUE_MPZ && op.type != VALUE_MPF) {
		value_error(1, "Argument Error: Trigonometric functions are undefined when op is %ts (number expected.", op);
		return value_init_error();
	}
	
	value res;
	res.type = VALUE_MPF;

	if (op.type == VALUE_MPZ)
		mpfr_init_set_z(res.core.u_mf, op.core.u_mz, value_mpfr_round);
	else mpfr_init_set(res.core.u_mf, op.core.u_mf, value_mpfr_round);
	
	switch (func) {
		case VALUE_SIN:
			mpfr_sin(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_COS:
			mpfr_cos(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_TAN:
			mpfr_tan(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_CSC:
			mpfr_csc(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_SEC:
			mpfr_sec(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_COT:
			mpfr_cot(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_ASIN:
			if (mpfr_cmp_si(res.core.u_mf, -1) < 0 || mpfr_cmp_si(res.core.u_mf, 1) > 0) {
				value_error(1, "Argument Error: In asin(), argument is out of the function domain.");
				return value_init_error();
			}
			mpfr_asin(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_ACOS:
			if (mpfr_cmp_si(res.core.u_mf, -1) < 0 || mpfr_cmp_si(res.core.u_mf, 1) > 0) {
				value_error(1, "Argument Error: In cos(), argument is out of the function domain.");
				return value_init_error();
			}
			mpfr_acos(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_ATAN:
			mpfr_atan(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_SINH:
			mpfr_sinh(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_COSH:
			mpfr_cosh(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_TANH:
			mpfr_tanh(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_CSCH:
			mpfr_csch(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_SECH:
			mpfr_sech(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_COTH:
			mpfr_coth(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_ASINH:
			if (mpfr_cmp_si(res.core.u_mf, -1) < 0 || mpfr_cmp_si(res.core.u_mf, 1) > 0) {
				value_error(1, "Argument Error: In asin(), argument is out of the function domain.");
				return value_init_error();
			}
			mpfr_asinh(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_ACOSH:
			if (mpfr_cmp_si(res.core.u_mf, -1) < 0 || mpfr_cmp_si(res.core.u_mf, 1) > 0) {
				value_error(1, "Argument Error: In cos(), argument is out of the function domain.");
				return value_init_error();
			}
			mpfr_acosh(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
		case VALUE_ATANH:
			mpfr_atanh(res.core.u_mf, res.core.u_mf, value_mpfr_round);
			break;
	}
		
	return res;
}

value value_sin_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "sin") ? value_init_error() : value_sin(argv[0]);
}

value value_cos_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "cos") ? value_init_error() : value_cos(argv[0]);
}

value value_tan_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "tan") ? value_init_error() : value_tan(argv[0]);
}

value value_csc_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "csc") ? value_init_error() : value_csc(argv[0]);
}

value value_sec_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "sec") ? value_init_error() : value_sec(argv[0]);
}

value value_cot_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "cot") ? value_init_error() : value_cot(argv[0]);
}

value value_asin_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "asin") ? value_init_error() : value_asin(argv[0]);
}

value value_acos_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "acos") ? value_init_error() : value_acos(argv[0]);
}

value value_atan_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "atan") ? value_init_error() : value_atan(argv[0]);
}

value value_sinh_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "sin") ? value_init_error() : value_sinh(argv[0]);
}

value value_cosh_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "cos") ? value_init_error() : value_cosh(argv[0]);
}

value value_tanh_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "tan") ? value_init_error() : value_tanh(argv[0]);
}

value value_csch_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "csc") ? value_init_error() : value_csch(argv[0]);
}

value value_sech_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "sec") ? value_init_error() : value_sech(argv[0]);
}

value value_coth_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "cot") ? value_init_error() : value_coth(argv[0]);
}

value value_asinh_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "asin") ? value_init_error() : value_asinh(argv[0]);
}

value value_acosh_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "acos") ? value_init_error() : value_acosh(argv[0]);
}

value value_atanh_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "atan") ? value_init_error() : value_atanh(argv[0]);
}

/* 
 * Returns a new function which is the derivative of (op). (op) must be in standard S-expression 
 * form.
 * 
 * It currently assumes that all variables represent the same variable.
 * 
 * In order to simplify the results, value_optimize1_now() is called. This removes addition by 
 * zero, multiplication by 1, etc.
 */
value value_deriv(value op)
{
	if (op.type == VALUE_MPZ || op.type == VALUE_MPF) {
		/* The derivative of a number is 0. */
		return value_set_long(0);
	} else if (op.type == VALUE_VAR) {
		/* The derivative of a lone variable is 1. */
		return value_set_long(1);
	} else if (op.type != VALUE_BLK) {
		value_error(1, "Type Error: deriv() is undefined for %ts (number, variable or block expected).", op);
		return value_init_error();
	}
	
	if (op.core.u_blk.length == 0) {
		return value_set(op);
	} else if (op.core.u_blk.length == 1) {
		return value_deriv(op.core.u_blk.a[0]);
	}
	
	if (op.core.u_blk.a[0].type != VALUE_BIF) {
		value_error(1, "Error: In deriv(), undefined syntax for the first element of %s.", op);
		return value_init_error();
	}
	
	value res = value_init_nil();
	if (op.core.u_blk.length == 2) {
		res = value_deriv_2(op);
	} else if (op.core.u_blk.length == 3) {
		res = value_deriv_3(op);
	}
	
	return res;
}

/* 
 * Takes the derivative of a block with two elements.
 */
value value_deriv_2(value op)
{
	value res = value_init_nil();
	value der = value_init_nil();
	value blk = value_init_nil();
	value fun = value_init_nil();
	
	value (*f)(int argc, value *argv) = op.core.u_blk.a[0].core.u_bif->f;
	
	if (f == &value_uminus_arg) {
		/* deriv (-X) = -deriv(X) */
		
		res = value_init(VALUE_BLK);
		fun = value_set_fun(&value_uminus_arg);
		der = value_deriv(op.core.u_blk.a[1]);
		value_append_now2(&res, &fun);
		value_append_now2(&res, &der);
		
	} else if (f == &value_sin_arg) {
		/* sine rule: deriv (sin(X)) = deriv(X) * cos(X) */
		
		// deriv(X)
		der = value_deriv(op.core.u_blk.a[1]);
		
		// Create a block containing cos(X).
		fun = value_set_fun(&value_cos_arg);
		blk = value_init(VALUE_BLK);
		value_append_now2(&blk, &fun);
		value_append_now(&blk, op.core.u_blk.a[1]);
		
		// Create a block containing deriv(X) * cos(X).
		res = value_init(VALUE_BLK);
		fun = value_set_fun(&value_mul_arg);
		value_append_now2(&res, &fun);
		value_append_now2(&res, &blk);
		value_append_now2(&res, &der);
		
	} else if (f == &value_cos_arg) {
		/* sine rule: deriv (cos(X)) = deriv(X) * -sin(X) */
		
		// deriv(X)
		der = value_deriv(op.core.u_blk.a[1]);
		
		// Create a block containing sin(X).
		fun = value_set_fun(&value_sin_arg);
		blk = value_init(VALUE_BLK);
		value_append_now2(&blk, &fun);
		value_append_now(&blk, op.core.u_blk.a[1]);
		
		// Create a block containing -sin(X).
		value tmp = blk;
		blk = value_init(VALUE_BLK);
		fun = value_set_fun(&value_uminus_arg);
		value_append_now2(&blk, &fun);
		value_append_now2(&blk, &tmp);
		
		// Create a block containing deriv(X) * -cos(X).
		res = value_init(VALUE_BLK);
		fun = value_set_fun(&value_mul_arg);
		value_append_now2(&res, &fun);
		value_append_now2(&res, &blk);
		value_append_now2(&res, &der);
		
	} else if (f == &value_log_arg) {
		/* logarithm rule: deriv(log(X)) = 1 / X */
		
		// deriv(X)
		der = value_deriv(op.core.u_blk.a[1]);
		
		// Create a block containing deriv(X) / X.
		res = value_init(VALUE_BLK);
		fun = value_set_fun(&value_div_arg);
		value_append_now2(&res, &fun);
		value_append_now2(&res, &der);
		value_append_now(&res, op.core.u_blk.a[1]);
		
	} else if (f == &value_exp_arg) {
		/* exponent rule: deriv(e**X) = e**X */
		
		// deriv(X)
		der = value_deriv(op.core.u_blk.a[1]);
		
		res = value_init(VALUE_BLK);
		fun = value_set_fun(&value_mul_arg);
		value_append_now2(&res, &fun);
		value_append_now(&res, op);
		value_append_now2(&res, &der);
	}
	
	value_optimize_now(&res, O_1 | O_2 | O_ASSUME_NUMERIC);
	
	return res;
}

/* 
 * Takes the derivative of a block with three elements.
 */
value value_deriv_3(value op)
{
	value res = value_init_nil();
	value der = value_init_nil();
	value blk = value_init_nil();
	value fun = value_init_nil();
	
	value (*f)(int argc, value *argv) = op.core.u_blk.a[0].core.u_bif->f;
	if (f == &value_add_arg || f == &value_sub_arg) {
		/* Addition and subtraction rules: deriv (X + Y) = deriv(X) + deriv(Y) */
		
		res = value_init(VALUE_BLK);
		value_append_now(&res, op.core.u_blk.a[0]);
		der = value_deriv(op.core.u_blk.a[1]);
		value_append_now2(&res, &der);
		der = value_deriv(op.core.u_blk.a[2]);
		value_append_now2(&res, &der);
				
	} else if (f == &value_mul_arg) {
		/* Multiplication rule: deriv (X * Y) = Y*deriv(X) + X*deriv(Y) */
		
		res = value_init(VALUE_BLK);
		fun = value_set_fun(&value_add_arg);
		value_append_now2(&res, &fun);
		
		// Create a block containing Y*deriv(X) and add it to (res)
		der = value_deriv(op.core.u_blk.a[1]);
		blk = value_init(VALUE_BLK);
		fun = value_set_fun(&value_mul_arg);
		value_append_now2(&blk, &fun);
		value_append_now(&blk, op.core.u_blk.a[2]);
		value_append_now2(&res, &blk);
		value_append_now2(&blk, &der);
		
		// Create a block containing X*deriv(Y) and add it to (res)
		der = value_deriv(op.core.u_blk.a[2]);
		blk = value_init(VALUE_BLK);
		fun = value_set_fun(&value_mul_arg);
		value_append_now2(&blk, &fun);
		value_append_now(&blk, op.core.u_blk.a[1]);
		value_append_now2(&res, &blk);
		value_append_now2(&blk, &der);
		
	} else if (f == &value_div_arg) {
		/* Quotient rule: deriv (X / Y) = (Y*deriv(X) - X*deriv(Y)) / (Y**2) */
		
		fun = value_set_fun(&value_sub_arg);
		res = value_init(VALUE_BLK);
		value_append_now2(&res, &fun);
				
		// Create a block containing Y*deriv(X) and add it to (res)
		blk = value_init(VALUE_BLK);
		der = value_deriv(op.core.u_a.a[1]);
		fun = value_set_fun(&value_mul_arg);
		value_append_now2(&blk, &fun);
		value_append_now(&blk, op.core.u_a.a[2]);
		value_append_now2(&blk, &der);
		value_append_now2(&res, &blk);
		
		// Create a block containing X*deriv(Y) and add it to (res)
		blk = value_init(VALUE_BLK);
		der = value_deriv(op.core.u_a.a[2]);
		fun = value_set_fun(&value_mul_arg);
		value_append_now2(&blk, &fun);
		value_append_now(&blk, op.core.u_a.a[1]);
		value_append_now2(&blk, &der);
		value_append_now2(&res, &blk);		
		
		// Create a block containing Y**2
		blk = value_init(VALUE_BLK);
		fun = value_set_fun(&value_pow_arg);
		value tmp = value_set_long(2);
		value_append_now2(&blk, &fun);
		value_append_now(&blk, op.core.u_a.a[2]);
		value_append_now2(&blk, &tmp);
		
		// Divide (res) by (Y**2)
		tmp = value_init(VALUE_BLK);
		fun = value_set_fun(&value_div_arg);
		value_append_now2(&tmp, &fun);
		value_append_now2(&tmp, &res);
		value_append_now2(&tmp, &blk);
				
		res = tmp;
				
	} else if (f == &value_pow_arg) {
		/* Power rule: deriv (X ** n) = deriv(X) * n * (X**(n-1))
		 * Exponent rule: deriv (n ** X) = deriv(X) * (n ** X) * log n
		 */
		
		// Don't forget about the chain rule.
		
		if (op.core.u_blk.a[2].type == VALUE_MPZ || op.core.u_blk.a[2].type == VALUE_MPF) {
			// Calculate the derivative of X ** n.
						
			// deriv(X)
			der = value_deriv(op.core.u_blk.a[1]);
			
			// n - 1
			blk = value_dec(op.core.u_blk.a[2]);
			
			// Create a block containing X**(n-1)
			res = value_init(VALUE_BLK);
			fun = value_set_fun(&value_pow_arg);
			value_append_now2(&res, &fun);
			value_append_now(&res, op.core.u_blk.a[1]);
			value_append_now2(&res, &blk);
			blk = res;
			
			// Create a block containing n * X**(n-1)
			res = value_init(VALUE_BLK);
			fun = value_set_fun(&value_mul_arg);
			value_append_now(&res, fun);
			value_append_now(&res, op.core.u_blk.a[2]);
			value_append_now2(&res, &blk);
			blk = res;
			
			// Create a block containing deriv(X) * n * X**(n-1)
			res = value_init(VALUE_BLK);
			value_append_now2(&res, &fun);
			value_append_now2(&res, &blk);
			value_append_now2(&res, &der);
		}
	
	}
		
	value_optimize_now(&res, O_1 | O_2 | O_ASSUME_NUMERIC);
	
	return res;
}

value value_deriv_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "deriv") ? value_init_error() : value_deriv(argv[0]);
}

value value_eint(value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_eint(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: eint() is undefined where op is %ts (float expected).", op);
		res = value_init_error();
	}
	
	return res;
}

value value_li2(value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_li2(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: li2() is undefined where op is %ts (float expected).", op);
		res = value_init_error();
	}
	
	return res;	
}

value value_gamma(value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_gamma(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: gamma() is undefined where op is %ts (float expected).", op);
		res = value_init_error();
	}
	
	return res;	
}

value value_lngamma(value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_lngamma(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: lngamma() is undefined where op is %ts (float expected).", op);
		res = value_init_error();
	}
	
	return res;	
}

value value_zeta(value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_zeta(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: zeta() is undefined where op is %ts (float expected).", op);
		res = value_init_error();
	}
	
	return res;	
}

value value_erf(value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_erf(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: erf() is undefined where op is %ts (float expected).", op);
		res = value_init_error();
	}
	
	return res;	
}

value value_erfc(value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_MPF) {
		res = value_init(VALUE_MPF);
		mpfr_erfc(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		return res;
	} else {
		value_error(1, "Type Error: erfc() is undefined where op is %ts (float expected).", op);
		res = value_init_error();
	}
	
	return res;	
}
