/*
 *  number.h
 *  Calculator
 *
 *  Created by Michael Dickens on 12/14/09.
 *  Copyright 2009 The Khabele School. All rights reserved.
 *
 */

/*

Number represents a number of arbitrary size. Smaller cells are to the left -- that is, set.core[0] is the smallest and set.core[set.length-1] is the largest.

*/
#include "earray.h"

#define NM_CAP_VAL 1000000000 // The greatest power of 10 that is still less than 2**32.
#define NM_MAX_VAL  999999999
#define NM_LOG2_MAX_VAL 29//.897352853986263
#define NM_LOG10_MAX_VAL 9
#define NM_DEFAULT_LENGTH 4
#define ULL unsigned long long
#define UINT unsigned int
#define UINTSIZE (sizeof(unsigned int))

typedef struct {
	EArray set;
} Number;

Number nm_init();
Number nm_init_with_length(int length);
Number nm_init_with_value(E_TYPE x);

int nm_compare(Number a, Number b, int less, int equal, int greater);
#define nm_is_equal(a, b)						nm_compare(a, b, 0, 1, 0)
#define nm_is_not_equal(a, b)					nm_compare(a, b, 1, 0, 1)
#define nm_is_less_than(a, b)					nm_compare(a, b, 1, 0, 0)
#define nm_is_greater_than(a, b)				nm_compare(a, b, 0, 0, 1)
#define nm_is_less_than_or_equal_to(a, b)		nm_compare(a, b, 1, 1, 0)
#define nm_is_greater_than_or_equal_to(a, b)	nm_compare(a, b, 0, 1, 1)

Number nm_add(Number a, Number b);
void nm_add_now(Number *a, Number *b);
Number nm_sub(Number a, Number b);
Number nm_mul(Number a, Number b);
void nm_private_mulint(Number *result, Number a, E_TYPE x, int shift);


Number nm_pow(Number a, Number b);

void nm_print(Number a);
void nm_print_uniform(Number a);
void nm_private_caseprint(E_TYPE x);
void nm_overflow(Number *a);
