/*
 *  optimize.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 8/10/10.
 *  
 *  All definitions for functions and variables in optimize.c can be found in value.h.
 *  
 */

#include "value.h"

/* 
 * Possible Optimizations
 * 
 * Trivial
 * -If there are two or more nested blocks with only one element, remove the extra blocks. 
 *  For example, ((1)) or (((1))) would become (1).
 * -Adding zero
 * -Multiplying by 1
 * -false && op, true || op, other functions with a guaranteed result.
 * -If expression with a constant condition, for example (if (true) (3)) would become 3.
 * 
 * Simple
 * -Evaluating built-in functions on constants, for example (+ 2 3) would be replaced with 5.
 * -Make small powers into repeated multiplications
 * 
 * Moderate
 * -Pull function calls out of loops.
 * 
 * Advanced 
 * -Convert tail-recursive calls to loops.
 * -Find and prevent/warn against infinite loops.
 * 
 */


#define OTHER_NUMERIC(assume_p, type) ((assume_p) && ((type) == VALUE_VAR || (type) == VALUE_BLK))

value value_optimize(value op, int flags)
{
	value res = value_set(op);
	value error = value_optimize_now(&res, flags);
	if (error.type == VALUE_ERROR) {
		value_clear(&res);
		return error;
	}
	return res;
}

value value_optimize_std(value op, value flags)
{
	int iflags = 0;
	if (flags.type == VALUE_SYM) {
		if (streq(flags.core.u_s, "assume_numeric"))
			iflags |= O_ASSUME_NUMERIC;
	} else if (flags.type == VALUE_ARY) {
		size_t i;
		for (i = 0; i < flags.core.u_a.length; ++i) {
			if (flags.core.u_a.a[i].type != VALUE_SYM) {
				value_error(1, "Type Error: In optimize(), undefined flag %ts (symbol expected).", flags.core.u_a.a[i]);
				return value_init_error();
			} else if (streq(flags.core.u_a.a[i].core.u_s, "assume_numeric"))
				iflags |= O_ASSUME_NUMERIC;
			else if (streq(flags.core.u_a.a[i].core.u_s, "o1"))
				iflags |= O_1;
			else if (streq(flags.core.u_a.a[i].core.u_s, "o2"))
				iflags |= O_2;
			else if (streq(flags.core.u_a.a[i].core.u_s, "o3"))
				iflags |= O_3;
			else if (streq(flags.core.u_a.a[i].core.u_s, "o4"))
				iflags |= O_4;
		}
	}
	
	return value_optimize(op, iflags);
}

/* 
 * Performs an optimization.
 * 
 * Flags
 * 
 * O_ASSUME_NUMERIC_P: Assume that all blocks contain a numeric value. If this is guaranteed 
 * to be true, more optimizations are possible. If you say that it's true and it's not, there 
 * will be incorrect optimizations.
 */
value value_optimize_now(value *op, int flags)
{
	value res = value_init_nil();
	
	if (flags & O_1) {
		res = value_optimize1_now(op, flags);
		return_if_error(res);
	}
	if (flags & O_2) {
		res = value_optimize2_now(op, flags);
		return_if_error(res);
	}
	if (flags & O_3) {
		res = value_optimize3_now(op, flags);
		return_if_error(res);
	}
	if (flags & O_4) {
		res = value_optimize4_now(op, flags);
		return_if_error(res);
	}
	
	return res;
}

/* 
 * Performs a level 1 optimization. This does trivial optimizations.
 */
