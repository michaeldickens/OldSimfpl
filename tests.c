/*
 *  tests.c
 *  Calculator
 *
 *  Created by Michael Dickens on 7/6/10.
 *  Copyright 2010 The Khabele School. All rights reserved.
 *
 */

#include "tests.h"

int init_tests()
{
	test_vars = value_hash_init();
	return 0;
}

time_t usec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec + 1000000 * (size_t) tv.tv_sec;
}

/* 
 * Each test requires that the previous test passed because it relies on 
 * certain utilities that are tested in previous tests.
 */
int run_tests()
{
	if (test_inputs()) {
		printf("Test suite aborted.\n\n");
		return 1;
	}
	if (test_to_prefix()) {
		printf("Test suite aborted.\n\n");
		return 1;
	}
	if (test_errors()) {
		printf("Test suite aborted.\n\n");
		return 1;
	}
	if (test_numbers()) {
		printf("Test suite aborted.\n\n");
		return 1;
	}
	if (test_strings()) {
		printf("Test suite aborted.\n\n");
		return 1;
	}
	if (test_arrays()) {
		printf("Test suite aborted.\n\n");
		return 1;
	}
	if (test_lists()) {
		printf("Test suite aborted.\n\n");
		return 1;
	}
	
	printf("Test suite succeeeded.\n\n");
	
	return 0;
}

int run_benchmarks()
{
	import_benchmark("benchmarks/array benchmark 1.simf");
	import_benchmark("benchmarks/hash benchmark 1.simf");
	import_benchmark("benchmarks/long benchmark 1.simf");
	import_benchmark("benchmarks/long benchmark 2.simf");
//	import_benchmark("benchmarks/loop benchmark 1.simf");
	import_benchmark("benchmarks/math benchmark 1.simf");
	import_benchmark("benchmarks/math benchmark 2.simf");
	import_benchmark("benchmarks/misc benchmark 1.simf");
	import_benchmark("benchmarks/recursion benchmark 1.simf");
	import_benchmark("benchmarks/sort benchmark 1.simf");
//	import_benchmark("benchmarks/sort benchmark 2.simf");
	
//	import_benchmark("benchmarks/math benchmark 3.simf"); // buggy

	return 0;
}


int import_benchmark(char *name)
{
	printf("\n*****\n*****\n*****\n\nRunning benchmark \"%s\".\n\n", name);
	
	value benchmark = value_set_str(name);
	value res = value_import(benchmark);
	
	value_printf("\nBenchmark returned %ts.\n\n", res);
	printf("*****\n*****\n*****\n\n");
	
	return 0;
}

/* This has a lot of memory leakage, but that's okay because it will 
 * probably only be called once. Memory leakage is necessary because if 
 * the values were cleared then the timings would be skewed.
 */
int set_benchmark1()
{
	time_t start, finish;
	size_t i;
	value op;
	
	start = usec();
	for (i = 0; i < 10000; ++i) {
		op = value_set_long(1828);
		op = value_set(op);
		op = value_set_double(2.43);
		op = value_set(op);
		op = value_set_str("hello there");
		op = value_set(op);
		op = value_init(VALUE_ARY);
		op = value_set(op);
		op = value_init(VALUE_LST);
		op = value_set(op);
		op = value_init(VALUE_BLK);
		op = value_set(op);
	}
	finish = usec();
	printf("time to set: %ld sec, %ld usec\n", (finish - start) / 1000000, (finish - start) % 1000000);
	
	return 0;
}

int test_assert(int p, char *description)
{
	if (!p) {
		value_error(1, "Assertion failed: %c", description);
		return VALUE_ERROR;
	}
	
	return 0;
}

int test_sexp(char *str, value sexp, value expected)
{
	value res = eval(&test_vars, sexp);
	int error_p = 0;
	if (value_ne(res, expected)) {
		value_error(0, "Test failed: for %c \n\t%.60ts expected, %.60ts found.\n", str, expected, res);
		error_p = VALUE_ERROR;
	}

	value_clear(&res);
	value_clear(&expected);
	
	return error_p;
}

int test_string(char *str, value expected)
{
	value sexp = compile_statement(str);
	if (sexp.type == VALUE_ERROR)
		return 0;
	
	int error_p = test_sexp(str, sexp, expected);
	if (error_p == 0) {
//		printf("succeeded: %s\n", str);
	}
	
	return error_p;
}

/* 
 * Test various inputs to make sure that they are read in properly.
 */
