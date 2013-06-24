/*
 *  earray.c
 *  Calculator
 *
 *  Created by Michael Dickens on 12/5/09.
 *  Copyright 2009 The Khabele School. All rights reserved.
 *
 */

#include "earray.h"


EArray ea_init()
{
	EArray result;
	result.size = EA_SIZE_INC;
	result.core = malloc(EA_SIZE_INC * sizeof(E_TYPE));
	result.length = 1;
	result.core[0] = 0;
	return result;
}

EArray ea_init_with_length(int length)
{
	EArray result;
	result.size = (length / EA_SIZE_INC + 1) * EA_SIZE_INC;
	result.core = malloc(result.size * sizeof(E_TYPE));
	result.length = length;
	int i;
	for (i = length-1; 0 <= i; --i)
		result.core[i] = 0;
	result.length = 0;
	
	return result;
}

EArray ea_init_with_array(E_TYPE a[], int length)
{
	EArray result;
	result.size = (length / EA_SIZE_INC + 1) * EA_SIZE_INC;
	result.core = malloc(result.size * sizeof(E_TYPE));
	result.length = length;
	int i;
	for (i = length-1; 0 <= i; --i)
		result.core[i] = a[i];
	
	return result;
}

/* This really ought to use memcopy(). */
EArray ea_copy(EArray ea)
{
	E_TYPE copy_arr[ea.length];
	int i;
	for (i = ea.length-1; 0 <= i; --i)
		copy_arr[i] = ea.core[i];
	
	return ea_init_with_array(copy_arr, ea.length);
}

int ea_is_equal(EArray ea, EArray ea2)
{
	if (ea.length != ea2.length)
		return 0;
	
	int i;
	for (i = ea.length-1; 0 <= i; --i)
		if (ea.core[i] != ea2.core[i])
			return 0;
	
	return 1;
}

/* Returns the first index of (x) in (ea). If not found, returns -1. */
int ea_index(EArray ea, E_TYPE x)
{
	int i;
	EA_EACH_INDEX(ea, i)
		if (ea.core[i] == x) return i;
	return -1;
}


/* Returns the last index of (x) in (ea). If not found, returns -1. */
int ea_last_index(EArray ea, E_TYPE x)
{
	int i;
	EA_EACH_INDEX_DOWN(ea, i)
		if (ea.core[i] == x) return i;
	return -1;
}


EArray ea_clear(EArray ea)
{
	int i;
	EA_EACH_INDEX_DOWN(ea, i)
		ea.core[i] = 0;
	return ea;
}

void ea_clear_now(EArray *ea)
{
	int i;
	EA_EACH_INDEX_DOWN((*ea), i)
		ea->core[i] = 0;
}

/* Appends element x. */
EArray ea_append(EArray ea, E_TYPE x)
{
	if (ea.length >= ea.size) {
		ea.size += EA_SIZE_INC;
		ea.core = realloc(ea.core, ea.size * sizeof(E_TYPE));
	}
	
	ea.core[ea.length++] = x;
	return ea;
}

void ea_append_now(EArray *ea, E_TYPE x)
{
	if (ea->length >= ea->size) {
		ea->size += EA_SIZE_INC;
		ea->core = realloc(ea->core, ea->size * sizeof(E_TYPE));
	}
	
	ea->core[ea->length++] = x;
}

void ea_append_zero_now(EArray *ea)
{
	if (ea->length >= ea->size) {
		ea->size += EA_SIZE_INC;
		ea->core = realloc(ea->core, ea->size * sizeof(E_TYPE));
	}
	
	ea->core[ea->length++] = 0;
}

/* Pop off the last element and return it. */
E_TYPE ea_pop_now(EArray *ea)
{
	--ea->length;
	return ea->core[ea->length];
}

/* Pop off the first element and return it. This function causes a memory allocation error, because 
 * the memory at the previous value of ea->core is never being dereferenced. 
 */
E_TYPE ea_popleft_now(EArray *ea)
{
	E_TYPE result = ea->core[0];
	--ea->length;
	--ea->size;
	++ea->core;
	return result;
}

/* Concatenates ea and ea2. */
EArray ea_concat(EArray ea, EArray ea2)
{
	E_TYPE arr[ea.length + ea2.length];
	int i;
	for (i = 0; i < ea.length; i++)
		arr[i] = ea.core[i];
	for (i = 0; i < ea2.length; i++)
		arr[i+ea.length] = ea2.core[i];
	
	return ea_init_with_array(arr, ea.length + ea2.length);
}

EArray ea_insert(EArray ea, E_TYPE x, int index)
{
	EArray result;
	result.length = ea.length + 1;
	if (ea.length >= ea.size)
		result.size = ea.size + EA_SIZE_INC;
	else result.size = ea.size;
	
	result.core = malloc(result.size);

	
	int i, offset = 0;
	EA_EACH_INDEX(ea, i) {
		if (i == index) {
			offset = 1;
			result.core[i] = x;
		}
		result.core[i+offset] = ea.core[i];
	}
	
	return result;
}

EArray ea_reverse(EArray ea)
{
	int len = ea.length;
	E_TYPE temp;
	int i; for (i = (len >> 1) - 1; 0 <= i; --i) {
		temp = ea.core[i];
		ea.core[i] = ea.core[len-1-i];
		ea.core[len-1-i] = temp;
	}
	
	return ea;
}

void ea_reverse_now(EArray *ea)
{
	int len = ea->length;
	E_TYPE temp;
	int i; for (i = (len >> 1) - 1; 0 <= i; --i) {
		temp = ea->core[i];
		ea->core[i] = ea->core[len-1-i];
		ea->core[len-1-i] = temp;
	}
}

void ea_sort_array(E_TYPE a[], int l, int r)
{
	int i = l, j = r;
	E_TYPE temp, pivot = a[(l + r) >> 1];

	do {
		while (a[i] < pivot) i++;
		while (a[j] > pivot) j--;
		if (i <= j) {
			temp = a[i];
			a[i] = a[j];
			a[j] = temp;
			i++; j--;
		}
	} while (i <= j);

	if (l < j) ea_sort_array(a, l, j);
	if (i < r) ea_sort_array(a, i, r);
}

EArray ea_sort(EArray ea)
{
	EArray result = ea_copy(ea);
	ea_sort_array(result.core, 0, ea.length-1);
	return result;
}

void ea_print(EArray ea)
{
	if (ea.length > 0) {
		printf("[ ");
		int i; for (i = 0; i < ea.length-1; i++)
			printf("%u ", ea.core[i]);
		printf("%u ]\n", ea.core[i]);
	} else printf("[]\n");
}
