/*
 *  number.c
 *  Calculator
 *
 *  Created by Michael Dickens on 12/14/09.
 *  Copyright 2009 The Khabele School. All rights reserved.
 *
 */

#include "number.h"

Number nm_init()
{
	Number result;
	result.set = ea_init_with_length(NM_DEFAULT_LENGTH);
	result.set.length = 1;
	return result;
}

Number nm_init_with_length(int length)
{
	if (length < 1) {
		printf("Error: in nm_init_with_length(), length must be greater than 0, but instead is %d.\n", length);
		exit(1);
	}
		
	Number result;
	result.set = ea_init_with_length(length);
	result.set.length = length;
	return result;
}

Number nm_init_with_value(E_TYPE x)
{
	E_TYPE arr[NM_DEFAULT_LENGTH];
	arr[0] = x % NM_CAP_VAL;
	
	int i;
	for (i = 1; i < NM_DEFAULT_LENGTH; i++)
		arr[i] = 0;
	Number result;
	result.set = ea_init_with_array(arr, NM_DEFAULT_LENGTH);
	result.set.length = 1;
	return result;	
}

Number nm_copy(Number a)
{
	a.set = ea_copy(a.set);
	return a;
}

int nm_compare(Number a, Number b, int less, int equal, int greater)
{
	if (a.set.length < b.set.length)
		return less;
	if (a.set.length > b.set.length)
		return greater;
	
	int i;
	EA_EACH_INDEX_DOWN(a.set, i) {
		if (a.set.core[i] < b.set.core[i])
			return less;
		if (a.set.core[i] > b.set.core[i])
			return greater;
	}
	
	return equal;
}

Number nm_add(Number a, Number b)
{
	a = nm_copy(a);
	int i; E_TYPE carry;
	int b_len_minus_1 = b.set.length - 1;
	for (i = 0, carry = 0; i < b.set.length || carry != 0; i++) {
		ULL tmp = (ULL)a.set.core[i] + b.set.core[i] + carry;
		if (tmp >= NM_CAP_VAL) {
			// tmp will never be more than NM_CAP_VAL too big, so subtraction (and not modulo) will be sufficient.
			a.set.core[i] = tmp - NM_CAP_VAL;
			carry = 1; // (tmp / NM_CAP_VAL) will always equal 1.
		} else {
			a.set.core[i] = tmp;
			carry = 0;
		}
		
		if (i == a.set.length && (carry > 0 || i < b_len_minus_1))
			nm_overflow(&a);
	}
	
	return a;
}

/* Add b to a. This has not been checked for errors. It is very likely that it will break when *a and *b are both 
 * references to the same data structure.
 */
void nm_add_now(Number *a, Number *b)
{
	int i; E_TYPE carry;
	
	if (a->set.length == 0)
		nm_overflow(a);
	
	int b_len_minus_1 = b->set.length - 1;
	for (i = 0, carry = 0; i < b->set.length || carry != 0; i) {
		ULL tmp = (ULL)a->set.core[i] + b->set.core[i] + carry;
		if (tmp >= NM_CAP_VAL) {
			// tmp will never be more than NM_CAP_VAL too big, so subtraction (and not modulo) will be sufficient.
			a->set.core[i] = tmp - NM_CAP_VAL;
			carry = 1; // (tmp / NM_CAP_VAL) will always equal 1.
		} else {
			a->set.core[i] = tmp;
			carry = 0;
		}
		
		if (i == a->set.length-1 && (carry > 0 || i < b_len_minus_1))
			nm_overflow(a);
	}
}

Number nm_sub(Number a, Number b)
{
	a = nm_copy(a);
	if (nm_is_less_than(a, b)) {
		// The subtraction will result in a negative number. To fix this, calculate MAX_VAL - (b - a) = -(b - a) = -b + a = a - b
		
	} else {
		int i; E_TYPE carry;
		int b_len_minus_1 = b.set.length - 1;
		for (i = b_len_minus_1; 0 <= i; --i) {
			
		}
	}
	
	return a;
}

/* Uses a long multiplication algorithm, very similar to what humans do.
 */
Number nm_mul(Number a, Number b)
{
	Number result = nm_init();
	int i;
	EA_EACH_INDEX_DOWN(b.set, i) {
		nm_private_mulint(&result, a, b.set.core[i], i);
	}
	return result;
}

void nm_private_mulint(Number *result, Number a, E_TYPE x, int shift)
{
	Number c = nm_init_with_length(shift+1);

	int a_len_minus_1 = a.set.length - 1;
	E_TYPE carry; int i;
	for (i = 0, carry = 0; i < a.set.length || carry != 0; ++i) {
		int i_plus_shift = i + shift;
		ULL tmp = (ULL)a.set.core[i] * x + carry;
		if (x != 0 && tmp >= NM_CAP_VAL) {
			// The above if statement means that there will be carrying. The below statements try to handle it.
			carry = (UINT)(tmp / NM_CAP_VAL);
			c.set.core[i_plus_shift] = (UINT)(tmp % NM_CAP_VAL);
		} else {
			// There will not be carrying.
			c.set.core[i_plus_shift] = (a.set.core[i] * x) + carry;
			carry = 0;
		}
		
		if (i_plus_shift == c.set.length-1 && (carry > 0 || i < a_len_minus_1))
			nm_overflow(&c);
		
	}
	nm_add_now(result, &c);
}

Number nm_pow(Number a, Number b)
{
	// Check which power of 2 (a) is, or whether it is a power of 2 at all.
}

void nm_print(Number a)
{	
	int is_started = FALSE;
	E_TYPE x; int i;
	EA_EACH_DOWN(a.set, x, i) {
		if (is_started)
			nm_private_caseprint(x);
		else
			if (x != 0) {
				is_started = TRUE;
				printf("%u", x);
			} else if (i == 0) {
				// If every index is zero, just print the first index.
				printf("%u", x);
			}
	}
	
	printf("\n");
}

void nm_print_uniform(Number a)
{
	E_TYPE x; int i;
	EA_EACH_DOWN(a.set, x, i) {
		nm_private_caseprint(x);
	}
	printf("\n");
}

/* Prints x, assuming that x is an unsigned integer. */
void nm_private_caseprint(E_TYPE x)
{
	switch (NM_LOG10_MAX_VAL) {
		case 1:
			printf(" %01u", x);
			break;
		case 2:
			printf(" %02u", x);
			break;
		case 3:
			printf(" %03u", x);
			break;
		case 4:
			printf(" %04u", x);
			break;
		case 5:
			printf(" %05u", x);
			break;
		case 6:
			printf(" %06u", x);
			break;
		case 7:
			printf(" %07u", x);
			break;
		case 8:
			printf(" %08u", x);
			break;
		case 9:
			printf(" %09u", x);
			break;
		/* Goes up to 9 because log10(pow(2, 32)) ~= 9. */
	}
}

void nm_overflow(Number *a)
{
	ea_append_zero_now(&(a->set));
}