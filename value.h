/*
 *  value.h
 *  Simfpl
 *
 *  Created by Michael Dickens on 1/23/10.
 *
 */

#ifndef TOOLS_H
#include "tools.h"
#endif

/* Single-value types. */
#define VALUE_NIL 0
#define VALUE_NAN 1
#define VALUE_INF 2

#define VALUE_TYP 3

#define VALUE_SPEC 4

/* General types. */
#define VALUE_BOO 10
#define VALUE_MPZ 11
#define VALUE_MPF 12
#define VALUE_STR 13
#define VALUE_ID  14
#define VALUE_VAR 15
#define VALUE_RGX 16	// Regular expression.
#define VALUE_SYM 17	// Symbol.

/* Containers and iterables. */
#define VALUE_ARY 18
#define VALUE_LST 19
#define VALUE_PAR 20	// Pair.
#define VALUE_HSH 21
#define VALUE_TRE 22	// Tree.
#define VALUE_PTR 23
#define VALUE_RNG 24	// Range.
#define VALUE_BLK 25	// Block, in the form of an S-expression.

#define VALUE_STOP 26	// Stop the execution of a loop or iterator.

#define VALUE_BIF 30	// Built-in function.
#define VALUE_UDF 31	// User-defined function.
#define VALUE_UDF_SHELL 32
#define VALUE_MAC 33	// Macro.

#define VALUE_EXC 40	// Exception.
#define VALUE_MISSING_ARG 41


#define VALUE_ERROR -1


/*
 * A structure for holding a dynamic value. The types it supports can be seen in the macros 
 * above, named VALUE_XYZ.
 * 
 * See "using the value type.rtf" for more information.
 */



/* 
 * A Note on The Categorization of Functions: 
 * 
 * Functions are placed in several different files.
 * 
 * Some functions can be applied to multiple types. For example, addition typically refers to 
 * addition of numbers, but it is also possible to add (concat) strings or arrays. In such cases, 
 * the function is placed in the file of whichever type it is most used for. If there is no clear 
 * most-used type, then it is placed in whichever file comes first in the below list.
 * 
 * value.c: Generic functions and functions for types too small to warrant their own file. 
 *   Includes pointers and I/O.
 * value_number.c: Numeric functions for integers and floats.
 * value_string.c: String functions.
 * value_regex.c: Functions for regular expressions and strings.
 * value_array.c: Functions for arrays. All of these functions should work on lists as well.
 * value_list.c: Functions for lists. Optimized for lists, but should work on arrays as well.
 * value_hash.c: Functions for hashes. Many of these have special names, and work fairly 
 *   independently of the other value functions. This is primarily because hashes are used 
 *   internally so it's easier if they work somewhat differently.
 * value_range.c: Functions for ranges.
 * value_block.c: Functions for blocks, control structures, and user-defined functions.
 * value_exception.c: Functions for exceptions.
 */

// The actual definition for the value type is in tools.h.

/* 
 * These have to be put here because C's file hierarchy is stupid.
 */

value global_variables;

// These are initialized in init_interpreter().
value primitive_funs;
value primitive_specs;
value primitive_names;
value symbol_ids;
value function_ids;

/* 
 * Evaluates the given S-expression.
 */
#define eval(variables, sexp) eval_generic(variables, sexp, FALSE)

/* The generic version of eval(), which takes additional arguments.
 * outer_was_block_p: If the outer sexp was a block, then a stack trace is 
 * not printed. when the program throws an error. This prevents excessively 
 * deep stack traces.
 */
value eval_generic(value *variables, value sexp, int outer_was_block_p);

struct value_spec compile_spec(char *str);

/* These only deal with references to values, so be sure to make a copy if you 
 * need to.
 */
value line_queue;
value line_queue_back;
void line_enqueue(value op);
void line_enqueue_front(value op);
value line_dequeue();

/* Returns an array containing the next word. May contain multiple values in the case 
 * of parentheses or brackets.
 */
value get_next_word(value words[], size_t length);


/* 
 * Declarations for optimize.c
 * 
 * See optimize.c for documentation.
 */

#define O_1 0x1
#define O_2 0x2
#define O_3 0x4
#define O_4 0x8
#define O_ASSUME_NUMERIC 0x10

value value_optimize(value op, int flags);
value value_optimize_std(value op, value flags);
value value_optimize_now(value *op, int flags);
value value_optimize1_now(value *op, int flags);
value value_optimize2_now(value *op, int flags);
value value_optimize3_now(value *op, int flags);
value value_optimize4_now(value *op, int flags);

int optimize_put_constants_first(value *op, value words[], size_t length, int assume_numeric);
int optimize_simplify_constant_across_block(value *op, value words[], size_t length, int assume_numeric);

value value_optimize_arg(int argc, value argv[]);


/* 
 * Declarations for the Value Type
 */


// These are initialized in init_values().
mpfr_prec_t value_mpfr_default_prec;
mpfr_rnd_t value_mpfr_round, value_mpfr_round_cast;
value value_int_min, value_zero, value_one, value_int_max, value_nil;
value value_symbol_in, value_symbol_dotimes, value_symbol_if;
struct value_spec value_nil_function_spec;

exception argument_error, generic_error, memory_error, runtime_error, syntax_error, type_error;


/* 
 * Initializes the value type. This should be done before any value is used.
 */
int init_values();

#define return_if_error(op) if ((op).type == VALUE_ERROR) return (op)
#define return_if_null(op) if ((op) == NULL) return value_init_error()

/* 
 * Determines whether the function called (name) has any missing arguments. This is 
 * mostly for internal use.
 */
int missing_arguments(int argc, value argv[], char *name);

const char * type_to_string(int type);

int value_array_id_index(value vals[], int length, char *val);
int value_array_copy(value target[], value source[], int size);

/* Initializes a value to nil.
 */
value value_init_nil();

/* Initializes a value to (error).
 */
value value_init_error();

/* Initializes a value of the given type. Allocates memory, sets containers to 
 * empty and sets numbers to 0.
 */
value value_init(int type);

/* Clears the given value. Values must be cleared after you are done using them.
 */
int value_clear(value *op);

/* Returns a deep copy of (op).
 */
value value_set(value op);
value value_set_arg(int argc, value argv[]);

value switch_value_set(value op);
value if_value_set(value op);

