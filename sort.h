/*
 *  sort.h
 *  Simfpl
 *
 *  Created by Michael Dickens on 8/1/10.
 *  
 *  Contains a number of sorting algorithms that sort arrays of integers. 
 *  Designed to test sorting speeds at different sizes in order to optimize 
 *  the built-in sorting function.
 *
 */

#include "interpreter.h"

/* 
 * Worst O(n^2), Average O(n^2), Memory 1
 * 
 * Optimized implementation, from http://en.wikipedia.org/wiki/Bubble_sort#Optimizing_bubble_sort
 */
int bubble_sort(int array[], int length);

/* 
 * I designed this sorting algorithm. As far as I know it has the shortest 
 * source code of any sorting algorithm. It appears to be a version of 
 * insertion sort.
 * 
 * Worst O(n^2), Average O(n^2), Best O(n), Memory 1
 */
int insertion_sort1(int array[], int length);

/* 
 * Worst O(n^2), Average O(n^2), Memory 1
 */
int insertion_sort2(int array[], int length);

/* 
 * Worst O(n log n), Average O(n log n), Memory n
 */
int merge_sort(int array[], int length);
void merge_sort_run(int array[], int length);

int vmerge_sort(value array[], int length);

/* 
 * Worst O(n^2), Average O(n log n), Memory log n
 * 
 * Uses the implementation found on Wikipedia.
 */
int quicksort1(int array[], int length);
void quicksort1_run(int array[], int left, int right);

/* 
 * Uses the implementation from an old Java program I wrote, pulled from 
 * somewhere else but I don't know where.
 */
int quicksort2(int array[], int length);
void quicksort2_run(int array[], int left, int right);

/* 
 * Uses quicksort for larger arrays and insertion2 for smaller ones.
 */
int custom_sort1(int array[], int length);
void custom_sort1_run(int array[], int left, int right);

#define vcustom_sort1(array, length) vcustom_sort2(array, length, 16)

int custom_sort2(int array[], int length, int cutoff);
void custom_sort2_run(int array[], int left, int right, int cutoff);

int vcustom_sort2(value array[], int length, int cutoff);
int vcustom_sort2_run(value array[], int left, int right, int cutoff);

int fill_array(int array[], int length);
int vfill_array(value array[], int length);
int vshuffle_array(value array[], int length);
int sorted_p(int array[], int length);
int sort_test();

