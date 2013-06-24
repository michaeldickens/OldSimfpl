/*
 *  tests.h
 *  Simfpl
 *
 *  Created by Michael Dickens on 7/6/10.
 *  Copyright 2010 The Khabele School. All rights reserved.
 *
 */
 
#include <sys/time.h>
#include "sort.h"

value test_vars;

int init_tests();

time_t usec();

/* 
 * Runs several tests in order. Each test assumes that the previous one 
 * succeeds.
 */
int run_tests();

/*
 * Run several benchmarks to determine how fast the program is running.
 */
int run_benchmarks();


int import_benchmark(char *name);
int set_benchmark1();

int test_assert(int p, char *description);
int test_sexp(char *str, value sexp, value expected);
int test_string(char *str, value expected);

int test_inputs();
int test_to_prefix();
int test_primitives();
int test_numbers();
int test_strings();
int test_arrays();
int test_lists();
int test_errors();
int test_inputs();
int test_controls();


int sort_speeds(int min_length, int max_length, int max_repeats);
int value_sort_speeds(int min_length, int max_length, int max_repeats);
int small_sort_speeds();