value value_set_bool(int x);
value value_set_long(long x);
value value_set_bigint(long x);
value value_set_ulong(unsigned long x);
value value_set_double(double x);
value value_set_str(char *str);
value value_set_str_length(char *str, size_t length);
value value_set_symbol(char *str);
value value_set_ary(value array[], size_t length);
value value_set_ary_ref(value array[], size_t length);
value value_set_ary_bool(int array[], size_t length);
value value_set_ary_long(long array[], size_t length);
value value_set_ary_ulong(unsigned long array[], size_t length);
value value_set_ary_double(double array[], size_t length);
value value_set_ary_str(char *array[], size_t length);
value value_set_block(value array[], size_t length);

/* Returns a shallow copy of (op).
 */
value value_copy(value op);

/* Set the value based on (str). It automatically converts to an integer, a double, or whatever 
 * is necessary.
 */
value value_set_str_smart(char *str, int base);
value value_set_id(char *id);
value value_set_fun(value (*fun)(int argc, value argv[]));

/* If the given function is commutative, returns TRUE. If not, or if the given value is 
 * not a function, returns FALSE.
 */
int value_commutative_p(value fun);

// value_malloc() and related functions are declared in tools.c.


/* 
 * 
 * 
 * External functions start here.
 * 
 * 
 */
 
 
/* Imports a file and runs its contents. op is a string representing the file nmae.
 */
value value_import(value op);
value value_import_arg(int argc, value argv[]);

/* 
 * Constructor functions for data structures.
 */
value value_array_arg(int argc, value argv[]);
value value_list_arg(int argc, value argv[]);
value value_hash_arg(int argc, value argv[]);


/* 
 * Conversion functions.
 */
value value_cast(value op, int type);

/* to array */
value value_to_a_arg(int argc, value argv[]);

/* to float */
value value_to_f_arg(int argc, value argv[]);

/* to hash */
value value_to_h_arg(int argc, value argv[]);

/* to integer */
value value_to_i_arg(int argc, value argv[]);

/* to list */
value value_to_l_arg(int argc, value argv[]);

/* to string */
value value_to_s_arg(int argc, value argv[]);

/* to regex */
value value_to_r_arg(int argc, value argv[]);

/* Converts op1 to a string in the given base.
 */
value value_to_s_base_arg(int argc, value argv[]);

/* Returns the value's type.
 */
value value_type(value op);
value value_type_arg(int argc, value argv[]);

/* Determines whether (op) is considered true.
 */
int value_true_p(value op);
value value_true_p_arg(int argc, value argv[]);


/* 
 * Assignment and data manipulation functions.
 */
value value_assign(value *variables, value op1, value op2);
value value_assign_arg(int argc, value argv[]);

// These functions are not implemented, as I see no need to implement them.
// assign_(operation)_arg works just fine.
value value_assign_add(value *variables, value op1, value op2);
value value_assign_sub(value *variables, value op1, value op2);
value value_assign_mul(value *variables, value op1, value op2);
value value_assign_div(value *variables, value op1, value op2);
value value_assign_mod(value *variables, value op1, value op2);
value value_assign_and(value *variables, value op1, value op2);
value value_assign_xor(value *variables, value op1, value op2);
value value_assign_or(value *variables, value op1, value op2);
value value_assign_shl(value *variables, value op1, value op2);
value value_assign_shr(value *variables, value op1, value op2);

value value_assign_add_arg(int argc, value argv[]);
value value_assign_sub_arg(int argc, value argv[]);
value value_assign_mul_arg(int argc, value argv[]);
value value_assign_div_arg(int argc, value argv[]);
value value_assign_mod_arg(int argc, value argv[]);
value value_assign_and_arg(int argc, value argv[]);
value value_assign_xor_arg(int argc, value argv[]);
value value_assign_or_arg(int argc, value argv[]);
value value_assign_shl_arg(int argc, value argv[]);
value value_assign_shr_arg(int argc, value argv[]);

/* 
 * Pair functions.
 */
value value_make_pair(value op1, value op2);

// These need better names.
value value_pair_first(value op);
value value_pair_second(value op);

value value_make_pair_arg(int argc, value argv[]);

/* 
 * Pointer functions.
 */
// It would be nice to get these to work externally.
value value_refer(value *op);
value * value_deref(value op);

value value_refer_arg(int argc, value argv[]);

/* 
 * I/O Functions.
 */
value value_gets();

value value_gets_arg(int argc, value argv[]);

//value value_point_arg(int argc, value argv[]);
//value value_deref_arg(int argc, value argv[]);

value value_set_default_prec(value prec);
void value_set_prec_now(value *op, value prec);
value value_set_prec_arg(int argc, value argv[]);
value value_set_default_prec_arg(int argc, value argv[]);

/* 
 * Arithmetic functions. Nearly all functions use GMP and MPFR to perform 
 * computations, so see the GMP and MPFR manuals for further details. Uses 
 * GMP version 4.3.2 and MPFR version 2.4.2.
 * 
 * http://www.gnu.org/software/gmp/manual/html_mono/gmp.html
 * http://www.mpfr.org/mpfr-current/mpfr.html
 * 
 */

/* Adds two values together. It is defined differently for different types. 
 * In order of precedence: 
 * 
 * number and number: Numeric addition. If one or more of the numbers is a 
 *   float, the result is a float; otherwise, the result is an integer.
 * string and any: Converts op2 to a string and concatenates it to op1.
 * (regex, array or list) and same: Concatenates op1 and op2.
 * array and any: Appends op2 to op1.
 * list and any: Appends op2 to op1.
 * any and array: Prepends op1 to op2.
 * any and list: Conses op1 to op2.
 * any and string: Converts op1 to a string and concatenates it to op2.
 * 
 * Because this function cannot tell whether nil is supposed to represent 
 * a null value or the end of a list, addition with nil as an argument will 
 * return an error. This function will NOT assume that nil is the end of 
 * a list. If you want to construct a list, use (cons).
 */
value value_add(value op1, value op2);
value value_add_now(value *op1, value op2);

/* Subtracts op2 from op1. Only defined where op1 and op2 are numbers.
 */
value value_sub(value op1, value op2);
value value_sub_now(value *op1, value op2);

/* Multiplies op1 by op2. Defined differently for different types. In 
 * order of precedence: 
 * 
 * number and number: Numeric multiplication, as you'd expect.
 * (string, array or list) and integer: Concatenates op1 to itself op2 times.
 * integer and (string, array or list): Concatenates op2 to itself op1 times.
 */
value value_mul(value op1, value op2);
value value_mul_now(value *op1, value op2);

