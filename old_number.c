///*
// *  numbers.c
// *  Calculator
// *
// *  Created by Michael Dickens on 12/3/09.
// *  Copyright 2009 The Khabele School. All rights reserved.
// *
// */
//
//#include "old_number.h"
//
///* Increments *a. */
//void nm_inc(Number *a)
//{
//	int i, carry = 1, next_carry = 0;
//	for (i = 0; i < a->size || carry != 0; i++) {
//		if (i < a->size) {
//			if((long long)a->core[i] + carry >= MAX_VAL) {
//				next_carry = 1;
//				a->core[i] = 0;
//			} else {
//				next_carry = 0;
//				a->core[i] += carry;
//			}
//		} else {
//			if ((long long)a->core[i] + carry >= MAX_VAL) {
//				next_carry = 1;
//				a->core[i] = 0;
//			} else {
//				next_carry = 0;
//				a->core[i] += 1;
//				if (a->size > NUMBER_LEN) nm_number_overflow(a);
//			}
//			if (i >= a->size) a->size++;
//		}
//		carry = next_carry;
//	}
//}
//
///* Could be faster, but it works. */
//void nm_add_int(int x, Number *a)
//{
//	Number b;
//	nm_set(&b, x);
//	nm_add(&b, a);
//}
//
///* Add the contents of *a to *b, preserving *a. */
//void nm_addstuff(Number *a, Number *b)
//{
//	int i;
//	int carry = 0;
//	for (i = 0; i < a->size || carry != 0; i++) {
//		if (i < a->size) {
//			b->core[i] = (b->core[i] + a->core[i] + carry) % MAX_VAL;
//			if ((long long)a->core[i] + b->core[i] + carry >= MAX_VAL)
//				carry = 1;
//			else carry = 0;
//		} else {
//			b->core[i] = (b->core[i] + carry) % MAX_VAL;
//			if (b->size > NUMBER_LEN) nm_number_overflow(b);
//			if ((long long)b->core[i] + carry >= MAX_VAL)
//				carry = 1;
//			else carry = 0;
//		}
//		if (i >= b->size) b->size++;
//	}
//}
//
///* Add the contents of *a to *b and store the result in *a, preserving *b. */
//void nm_sub(Number *a, Number *b)
//{
//	if (nm_is_less_than(b, a)) {
//		/* At this point, we are guaranteed that a <= b, so we should have no subtraction problems. */
//		int i;	
//		for (i = 0; i < a->size; i++) {
//			if (a->core[i] >= b->core[i]) {
//				a->core[i] -= b->core[i];
//			} else {
//				int j = 0;
//				while (a->core[j] == 0) j++;
//				a->core[j]--;
//				a->core[i] = MAX_VAL - (b->core[i] - a->core[i]);
//			}
//		}
//	
//	} else {
//		/* We are going to end up with a negative number. The solution is to subtract a from b, and then subtract the result from 
//		 * the greatest possible number.
//		 */
//		Number p, q;
//		nm_copy(a, &p);
//		nm_copy(b, &q);
//		nm_sub(&q, &p);
//		nm_set_as_max(a);
//		nm_sub(a, &q);    // This and the above two lines calculate: MAXIMUM - (q - p)
//		nm_inc(a); // When you subtract the previously described result from the maximum value, the result you get is one too low.
//	}
//	
//}
//
///* This could be a lot faster, but it works. */
//void nm_mul_by_int(int x, Number *a)
//{
//	Number b;
//	nm_set(&b, x);
//	nm_mul(&b, a);
//}
//
///* Uses a long multiplication algorithm, very similar to what humans do.
//*/
//void nm_mul(Number *a, Number *c)
//{
//	int i;
//	for (i = 0; i < a->size; i++) {
//		nm_private_mulint(c, a->core[i], i);
//	}
//	nm_calc_size(c);
//}
//
//void nm_private_mulint(Number *a, unsigned int x, int shift)
//{
//	unsigned int carry = 0;
//    int i;
//	for (i = shift; i < a->size || carry != 0; i++) {
////		if ((long long)a->core[i] * x + carry >= MAX_VAL) {
//		if (x != 0 && a->core[i] >= ((unsigned int)MAX_VAL - carry) / x
//				|| carry >= MAX_VAL) {			
//			// The above if statement means that there will be overflow. The below statements try to handle it.
//			long long tmp = (long long)a->core[i] * x + carry;
//			carry = (unsigned int)(tmp / MAX_VAL);
//			a->core[i] = (unsigned int)(tmp % MAX_VAL);
//		} else {
//			// There will not be overflow.
//			a->core[i] = (a->core[i] * x) + carry;
//			carry = 0;
//		}
//		
//		if (i == a->size - 1 && carry > 0)
//			if (a->size >= NUMBER_LEN) nm_number_overflow(a);
//			else a->size++;
//	}
//}
//
///* Divides c by b and stores the quotient in c and the remainder in b. */
//void nm_div(Number *a, Number *b)
//{
//	Number dividend; nm_copy(a, &dividend); nm_clear(a);
//	Number divisor; nm_copy(b, &divisor); nm_clear(b);
//	Number quotient; nm_init(&quotient);
//	Number remainder; nm_init(&remainder);
//		
//	if (dividend.size - divisor.size < 0) {
//		nm_clear(a); // The quotient is zero
//		nm_copy(&dividend, &divisor); // The remainder is (a)
//		return;
//	}
//	
//	int offset = dividend.size - divisor.size;
//	nm_cshl(&divisor, offset);
//	
//	Number zero; nm_init(&zero); nm_clear(&zero);
//	
//	if (nm_is_equal(&divisor, &zero)) {
//		printf("Error: Number division by zero\n");
//		exit(1);
//	}
//	
//	while (nm_is_greater_than(&dividend, &zero)) {
//		nm_inc(&quotient);
//		nm_sub(&dividend, &divisor);
//	}
//	
//	nm_cshl(&quotient, offset);
//	
//	nm_copy(&quotient, a);
//	nm_copy(&divisor, b);
//}
//
//void nm_and(Number *a, Number *b)
//{
//	int i;
//	int max = a->size > b->size ? a->size : b->size;
//	for (i = 0; i < max; i++) {
//		b->core[i] &= a->core[i];
//	}
//}
//
//void nm_or(Number *a, Number *b)
//{
//	int i;
//	int min = a->size > b->size ? b->size : a->size;
//	for (i = 0; i < min; i++) {
//		b->core[i] |= a->core[i];
//	}
//}
//
//void nm_xor(Number *a, Number *b)
//{
//	int i;
//	int min = a->size > b->size ? b->size : a->size;
//	for (i = 0; i < min; i++) {
//		b->core[i] ^= a->core[i];
//	}
//}
//
//void nm_not(Number *a)
//{
//	int i;
//	for (i = 0; i < NUMBER_LEN; i++) {
//		a->core[i] = ~a->core[i];
//	}
//}
//
///* Shift (x) cells up. This pretends that left is bigger, even though right is actually bigger. */
//void nm_cshl(Number *a, unsigned int x)
//{
//	int i;
//	for (i = NUMBER_LEN-1; i >= 0; i--) {
//		if (i+x < NUMBER_LEN) a->core[i+x] = a->core[i];
//		if (x != 0) a->core[i] = 0;
//	}
//	
//	a->size += x;
//	while (a->size >= NUMBER_LEN) a->size -= NUMBER_LEN;
//}
//
//void nm_cshr(Number *a, unsigned int x)
//{
//	int i;
//	for (i = a->size-1; i >= 1; i--) {
//		// This doesn't say if (i-x >= 0) because unsigned ints will never be negative, only too big.
//		if (i-x < NUMBER_LEN) a->core[i-x] = a->core[i];
//		if (x != 0) a->core[i] = 0;
//	}
//	
//	a->size -= x;
//	while (a->size < 1) a->size += NUMBER_LEN;
//}
//
//void nm_subnum(Number *a, Number *b, int start, int end)
//{
//	b->size = end - start;
//	int i;
//	for (i = 0; i < b->size; i++) {
//		b->core[i] = a->core[i+start];
//	}
//}
//
//void nm_init(Number *a)
//{
//	int i;
//	for (i = 0; i < NUMBER_LEN; i++)
//		a->core[i] = 0;
//	a->size = 1;
//}
//
//void nm_clear(Number *a)
//{
//	int i;
//	for (i = 0; i < a->size; i++)
//		a->core[i] = 0;
//	a->size = 1;
//}
//
//void nm_set_as_max(Number *a)
//{
//	int i;
//	for (i = 0; i < NUMBER_LEN; i++)
//		a->core[i] = MAX_VAL - 1;
//	
//	a->size = NUMBER_LEN;
//}
//
//void nm_set(Number *a, unsigned long long x)
//{
//	nm_clear(a);
//	int i;
//	for (i = 0; x > 0; i++) {
//		a->core[i] = (unsigned int)x % MAX_VAL;
//		x /= MAX_VAL;
//	}
//	
//	a->size = i;
//
//	for ( ; i < NUMBER_LEN; i++)
//		a->core[i] = 0;
//}
//
//void nm_calc_size(Number *a)
//{
//	int i;
//	a->size = 1;
//	for (i = 0; i < NUMBER_LEN; i++)
//		if (a->core[i] > 0)
//			a->size = i + 1;
//}
//
///* Copies the *a onto *b. */
//void nm_copy(Number *a, Number *b)
//{
//	b->size = a->size;
//	int i;
//	for (i = 0; i < NUMBER_LEN; i++)
//		b->core[i] = a->core[i];
//}
//
//int nm_private_do_comparison(Number *a, Number *b, int less, int greater, int equal)
//{
//	int i;
//
//	for (i = a->size-1; i >= 0; i--) {
//		if (a->core[i] < b->core[i])
//			return less;
//		if (a->core[i] > b->core[i])
//			return greater;
//	}
//	
//	return equal;
//
//}
//
//int nm_is_equal(Number *a, Number *b)
//{
//	return nm_private_do_comparison(a, b, FALSE, FALSE, TRUE);
//}
//
//int nm_is_not_equal(Number *a, Number *b)
//{
//	return nm_private_do_comparison(a, b, TRUE, TRUE, FALSE);
//}
//
//int nm_is_less_than(Number *a, Number *b)
//{
//	return nm_private_do_comparison(a, b, TRUE, FALSE, FALSE);
//}
//
//int nm_is_greater_than(Number *a, Number *b)
//{
//	return nm_private_do_comparison(a, b, FALSE, TRUE, FALSE);
//}
//
//int nm_is_less_or_equal(Number *a, Number *b)
//{
//	return nm_private_do_comparison(a, b, TRUE, FALSE, TRUE);
//}
//
//int nm_is_greater_or_equal(Number *a, Number *b)
//{
//	return nm_private_do_comparison(a, b, FALSE, TRUE, TRUE);
//}
//
//int nm_print(Number *a)
//{
//	int logmax = log10(MAX_VAL);
//	if (pow(10, logmax) < MAX_VAL)
//		++logmax;
//
//	int i;
//	for (i = a->size - 1; i >= 0; i--) {
//		/* There has to be an easier way to do this. */
//		switch (logmax) {
//			case 1:
//				printf(" %01u", a->core[i]);
//				break;
//			case 2:
//				printf(" %02u", a->core[i]);
//				break;
//			case 3:
//				printf(" %03u", a->core[i]);
//				break;
//			case 4:
//				printf(" %04u", a->core[i]);
//				break;
//			case 5:
//				printf(" %05u", a->core[i]);
//				break;
//			case 6:
//				printf(" %06u", a->core[i]);
//				break;
//			case 7:
//				printf(" %07u", a->core[i]);
//				break;
//			case 8:
//				printf(" %08u", a->core[i]);
//				break;
//			case 9:
//				printf(" %09u", a->core[i]);
//				break;
//			/* Goes up to 9 because log10(pow(2, 32)) ~= 9. */
//		}
//	}
//	printf("\n");
//	return 0;
//}
//
///* Print the first (digits) digits of a. */
//int nm_print_uniform(Number *a, int digits)
//{
//	int logmax = log10(MAX_VAL);
//	if (pow(10, logmax) < MAX_VAL)
//		++logmax;
//	
//	if (digits >= NUMBER_LEN * logmax)
//		digits = NUMBER_LEN * logmax - 1;
//	
//	int i;
//	for (i = NUMBER_LEN - 1; digits >= 0; i--) {
//		if (digits >= i * log10(MAX_VAL))
//			printf("%u ", a->core[i]);
//		else
//			printf("%u ", a->core[i] % MAX_VAL);
//		digits -= logmax;
//	}
//	printf("\n");
//	
//	return 0;
//}
//
///* Decides what to do if there is number overflow. */
//void nm_number_overflow(Number *a)
//{
////	printf("err: overflow\n");
//	a->size = 1;
////	exit(1);
//}
//
///* Calculates sqrt(a). */
//void nm_sqrt(Number *a)
//{
//	Number guess, prev_guess; nm_init(&guess); nm_init(&prev_guess);
//	nm_set(&guess, 1);
//	
//}
//
///* Calculates pow(a, b) and stores the result in c. */
//void nm_pow(Number *a, Number *b, Number *c)
//{
//	nm_copy(a, c);
//	/* (i) starts at 1 instead of 0. The most obvious way to do this is to set (c) to 1 and then multiply (c) by (a) a total 
//	 * of (b) times. However, it is more efficient to set (c) to (a) and then multiply (c) by (a) a total of (b-1) times.
//	 */
//	Number i;
//	nm_init(&i);
//	nm_set(&i, 1);
//	for (nm_set(&i, 1); nm_is_less_than(&i, b); nm_inc(&i))
//		nm_mul(a, c);
//}