value value_optimize1_now(value *op, int flags)
{
	if (op->type != VALUE_BLK)
		return value_init_nil();
	
	value res = value_init_nil();
	
	value *words = op->core.u_blk.a;
	size_t i, length = op->core.u_blk.length;
	
	int assume_numeric = flags && O_ASSUME_NUMERIC;
	
	// Recursively optimize all inner blocks.
	for (i = 0; i < length; ++i) {
		if (words[i].type == VALUE_BLK) {
			res = value_optimize1_now(&words[i], flags);
			if (res.type == VALUE_ERROR)
				return res;
		}
	}
	
	/* Remove rundant nested blocks, for example turn (1) into 1.
	 */
	if (length == 1 && words[0].type != VALUE_BIF && words[0].type != VALUE_UDF && words[0].type != VALUE_UDF_SHELL) {
		value saved = words[0];
		words[0].type = VALUE_NIL;
		value_clear(op);
		*op = saved;
	}
	
	/* An additon of zero can be removed.
	 */
	else if (length == 3 && words[0].type == VALUE_BIF && (words[0].core.u_bif->f == &value_add_arg || words[0].core.u_bif->f == &value_sub_arg)) {
		if (	// 0 - x cannot be optimized, only x - 0.
				words[0].core.u_bif->f != &value_sub_arg && 
				
				// If words[2] is a float, words[1] can be an integer or a float.
				((words[2].type == VALUE_MPF && value_eq(words[1], value_zero)) || 
				
				// If words[2] is an integer, words[1] has to be an integer, otherwise this optimization will change the result of the function.
				((words[2].type == VALUE_MPZ || OTHER_NUMERIC(assume_numeric, words[2].type)) && words[1].type == VALUE_MPZ && value_eq(words[1], value_zero)))) {
			
			value saved = words[2];
			words[2].type = VALUE_NIL;
			value_clear(op);
			*op = saved;
		} else if (	// If words[1] is a float, words[2] can be an integer or a float.
				((words[1].type == VALUE_MPF && value_eq(words[2], value_zero)) || 
				
				// If words[1] is an integer, words[2] has to be an integer, otherwise this optimization will change the result of the function.
				((words[1].type == VALUE_MPZ || OTHER_NUMERIC(assume_numeric, words[1].type)) && words[2].type == VALUE_MPZ && value_eq(words[2], value_zero)))) {
			
			value saved = words[1];
			words[1].type = VALUE_NIL;
			value_clear(op);
			*op = saved;
		}		
	}
		
	/* A multiplication by 1 can be removed.
	 */
	else if (length == 3 && words[0].type == VALUE_BIF && (words[0].core.u_bif->f == &value_mul_arg || words[0].core.u_bif->f == &value_div_arg)) {
		if (	// 1 / x cannot be optimized, only x / 1.
				words[0].core.u_bif->f != &value_div_arg && 
				
				// If words[2] is a float, words[1] can be an integer or a float.
				((words[2].type == VALUE_MPF && value_eq(words[1], value_one)) || 
				
				// If words[2] is an integer, words[1] has to be an integer, otherwise this optimization will change the result of the function.
				((words[2].type == VALUE_MPZ || OTHER_NUMERIC(assume_numeric, words[2].type)) && words[1].type == VALUE_MPZ && value_eq(words[1], value_one)))) {
			
			value saved = words[2];
			words[2].type = VALUE_NIL;
			value_clear(op);
			*op = saved;
		} else if (	// If words[1] is a float, words[2] can be an integer or a float.
				((words[1].type == VALUE_MPF && value_eq(words[2], value_one)) || 
				
				// If words[1] is an integer, words[2] has to be an integer, otherwise this optimization will change the result of the function.
				((words[1].type == VALUE_MPZ || OTHER_NUMERIC(assume_numeric, words[1].type)) && words[2].type == VALUE_MPZ && value_eq(words[2], value_one)))) {
			
			value saved = words[1];
			words[1].type = VALUE_NIL;
			value_clear(op);
			*op = saved;
		}
		
		/* A multiplication by 0 can become just 0.
		 */
		else if ( // 0 / x cannot be optimized because x might be 0.
				words[0].core.u_bif->f != &value_div_arg && 
				((words[2].type == VALUE_MPF && value_eq(words[1], value_zero)) || 
				((words[2].type == VALUE_MPZ || OTHER_NUMERIC(assume_numeric, words[2].type)) && words[1].type == VALUE_MPZ && value_eq(words[1], value_zero)))) {
			
			value saved = words[1];
			words[1].type = VALUE_NIL;
			value_clear(op);
			*op = saved;
		} else if ( // x / 0 cannot be optimized because it is undefined.
				words[0].core.u_bif->f != &value_div_arg && 
				((words[1].type == VALUE_MPF && value_eq(words[2], value_zero)) || 
				((words[1].type == VALUE_MPZ || OTHER_NUMERIC(assume_numeric, words[1].type)) && words[2].type == VALUE_MPZ && value_eq(words[2], value_zero)))) {
			
			value saved = words[2];
			words[2].type = VALUE_NIL;
			value_clear(op);
			*op = saved;
		}
	}
		
	/* To the 1 power can be removed.
	 */
	else if (length == 3 && words[0].type == VALUE_BIF && words[0].core.u_bif->f == &value_pow_arg && 
			((words[1].type == VALUE_MPF && value_eq(words[2], value_one)) || 
			
			// If words[1] is an integer, words[2] has to be an integer, otherwise this optimization will change the result of the function.
			((words[1].type == VALUE_MPZ || OTHER_NUMERIC(assume_numeric, words[1].type)) && words[2].type == VALUE_MPZ && value_eq(words[2], value_one)))) {
		
		value saved = words[1];
		words[1].type = VALUE_NIL;
		value_clear(op);
		*op = saved;
	}
	
	/* The negative of a negative can be turned into a positive.
	 */
	else if (assume_numeric && length == 2 && words[0].type == VALUE_BIF && words[0].core.u_bif->f == &value_uminus_arg && 
			words[1].type == VALUE_BLK && words[1].core.u_blk.length == 2 && words[1].core.u_blk.a[0].type == VALUE_BIF && 
			words[1].core.u_blk.a[0].core.u_bif->f == &value_uminus_arg) {
		
		// Remove the two outermost blocks.
		value saved = words[1].core.u_blk.a[1];
		words[1].core.u_blk.a[1].type = VALUE_NIL;
		value_clear(op);
		*op = saved;
	}
			
	return res;
	
}