/* Divides op1 by op2. Only defined for numbers. If op2 is zero, returns NaN.
 */
value value_div(value op1, value op2);
value value_div_now(value *op1, value op2);

/* Calculates op1 modulo op2. Only defined for numbers. if op2 is zero, 
 * returns NaN.
 */
value value_mod(value op1, value op2);
value value_mod_now(value *op1, value op2);

value value_add_arg(int argc, value argv[]);
value value_sub_arg(int argc, value argv[]);
value value_mul_arg(int argc, value argv[]);
value value_div_arg(int argc, value argv[]);
value value_mod_arg(int argc, value argv[]);

/* Increments op.
 */
value value_inc(value op);
value value_inc_now(value *op);

/* Decrements op.
 */
value value_dec(value op);
value value_dec_now(value *op);

value value_inc_arg(int argc, value argv[]);
value value_dec_arg(int argc, value argv[]);

/* Returns negative (op).
 */
value value_uminus(value op);

/* Returns positive (op). This is NOT absolute value; uplus(1) will 
 * return 1, and uplus(-1) will return -1.
 */
value value_uplus(value op);

/* Returns the absolute value of op.
 */
value value_abs(value op);

value value_uminus_arg(int argc, value argv[]);
value value_uplus_arg(int argc, value argv[]);
value value_abs_arg(int argc, value argv[]);


/* 
 * Comparison functions.
 * 
 * For integers and floats: The values of the numbers are compared. If 
 * the types otherwise do not match, the types themselves are compared.
 * 
 * For single-type values such as nil and infinity: the values are 
 * considered equal.
 * 
 * For booleans: false is considered less than true.
 * 
 * For arrays and for lists; Each element of the data set is compared. 
 * If they are not equal, the result of the comparison is returned. If 
 * every value is equal up to length(op1) or length(op2), whichever is 
 * smaller, then the lengths are compared.
 * 
 * For pointers: The values inside the pointers are compared.
 */
int value_cmp(value op1, value op2);
value value_cmp_std(value op1, value op2);
int value_cmp_any(value op1, value op2);
int value_lt(value op1, value op2);
value value_lt_std(value op1, value op2);
int value_le(value op1, value op2);
value value_le_std(value op1, value op2);
int value_eq(value op1, value op2);
value value_eq_std(value op1, value op2);
int value_ne(value op1, value op2);
value value_ne_std(value op1, value op2);
int value_ge(value op1, value op2);
value value_ge_std(value op1, value op2);
int value_gt(value op1, value op2);
value value_gt_std(value op1, value op2);

/* Tests for equality based on object identity. Usually you want to use 
 * (eq) when comparing two values.
 */
int velue_equal_p(value op1, value op2);
value value_equal_p_std(value op1, value op2);

value value_cmp_arg(int argc, value argv[]);
value value_lt_arg(int argc, value argv[]);
value value_le_arg(int argc, value argv[]);
value value_eq_arg(int argc, value argv[]);
value value_ne_arg(int argc, value argv[]);
value value_ge_arg(int argc, value argv[]);
value value_gt_arg(int argc, value argv[]);


/*
 * Logical operations. The std versions of these return not a boolean, but 
 * whichever of the operands is the deciding factor. For instance: 
 * 
 * ("hi" || 10) returns "hi", because "hi" makes the expression true
 * (false || 10) returns 10
 * ("hi" && 10) returns 10, because 10 makes the expression true
 * (false && "hi") returns (false), because (false) makes the expression false
 * 
 * The versions that return ints merely return true (nonzero) or false (zero).
 */
int value_and_p(value op1, value op2);
value value_and_p_std(value op1, value op2);
int value_or_p(value op1, value op2);
value value_or_p_std(value op1, value op2);
int value_not_p(value op);
value value_not_p_std(value op);

value value_and_p_arg(int argc, value argv[]);
value value_or_p_arg(int argc, value argv[]);
value value_not_p_arg(int argc, value argv[]);


/* 
 * Bitwise operations.
 */

/* Calculate (op1) * 2**(op2) using bit shifts.
 */
value value_2exp(value op1, long op2);

/* Returns op1 shifted left by op2 bits.
 */
value value_shl(value op1, unsigned long op2);
value value_shl_std(value op1, value op2);

/* Returns op1 shifted right by op2 bits.
 */
value value_shr(value op1, unsigned long op2);
value value_shr_std(value op1, value op2);

/* Returns the bitwise AND of op1 and op2.
 */
value value_and(value op1, value op2);

/* Returns the bitwise inclusive-OR of op1 and op2.
 */
value value_or(value op1, value op2);

/* Returns the bitwise exclusive-OR of op1 and op2.
 */
value value_xor(value op1, value op2);

/* Returns the bitwise NOT of op.
 */
value value_not(value op);

value value_2exp_arg(int argc, value argv[]);
value value_shl_arg(int argc, value argv[]);
value value_shr_arg(int argc, value argv[]);
value value_and_arg(int argc, value argv[]);
value value_or_arg(int argc, value argv[]);
value value_xor_arg(int argc, value argv[]);
value value_not_arg(int argc, value argv[]);


/* 
 * Number theory functions.
 */
 
/* Returns TRUE if op is probably prime, and FALSE if it is definitely 
 * composite.
 */
int value_probab_prime_p(value op);
value value_probab_prime_p_std(value op);

/* Returns the next probable prime after op.
 */
value value_nextprime(value op);

/* Calculates the greatest common divisor of op1 and op2.
 */
value value_gcd(value op1, value op2);

value value_probab_prime_p_arg(int argc, value argv[]);
value value_nextprime_arg(int argc, value argv[]);
value value_gcd_arg(int argc, value argv[]);

/* Returns the number of seconds on the system clock.
 */
value value_seconds();
value value_seconds_arg(int argc, value argv[]);

/* 
 * Exponential functions.
 */

/* Calculates op1 to the op2 power.
 */
value value_pow(value op1, value op2);

/* Calculates the modular exponent with base (base), exponent (exp), and 
 * modulo (mod). Equivaleant to (base**exp % mod).
 */
value value_modexp(value base, value exp, value mod);

/* Calculates the binomial coefficient (n choose k) of op1 and op2.
 */
value value_choose(value op1, value op2);

/* Calculates e to the power of op.
 */
value value_exp(value op);

/* Calculates the natural logarithm of op.
 */
value value_log(value op);

/* Calculates the logarithm base 2 of op.
 */
