/*
 *  sort.c
 *  Calculator
 *
 *  Created by Michael Dickens on 8/1/10.
 *  Copyright 2010 The Khabele School. All rights reserved.
 *
 */

#include "sort.h"

int bubble_sort(int array[], int length)
{
	int i, n = length;
	do {
		int newn = 0;
		for (i = 0; i < n-1; ++i) {
			if (array[i] > array[i+1]) {
				int temp = array[i];
				array[i] = array[i+1];
				array[i+1] = temp;
				newn = i + 1;
			}
		}
		
		n = newn;
	} while (n > 1);
	
	return 0;
}

int insertion_sort1(int array[], int length)
{
	int i;
	for (i = 0; i < length-1; ) {
		if (array[i] > array[i+1]) {
			int temp = array[i];
			array[i] = array[i+1];
			array[i+1] = temp;
			if (i > 0) --i;
			else ++i;
		} else ++i;
	}
	
	return 0;
}

int insertion_sort2(int array[], int length)
{
	int i, j;
	for (i = 1; i < length; ++i) {
		int temp = array[i];
		j = i - 1;
		while (j >= 0 && array[j] > temp) {
			array[j+1] = array[j--];
		}
		
		array[j+1] = temp;
	}
	
	return 0;
}

int merge_sort(int array[], int length)
{
	merge_sort_run(array, length);
	return 0;
}

void merge_sort_run(int array[], int length)
{
	if (length > 1) {
		int half = length >> 1, blen;
		
		int a[half];
		int b[blen = length - half];
		int i, j;
		for (i = 0; i < half; ++i)
			a[i] = array[i];
		for (j = 0; i < length; ++i, ++j)
			b[j] = array[i];
		
		merge_sort_run(a, half);
		merge_sort_run(b, blen);
				
		i = j = 0;
        while(i < half && j < blen) {
            if (a[i] < b[j]) {
                array[i + j] = a[i++];
            } else {
                array[i + j] = b[j++];
            }
        }
        for(; i < half; ++i) array[i + j] = a[i];
        for(; j < blen; ++j) array[i + j] = b[j];
	}
}

int vmerge_sort(value array[], int length)
{
	if (length > 1) {
		int half = length >> 1, blen;
		
		value a[half];
		value b[blen = length - half];
		int i, j;
		for (i = 0; i < half; ++i)
			a[i] = array[i];
		for (j = 0; i < length; ++i, ++j)
			b[j] = array[i];
		
		if (vmerge_sort(a, half) == VALUE_ERROR)
			return VALUE_ERROR;
		if (vmerge_sort(b, blen) == VALUE_ERROR)
			return VALUE_ERROR;
				
		i = j = 0;
		int cmp;
        while(i < half && j < blen) {
            if ((cmp = value_cmp_any(a[i], b[j])) == -1) {
                array[i + j] = a[i++];
			} else if (cmp == -2) {
				return VALUE_ERROR;
            } else {
                array[i + j] = b[j++];
            }
        }
        for(; i < half; ++i) array[i + j] = a[i];
        for(; j < blen; ++j) array[i + j] = b[j];
	}
	
	return 0;
}

int quicksort1(int array[], int length)
{
	quicksort1_run(array, 0, length-1);
	return 0;
}

void quicksort1_run(int array[], int left, int right)
{
	int pivot_index = (left + right) / 2;
	int pivot_val = array[pivot_index];
	array[pivot_index] = array[right];
	array[right] = pivot_val;
	
	int store_index = left;
	int i;
	for (i = left; i < right; ++i) {
		if (array[i] <= pivot_val) {
			int temp = array[i];
			array[i] = array[store_index];
			array[store_index] = temp;
			++store_index;
		}
	}
	
	int temp = array[store_index];
	array[store_index] = array[right];
	array[right] = temp;
	
	if (left < store_index - 1)
		quicksort1_run(array, left, store_index - 1);
	if (store_index + 1 < right)
		quicksort1_run(array, store_index + 1, right);
}


int quicksort2(int array[], int length)
{
	quicksort2_run(array, 0, length-1);
	return 0;
}

void quicksort2_run(int array[], int left, int right)
{
	int i = left;
	int j = right;
	int pivot = array[(left+right)/2];
	
	do {
		while (array[i] < pivot) ++i;
		while (array[j] > pivot) --j;
		if (i <= j) {
			int temp = array[i];
			array[i] = array[j];
			array[j] = temp;
			++i; --j;
		}
		
	} while (i <= j);
	
	if (left < j) quicksort2_run(array, left, j);
	if (i < right) quicksort2_run(array, i, right);
}


int custom_sort1(int array[], int length)
{
	custom_sort1_run(array, 0, length-1);
	return 0;
}

void custom_sort1_run(int array[], int left, int right)
{
	if (right - left > 10) {
		int i = left;
		int j = right;
		int pivot = array[(left+right)/2];
		
		do {
			while (array[i] < pivot) ++i;
			while (array[j] > pivot) --j;
			if (i <= j) {
				int temp = array[i];
				array[i] = array[j];
				array[j] = temp;
				++i; --j;
			}
			
		} while (i <= j);
		
		if (left < j) custom_sort1_run(array, left, j);
		if (i < right) custom_sort1_run(array, i, right);
	} else {
		int i, j;
		for (i = left; i <= right; ++i) {
			int temp = array[i];
			j = i - 1;
			while (j >= 0 && array[j] > temp) {
				array[j+1] = array[j--];
			}
			
			array[j+1] = temp;
		}
	}

}