value value_optimize2_now(value *op, int flags)
{
	if (op->type != VALUE_BLK)
		return value_init_nil();
	
	value res = value_init_nil();
	
	value *words = op->core.u_blk.a;
	size_t i, length = op->core.u_blk.length;
	
	int assume_numeric = flags & O_ASSUME_NUMERIC;
	
	// Recursively optimize all inner blocks.
	for (i = 0; i < length; ++i) {
		if (words[i].type == VALUE_BLK) {
			res = value_optimize2_now(&words[i], flags);
			if (res.type == VALUE_ERROR)
				return res;
		}
	}
		
	if (length > 1 && words[0].type == VALUE_BIF) {
		
		/* If the block is made up entirely of constants, evaluate it.
		 */
		int all_constants_p = TRUE;
		for (i = 1; i < length; ++i) {
			all_constants_p &= words[i].type == VALUE_NIL || words[i].type == VALUE_BOO || words[i].type == VALUE_MPZ || words[i].type == VALUE_MPF;
			if (all_constants_p == FALSE)
				break;
		}
		
		if (all_constants_p) {
			// The variables don't actually matter because there aren't any inside of the block. 
			value saved = eval(&value_nil, *op);
			value_printf("clearing %ts\n", *op);
			value_printf("putting in %ts\n", saved);
			value_clear(op);
			*op = saved;
		}
		
		optimize_put_constants_first(op, words, length, assume_numeric);
		optimize_simplify_constant_across_block(op, words, length, assume_numeric);
		
	}
	
	
	return res;
}

/* 
 * It's simpler if a compound expression like (* (+ x 3) 4) puts the constant first, such as 
 * (* 4 (+ 3 x)). This simplifies the number of cases that other optimizations have to deal 
 * with, and is also easier for people to read.
 */
int optimize_put_constants_first(value *op, value words[], size_t length, int assume_numeric)
{
	if (assume_numeric && length == 3) {
		if (value_commutative_p(words[0]) && !(words[1].type == VALUE_MPZ || words[1].type == VALUE_MPF) && 
				(words[2].type == VALUE_MPZ || words[2].type == VALUE_MPF)) {
			value temp = words[1];
			words[1] = words[2];
			words[2] = temp;
		}
	}
	
	return 0;
}

/* 
 * Simplify (* 10 (* 3 x)) to (* 30 x), and such things.
 * 
 * Assumes that optimize_put_constants_first() has already been called. If it hasn't, the result 
 * will not be fully optimized.
 */
int optimize_simplify_constant_across_block(value *op, value words[], size_t length, int assume_numeric)
{
	if (assume_numeric && length == 3 && value_commutative_p(words[0])) {
		// if words[2] is a block containing three elements: the same function as words[0], 
		// a number, and something else
		if (words[2].type == VALUE_BLK && words[2].core.u_blk.length == 3 && words[2].core.u_blk.a[1].type == VALUE_MPZ &&
				words[2].core.u_blk.a[0].type == VALUE_BIF && 
				words[2].core.u_blk.a[0].core.u_bif->f == words[0].core.u_bif->f) {
			if (words[1].type == VALUE_MPZ) {
				// Perform the operation on the number in words[2] by words[1].
				value argv[] = { words[1], words[2].core.u_blk.a[1] };
				value simplified_number = value_bifcall(words[0], 2, argv);
				value_clear(&words[2].core.u_blk.a[1]);
				words[2].core.u_blk.a[1] = simplified_number;
				
				// Clear the outer block containing words[1] and words[2].
				value saved_block = words[2];
				words[2].type = VALUE_NIL;
				value_clear(op);
				*op = saved_block;
					
			} else if (words[1].type == VALUE_BLK && words[1].core.u_blk.length == 3 && words[1].core.u_blk.a[1].type == VALUE_MPZ &&
					words[1].core.u_blk.a[0].type == VALUE_BIF && 
					words[1].core.u_blk.a[0].core.u_bif->f == words[0].core.u_bif->f) {
				// Two blocks have been found, such as (3 * x) * (5 * x). Simplify to (15 * x) * x
				
				// Perform the operation on the number in words[1] by the number in words[2].
				value argv[] = { words[1].core.u_blk.a[1], words[2].core.u_blk.a[1] };
				value simplified_number = value_bifcall(words[0], 2, argv);
				value_clear(&words[1].core.u_blk.a[1]);
				words[1].core.u_blk.a[1] = simplified_number;
				
				// Replace words[2] with just the non-number part of words[2].
				value saved_part = words[2].core.u_blk.a[2];
				words[2].core.u_blk.a[2].type = VALUE_NIL;
				value_clear(&words[2]);
				words[2] = saved_part;
			}
		}
	}
	
	return 0;
}

value value_optimize3_now(value *op, int flags)
{
	value_error(1, "Implementation Error: optimize3!() not yet implemented.");
	return value_init_error();
}

value value_optimize4_now(value *op, int flags)
{
	value_error(1, "Implementation Error: optimize3!() not yet implemented.");
	return value_init_error();
}


value value_optimize_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "optimize()") ? value_init_error() : value_optimize_std(argv[0], argv[1]);
}