value value_log2(value op);

/* Calculates the logarithm base 10 of op.
 */
value value_log10(value op);

/* Calculates the square root of op.
 */
value value_sqrt(value op);

/* Calculates the factorial of op.
 */
value value_factorial(value op);
value value_product_private(int n); /* A helper function for factorial(). */

value value_pow_arg(int argc, value argv[]);
value value_modexp_arg(int argc, value argv[]);
value value_choose_arg(int argc, value argv[]);
value value_exp_arg(int argc, value argv[]);
value value_log_arg(int argc, value argv[]);
value value_log10_arg(int argc, value argv[]);
value value_log2_arg(int argc, value argv[]);
value value_sqrt_arg(int argc, value argv[]);
value value_factorial_arg(int argc, value argv[]);



/* A function to perform any of the trigonometric or hyperbolic functions. 
 * Having them  all in one function is not quite as fast, but it is more 
 * concise.
 */
value value_trig(value op, int func);

value value_sin_arg(int argc, value argv[]);
value value_cos_arg(int argc, value argv[]);
value value_tan_arg(int argc, value argv[]);
value value_csc_arg(int argc, value argv[]);
value value_sec_arg(int argc, value argv[]);
value value_cot_arg(int argc, value argv[]);
value value_asin_arg(int argc, value argv[]);
value value_acos_arg(int argc, value argv[]);
value value_atan_arg(int argc, value argv[]);

value value_sinh_arg(int argc, value argv[]);
value value_cosh_arg(int argc, value argv[]);
value value_tanh_arg(int argc, value argv[]);
value value_csch_arg(int argc, value argv[]);
value value_sech_arg(int argc, value argv[]);
value value_coth_arg(int argc, value argv[]);
value value_asinh_arg(int argc, value argv[]);
value value_acosh_arg(int argc, value argv[]);
value value_atanh_arg(int argc, value argv[]);

/* 
 * Calculus functions.
 */

/* Calculates the single-variable derivative of a given function. It must 
 * be in standard S-expression notation.
 */
value value_deriv(value op);
value value_deriv_2(value op);
value value_deriv_3(value op);

value value_deriv_arg(int argc, value argv[]);

/* 
 * Special functions for floating-point numbers.
 * See http://www.mpfr.org/mpfr-current/mpfr.html#Special-Functions
 */
value value_eint(value op);
value value_li2(value op);
value value_gamma(value op);
value value_lngamma(value op);
value value_zeta(value op);
value value_erf(value op);
value value_erfc(value op);

value value_srand(value seed);
value value_rand(value op);

value value_srand_arg(int argc, value argv[]);
value value_rand_arg(int argc, value argv[]);

/* 
 * Numeric iterators.
 */

/* Repeatedly calls (func), (op) times.
 */
value value_times(value *variables, value op, value func);

/* Calculates the summation over (op) of the result of (func). (op) 
 * should be an array or range. For example, summation(1..10, 2*n)
 * will calculate the summation from n=1 to 10 of 2*n.
 */
value value_summation(value *variables, value op, value func);

value value_times_arg(int argc, value argv[]);
value value_summation_arg(int argc, value argv[]);


#define VALUE_SIN 0
#define VALUE_COS 1
#define VALUE_TAN 2
#define VALUE_CSC 3
#define VALUE_SEC 4
#define VALUE_COT 5
#define VALUE_ASIN 6
#define VALUE_ACOS 7
#define VALUE_ATAN 8

#define VALUE_SINH 9
#define VALUE_COSH 10
#define VALUE_TANH 11
#define VALUE_CSCH 12
#define VALUE_SECH 13
#define VALUE_COTH 14
#define VALUE_ASINH 15
#define VALUE_ACOSH 16
#define VALUE_ATANH 17

// These are done with macros instead of functions because it is more concise.
#define value_sin(op) value_trig(op, 0)
#define value_cos(op) value_trig(op, 1)
#define value_tan(op) value_trig(op, 2)
#define value_csc(op) value_trig(op, 3)
#define value_sec(op) value_trig(op, 4)
#define value_cot(op) value_trig(op, 5)
#define value_asin(op) value_trig(op, 6)
#define value_acos(op) value_trig(op, 7)
#define value_atan(op) value_trig(op, 8)

#define value_sinh(op) value_trig(op, 9)
#define value_cosh(op) value_trig(op, 10)
#define value_tanh(op) value_trig(op, 11)
#define value_csch(op) value_trig(op, 12)
#define value_sech(op) value_trig(op, 13)
#define value_coth(op) value_trig(op, 14)
#define value_asinh(op) value_trig(op, 15)
#define value_acosh(op) value_trig(op, 16)
#define value_atanh(op) value_trig(op, 17)


/* 
 * 
 * String functions.
 * 
 */

/* Converts the first character to uppercase and every 
 * other character to lowercase.
 */
value value_capitalize(value op);

/* Returns a new string with the last character removed.
 */
value value_chop(value op);
value value_chop_now(value *op);

/* Converts an ASCII character to a number.
 */
value value_asc(value op);

/* Converts a number to an ASCII character.
 */
value value_chr(value op);

/* If op1 contains op2, returns true. Otherwise, returns false.
 */
int value_contains_p(value op1, value op2);
value value_contains_p_std(value op1, value op2);

/* If op1 ends with 2, returns true. Otherwise, returns false.
 */
int value_ends_with_p(value op1, value op2);
value value_ends_with_p_std(value op1, value op2);

/* Returns the index of op2 in op1. If not found, returns nil or -1.
 */
size_t value_index(value op1, value op2);
value value_index_std(value op1, value op2);

/* Inserts op2 into op1 at (index).
 */
value value_insert(value op1, value index, value op3);
value value_insert_now(value *op1, value index, value op3);

/* If op is an alphabetical character (a-zA-Z), returns true. Otherwise, 
 * returns false.
 */
int value_alpha_p(value op);
value value_alpha_p_std(value op);

/* If op is an alphanumeric character (a-zA-Z0-9), returns true. Otherwise, 
 * returns false.
 */
int value_alnum_p(value op);
value value_alnum_p_std(value op);

/* If op is a numeric character (0-9), returns true. Otherwise, returns 
 * false.
 */
int value_num_p(value op);
value value_num_p_std(value op);

/* Returns the length of the sequence.
 */
size_t value_length(value op);
value value_length_std(value op);

/* Strips whitespace from the left side of the string.
 */
value value_lstrip(value op);

