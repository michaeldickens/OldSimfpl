///*
// *  numbers.h
// *  Calculator
// *
// *  Created by Michael Dickens on 12/3/09.
// *  Copyright 2009 The Khabele School. All rights reserved.
// *
// */
// 
///*
//
//Number represents a number of virtually arbitrary size. It is not unlimited, since it must be preset by #define NUMBER_LEN.
//
//Most functions take two arguments. Most perform the operation with the first and second arguments and store the result in the second, without changing the 
//first. The exception is when the function is obviously one-directional, such as with division, in which case the first number is divided by the second number 
//and the result is stored in the first.
//
//Functions with three arguments store the result in the third; functions with only one argument usually do not modify it.
//
//*/
//
//#include <stdio.h>
//#include <math.h>
//#include <string.h>
//#include <limits.h>
//
//#define TRUE  1
//#define FALSE 0
//
//#define NUMBER_LEN 200
//#define MAX_VAL 1000000000 // The greatest power of 10 that is still less than 2**32.
//#define LOG2_MAX_VAL 29//.897352853986263
//#define INTSIZE (sizeof(unsigned int))
//
///* Represents an integer. Lower bits are to the left: the lowest bit is at 0, etc. This direction is chosen for efficiency of implementation. */
//typedef struct {
//	unsigned int core[NUMBER_LEN]; // Up to NUMBER_LEN * sizeof(int) bits
//	int size;
//} Number;
//
//
//void nm_inc(Number *a);
//void nm_add_int(int x, Number *a);
//void nm_add(Number *a, Number *b);
//void nm_sub(Number *a, Number *b);
//void nm_mul_by_int(int x, Number *a);
//void nm_mul(Number *a, Number *b);
//void nm_private_mulint(Number *a, unsigned int x, int shift);
//void nm_div(Number *c, Number *b);
//void nm_mod(Number *c, Number *b);
//
//void nm_and(Number *a, Number *b);
//void  nm_or(Number *a, Number *b);
//void nm_xor(Number *a, Number *b);
//void nm_not(Number *a);
//void nm_shl(Number *a, unsigned int x);
//void nm_shr(Number *a, unsigned int x);
//void nm_cshl(Number *a, unsigned int x);
//void nm_cshr(Number *a, unsigned int x);
//void nm_rotl(Number *a, unsigned int x);
//void nm_rotr(Number *a, unsigned int x);
//void nm_subnum(Number *a, Number *b, int start, int end);
//
//void nm_init(Number *a);
//void nm_clear(Number *a);
//void nm_set_as_max(Number *a);
//void nm_set(Number *a, unsigned long long x);
//void nm_calc_size(Number *a);
//void nm_copy(Number *a, Number *b);
//
//int nm_private_do_comparison(Number *a, Number *b, int less, int greater, int equal);
//int nm_is_equal(Number *a, Number *b);
//int nm_is_not_equal(Number *a, Number *b);
//int nm_is_less_than(Number *a, Number *b);
//int nm_is_greater_than(Number *a, Number *b);
//int nm_is_less_or_equal(Number *a, Number *b);
//int nm_is_greater_or_equal(Number *a, Number *b);
//int nm_print(Number *a);
//int print_uniform(Number *a, int digits);
//
//void nm_number_overflow(Number *a);
//void nm_sqrt(Number *a);
//void nm_pow(Number *a, Number *b, Number *c);
//
//
