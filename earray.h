/*
 *  earray.h
 *  Calculator
 *
 *  Created by Michael Dickens on 12/5/09.
 *
 */
 
/*
 * This is a library that allows mutable arrays. The 'E' in EAarray stands for 'Easy'. 
 * A function that ends in _now performs the operation on the given struct. A function that does not end in _now 
 * returns a new struct and preserves the old one.
 */

#include "value.h"

#define EA_SIZE_INC 10
 
typedef struct {
	E_TYPE *core;
	int length, size;
} EArray;

#define EA_EACH_INDEX(ea, i) for (i = 0; i < ea.length; ++i)
#define EA_EACH_INDEX_DOWN(ea, i) for (i = ea.length-1; 0 <= i; --i)
#define EA_EACH(ea, a, i) for (i = 0, a = ea.core[i]; i < ea.length; ++i, a = ea.core[i])
#define EA_EACH_DOWN(ea, a, i) for (i = ea.length-1, a = ea.core[i]; 0 <= i; --i, a = ea.core[i])

/* Basic creation functions. */
EArray ea_init();
EArray ea_init_with_length(int length);
EArray ea_init_with_array(E_TYPE a[], int length);
EArray ea_copy(EArray ea);

/* Data access. */
int ea_is_equal(EArray ea, EArray ea2);
int ea_index(EArray ea, E_TYPE x);
int ea_last_index(EArray ea, E_TYPE x);

/* Basic data manipulation. */
EArray ea_clear(EArray ea);
void ea_clear_now(EArray *ea);
EArray ea_append(EArray ea, E_TYPE x);
void ea_append_now(EArray *ea, E_TYPE x);
void ea_append_zero_now(EArray *ea);
E_TYPE ea_pop_now(EArray *ea);
E_TYPE ea_popleft_now(EArray *ea);
EArray ea_concat(EArray ea, EArray ea2);
EArray ea_insert(EArray ea, E_TYPE x, int index);

/* Advanced data manipulation. */
EArray ea_reverse(EArray ea);
void ea_reverse_now(EArray *ea);
void ea_sort_array(unsigned int a[], int l, int r);
EArray ea_sort(EArray ea);

void ea_print(EArray ea);