/* Returns a sub-array with the first element at (start) and the last element 
 * at (end-1).
 */
value value_range(value op, value start, value end);

/* Returns a copy of (op1) where all instances of (op2) are replaced with (op3).
 */
value value_replace(value op1, value op2, value op3);
value value_replace_now(value *op1, value op2, value op3);

/* Reverses the order of op.
 */
value value_reverse(value op);
value value_reverse_now(value *op);

/* Strips whitespace from the right side of the string.
 */
value value_rstrip(value op);

value value_scan(value op1, value op2);

/* Splits string (op1) at every occurrence of (op2).
 */
value value_split(value op1, value op2);

/* If op1 starts with op2, returns true. Otherwise, returns false.
 */
int value_starts_with_p(value op1, value op2);
value value_starts_with_p_std(value op1, value op2);

/* Strips the whitespace from both sides of the string.
 */
value value_strip(value op);
value value_strip_now(value *op);

/* Converts every lowercase character to uppercase.
 */
value value_to_upper(value op);

/* Converts evey uppercase character to lowercase.
 */
value value_to_lower(value op);

value value_asc_arg(int argc, value argv[]);
value value_capitalize_arg(int argc, value argv[]);
value value_chop_arg(int argc, value argv[]);
value value_chop_now_arg(int argc, value argv[]);
value value_chr_arg(int argc, value argv[]);
value value_contains_p_arg(int argc, value argv[]);
value value_ends_with_p_arg(int argc, value argv[]);
value value_index_arg(int argc, value argv[]);
value value_insert_arg(int argc, value argv[]);
value value_insert_now_arg(int argc, value argv[]);
value value_alpha_p_arg(int argc, value argv[]);
value value_alnum_p_arg(int argc, value argv[]);
value value_num_p_arg(int argc, value argv[]);
value value_length_arg(int argc, value argv[]);
value value_lstrip_arg(int argc, value argv[]);
value value_range_arg(int argc, value argv[]);
value value_replace_arg(int argc, value argv[]);
value value_replace_now_arg(int argc, value argv[]);
value value_reverse_arg(int argc, value argv[]);
value value_reverse_now_arg(int argc, value argv[]);
value value_rstrip_arg(int argc, value argv[]);
value value_scan_arg(int argc, value argv[]);
value value_split_arg(int argc, value argv[]);
value value_starts_with_p_arg(int argc, value argv[]);
value value_strip_arg(int argc, value argv[]);
value value_strip_now_arg(int argc, value argv[]);
value value_to_upper_arg(int argc, value argv[]);
value value_to_lower_arg(int argc, value argv[]);

/* 
 * 
 * Regular expression functions.
 *
 */

/* Compiles the regular expression given in (regex) into (compiled).
 */
int compile_regex(regex_t *compiled, char *regex, int flags);


/* If (regex) matches (str), return true. Otherwise, returns false.
 */
int value_match_p(value regex, value str);
value value_match_p_std(value regex, value str);
value value_match_p_arg(int argc, value argv[]);

/* Returns the index in (str) first matched by (regex).
 */
regmatch_t value_match(value regex, value str);
regmatch_t value_match_str(value regex, char *str);
value value_match_std(value regex, value str);
value value_match_arg(int argc, value argv[]);


/* 
 * 
 * Array functions.
 * 
 */
 
value block_array_cast(value op);

/* Appends (op2) to the back of (op1). */
value value_append(value op1, value op2);
value value_append_now(value *op1, value op2);
value value_append_now2(value *op1, value *op2);

/* 
 * Returns the value in (op) at (index).
 * 
 * more: An array containing zero or more indices. Used for repeated calls. 
 *   value_at_ref(op, 0, { 2, 1 }, 2) is equivalent to op[0][2][1].
 * length: The length of (more).
 * 
 * If you only wish to call at() one time, pass in NULL and 0 for (more) and 
 * (length).
 */
value value_at(value op, value index, value more[], size_t length);
value * value_at_ref(value op, value index, value more[], size_t length);

/* Assigns the value in (op1) at (index) to (op2).
 */
value value_at_assign_do(value *variables, value *op1, value index, value more[], size_t length, value func, value op2);

// I see no need to implement these functions. See value_assign_add_arg(), etc.
value value_at_assign_add(value *op1, value index, value op2);
value value_at_assign_sub(value *op1, value index, value op2);
value value_at_assign_mul(value *op1, value index, value op2);
value value_at_assign_div(value *op1, value index, value op2);
value value_at_assign_mod(value *op1, value index, value op2);
value value_at_assign_and(value *op1, value index, value op2);
value value_at_assign_xor(value *op1, value index, value op2);
value value_at_assign_or(value *op1, value index, value op2);
value value_at_assign_shl(value *op1, value index, value op2);
value value_at_assign_shr(value *op1, value index, value op2);

/* Concatenates op1 and op2.
 */
value value_concat(value op1, value op2);
value value_concat_now(value *op1, value op2);

/* Doesn't modify (op2), but if you later modify (op2), the resulting (op1) 
 * from this function will be modified.
 */
value value_concat_now2(value *op1, value *op2);

/* Returns a new version of (op1) with the first instance of (op2) deleted.
 */
value value_delete(value op1, value op2);

/* Deletes the first instance of (op2) in (op1). If it existed, return true.
 * Otherwise, return false.
 */
value value_delete_now(value *op1, value op2);

value value_delete_all(value op1, value op2);

/* Deletes all instances of (op2) in (op1).
 */
value value_delete_all_now(value *op1, value op2);

value value_delete_at(value op, value index);

/* Deletes the element op[index] and returns it.
 */
value value_delete_at_now(value *op, value index);

/* Iterates over each element in (op) and calls (func) for each one. (func) 
 * must be a type that can be called with value_call().
 */
value value_each(value *variables, value op, value func);

/* Iterates through each index in (op) and calls (func) for each one. (func) 
 * must be a type that can be called with value_call().
 */
value value_each_index(value *variables, value op, value func);

/* If (op) contains no elements, returns true. Otherwise, returns false.
 */
int value_empty_p(value op);
value value_empty_p_std(value op);

/* Returns a new container of the same type as (op). Each element is passed 
 * as an argument to (func), and only elements for which (func) returns true 
 * are put into the new container.
 */
value value_filter(value *variables, value op, value func);
value value_filter_f(value op, int (*f)(value));

/* Returns the first element of (op) for which (func) returns true, or nil 
 * if no such value is found.
 */