int test_inputs()
{
	printf("\n*****\n*****\n*****\n\nTesting inputs.\n\n");
	
	int orig_print_info_p = print_info_p;
	print_info_p = FALSE;
	
	int orig_print_errors_p = print_errors_p;
	print_errors_p = FALSE;
	
	int did_fail = FALSE;
	
	// Nils.
	did_fail |= test_string("", value_init_nil());
	did_fail |= test_string(" ", value_init_nil());
	did_fail |= test_string("()", value_init_nil());
	did_fail |= test_string("(())", value_init_nil());
	did_fail |= test_string(" (()) ", value_init_nil());
	did_fail |= test_string("( ( ) )", value_init_nil());
	did_fail |= test_string("nil", value_init_nil());
	did_fail |= test_string("((nil))", value_init_nil());
	
	// Simple errors.
	did_fail |= test_string("nil nil", value_init_error());
	did_fail |= test_string("nil ()", value_init_error());
	did_fail |= test_string("() nil", value_init_error());
	did_fail |= test_string("() ()", value_init_error());

	// Numbers.
	did_fail |= test_string("3", value_set_long(3));
	did_fail |= test_string("32", value_set_long(32));
	did_fail |= test_string("--3", value_set_long(-3));
	did_fail |= test_string("-- 3", value_set_long(-3));
	did_fail |= test_string("32.1", value_set_double(32.1));
	did_fail |= test_string("-- 3.1", value_set_double(-3.1));
	did_fail |= test_string("032", value_set_long(032));
	did_fail |= test_string("0x3", value_set_long(0x3));
	did_fail |= test_string("0xf", value_set_long(0xf));
	did_fail |= test_string("0x3f", value_set_long(0x3f));
	did_fail |= test_string("0xf3", value_set_long(0xf3));
	
	// Operations.
	did_fail |= test_string("3 * 10", value_set_long(30));
	did_fail |= test_string("3*10", value_set_long(30));
	did_fail |= test_string("3-10", value_set_long(-7));
	did_fail |= test_string("3 - 5*2", value_set_long(-7));
	did_fail |= test_string("3-5 * 2", value_set_long(-7));
	did_fail |= test_string("3 - 4 * 5 / 2", value_set_long(-7));
	
	// Prefix notation.
	did_fail |= test_string("* 3 10", value_set_long(30));
	did_fail |= test_string("+ 1 * 3 10", value_set_long(31));
	did_fail |= test_string("+ * 3 10 1", value_set_long(31));
	did_fail |= test_string("- + * 3 10 5 12", value_set_long(23));
	did_fail |= test_string("- 12 + 5 * 10 3", value_set_long(-23));
	
	// Parentheses.
	did_fail |= test_string("(3)", value_set_long(3));
	did_fail |= test_string("(3 + 2)", value_set_long(5));
	did_fail |= test_string("(3) + 2", value_set_long(5));
	did_fail |= test_string("3 + (2)", value_set_long(5));
	did_fail |= test_string("(3) + (2)", value_set_long(5));
	did_fail |= test_string("((3) + (2))", value_set_long(5));
	did_fail |= test_string("((3))", value_set_long(3));
	did_fail |= test_string("((3 + 2))", value_set_long(5));
	
	
	// Negatives with other operations.
	did_fail |= test_string("5-2", value_set_long(3));
	did_fail |= test_string("5+ --2", value_set_long(3));
	did_fail |= test_string("5+--2", value_init_error()); // It parses +-- as a single function, which is undefined.
	did_fail |= test_string("--2-5", value_set_long(-7));
	did_fail |= test_string("--2 * --3", value_set_long(6));
	did_fail |= test_string("--2* --3", value_set_long(6));
	did_fail |= test_string("--2- --3", value_set_long(1));

	// Numbers in strings.
	did_fail |= test_string("\"3\"", value_set_str("3"));
	did_fail |= test_string("\"-3\"", value_set_str("-3"));
	did_fail |= test_string("\"3.1\"", value_set_str("3.1"));
	did_fail |= test_string("\"-3.1\"", value_set_str("-3.1"));
	did_fail |= test_string("\"03\"", value_set_str("03"));
	did_fail |= test_string("\"0x3\"", value_set_str("0x3"));
	did_fail |= test_string("\"0xf\"", value_set_str("0xf"));
	
	// Array and list inputs.
	long long_arr[] = { 1, 2, 3 };
	did_fail |= test_string("array 1 2 3", value_set_ary_long(long_arr, 3));
	
	// Statements.
	did_fail |= test_string("i = 0; i = i + 1", value_set_long(1));
	did_fail |= test_string("i = 0; i = i + 1; i = 5", value_set_long(5));

	if (did_fail) {
		printf("Test of inputs failed.\n\n");
	} else {
		printf("Test of inputs succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	print_errors_p = orig_print_errors_p;
	return did_fail;
}

/* 
 * Tests whether statements are being properly converted to prefix notation.
 * Primarily focuses on ternary operators, because those can be the most confusing and 
 * difficult to deal with.
 */
int test_to_prefix()
{
	printf("\n*****\n*****\n*****\n\nTesting to prefix.\n\n");
	
	int orig_print_info_p = print_info_p;
	print_info_p = FALSE;
	
	int did_fail = FALSE;
	
	did_fail |= test_string("` (3 * 2 + 1 / 5)", compile_statement("+ * 3 2 / 1 5"));
	did_fail |= test_string("` (3 + 2 * 1 - 5)", compile_statement("- + 3 * 2 1 5"));
//	did_fail |= test_string("` (= a[0] b[1])", compile_statement("at_equals a (0) (at b (1))")); // This fails.
	did_fail |= test_string("` (a[0] = b[1])", compile_statement("at_equals a (0) (at b (1))"));
	did_fail |= test_string("` (a 0 at_equals b at 1)", compile_statement("at_equals a 0 (at b 1)"));
	did_fail |= test_string("` ((array 1 2 3) insert 3 4 append 5)", compile_statement("append (insert (array 1 2 3) 3 4) 5"));
	did_fail |= test_string("` ((array 1 2 3) append 4 insert 4 5)", compile_statement("insert (append (array 1 2 3) 4) 4 5"));

	if (did_fail) {
		printf("Test of to prefix failed.\n\n");
	} else {
		printf("Test of to prefix succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	return did_fail;
}

/* 
 * Tests most primitives, but only once. Does not check to see how they handle 
 * illegal inputs.
 * 
 * I have added some new functions, so I know that this does not include every 
 * primitive. Also, it only includes functions for numbers, strings and arrays.
 */
int test_primitives()
{
	printf("\n*****\n*****\n*****\n\nTesting primitives.\n\n");
	
	int orig_print_info_p = print_info_p;
	print_info_p = FALSE;
	
	int did_fail = FALSE;

	did_fail |= test_string("3 + 4 * 2 - 5 % 2", value_set_long(10));
	did_fail |= test_string("3 == 4 && 2 < 3", value_set_bool(FALSE));
	did_fail |= test_string("3 != 4 || 2 > 3", value_set_bool(TRUE));
	did_fail |= test_string("! (3 <= 4 && 2 <= 3)", value_set_bool(FALSE));
	did_fail |= test_string("(1 << 10) >> 6", value_set_long(16));
	did_fail |= test_string("15 & 7 & 12", value_set_long(4));
	did_fail |= test_string("7 | 12", value_set_long(15));
	did_fail |= test_string("7 ^ 12", value_set_long(11));
	did_fail |= test_string("~7", value_set_long(-8));
	did_fail |= test_string("2**5", value_set_long(32));
	did_fail |= test_string("4**0.5", value_set_double(2.0));
	did_fail |= test_string("2**(--1)", value_set_double(0.5));
	did_fail |= test_string("exp 2", value_set_double(exp(2)));
	did_fail |= test_string("log 5", value_set_double(log(5)));
	did_fail |= test_string("log10 5", value_set_double(log10(5)));
	did_fail |= test_string("sqrt 25", value_set_double(5.0));
	did_fail |= test_string("8 factorial", value_set_long(40320));
	did_fail |= test_string("sin 2", value_set_double(sin(2)));
	did_fail |= test_string("cos 2", value_set_double(cos(2)));
	did_fail |= test_string("tan 2", value_set_double(tan(2)));
	did_fail |= test_string("csc 2", value_set_double(1.0 / sin(2)));
	did_fail |= test_string("sec 2", value_set_double(1.0 / cos(2)));
	did_fail |= test_string("cot 2", value_set_double(1.0 / tan(2)));
	did_fail |= test_string("asin 0.5", value_set_str_smart("0.5235987755982989266811955531011335551738739013671875", 10)); // This is necessary because of MPFR's precision.
	did_fail |= test_string("acos 0.5", value_set_str_smart("1.047197551196597853362391106202267110347747802734375", 10));
	did_fail |= test_string("atan 0.5", value_set_str_smart("0.463647609000806093515478778499527834355831146240234375", 10));
	did_fail |= test_string("\"heLlO\" capitalize", value_set_str("Hello"));
	did_fail |= test_string("\"hello\" chop", value_set_str("hell"));
	did_fail |= test_string("\"hello\" contains? \"el\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" ends_with? \"llo\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" index \"el\"", value_set_long(1));
	did_fail |= test_string("insert \"ho\" 1 \"ell\"", value_set_str("hello"));
	did_fail |= test_string("alpha? \"hello1\"", value_set_bool(FALSE));
	did_fail |= test_string("alnum? \"hello1\"", value_set_bool(TRUE));
	did_fail |= test_string("num? \"hello1\"", value_set_bool(FALSE));
	did_fail |= test_string("\"hello\" length", value_set_long(5));
	did_fail |= test_string("\"  hello  \" lstrip", value_set_str("hello  "));
	did_fail |= test_string("range \"hello\" 2 4", value_set_str("ll"));
//	did_fail |= test_string("replace \"hello\" \"ell\" \"ey h\"", value_set_str("hey ho"));
	did_fail |= test_string("\"  hello  \" rstrip", value_set_str("  hello"));
	
	char *strarr[] = { "one", "two", "three" };
	// This one is crashing the program.
	did_fail |= test_string("\"one two three\" split \" \"", value_set_ary_str(strarr, 3));
	did_fail |= test_string("\"hello\" starts_with? \"ello\"", value_set_bool(FALSE));
	did_fail |= test_string("\"  hello  \" strip", value_set_str("hello"));
	did_fail |= test_string("\"heLLO\" to_lower", value_set_str("hello"));
	did_fail |= test_string("\"heLLO\" to_upper", value_set_str("HELLO"));
	
	long andres[] = { 2, 4 };
	long orres[] = { 2, 4, 8, 3, 5 };
	long xorres[] = { 8, 3, 5 };
	long mulres[] = { 2, 4, 2, 4, 2, 4 };
	long appendres[] = { 2, 4, 8, 2, 4, 5 };
	long concatres[] = { 2, 4, 8, 2, 4, 5, 7, 7 };
	long deleteres[] = { 2, 8, 2, 4 };
	long deleteallres[] = { 2, 8, 2 };
	long deleteatres[] = { 2, 4, 2, 4 };
	long insertres[] = { 2, 4, 1000, 8, 2, 4 };
	long sortres[] = { 2, 2, 4, 4, 8 };
	long uniqres[] = { 2, 4, 8 };
	
	long consres[] = { 1, 2, 4, 8, 2 };
	long dropres[] = { 8, 2 };
	long tailres[] = { 4, 8, 2 };
	long takeres[] = { 2, 4 };
	did_fail |= test_string("(array 2 4 8 2 4) & (array 2 3 4 5)", value_set_ary_long(andres, 2));
	did_fail |= test_string("(array 2 4 8 2 4) | (array 2 3 4 5)", value_set_ary_long(orres, 5));
	did_fail |= test_string("(array 2 4 8 2 4) ^ (array 2 3 4 5)", value_set_ary_long(xorres, 3));
	did_fail |= test_string("(array 2 4) * 3", value_set_ary_long(mulres, 6));
	did_fail |= test_string("(array 2 4 8 2 4) append 5", value_set_ary_long(appendres, 6));
	did_fail |= test_string("(array 2 4 8 2 4) + 5", value_set_ary_long(appendres, 6));
	did_fail |= test_string("(array 2 4 8 2 4) at 2", value_set_long(8));
	did_fail |= test_string("(array 2 4 8 2 4) concat (array 5 7 7)", value_set_ary_long(concatres, 8));
	did_fail |= test_string("(array 2 4 8 2 4) + (array 5 7 7)", value_set_ary_long(concatres, 8));
	did_fail |= test_string("(array 2 4 8 2 4) delete 4", value_set_ary_long(deleteres, 4));
	did_fail |= test_string("(array 2 4 8 2 4) delete_all 4", value_set_ary_long(deleteallres, 3));
	did_fail |= test_string("(array 2 4 8 2 4) delete_at 2", value_set_ary_long(deleteatres, 4));
	did_fail |= test_string("insert (array 2 4 8 2 4) 2 1000", value_set_ary_long(insertres, 6));	
	did_fail |= test_string("(array 2 4 8 2 4) empty?", value_set_bool(FALSE));
//	did_fail |= test_string("(array (array 2 4) (array 8 2) 4) flatten", value_set_ary_long(numarr, 5));
	did_fail |= test_string("(array 2 4 8 2 4) join \"!\"", value_set_str("2!4!8!2!4"));
	did_fail |= test_string("(array 2 4 8 2 4) last", value_set_long(4));
	did_fail |= test_string("(array 2 4 8 2 4) sort", value_set_ary_long(sortres, 5));
	did_fail |= test_string("(array 2 4 8 2 4) uniq", value_set_ary_long(uniqres, 3));
	
	did_fail |= test_string("1 cons (array 2 4 8 2)", value_set_ary_long(consres, 5));
	did_fail |= test_string("(array 2 4 8 2) drop 2", value_set_ary_long(dropres, 2));
	did_fail |= test_string("(array 2 4 8 2) head", value_set_long(2));
	did_fail |= test_string("(array 2 4 8 2) tail", value_set_ary_long(tailres, 3));
	did_fail |= test_string("(array 2 4 8 2) take 2", value_set_ary_long(takeres, 2));

	if (did_fail) {
		printf("Test of primitives failed.\n\n");
	} else {
		printf("Test of primitives succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	return did_fail;
}

/*
 * Extensively tests that numbers are working.
 */
int test_numbers()
{
	printf("\n*****\n*****\n*****\n\nTesting numbers.\n\n");
	
	int orig_print_info_p = print_info_p, orig_print_errors_p = print_errors_p;
	print_info_p = FALSE;
	print_errors_p = FALSE;
		
	int did_fail = FALSE;
	did_fail |= test_string("1", value_set_long(1));
	did_fail |= test_string("2", value_set_long(2));
	did_fail |= test_string("3", value_set_long(3));
	did_fail |= test_string("10", value_set_long(10));
	did_fail |= test_string("999", value_set_long(999));
	did_fail |= test_string("3.0", value_set_double(3.0));
	did_fail |= test_string("999.0", value_set_double(999.0));
	did_fail |= test_string("3.5", value_set_double(3.5));
	did_fail |= test_string("0.1", value_set_double(0.1));
	did_fail |= test_string("0.0", value_set_double(0.0));
	did_fail |= test_string("2.", value_set_double(2.0));
	did_fail |= test_string(".0", value_init_error());
	
	did_fail |= test_string("x = 3", value_set_long(3));
	did_fail |= test_string("x", value_set_long(3));
	did_fail |= test_string("x += 0.5", value_set_double(3.5));
	did_fail |= test_string("x", value_set_double(3.5));
	did_fail |= test_string("x -= 3", value_set_double(0.5));
	did_fail |= test_string("x", value_set_double(0.5));
	did_fail |= test_string("x *= 2", value_set_double(1.0));
	did_fail |= test_string("x", value_set_double(1.0));
	did_fail |= test_string("x /= 2", value_set_double(0.5));
	did_fail |= test_string("x", value_set_double(0.5));
	did_fail |= test_string("x /= 0.25", value_set_double(2.0));
	did_fail |= test_string("x", value_set_double(2.0));
	did_fail |= test_string("x = 11", value_set_long(11));
	did_fail |= test_string("x %= 4", value_set_long(3));
	did_fail |= test_string("x", value_set_long(3));
	did_fail |= test_string("x %= 2.5", value_init_error());
	did_fail |= test_string("x = 2.5", value_set_double(2.5));
	did_fail |= test_string("x %= 4", value_init_error());
	did_fail |= test_string("x = 15", value_set_long(15));
	did_fail |= test_string("x &= 30", value_set_long(14));
	did_fail |= test_string("x", value_set_long(14));
	did_fail |= test_string("x |= 3", value_set_long(15));
	did_fail |= test_string("x", value_set_long(15));
	did_fail |= test_string("x ^= 37", value_set_long(42));
	did_fail |= test_string("x", value_set_long(42));
	did_fail |= test_string("x <<= 3", value_set_long(336));
	did_fail |= test_string("x", value_set_long(336));
	did_fail |= test_string("x >>= 5", value_set_long(10));
	did_fail |= test_string("x", value_set_long(10));
	
	did_fail |= test_string("1**0", value_set_long(1));
	did_fail |= test_string("1**1", value_set_long(1));
	did_fail |= test_string("1**5", value_set_long(1));
	did_fail |= test_string("2**0", value_set_long(1));
	did_fail |= test_string("2**1", value_set_long(2));
	did_fail |= test_string("2**2", value_set_long(4));
	did_fail |= test_string("2**8", value_set_long(256));
	did_fail |= test_string("2**2**2", value_set_long(16));
	did_fail |= test_string("2**2**2**2", value_set_long(65536));
	did_fail |= test_string("(--1)**0", value_set_long(1));
	did_fail |= test_string("(--1)**1", value_set_long(-1));
	did_fail |= test_string("(--1)**2", value_set_long(1));
	did_fail |= test_string("(--1)**3", value_set_long(-1));
	did_fail |= test_string("10**3", value_set_long(1000));
	did_fail |= test_string("2**10", value_set_long(1024));
	did_fail |= test_string("2**(--1)", value_set_double(0.5));
	did_fail |= test_string("4**0.5", value_set_double(2.0));
	did_fail |= test_string("4.0**0.5", value_set_double(2.0));
	did_fail |= test_string("2**0.5", value_set_double(sqrt(2)));
	did_fail |= test_string("2.0**0.5", value_set_double(sqrt(2)));

	did_fail |= test_string("!1", value_set_bool(FALSE));
	did_fail |= test_string("! 1", value_set_bool(FALSE));
	did_fail |= test_string("! 0", value_set_bool(TRUE));
	did_fail |= test_string("! 3", value_set_bool(FALSE));
	did_fail |= test_string("!(1)", value_set_bool(FALSE));
	did_fail |= test_string("!1.5", value_set_bool(FALSE));
	did_fail |= test_string("! 1.75", value_set_bool(FALSE));
	did_fail |= test_string("! 0.2", value_set_bool(FALSE));
	did_fail |= test_string("! 3.99", value_set_bool(FALSE));
	did_fail |= test_string("!(1.75)", value_set_bool(FALSE));
	did_fail |= test_string("~1", value_set_long(-2));
	did_fail |= test_string("~ 1", value_set_long(-2));
	did_fail |= test_string("~ 0", value_set_long(-1));
	did_fail |= test_string("~ 3", value_set_long(-4));
	did_fail |= test_string("~(1)", value_set_long(-2));
	did_fail |= test_string("~ 3.5", value_init_error());
	did_fail |= test_string("~ 0.0", value_init_error());

	did_fail |= test_string("++ 1", value_set_long(1));
	did_fail |= test_string("-- 1", value_set_long(-1));
	did_fail |= test_string("--2", value_set_long(-2));
	did_fail |= test_string("-- 1.5", value_set_double(-1.5));
	did_fail |= test_string("++ -- 1", value_set_long(-1));
	did_fail |= test_string("-- ++ 1", value_set_long(-1));
	did_fail |= test_string("-- -- 1", value_set_long(1));
	did_fail |= test_string("-- -- -- 1", value_set_long(-1));
	did_fail |= test_string("++ -- -- 1", value_set_long(1));
	did_fail |= test_string("-- ++ -- 1", value_set_long(1));
	did_fail |= test_string("-- -- ++ 1", value_set_long(1));
	did_fail |= test_string("-- ++ ++ 1", value_set_long(-1));
	did_fail |= test_string("++ -- ++ 1", value_set_long(-1));
	did_fail |= test_string("++ ++ -- 1", value_set_long(-1));
	did_fail |= test_string("++ ++ 1", value_set_long(1));
	did_fail |= test_string("++ ++ ++ 1", value_set_long(1));
	did_fail |= test_string("1 abs --", value_set_long(-1));
	did_fail |= test_string("abs -- 1", value_set_long(1));
	did_fail |= test_string("1 -- abs", value_set_long(1));
	did_fail |= test_string("-- abs 1", value_set_long(-1));
	did_fail |= test_string("abs -- 1.5", value_set_double(1.5));
	did_fail |= test_string("abs 1", value_set_long(1));
	did_fail |= test_string("abs 1.5", value_set_double(1.5));
	did_fail |= test_string("--++ 1", value_init_error());
	did_fail |= test_string("++-- 1", value_init_error());
	did_fail |= test_string("---- 1", value_init_error());
	did_fail |= test_string("++++ 1", value_init_error());

	did_fail |= test_string("exp 0", value_set_double(1.0));
	did_fail |= test_string("exp 0.0", value_set_double(1.0));
	did_fail |= test_string("exp 1", value_set_double(exp(1.0)));
	did_fail |= test_string("exp 1.0", value_set_double(exp(1.0)));
	did_fail |= test_string("exp 2.5", value_set_double(exp(2.5)));
	did_fail |= test_string("exp -- 1", value_set_double(exp(-1)));
	did_fail |= test_string("exp -- 3", value_set_double(exp(-3)));
	did_fail |= test_string("exp -- 3.5", value_set_double(exp(-3.5)));
	did_fail |= test_string("log 0", value_init(VALUE_NAN));
	did_fail |= test_string("log 0.0", value_init(VALUE_NAN));
	did_fail |= test_string("log 1", value_set_double(log(1.0)));
	did_fail |= test_string("log 1.0", value_set_double(log(1.0)));
	did_fail |= test_string("log 2.5", value_set_double(log(2.5)));
	did_fail |= test_string("log -- 1", value_init(VALUE_NAN));
	did_fail |= test_string("log2 0", value_init(VALUE_NAN));
	did_fail |= test_string("log2 0.0", value_init(VALUE_NAN));
	did_fail |= test_string("log2 1", value_set_double(log2(1.0)));
	did_fail |= test_string("log2 1.0", value_set_double(log2(1.0)));
	did_fail |= test_string("log2 2.5", value_set_double(log2(2.5)));
	did_fail |= test_string("log2 -- 1", value_init(VALUE_NAN));
	did_fail |= test_string("log10 0", value_init(VALUE_NAN));
	did_fail |= test_string("log10 0.0", value_init(VALUE_NAN));
	did_fail |= test_string("log10 1", value_set_double(log10(1.0)));
	did_fail |= test_string("log10 1.0", value_set_double(log10(1.0)));
	did_fail |= test_string("log10 2.5", value_set_double(log10(2.5)));
	did_fail |= test_string("log10 -- 1", value_init(VALUE_NAN));
	did_fail |= test_string("sqrt 1", value_set_double(1.0));
	did_fail |= test_string("sqrt 1.0", value_set_double(1.0));
	did_fail |= test_string("sqrt 4", value_set_double(2.0));
	did_fail |= test_string("sqrt 2", value_set_double(sqrt(2)));
	did_fail |= test_string("sqrt 49", value_set_double(7.0));
	did_fail |= test_string("sqrt 99", value_set_double(sqrt(99)));
	did_fail |= test_string("sqrt -- 1", value_init(VALUE_NAN));
	did_fail |= test_string("sqrt -- 0.5", value_init(VALUE_NAN));
	did_fail |= test_string("sqrt 0.5", value_set_double(sqrt(0.5)));
	did_fail |= test_string("sqrt 2.5", value_set_double(sqrt(2.5)));

	did_fail |= test_string("factorial 0", value_set_long(1));
	did_fail |= test_string("0 factorial", value_set_long(1));
	did_fail |= test_string("factorial 1", value_set_long(1));
	did_fail |= test_string("factorial 3", value_set_long(6));
	did_fail |= test_string("factorial 5", value_set_long(120));
	did_fail |= test_string("2.5 factorial", value_init_error());
	did_fail |= test_string("3 choose 2", value_set_long(3));
	did_fail |= test_string("5 choose 2", value_set_long(10));
	did_fail |= test_string("20 choose 8", value_set_long(125970));
	did_fail |= test_string("2 choose 4", value_set_long(0));
	did_fail |= test_string("100 choose 100", value_set_long(1));
	did_fail |= test_string("0 choose 1", value_set_long(0));
	did_fail |= test_string("1.6 choose 4", value_init_error());
	did_fail |= test_string("5 choose 4.2", value_init_error());
	did_fail |= test_string("1.6 choose 0.5", value_init_error());
	
	// These will break if you change the precision.
	did_fail |= test_string("sin 1", value_set_str_smart("0.8414709848078965048756572286947630345821380615234375", 10));
	did_fail |= test_string("sin 1.0", value_set_str_smart("0.8414709848078965048756572286947630345821380615234375", 10));
	did_fail |= test_string("sin 10.0", value_set_str_smart("-0.5440211108893697744548489936278201639652252197265625", 10));
	did_fail |= test_string("cos 1", value_set_str_smart("0.540302305868139765010482733487151563167572021484375", 10));
	did_fail |= test_string("cos 1.0", value_set_str_smart("0.540302305868139765010482733487151563167572021484375", 10));
	did_fail |= test_string("cos 10.0", value_set_str_smart("-0.83907152907645243811174395887064747512340545654296875", 10));
	did_fail |= test_string("tan 1", value_set_str_smart("1.557407724654902292371616567834280431270599365234375", 10));
	did_fail |= test_string("tan 1.0", value_set_str_smart("1.557407724654902292371616567834280431270599365234375", 10));
	did_fail |= test_string("tan 10.0", value_set_str_smart("0.6483608274590866304976088940748013556003570556640625", 10));
	did_fail |= test_string("csc 1", value_set_str_smart("1.18839510577812124125784976058639585971832275390625", 10));
	did_fail |= test_string("csc 1.0", value_set_str_smart("1.18839510577812124125784976058639585971832275390625", 10));
	did_fail |= test_string("csc 10.0", value_set_str_smart("-1.8381639608896656046255202454631216824054718017578125", 10));
	did_fail |= test_string("sec 1", value_set_str_smart("1.850815717680925676091874265694059431552886962890625", 10));
	did_fail |= test_string("sec 1.0", value_set_str_smart("1.850815717680925676091874265694059431552886962890625", 10));
	did_fail |= test_string("sec 10.0", value_set_str_smart("-1.1917935066878957428571084165014326572418212890625", 10));
	did_fail |= test_string("cot 1", value_set_str_smart("0.6420926159343307570992465116432867944240570068359375", 10));
	did_fail |= test_string("cot 1.0", value_set_str_smart("0.6420926159343307570992465116432867944240570068359375", 10));
	did_fail |= test_string("cot 10.0", value_set_str_smart("1.542351045356920025142244412563741207122802734375", 10));
	
	if (did_fail) {
		printf("\nTest of numbers failed.\n\n");
	} else {
		printf("\nTest of numbers succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	print_errors_p = orig_print_errors_p;
	return did_fail;
}

/* 
 * Extensively tests that strings are working. Does NOT test the following functions: 
 *   scan(), split()
 * 
 * Also does not test functions with regular expressions as arguments; exclusively uses 
 * strings and numbers.
 */
int test_strings()
{
	printf("\n*****\n*****\n*****\n\nTesting strings.\n\n");
	
	int orig_print_info_p = print_info_p;
	print_info_p = FALSE;
		
	int did_fail = FALSE;
	did_fail |= test_string("\"hello\"", value_set_str("hello"));
	did_fail |= test_string("\"hello \"", value_set_str("hello "));
	
	did_fail |= test_string("\"hello\" + \"world\"", value_set_str("helloworld"));
	did_fail |= test_string("\"hello\" + \" \" + \"world\"", value_set_str("hello world"));
	did_fail |= test_string("\"hello\" + 32 chr + \"world\"", value_set_str("hello world"));
	
	did_fail |= test_string("\"blah\" * 3", value_set_str("blahblahblah"));
	
	did_fail |= test_string("\"hello\" capitalize", value_set_str("Hello"));
	did_fail |= test_string("\"hELLO\" capitalize", value_set_str("Hello"));
	did_fail |= test_string("\"HELLO\" capitalize", value_set_str("Hello"));
	did_fail |= test_string("\"hello World\" capitalize", value_set_str("Hello world"));
	
	did_fail |= test_string("\"h\" asc", value_set_long((long) 'h'));
	
	did_fail |= test_string("\"hello\" chop", value_set_str("hell"));
	did_fail |= test_string("\"h\" chop", value_set_str(""));
	did_fail |= test_string("\"\" chop", value_set_str(""));
	
	did_fail |= test_string("97 chr", value_set_str("a"));
	did_fail |= test_string("10 chr", value_set_str("\n"));
	
	did_fail |= test_string("\"hello\" contains? \"h\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" contains? \"llo\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" contains? \"lo \"", value_set_bool(FALSE));
	did_fail |= test_string("\"hello\" contains? \"hello\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" contains? \"hello.\"", value_set_bool(FALSE));
	did_fail |= test_string("\"hello\" contains? \"hx\"", value_set_bool(FALSE));
	
	did_fail |= test_string("\"hello\" ends_with? \"h\"", value_set_bool(FALSE));
	did_fail |= test_string("\"hello\" ends_with? \"llo\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" ends_with? \"lo \"", value_set_bool(FALSE));
	did_fail |= test_string("\"hello\" ends_with? \"hello\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" ends_with? \"hello.\"", value_set_bool(FALSE));
	did_fail |= test_string("\"hello\" ends_with? \"o\"", value_set_bool(TRUE));
	
	did_fail |= test_string("\"hello\" index \"h\"", value_set_long(0));
	did_fail |= test_string("\"hello\" index \"llo\"", value_set_long(2));
	did_fail |= test_string("\"hello\" index \"lo \"", value_init_nil());
	did_fail |= test_string("\"hello\" index \"hello\"", value_set_long(0));
	did_fail |= test_string("\"hello\" index \"hello.\"", value_init_nil());
	did_fail |= test_string("\"hello\" index \"hx\"", value_init_nil());
	did_fail |= test_string("\"hello\" index \"o\"", value_set_long(4));
	
	did_fail |= test_string("\"hello\" insert 2 \"XX\"", value_set_str("heXXllo"));
	did_fail |= test_string("\"hello\" insert 3 \"XX\"", value_set_str("helXXlo"));
	did_fail |= test_string("\"hello\" insert 0 \"XX\"", value_set_str("XXhello"));
	did_fail |= test_string("\"hello\" insert 5 \"XX\"", value_set_str("helloXX"));
	
	did_fail |= test_string("\"hello\" alpha?", value_set_bool(TRUE));
	did_fail |= test_string("\"hel30lo\" alpha?", value_set_bool(FALSE));
	did_fail |= test_string("\"30hello\" alpha?", value_set_bool(FALSE));
	did_fail |= test_string("\"350\" alpha?", value_set_bool(FALSE));
	did_fail |= test_string("\"35.0\" alpha?", value_set_bool(FALSE));
	
	did_fail |= test_string("\"hello\" alnum?", value_set_bool(TRUE));
	did_fail |= test_string("\"hel30lo\" alnum?", value_set_bool(TRUE));
	did_fail |= test_string("\"30hello\" alnum?", value_set_bool(TRUE));
	did_fail |= test_string("\"350\" alnum?", value_set_bool(TRUE));
	did_fail |= test_string("\"35.0\" alnum?", value_set_bool(FALSE));
	
	did_fail |= test_string("\"hello\" num?", value_set_bool(FALSE));
	did_fail |= test_string("\"hel30lo\" num?", value_set_bool(FALSE));
	did_fail |= test_string("\"30hello\" num?", value_set_bool(FALSE));
	did_fail |= test_string("\"350\" num?", value_set_bool(TRUE));
	did_fail |= test_string("\"35.0\" num?", value_set_bool(FALSE));
	
	did_fail |= test_string("\"hello\" length", value_set_long(5));
	did_fail |= test_string("\"hello world\" length", value_set_long(11));
	did_fail |= test_string("\"3 \" length", value_set_long(2));
	did_fail |= test_string("\"xxx\" length", value_set_long(3));
	did_fail |= test_string("\"x x x\" length", value_set_long(5));
	
	did_fail |= test_string("\"hello\" lstrip", value_set_str("hello"));
	did_fail |= test_string("\" hello\" lstrip", value_set_str("hello"));
	did_fail |= test_string("\"  hello\" lstrip", value_set_str("hello"));
	did_fail |= test_string("\"hello \" lstrip", value_set_str("hello "));
	did_fail |= test_string("\"hello  \" lstrip", value_set_str("hello  "));
	did_fail |= test_string("\" hello \" lstrip", value_set_str("hello "));
	did_fail |= test_string("\"  hello  \" lstrip", value_set_str("hello  "));
	
	did_fail |= test_string("\"long string for range\" range 0 4", value_set_str("long"));
	did_fail |= test_string("\"long string for range\" range 0 11", value_set_str("long string"));
	did_fail |= test_string("\"long string for range\" range 12 22", value_set_str("for range"));
	did_fail |= test_string("\"long string for range\" range 0 22", value_set_str("long string for range"));
	did_fail |= test_string("\"long string for range\"[0...4]", value_set_str("long"));
	did_fail |= test_string("\"long string for range\"[0...11]", value_set_str("long string"));
	did_fail |= test_string("\"long string for range\"[12...22]", value_set_str("for range"));
	did_fail |= test_string("\"long string for range\"[0...22]", value_set_str("long string for range"));
	
	did_fail |= test_string("\"reppettitions\" replace \"p\" \"NEW\"", value_set_str("reNEWNEWettitions"));
	did_fail |= test_string("\"reppettitions\" replace \"t\" \"NEW\"", value_set_str("reppeNEWNEWiNEWions"));
	did_fail |= test_string("\"reppettitions\" replace \"repp\" \"BEGINNING\"", value_set_str("BEGINNINGettitions"));
	did_fail |= test_string("\"reppettitions\" replace \"reppettitions\" \"WHOLE NEW WORD\"", value_set_str("WHOLE NEW WORD"));
	
	did_fail |= test_string("\"hello\" reverse", value_set_str("olleh"));
	did_fail |= test_string("\"hello world\" reverse", value_set_str("dlrow olleh"));
	did_fail |= test_string("\"\" reverse", value_set_str(""));
	did_fail |= test_string("\"h\" reverse", value_set_str("h"));
	
	did_fail |= test_string("\"hello\" rstrip", value_set_str("hello"));
	did_fail |= test_string("\" hello\" rstrip", value_set_str(" hello"));
	did_fail |= test_string("\"  hello\" rstrip", value_set_str("  hello"));
	did_fail |= test_string("\"hello \" rstrip", value_set_str("hello"));
	did_fail |= test_string("\"hello  \" rstrip", value_set_str("hello"));
	did_fail |= test_string("\" hello \" rstrip", value_set_str(" hello"));
	did_fail |= test_string("\"  hello  \" rstrip", value_set_str("  hello"));
	
	// scan() and split() are not tested because they return arrays, which are more difficult to 
	// quickly create.
		
	did_fail |= test_string("\"hello\" starts_with? \"h\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" starts_with? \"hel\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" starts_with? \"heo\"", value_set_bool(FALSE));
	did_fail |= test_string("\"hello\" starts_with? \"hello\"", value_set_bool(TRUE));
	did_fail |= test_string("\"hello\" starts_with? \"hello.\"", value_set_bool(FALSE));
	did_fail |= test_string("\"hello\" starts_with? \"ello\"", value_set_bool(FALSE));
	
	did_fail |= test_string("\"hello\" strip", value_set_str("hello"));
	did_fail |= test_string("\" hello\" strip", value_set_str("hello"));
	did_fail |= test_string("\"  hello\" strip", value_set_str("hello"));
	did_fail |= test_string("\"hello \" strip", value_set_str("hello"));
	did_fail |= test_string("\"hello  \" strip", value_set_str("hello"));
	did_fail |= test_string("\" hello \" strip", value_set_str("hello"));
	did_fail |= test_string("\"  hello  \" strip", value_set_str("hello"));
	
	did_fail |= test_string("\"hello\" to_upper", value_set_str("HELLO"));
	did_fail |= test_string("\"hELLO\" to_upper", value_set_str("HELLO"));
	did_fail |= test_string("\"HELLO\" to_upper", value_set_str("HELLO"));
	did_fail |= test_string("\"hello World\" to_upper", value_set_str("HELLO WORLD"));
	
	did_fail |= test_string("\"hello\" to_lower", value_set_str("hello"));
	did_fail |= test_string("\"hELLO\" to_lower", value_set_str("hello"));
	did_fail |= test_string("\"HELLO\" to_lower", value_set_str("hello"));
	did_fail |= test_string("\"hello World\" to_lower", value_set_str("hello world"));
	
	if (did_fail) {
		printf("\nTest of strings failed.\n\n");
	} else {
		printf("\nTest of strings succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	return did_fail;
}

/* 
 * Extensively tests arrays. Currently does NOT test: 
 * - some arithmetic functions such as * and &
 * - iterators
 * - shuffle()
 */
int test_arrays()
{
	printf("\n*****\n*****\n*****\n\nTesting arrays.\n\n");
	
	int orig_print_info_p = print_info_p;
	print_info_p = FALSE;
	
	long internal_arr[] = { 2, 4, 5, 8 };
	value arr = value_set_ary_long(internal_arr, 4);
	
	int did_fail = FALSE;
	did_fail |= test_string("array 2 4 5 8", value_set(arr));
	did_fail |= test_string("(array 2 4 5 8)", value_set(arr));
	did_fail |= test_string("array", value_init(VALUE_ARY));
	did_fail |= test_string("(array)", value_init(VALUE_ARY));
	
	did_fail |= test_string("(array 2 4 5) append 8", value_set(arr));
	did_fail |= test_string("(array 2 4) append 5 append 8", value_set(arr));
	did_fail |= test_string("(array) append 2 append 4 append 5 append 8", value_set(arr));
	
	did_fail |= test_string("(array 2 4 5 8)[0]", value_set_long(2));
	did_fail |= test_string("(array 2 4 5 8)[1]", value_set_long(4));
	did_fail |= test_string("(array 2 4 5 8)[2]", value_set_long(5));
	did_fail |= test_string("(array 2 4 5 8)[3]", value_set_long(8));
	did_fail |= test_string("(array 2 4 5 8)[4]", value_init_error());
	did_fail |= test_string("(array 2 4 5 8)[--1]", value_init_error());
	
	did_fail |= test_string("(array 2 4) concat (array 5 8)", value_set(arr));
	did_fail |= test_string("(array 2) concat (array 4 5 8)", value_set(arr));
	did_fail |= test_string("(array 2) concat (array 4) concat (array 5) concat (array 8)", value_set(arr));
	
	did_fail |= test_string("(array) concat (array 2 4 5 8) concat (array)", value_set(arr));
	did_fail |= test_string("(array 2 4 5) + 8", value_set(arr));
	did_fail |= test_string("(array 2 4) + 5 + 8", value_set(arr));
	did_fail |= test_string("(array) + 2 + 4 + 5 + 8", value_set(arr));
	did_fail |= test_string("(array 2 4) + (array 5 8)", value_set(arr));
	did_fail |= test_string("(array 2) + (array 4 5 8)", value_set(arr));

	did_fail |= test_string("(array 2 4 5 0 8) delete 0", value_set(arr));
	did_fail |= test_string("(array 4 2 4 5 8) delete 4", value_set(arr));
	
	did_fail |= test_string("(array 0 2 0 4 5 0 8) delete_all 0", value_set(arr));
	did_fail |= test_string("(array 2 0 0 4 5 8 0) delete_all 0", value_set(arr));
	
	did_fail |= test_string("(array 2 4 10 5 8) delete_at 2", value_set(arr));
	did_fail |= test_string("(array 2 4 5 8 10) delete_at 4", value_set(arr));
	did_fail |= test_string("(array 2 2 4 5 8) delete_at 1", value_set(arr));
	
	did_fail |= test_string("(array 1 2 3 4) empty?", value_set_bool(FALSE));
	did_fail |= test_string("(array 99) empty?", value_set_bool(FALSE));
	did_fail |= test_string("(array) empty?", value_set_bool(TRUE));

	did_fail |= test_string("(array (array 2 4) 5 8) flatten", value_set(arr));
	did_fail |= test_string("(array (array 2) (array 4 5) 8) flatten", value_set(arr));
	did_fail |= test_string("(array (array 2 4) (array 5 8)) flatten", value_set(arr));

	did_fail |= test_string("(array 2 4 5 8) join \"+\"", value_set_str("2+4+5+8"));
	
	did_fail |= test_string("(array 2 4 5 8) last", value_set_long(8));
	did_fail |= test_string("(array 2 4) last", value_set_long(4));
	did_fail |= test_string("(array) last", value_init_nil());

	did_fail |= test_string("(array 2 4 5 8 10) pop", value_set(arr));
	did_fail |= test_string("(array 7) pop", value_init(VALUE_ARY));
	
	did_fail |= test_string("(array 2 4 5 8) size", value_set_long(4));
	did_fail |= test_string("(array) size", value_set_long(0));
	
	did_fail |= test_string("(array 8 5 4 2) sort", value_set(arr));
	did_fail |= test_string("(array 5 4 8 2) sort", value_set(arr));
	
	did_fail |= test_string("(array 2 4 2 5 8) uniq", value_set(arr));
	did_fail |= test_string("(array 2 4 5 4 2 8) uniq", value_set(arr));
	did_fail |= test_string("(array 2 4 2 5 5 8 2 8) uniq", value_set(arr));
	
	did_fail |= test_string("(array 2 4 2 5 8) uniq_sort", value_set(arr));
	did_fail |= test_string("(array 4 2 5 4 2 8) uniq_sort", value_set(arr));
	did_fail |= test_string("(array 4 2 8 5 5 8 2 8) uniq_sort", value_set(arr));
	
	did_fail |= test_string("(array 2 4 5 8) contains? 5", value_set_bool(TRUE));
	did_fail |= test_string("(array 2 4 5 8) contains? 2", value_set_bool(TRUE));
	did_fail |= test_string("(array 2 4 5 8) contains? 8", value_set_bool(TRUE));
	did_fail |= test_string("(array 2 4 5 8) contains? 10", value_set_bool(FALSE));
	did_fail |= test_string("(array 2 4 5 8) contains? \"hello\"", value_set_bool(FALSE));

	did_fail |= test_string("(array 2 4 5 8) index 2", value_set_long(0));
	did_fail |= test_string("(array 2 4 5 8) index 4", value_set_long(1));
	did_fail |= test_string("(array 2 4 5 8) index 5", value_set_long(2));
	did_fail |= test_string("(array 2 4 5 8) index 8", value_set_long(3));
	did_fail |= test_string("(array 2 4 5 8) index 10", value_init_nil());

	did_fail |= test_string("(array 4 5 8) insert 0 2", value_set(arr));
	did_fail |= test_string("(array 2 5 8) insert 1 4", value_set(arr));
	did_fail |= test_string("(array 2 4 5) insert 3 8", value_set(arr));

	did_fail |= test_string("(array 2 4 5 8) length", value_set_long(4));
	did_fail |= test_string("(array) length", value_set_long(0));
	did_fail |= test_string("(array 2 4) length", value_set_long(2));
	did_fail |= test_string("(array 1 2 3 4 5 6 7 8 9) length", value_set_long(9));
	
	did_fail |= test_string("(array 8 5 4 2) reverse", value_set(arr));
	did_fail |= test_string("(array) reverse", value_init(VALUE_ARY));

	did_fail |= test_string("2 cons (array 4 5 8)", value_set(arr));
	did_fail |= test_string("2 cons 4 cons (array 5 8)", value_set(arr));

	did_fail |= test_string("(array 0 1 2 4 5 8) drop 2", value_set(arr));
	did_fail |= test_string("(array 2 4 5 8) drop 0", value_set(arr));
	did_fail |= test_string("(array 1 2 3 4) drop 4", value_init_nil());

	did_fail |= test_string("(array 2 4 5 8) head", value_set_long(2));
	
	did_fail |= test_string("(array 1 2 4 5 8) tail", value_set(arr));
	did_fail |= test_string("(array 8) tail", value_init(VALUE_ARY));

	did_fail |= test_string("(array 2 4 5 8 9 10 11) take 4", value_set(arr));
	did_fail |= test_string("(array 1 2 3) take 0", value_init_nil());


	did_fail |= test_string("(array 2 4 5 8)", value_set(arr));

	if (did_fail) {
		printf("\nTest of arrays failed.\n\n");
	} else {
		printf("\nTest of arrays succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	return did_fail;
}

/* 
 * See test_arrays()
 */
int test_lists()
{
	printf("\n*****\n*****\n*****\n\nTesting lists.\n\n");
	
	int orig_print_info_p = print_info_p;
	print_info_p = FALSE;
	
	value x, arr = value_init_nil();
	x = value_set_long(8);
	value_cons_now2(&x, &arr);
	x = value_set_long(5);
	value_cons_now2(&x, &arr);
	x = value_set_long(4);
	value_cons_now2(&x, &arr);
	x = value_set_long(2);
	value_cons_now2(&x, &arr);
		
	int did_fail = FALSE;
	did_fail |= test_string("list 2 4 5 8", value_set(arr));
	did_fail |= test_string("(list 2 4 5 8)", value_set(arr));
	did_fail |= test_string("list", value_init_nil());
	did_fail |= test_string("(list)", value_init_nil());
	
	did_fail |= test_string("2 cons (list 4 5 8)", value_set(arr));
	did_fail |= test_string("2 cons 4 cons (list 5 8)", value_set(arr));

	did_fail |= test_string("(list 0 1 2 4 5 8) drop 2", value_set(arr));
	did_fail |= test_string("(list 2 4 5 8) drop 0", value_set(arr));
	did_fail |= test_string("(list 1 2 3 4) drop 4", value_init_nil());

	did_fail |= test_string("(list 2 4 5 8) head", value_set_long(2));
	
	did_fail |= test_string("(list 1 2 4 5 8) tail", value_set(arr));
	did_fail |= test_string("(list 8) tail", value_init_nil());

	did_fail |= test_string("(list 2 4 5 8 9 10 11) take 4", value_set(arr));
	did_fail |= test_string("(list 1 2 3) take 0", value_init_nil());

	did_fail |= test_string("(list 2 4 5) append 8", value_set(arr));
	did_fail |= test_string("(list 2 4) append 5 append 8", value_set(arr));
//	did_fail |= test_string("(list) append 2 append 4 append 5 append 8", value_set(arr));
	
	did_fail |= test_string("(list 2 4 5 8)[0]", value_set_long(2));
	did_fail |= test_string("(list 2 4 5 8)[1]", value_set_long(4));
	did_fail |= test_string("(list 2 4 5 8)[2]", value_set_long(5));
	did_fail |= test_string("(list 2 4 5 8)[3]", value_set_long(8));
	did_fail |= test_string("(list 2 4 5 8)[4]", value_init_error());
	did_fail |= test_string("(list 2 4 5 8)[--1]", value_init_error());
	
	did_fail |= test_string("(list 2 4) concat (list 5 8)", value_set(arr));
	did_fail |= test_string("(list 2) concat (list 4 5 8)", value_set(arr));
	did_fail |= test_string("(list 2) concat (list 4) concat (list 5) concat (list 8)", value_set(arr));
	
	did_fail |= test_string("(list) concat (list 2 4 5 8) concat (list)", value_set(arr));
	did_fail |= test_string("(list 2 4 5) + 8", value_set(arr));
	did_fail |= test_string("(list 2 4) + 5 + 8", value_set(arr));
//	did_fail |= test_string("2 + 4 + 5 + 8 + (list)", value_set(arr));
	did_fail |= test_string("(list 2 4) + (list 5 8)", value_set(arr));
	did_fail |= test_string("(list 2) + (list 4 5 8)", value_set(arr));

	did_fail |= test_string("(list 2 4 5 0 8) delete 0", value_set(arr));
	did_fail |= test_string("(list 4 2 4 5 8) delete 4", value_set(arr));
	
	did_fail |= test_string("(list 0 2 0 4 5 0 8) delete_all 0", value_set(arr));
	did_fail |= test_string("(list 2 0 0 4 5 8 0) delete_all 0", value_set(arr));
	
//	did_fail |= test_string("(list 2 4 10 5 8) delete_at 2", value_set(arr));
//	did_fail |= test_string("(list 2 4 5 8 10) delete_at 4", value_set(arr));
//	did_fail |= test_string("(list 2 2 4 5 8) delete_at 1", value_set(arr));
	
	did_fail |= test_string("(list 1 2 3 4) empty?", value_set_bool(FALSE));
	did_fail |= test_string("(list 99) empty?", value_set_bool(FALSE));
	did_fail |= test_string("(list) empty?", value_set_bool(TRUE));

	did_fail |= test_string("(list (list 2 4) 5 8) flatten", value_set(arr));
	did_fail |= test_string("(list (list 2) (list 4 5) 8) flatten", value_set(arr));
	did_fail |= test_string("(list (list 2 4) (list 5 8)) flatten", value_set(arr));

	did_fail |= test_string("(list 2 4 5 8) join \"+\"", value_set_str("2+4+5+8"));
	
	did_fail |= test_string("(list 2 4 5 8) last", value_set_long(8));
	did_fail |= test_string("(list 2 4) last", value_set_long(4));
	did_fail |= test_string("(list) last", value_init_nil());

	did_fail |= test_string("(list 2 4 5 8 10) pop", value_set(arr));
	did_fail |= test_string("(list 7) pop", value_init_nil());
	
	did_fail |= test_string("(list 2 4 5 8) size", value_set_long(4));
//	did_fail |= test_string("(list) size", value_set_long(0));
	
	did_fail |= test_string("(list 8 5 4 2) sort", value_set(arr));
	did_fail |= test_string("(list 5 4 8 2) sort", value_set(arr));
	
//	did_fail |= test_string("(list 2 4 2 5 8) uniq", value_set(arr));
//	did_fail |= test_string("(list 2 4 5 4 2 8) uniq", value_set(arr));
//	did_fail |= test_string("(list 2 4 2 5 5 8 2 8) uniq", value_set(arr));
	
	did_fail |= test_string("(list 2 4 2 5 8) uniq_sort", value_set(arr));
	did_fail |= test_string("(list 4 2 5 4 2 8) uniq_sort", value_set(arr));
	did_fail |= test_string("(list 4 2 8 5 5 8 2 8) uniq_sort", value_set(arr));
	
	did_fail |= test_string("(list 2 4 5 8) contains? 5", value_set_bool(TRUE));
	did_fail |= test_string("(list 2 4 5 8) contains? 2", value_set_bool(TRUE));
	did_fail |= test_string("(list 2 4 5 8) contains? 8", value_set_bool(TRUE));
	did_fail |= test_string("(list 2 4 5 8) contains? 10", value_set_bool(FALSE));
	did_fail |= test_string("(list 2 4 5 8) contains? \"hello\"", value_set_bool(FALSE));

	did_fail |= test_string("(list 2 4 5 8) index 2", value_set_long(0));
	did_fail |= test_string("(list 2 4 5 8) index 4", value_set_long(1));
	did_fail |= test_string("(list 2 4 5 8) index 5", value_set_long(2));
	did_fail |= test_string("(list 2 4 5 8) index 8", value_set_long(3));
	did_fail |= test_string("(list 2 4 5 8) index 10", value_init_nil());

	did_fail |= test_string("(list 4 5 8) insert 0 2", value_set(arr));
	did_fail |= test_string("(list 2 5 8) insert 1 4", value_set(arr));
	did_fail |= test_string("(list 2 4 5) insert 3 8", value_set(arr));

	did_fail |= test_string("(list 2 4 5 8) length", value_set_long(4));
	did_fail |= test_string("(list) length", value_set_long(0));
	did_fail |= test_string("(list 2 4) length", value_set_long(2));
	did_fail |= test_string("(list 1 2 3 4 5 6 7 8 9) length", value_set_long(9));
	
	did_fail |= test_string("(list 8 5 4 2) reverse", value_set(arr));
	did_fail |= test_string("(list) reverse", value_init_nil());

	did_fail |= test_string("(list 2 4 5 8)", value_set(arr));

	if (did_fail) {
		printf("\nTest of lists failed.\n\n");
	} else {
		printf("\nTest of lists succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	return did_fail;

}

/* 
 * Test various errors to make sure they are giving the right message, and not breaking.
 */
int test_errors()
{
	printf("\n*****\n*****\n*****\n\nTesting errors.\n\n");
	
	int orig_print_info_p = print_info_p;
	print_info_p = FALSE;
	
	value err = value_init_error();
	
	int did_fail = FALSE;
	did_fail |= test_string("( 3", err); // closing parenthesis not found
	did_fail |= test_string("3 )", err); // opening parenthesis not found
	did_fail |= test_string("3 + ", err); // missing argument in binary operator
	did_fail |= test_string("+ 3", err); // missing argument in binary operator
	did_fail |= test_string("^&# 3", err); // unrecognized function or value ^&#
	
	if (did_fail) {
		printf("\nTest of errors failed.\n\n");
	} else {
		printf("\nTest of errors succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	return did_fail;
}

/* 
 * Tests various control structures.
 */
int test_controls()
{
	printf("\n*****\n*****\n*****\n\nTesting control structures.\n\n");
	
	int orig_print_info_p = print_info_p;
	int orig_print_errors_p = print_errors_p;
	print_info_p = TRUE;
			
	int did_fail = FALSE;
	
	// If statements.
	did_fail |= test_string("if true 3", value_set_long(3));
	did_fail |= test_string("if false 3", value_init_nil());
	did_fail |= test_string("if true 3 else 5", value_set_long(3));
	did_fail |= test_string("if false 3 else 5", value_set_long(5));

	did_fail |= test_string("if true { 3 }", value_set_long(3));
	did_fail |= test_string("if false { 3 }", value_init_nil());
	did_fail |= test_string("if true { 3 } { 5 }", value_set_long(3));
	did_fail |= test_string("if false { 3 } { 5 }", value_set_long(5));

	did_fail |= test_string("if true { 3 } 5", value_set_long(3));
	did_fail |= test_string("if false { 3 } 5", value_set_long(5));
	did_fail |= test_string("if true then 3 { 5 }", value_set_long(3));
	did_fail |= test_string("if false then 3 { 5 }", value_set_long(5));

	print_errors_p = orig_print_errors_p;

	if (did_fail) {
		printf("\nTest of control structures failed.\n\n");
	} else {
		printf("\nTest of control structures succeeded.\n\n");
	}
	
	printf("*****\n*****\n*****\n\n");
	
	print_info_p = orig_print_info_p;
	return did_fail;
}


int print_time(time_t time)
{
	printf("%ld sec, %ld msec\n", time / 1000000, time % 1000);
}

/* 
 * Calculates how fast different sorting algorithms are for arrays of different lengths.
 * 
 * Pure Sorting Algorithm Speed
 * 
 * insertion_sort1() wins for an array of 4 elements. bubble_sort() wins from 8 to 2048, and 
 * quicksort() wins for every value 4096 and up.
 * 
 * 
 */
int sort_speeds(int min_length, int max_length, int max_repeats)
{
	time_t start, finish, time;
	int i, length;
	for (length = min_length; length <= max_length; length *= 2) {
		int array[length];
		
		start = usec();
		for (i = 0; i < max_repeats / length; ++i)
			fill_array(array, length);
		finish = usec();
		time = finish - start;
		
		// The O(n^2) can't handle an array any longer than this. The time an O(n^2) algorithm takes is 
		// directly proportional to (max_repeats) and proportional to the square of (length).
		if ((length/(double) (1 << 5)) * (length/(double) (1 << 5)) * (max_repeats/(double) (1 << 22)) <= 1) {
			start = usec();
			for (i = 0; i < max_repeats / length; ++i) {
				fill_array(array, length);
				bubble_sort(array, length);
			}
			finish = usec();
			printf("time to sort %d items using bubble: ", length);
			print_time(finish - start - time);

			start = usec();
			for (i = 0; i < max_repeats / length; ++i) {
				fill_array(array, length);
				insertion_sort1(array, length);
			}
			finish = usec();
			printf("time to sort %d items using insertion1: ", length);
			print_time(finish - start - time);

			start = usec();
			for (i = 0; i < max_repeats / length; ++i) {
				fill_array(array, length);
				insertion_sort2(array, length);
			}
			finish = usec();
			printf("time to sort %d items using insertion2: ", length);
			print_time(finish - start - time);
		}

		start = usec();
		for (i = 0; i < max_repeats / length; ++i) {
			fill_array(array, length);
			merge_sort(array, length);
		}
		finish = usec();
		printf("time to sort %d items using merge: ", length);
		print_time(finish - start - time);

		start = usec();
		for (i = 0; i < max_repeats / length; ++i) {
			fill_array(array, length);
			quicksort2(array, length);
		}
		finish = usec();
		printf("time to sort %d items using quicksort2: ", length);
		print_time(finish - start - time);
		
		start = usec();
		for (i = 0; i < max_repeats / length; ++i) {
			fill_array(array, length);
			custom_sort1(array, length);
		}
		finish = usec();
		printf("time to sort %d items using custom1: ", length);
		print_time(finish - start - time);		
		
		printf("\n");
	}
	
	return 0;
}

int value_sort_speeds(int min_length, int max_length, int max_repeats)
{
	time_t start, finish, time;
	int i, length;
	for (length = min_length; length <= max_length; length *= 2) {
		value array[length];
		vfill_array(array, length);
		
		start = usec();
		for (i = 0; i < max_repeats / length; ++i)
			vshuffle_array(array, length);
		finish = usec();
		time = finish - start;
		
//		start = usec();
//		for (i = 0; i < max_repeats / length; ++i) {
//			vshuffle_array(array, length);
//			vmerge_sort(array, length);
//		}
//		finish = usec();
//		printf("time to sort %d items using vmerge: ", length);
//		print_time(finish - start - time);
		
		start = usec();
		for (i = 0; i < max_repeats / length; ++i) {
			vshuffle_array(array, length);
			vcustom_sort2(array, length, 8);
		}
		finish = usec();
		printf("time to sort %d items using cutoff 8: ", length);
		print_time(finish - start - time);
		
		start = usec();
		for (i = 0; i < max_repeats / length; ++i) {
			vshuffle_array(array, length);
			vcustom_sort2(array, length, 16);
		}
		finish = usec();
		printf("time to sort %d items using cutoff 16: ", length);
		print_time(finish - start - time);
		
		start = usec();
		for (i = 0; i < max_repeats / length; ++i) {
			vshuffle_array(array, length);
			vcustom_sort2(array, length, 24);
		}
		finish = usec();
		printf("time to sort %d items using cutoff 24: ", length);
		print_time(finish - start - time);
		
		printf("\n");
	}
	
	return 0;
}

int small_sort_speeds()
{
	return value_sort_speeds(1 << 10, 1 << 15, 1 << 25);
}