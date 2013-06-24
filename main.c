/*
 *  main.m
 *  Simfpl
 *
 *  Created by Michael Dickens on 12/5/09.
 *
 */


#include "sexp_to_c.h"


value f(value op);
int test_regex();
int test_array();
int test_concat();
int test_list();
int test_hash();
int test_scope_change_speed();
int test_memory();

int consecutive_primes();

int main(int argc, const char *argv[])
{
	init_tools();
	init_values();
	init_evaluator();
	init_interpreter();
	init_tests();
	init_sexp_to_c();
	
	// TO TRY: Add an -O3 flag to Xcode's compile. Then compile and profile, and see if it runs faster.
	
//	run_tests();
//	run_benchmarks();
		
	if (argc > 1) {
		if (streq(argv[1], "test")) {
			run_tests();
		} else if (streq(argv[1], "benchmark")) {
			run_benchmarks();
		} else {
			value str = value_set_str(argv[1]);
			value_import(str);
			value_clear(&str);
		}
	} else {
		run_interpreter();
	}
	
	return 0;
}



value f(value op)
{
	return op;
}

int test_regex()
{
	regex_match("lalala happy", "([a-z]+) ([a-z]+)");
	regex_match("234between5", "[a-z]+");
	regex_match("9238ASLDKF", "[a-z]+");
	
	return 0;
}

int regex_match(char *str, char *regex)
{
	regex_t compiled;
	int r = regcomp(&compiled, regex, REG_EXTENDED);
	if (r != 0) {
		printf("Error: Regular expression did not compile.\n");
		return 1;
	}
	
	regmatch_t matches[5];
	int i, j;
	int match = regexec(&compiled, str, 5, matches, 0);
	if (match == 0)
		for (i = 0; i < 5; ++i) {
			if (matches[i].rm_so >= 0) {
				printf("match: ");
				for (j = matches[i].rm_so; j < matches[i].rm_eo; ++j)
					printf("%c", str[j]);
				printf("\n");
			}
		}
	else printf("no match\n");
	printf("\n");
	
	return 0;
}

int test_list()
{
	value list = value_init_nil();
	value num = value_set_long(10);
	long i, count;
	size_t start = usec();
	
	for (count = 0; count < 5; ++count) {
		for (i = 0; i < 10000000; ++i) {
			value_cons_now(num, &list);
		}
		value_clear(&list);
	}
	
	size_t finish = usec();
	printf("time to do %ld iterations: %ld\n", i, (long) (finish - start) / count);
	
	return 0;
}


/* 
 * For 32,000 elements
 * NIL: 0.01 sec
 * all: 0.17 sec
 * MPZ: 0.21 sec
 * MPF: 0.19 sec
 * STR: 0.24 sec
 * ID : 0.18 sec
 * LST: 0.04 sec
 * 
 * 
 */ 
int test_hash()
{
#define NUMBER ((1 << 15) + 1)
#define REPEATS 20
	
	value x = value_init_nil();
	
	value val = value_set_str("result");
	size_t i, j, k, start, finish;
	
	value keys[NUMBER];
	char *skeys[NUMBER];
	for (i = 0; i < NUMBER; ++i) {
		skeys[i] = value_malloc(NULL, 30);
		sprintf(skeys[i], "%ld", (long) i);
		
		value num = value_set_long(i);
		
		switch (i % 4) {
			case 0:
				keys[i] = value_set(num);
				break;
			case 1:
				keys[i] = value_set_double(i);
				break;
			case 2:
				keys[i] = value_cast(num, VALUE_STR);
				break;
			case 3:
				keys[i] = value_cast(num, VALUE_STR);
				keys[i].type = VALUE_ID;
				break;
			case 4:
				keys[i] = value_init(VALUE_LST);
				value_cons_now2(&num, &keys[i]);
				break;
			default:
				keys[i] = value_init_nil();
				break;
		}
		
		value_clear(&num);
		
		// This overrides the setting done above.
//		keys[i] = value_cast(value_set_long(i), VALUE_STR);
	}
	
	for (i = 1 << 4; i < NUMBER; i <<= 1) {
		long average = 0;
		long difference = 0;
		
		for (j = 0; j < REPEATS; ++j) {
			x = value_init(VALUE_HSH);
//			StrMap *map = strmap_new(HASH_DEFAULT_CAPACITY);
			
			start = usec();
			
			for (k = 0; k < i; ++k)
				value_hash_put(&x, keys[k], val);
//				strmap_put(map, skeys[k], sval);
			
			finish = usec();
			average += finish - start;
			if (labs((long) (average / (j+1)) - (finish - start)) > difference)
				difference = labs((long) (average / (j+1)) - (finish - start));

			value_clear(&x);
//			strmap_delete(map);
		}
		
		printf("time for %ld elements: %ld usec +/= %ld\n", (long) i, average / j, difference);
	}

#undef NUMBER
#undef REPEATS

	return 0;
}

int test_memory()
{
	int i, j;
	value var, ten = value_set_long(10);
	value container = value_init_nil();

	printf("testing memory\n");
	
	var.type = VALUE_VAR;
	var.core.u_var = "x";
	
	for (i = 1; i < 10000000; i *= 2) {
		container = value_init(VALUE_MPZ);
		size_t start = usec();
		for (j = 0; j < i; ++j) {
			value_add_now(&container, ten);
		}
		
		
		value_clear(&container);
		size_t finish = usec();
		printf("time to do and clear %d elements: %ld usec\n", i, (unsigned long) finish - start);
	}
	
//	size_t time = usec();
//	while (usec() - time < 5000000) ;
	
	return 0;
}

/* 
 * Find the longest sum of consecutive primes that add up to a prime number 
 * below one million.
 */
int consecutive_primes()
{
	value primes = value_init(VALUE_ARY);
	value i, million = value_set_long(1000000);
	for (i = value_set_long(0); value_lt(i, million); value_inc_now(&i))
		if (value_probab_prime_p(i))
			value_append_now(&primes, i);
	
	return 0;
}