value value_find(value *variables, value op, value func);

/* Maps an array of arrays to a single array, or a list of lists to a single 
 * list. Unlike flatten(), this only flattens one layer, so an array of arrays 
 * of arrays would end up as an array of arrays.
 */
value value_flatmap(value *variables, value op, value func);

/* If (op) is a container that contains other containers, flattens all the 
 * containers so that there is only one level of depth.
 */
value value_flatten(value op);
value value_flatten_now(value *op);
size_t value_private_flatten_recursive(value array[], value op, size_t index);
size_t value_private_total_length(value op);

/* Combine all elements of (op) by calculating block(prev, op[i]).
 */
value value_fold(value *variables, value op, value initial, value func);

value value_join(value op1, value op2);

/* Returns the last element of (op).
 */
value value_last(value op);

/* Returns a new array where each element is transformed by the function f().
 */
value value_map(value *variables, value op, value func);

/* Similar to map(), but drops all elements equal to (drop).
 */
value value_map_drop(value *variables, value op, value func, value drop);

/* Returns a new array with the last element removed.
 */
value value_pop(value op);

/* Removes the last element and returns it.
 */
value value_pop_now(value *op);

/* Shuffles (op) randomly.
 */
value value_shuffle(value op);
value value_shuffle_now(value *op);

/* Returns the number of items contained in op. If op is not a container, 
 * returns 1.
 */
size_t value_size(value op);
value value_size_std(value op);

/* 
 * Sorts an array or list.
 */
value value_sort(value op);
value value_sort_now(value *op);
value value_private_sort_pivot;
int value_private_sort_lt(value op);
int value_private_sort_eq(value op);
int value_private_sort_gt(value op);
int value_private_sort_recursive(value array[], int left, int right);
int value_private_sort_list(value *op);

value value_swap_now(value *op, size_t i, size_t j);

/* Removes all duplicate values in (op) while preserving the order.
 */
value value_uniq(value op);
value value_uniq_now(value *op);

/* Removes all duplicate values and sorts the array. This is more efficient than 
 * just removing unique elements.
 */
value value_uniq_sort(value op);
value value_uniq_sort_now(value *op);

value value_append_arg(int argc, value argv[]);
value value_append_now_arg(int argc, value argv[]);
value value_array_with_length_arg(int argc, value argv[]);

/* Takes at least three arguments: op1, index[, more], op2.
 */
value value_at_arg(int argc, value argv[]);
value value_at_assign_arg(int argc, value argv[]);
value value_at_assign_add_arg(int argc, value argv[]);
value value_at_assign_sub_arg(int argc, value argv[]);
value value_at_assign_mul_arg(int argc, value argv[]);
value value_at_assign_div_arg(int argc, value argv[]);
value value_at_assign_mod_arg(int argc, value argv[]);
value value_at_assign_and_arg(int argc, value argv[]);
value value_at_assign_xor_arg(int argc, value argv[]);
value value_at_assign_or_arg(int argc, value argv[]);
value value_at_assign_shl_arg(int argc, value argv[]);
value value_at_assign_shr_arg(int argc, value argv[]);

value value_concat_arg(int argc, value argv[]);
value value_delete_arg(int argc, value argv[]);
value value_delete_all_arg(int argc, value argv[]);
value value_delete_at_arg(int argc, value argv[]);
value value_delete_at_now_arg(int argc, value argv[]);
value value_each_arg(int argc, value argv[]);
value value_each_index_arg(int argc, value argv[]);
value value_empty_p_arg(int argc, value argv[]);
value value_filter_arg(int argc, value argv[]);
value value_find_arg(int argc, value argv[]);
value value_flatmap_arg(int argc, value argv[]);
value value_flatten_arg(int argc, value argv[]);
value value_flatten_now_arg(int argc, value argv[]);
value value_fold_arg(int argc, value argv[]);
value value_join_arg(int argc, value argv[]);
value value_last_arg(int argc, value argv[]);
value value_map_arg(int argc, value argv[]);
value value_map_now_arg(int argc, value argv[]);
value value_map_drop_arg(int argc, value argv[]);
value value_pop_arg(int argc, value argv[]);
value value_pop_now_arg(int argc, value argv[]);
value value_shuffle_arg(int argc, value argv[]);
value value_shuffle_now_arg(int argc, value argv[]);
value value_size_arg(int argc, value argv[]);
value value_sort_arg(int argc, value argv[]);
value value_sort_now_arg(int argc, value argv[]);
value value_uniq_arg(int argc, value argv[]);
value value_uniq_now_arg(int argc, value argv[]);
value value_uniq_sort_arg(int argc, value argv[]);
value value_uniq_sort_now_arg(int argc, value argv[]);

/* 
 * 
 * List functions.
 *
 */

/* Attaches (op1) to the front of (op2).
 */
value value_cons(value op1, value op2);
value value_cons_now(value op1, value *op2);

/* If (op1) is modified later, it will also modify the copy contained 
 * in (op2).
 */
value value_cons_now2(value *op1, value *op2);

/* Returns a list without the first (n) elements of (op). If 
 * (n) is greater than the length of the list, returns nil.
 */
value value_drop(value op, value n);
value value_drop_now(value *op, value n);

/* Returns the first element of the list.
 */
value value_head(value op);

/* Returns a list containing every element but the first one.
 */
value value_tail(value op);
value value_tail_now(value *op);

/* Returns a list containing the first (n) elements of (op). If 
 * (n) is greater than the length of the list, returns the whole 
 * list.
 */
value value_take(value op, value n);
value value_take_now(value *op, value n);

value value_cons_arg(int argc, value argv[]);
value value_cons_now_arg(int argc, value argv[]);
value value_drop_arg(int argc, value argv[]);
value value_head_arg(int argc, value argv[]);
value value_tail_arg(int argc, value argv[]);
value value_take_arg(int argc, value argv[]);

/* 
 * 
 * Hash functions. These are defined separately because internal function rely 
 * so heavily upon hashes.
 * 
 */

#define HASH_DEFAULT_CAPACITY 10 // If this is 0, bad things will happen.

/* Initializes a hash with the default capacity.
 */
value value_hash_init();

/* Initializes a hash with the given capacity.
 */
value value_hash_init_capacity(size_t capacity);

/* Clears (hash).
 */
void value_hash_clear(value *hash);

/* Returns the length of the array in (hash).
 */
size_t value_hash_length(value hash);

/* Returns the number of occupied buckets in (hash).
 */