/* 

fastest for...

   4: insertion2
   8: insertion2
  16: insertion2/quicksort2`
  32: quicksort2/custom1
  64: custom1
 128: custom1
 256: custom1
 512: custom1
1024: custom1
2048:
4096:
8192:
16394:
32768:
65536:

fastest custom cutoffs (of 8, 16, 24) for...

 128: 24/16
 256: 24/16
 512: 24


position of custom1

   4: insertion1
   8: insertion2
  16: bubble
  32: merge
  64: insertion2
 128: quicksort2
 256:
 512:
1024:
2048:
4096:
8192:
16394:
32768:
65536:



   4:
   8:
  16:
  32:
  64:
 128:
 256:
 512:
1024:
2048:
4096:
8192:
16394:
32768:
65536:

 */
int custom_sort2(int array[], int length, int cutoff)
{
	custom_sort2_run(array, 0, length-1, cutoff);
	return 0;
}

void custom_sort2_run(int array[], int left, int right, int cutoff)
{
	if (right - left > cutoff) {
		int i = left;
		int j = right;
		int pivot = array[(left+right)/2];
		
		do {
			while (array[i] < pivot) ++i;
			while (array[j] > pivot) --j;
			if (i <= j) {
				int temp = array[i];
				array[i] = array[j];
				array[j] = temp;
				++i; --j;
			}
			
		} while (i <= j);
		
		if (left < j) custom_sort2_run(array, left, j, cutoff);
		if (i < right) custom_sort2_run(array, i, right, cutoff);
	} else {
		int i, j;
		for (i = left; i <= right; ++i) {
			int temp = array[i];
			j = i - 1;
			while (j >= 0 && array[j] > temp) {
				array[j+1] = array[j--];
			}
			
			array[j+1] = temp;
		}
	}

}


int vcustom_sort2(value array[], int length, int cutoff)
{
	vcustom_sort2_run(array, 0, length-1, cutoff);
	return 0;
}

int vcustom_sort2_run(value array[], int left, int right, int cutoff)
{
	if (right - left > cutoff) {
		int i = left;
		int j = right;
		int cmp;
		
		// The pivot's index is selected randomly between the two spots 
		// closest to the center, to decrease the probability of O(n^2) 
		// runtime.
		value pivot = array[(left+right)/2 + (genrand_int31() & 1)];
		
		do {
			while ((cmp = value_cmp_any(array[i], pivot)) == -1) ++i;
			if (cmp == -2) return VALUE_ERROR;
			while ((cmp = value_cmp_any(array[j], pivot)) > 0) --j;
			if (cmp == -2) return VALUE_ERROR;
			if (i <= j) {
				value temp = array[i];
				array[i] = array[j];
				array[j] = temp;
				++i; --j;
			}
			
		} while (i <= j);
		
		if (left < j) 
			if (vcustom_sort2_run(array, left, j, cutoff) == VALUE_ERROR)
				return VALUE_ERROR;
		if (i < right) 
			if (vcustom_sort2_run(array, i, right, cutoff) == VALUE_ERROR)
				return VALUE_ERROR;

	} else {
		int i, j, cmp;
		for (i = left; i <= right; ++i) {
			value temp = array[i];
			j = i - 1;
			while (j >= 0 && (cmp = value_cmp_any(array[j], temp)) > 0) {
				array[j+1] = array[j--];
			}
			if (cmp == -2) return VALUE_ERROR;
			
			array[j+1] = temp;
		}
	}
	
	return 0;
}





int fill_array(int array[], int length)
{
	srand(time(NULL));
	
	int i;
	for (i = 0; i < length; ++i)
		array[i] = rand() % 1000;
	
	return 0;
}

int vfill_array(value array[], int length)
{
	srand(time(NULL));
	
	value max_num = value_set_long(1000);
	int i;
	for (i = 0; i < length; ++i)
		array[i] = value_rand(max_num);
	value_clear(&max_num);
	
	return 0;
}

/* 
 * Because there's a significant overhead in creating new values, 
 * this is faster than vfill_array().
 */
int vshuffle_array(value array[], int length)
{
	int i, k;
	value temp;
	for (i = length - 1; i > 0; --i) {
		k = (int) (genrand_real2() * i);
		temp = array[k];
		array[k] = array[i];
		array[i] = temp;
	}
	
	return 0;
}

int sorted_p(int array[], int length)
{
	int i;
	for (i = 0; i < length-1; ++i)
		if (array[i] > array[i+1])
			return 0;
	return 1;
}

int sort_test()
{
	int i;
	
	int length = 100;
	int array[length];
	fill_array(array, length);
	
	custom_sort1(array, length);
	printf("%d\n", sorted_p(array, length));
	for (i = 0; i < length; ++i)
		printf("%d ", array[i]);
	printf("\n");
	
	return 0;
}