size_t value_hash_occupied(value hash);

/* Returns the number of elements in (hash).
 */
size_t value_hash_size(value hash);

/* Resizes (hash) to be able to hold more buckets.
 */
value value_hash_resize(value *hash);

/* Puts copies of (key) and (val) into (hash).
 */
value value_hash_put(value *hash, value key, value val);

/* Puts (key) and (val) directly into (hash). If you modify (key) or (val), 
 * its value inside the hash will be changed.
 */
value value_hash_put_refs(value *hash, value *key, value *val);
value value_hash_put_refs_override(value *hash, value *key, value *val);
value value_hash_put_refs_generic(value *hash, value *key, value *val, int clear_p);

/* Converts (key) into a VALUE_STR and calls value_hash_put() on the new key and (val).
 */
value value_hash_put_str(value *hash, const char *key, value val);

/* Converts (key) into a VALUE_VAR and calls value_hash_put() on the new key and (val).
 */
value value_hash_put_var(value *hash, const char *key, value val);

/* Converts (key) and (value) into VALUE_STRs and puts them into (hash).
 */
value value_hash_put_str_str(value *hash, const char *key, const char *val);

/* If (key) exists inside of (hash), returns TRUE. Otherwise, returns FALSE.
 */
int value_hash_exists(value hash, value key);

/* Converts (key) into a VALUE_STR and determines if it exists inside of (hash).
 */
int value_hash_exists_str(value hash, char *key);

/* If (op) contains (val) as a value, returns TRUE. Otherwise, returns FALSE.
 */
int value_contains_value(value op, value val);
value value_contains_value_std(value op, value val);
value value_contains_value_arg(int argc, value argv[]);

/* 
 * Deletes (key) in (hash) and returns the deleted value.
 */
value value_hash_delete_at(value *hash, value key);

/* 
 * Deletes (key) in (hash).
 */
void value_hash_delete_at_void(value *hash, value key);

/* Returns a reference to the corresponding value for key (key) in (hash). If 
 * none is found, returns NULL.
 */
value * value_hash_get_ref(value hash, value key);
value value_hash_get(value hash, value key);
value * value_hash_get_ref_str(value hash, char *key);
value value_hash_get_str(value hash, char *key);
value * value_hash_get_pair_ref(value hash, value key);

/* Returns the key-value pair for key (key) in (hash).
 */
value value_hash_get_pair(value hash, value key);

int value_private_put_pair_in_bucket(value *bucket, value *key, value *val, int clear_p);
value value_private_find_pair_in_bucket(value bucket, value key);
size_t value_private_hash_function(value op);

int value_hash_print(value hash);
int value_hash_println(value hash);


/* 
 * Range functions.
 */

/* Constructs a range on [op1, op2].
 */
value value_range_to(value op1, value op2);

/* Constructs a range on [op1, op2).
 */
value value_range_until(value op1, value op2);

value value_range_to_arg(int argc, value argv[]);
value value_range_until_arg(int argc, value argv[]);


/* 
 * Block and function functions.
 */

void value_add_to_line_queue(value op); // This function is defined in interpreter.c.

/* Call (func) and pass (argv) as the arguments. Must be a callable data 
 * type: BIF, UDF, or BLK.
 */
value value_call(value *variables, value func, int argc, value argv[]);

/* Takes a sexp with a BIF as the first element and calls value_bifcall().
 */
value value_bifcall_sexp(value *variables, value *ud_functions, value sexp);

/* Call a built-in function (op) with arguments (argv).
 */
value value_bifcall(value op, int argc, value argv[]);

/* Call a user-defined function.
 * variables: Pointer to a hash containing whatever the variable scope is at the 
 *   time of the function call.
 * op: User-defined function to be called.
 * argv: Argument list.
 */
value value_udfcall(value *variables, value op, int argc, value argv[]);

/* Defines a user-defined function.
 */
value value_def(value *variables, value name, value vars, value body);

value value_lambda(value *variables, value vars, value body);

/* Defines a macro.
 */
value value_defmacro(value *variables, value name, value vars, value body);

/* Do not evaluate (op), except for anything inside a call to (dq) or (dv). 
 * (dq x): Evaluates x.
 * (dv x): If x is a variable, returns the contents of x. Otherwise, returns x.
 */
value value_quote(value *variables, value op);

value value_def_arg(int argc, value argv[]);
value value_lambda_arg(int argc, value argv[]);

/* Do not evaluate (op), including anything inside a call to (dq) or (dv).
 */
value value_quote_all_arg(int argc, value argv[]);

value value_quote_arg(int argc, value argv[]);
value value_dequote_arg(int argc, value argv[]);
value value_devar_arg(int argc, value argv[]);
value value_eval_arg(int argc, value argv[]);

/* 
 * Control functions. These take s-expressions as arguments, which are then evaluated.
 */

value value_break_arg(int argc, value argv[]);
value value_continue_arg(int argc, value argv[]);
value value_yield_arg(int argc, value argv[]);
value value_return_arg(int argc, value argv[]);
value value_exit_arg(int argc, value argv[]);

/* If (condition) is true, evaluates and returns (body). Otherwise, evaluates and returns 
 * (else_body). That is, unless (reverse) is true, in which case (body) and (else_body) 
 * are reversed -- this is like an (unless) call.
 */
value value_if(value *variables, value condition, value body, value else_body, int reverse);

/* Calls (body) as long as (condition) is true. That is, unless (reverse) is true, in 
 * which case it calls (body) as long as (condition) is false.
 */
value value_while(value *variables, value condition, value body, int reverse);

/* Calls a (body) with one or more conditions, which acts like the switch statement in C 
 * or Java. However, it is more flexible. If (val) and (body) are both given, then it acts 
 * almost exactly like a switch statement in C or Java. If (val) contains a block and (body) 
 * is nil, then (val) is put into body and each condition acts as a single part of an if-else 
 * chain.
 * 
 * In the body, the :if symbol followed by two values serves as a single condition. If (val) 
 * is given then the first value is compared to (val). If they are equal, the second value is 
 * evaluated and returned. If (val) is not given, then it is treated as an if expression. 
 * The first value is evaluated, and if it returns a true value then the second value is 
 * evaluated and returned.
 * 
 * The other possible symbol is the :else symbol, which is followed by a single value. The 
 * value will always be evaluated and returned.
 * 
 * If the execution of an :if symbol and its two values returns false, execution moves on to 
 * the next :if symbol. If an :else symbol is found, its value is evaluated and returned, and 
 * execution ends. If every :if symbol returns false and there is no :else symbol, nil is 
 * returned.
 * 
 * examples: 
 * 
 * switch (x) (:if 0 "zero" :if 1 "one" :if 2 "two" :else "I don't know")
 *   If x == 0, returns "zero". If x == 1, returns "one". If x == 2, returns "two". Otherwise, 
 *   returns "I don't know".
 * 
 * switch (x) (:if "zero" 0)
 *   If x == "zero", returns 0. Otherwise, returns nil.
 * 
 * switch (:if (x type != Integer) "not an integer" :if (x % 2 == 0) "even" :else "odd")
 *   Acts as a series of if expressions. Equivalent to
 *     if (x type != Integer) "not an integer" (if (x % 2 == 0) "even" "odd")
 * 
 */
value value_switch(value *variables, value val, value body);

/* A sophisticated loop that at its most simple works like a (while) loop, but can do much 
 * more.
 */
value value_for(value *variables, value condition, value body);

/* Evaluates every value in (todo), and returns the last value evaluated.
 */
value value_do(value *variables, value todo[], size_t length);

value value_comma(value op1, value op2);

value value_if_arg(int argc, value argv[]);
value value_unless_arg(int argc, value argv[]);
value value_while_arg(int argc, value argv[]);
value value_until_arg(int argc, value argv[]);
value value_switch_arg(int argc, value argv[]);
value value_for_arg(int argc, value argv[]);
value value_do_both_arg(int argc, value argv[]);
value value_do_all_arg(int argc, value argv[]);
value value_comma_arg(int argc, value argv[]);





/* 
 * Exception functions.
 */
exception exception_init(exception *parent, char *name);
value value_throw(exception op, char *description);


/*
 * 
 * General-purpose output functions.
 * 
 */

double value_get_double(value op);
value value_get_from_index(value op, size_t index);
long value_get_long(value op);

char * value_get_string(value op);

/* 
 * Prints op as a string into the given buffer. This is the core function 
 * used by value_to_string(), value_print(), and most other output functions.
 * 
 * format: A format string that tells how to output (op). See value_printf() 
 *         for a detailed description of the possible formats. In this case, 
 *         no preceding '%' should be included in the format string.
 * 
 * 
 * Return Values
 * 
 * 0: op was successfully written to the buffer.
 * VALUE_ERROR: buffer was not large enough to hold op. The contents of buffer 
 * are undefined.
 * 
 */
int value_put(char buffer[], size_t length, value op, char *format);

/* Converts a value to a string. This is unsafe, as it does not allocate any 
 * memory so the contents of the returned string may change or be erased.
 */
char * value_to_string(value op);
char * value_to_string_base(value op, value base);

/* Allocates memory for the string, and is therefore safer than to_string().
 */
char * value_to_string_safe(value op);
unsigned long value_get_ulong(value op);

/* Print out a single value using the %v format specifier. See value_printf().
 */
int value_print(value op);
int value_println(value op);

/* 
 * Writes a formatted string to the standard output. A format tag looks like 
 * this: 
 * 
 * %[width][.precision]type
 * 
 * (width) indicates the maximum number of characters to be output. It is 
 * not guaranteed that exactly (width) characters will be output, even if 
 * the string would be longer than (width); but it is guaranteed that no 
 * more than (width) characters will be output. If the output string would 
 * have been longer than (width), an ellipsis "(...)" is used to indicate 
 * which part of the string was not printed. It is not guaranteed that the 
 * printed string will ever be shorter than 5 characters.
 * 
 * (precision) indicates the precision of a floating-point number. If the 
 * value is not a float, this is ignored. If it is a container, every float 
 * that is printed out is given this precision.
 * 
 * Although the format tag looks a lot like in C, it works differently --  
 * which you would expect from a dynamically-typed interface. The following  
 * types are available.
 * 
 * e: Prints floats in exponential notation. For non-floats, acts as 'v'.
 * s: The simplest format. Prints out the parameter the way it would look if  
 *    you used to_s().
 * v: Prints the value of the parameter. Works like 's', but it is printed 
 *    out more as a direct value. For example, the string "hello" is printed 
 *    as hello instead of as "hello" (with quotes). This is recommended for 
 *    most purposes. It is what print() and println() use.
 * b, o, x: Prints numbers as binary, octal, hex rather than decimal. 
 *    For non-numbers, acts as 'v'.
 * 
 * There are also two possible modifiers, which precede the type.
 * t: Prints the type of the value in addition to the value itself. By default, 
 *    restricts the output to 72 characters.
 * r: Recursive type printing. Acts like t, but also prints the types of any 
 *    values inside of the first value. For example, it will print the type of 
 *    each element in an array.

 */
int value_printf(const char *format, ...);
int value_fprintf(FILE *fp, const char *format, ...);
int value_vfprintf(FILE *fp, const char *format, va_list ap);
int value_sprintf(char *str, const char *format, ...);

/* 
 * This is the output function that does all the work. Every other output function 
 * eventually calls this one. It's a little bit complex, but it saves effort in 
 * the long run.
 * 
 * fp: File stream to print to. If NULL, output goes to (str) instead.
 * str: String to print to. Only used if (fp) is NULL.
 * format: Format string that determines what to print.
 * argc: Number of arguments in argv.
 * argv: Arguments that are printed by (format) tags. If NULL, (ap) is used instead.
 * ap: Variable argument list for arguments that are printed by (format) tags. Only 
 *     used if (argv) is NULL.
 * 
 * This function may be called with either an array as an argument list or a va_list.
 */
int value_vprintf_generic(FILE *fp, char *str, const char *format, int argc, value *argv, va_list ap);

/* 
 * Prints out an error message.
 * 
 * severity: How many errors to add to the error count. This is most commonly 0 or 1. 
 * If 0, then the error is considered only a warning.
 * 
 * value_error's format supports everything that value_printf supports. In addition, %d is 
 * a C int and %c is a C string (NOT a char -- %s was already taken by value_printf, and if 
 * you want to print out a character in an error message for some reason then you'll just 
 * have to figure out some other way to do it).
 */ 
int value_error(int severity, char *format, ...);

value value_print_arg(int argc, value argv[]);
value value_println_arg(int argc, value argv[]);
value value_printf_arg(int argc, value argv[]);

/* Prints every value in (values), separated by spaces.
 */
int print_values(value values[], size_t length);

