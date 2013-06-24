/*
 *  tools.h
 *  Simfpl
 *
 *  Created by Michael Dickens on 1/9/10.
 *  
 *  Contains miscellaneous tools, such as an error function and some functions for 
 *  manipulating arrays of words. Also contains variables and functions that need 
 *  to be included into every file.
 */

#include "strmap.h"
#include "random.h"

#include "gmp-4.3.2/gmp.h"
#include "mpfr-3.0.0/mpfr.h"
//#include <gmp.h>
//#include <mpfr.h>

#ifndef CTYPE_H
#include <ctype.h>
#endif
#ifndef FLOAT_H
#include <float.h>
#endif
#ifndef LIMITS_H
#include <limits.h>
#endif
#ifndef MATH_H
#include <math.h>
#endif
#ifndef REGEX_H
#include <regex.h>
#endif
#ifndef STDARG_H
#include <stdarg.h>
#endif
#ifndef STDINT_H
#include <stdint.h>
#endif
#ifndef STDIO_H
#include <stdio.h>
#endif
#ifndef STDLIB_H
#include <stdlib.h>
#endif
#ifndef TIME_H
#include <time.h>
#endif


#define E_TYPE unsigned int


#define SMALLBUFSIZE 32
#define BUFSIZE 256
#define BIGBUFSIZE 1048576

#define FALSE 0
#define TRUE 1

#define streq(str1, str2) (strcmp((str1), (str2)) == 0)
#define strne(str1, str2) (strcmp((str1), (str2)) != 0)


#define string_to_number(string) value_set_str(string, 0)

struct value_array {
	struct value_struct *a;
	size_t length;
};

struct value_hash {
	struct value_struct *a;
	size_t length, occupied, size;
};

struct value_block {
	struct value_struct *a;
	size_t length;
	char *s;
};

struct value_stop {
	int type : 8;
	struct value_struct *core;
};

#define STOP_BREAK 0
#define STOP_CONTINUE 1
#define STOP_YIELD 2
#define STOP_RETURN 3
#define STOP_EXIT 4

/* 
 * Function specification.
 */
struct value_spec {
	int change_scope_p : 2;
	int needs_variables_p : 2;
	int keep_arg_p : 2;
	int delay_eval_p : 2;
	int argc;
	int optional; // Every argument after this one is optional.
	int rest_p : 2;
	char associativity;
	int precedence : 8;
	int not_stop_p : 2; // Internal. Used by iterators only.
};

#define NEEDS_UD_FUNCTIONS -1

struct value_bif {
	struct value_spec spec;
	struct value_struct (*f)(int argc, struct value_struct *argv);
};

typedef struct value_exception {
	struct value_exception *parent;
	char *name;
	char *description;
	struct value_struct *stack_trace; // Pointer to an array containing the stack trace.
} exception;

/* 
 * The definition for the value struct.
 */
typedef struct value_struct {
	int type : 8;
	union {
		int u_b;
		int u_nil;
		long u_z;
		double u_f;
		mpz_t u_mz;
		mpfr_t u_mf;
		char *u_s;
		char *u_x; // Regex.
		struct value_array u_a;
		struct value_struct *u_l;
		struct value_pair *u_p;
		struct value_hash u_h;
		struct value_struct *u_ptr;
		struct tree_struct *u_t;
		struct value_range *u_r;
		struct value_block u_blk;
		char *u_id;
		char *u_var;
		struct value_stop u_stop;
		struct value_spec u_spec;
		int u_type;
		struct value_bif *u_bif;
		struct value_function *u_udf; // Contains a pointer to an ID with the name.
		struct value_exception u_exc;
	} core;
} value;

// These have to be defined after value_struct because they contain a value_struct.

struct value_range {
	int inclusive_p;
	struct value_struct min;
	struct value_struct max;
};

struct value_pair {
	struct value_struct head;
	struct value_struct tail;
};

struct value_function {
	char *name;
	struct value_spec spec;
	struct value_struct vars; // A block containing the variable names.
	struct value_struct body;
};

FILE *input_stream;
int print_interpreter_stuff;

// Is the file stream being read at the end of the file yet?
int is_eof;

int print_errors_p;
int error_count;
int linenum;

#define SAME_TYPE_VALUE 0
#define SAME_TYPE_TREE  1
int same_type_determiner;

/* 
 * Allocates (op) to be (size) blocks of memory. value_malloc() simply 
 * initializes the memory to NULL and makes a call to value_realloc(). 
 * value_realloc() uses op->type to determine how many bytes to allocate. 
 * (size) should be however many blocks of memory you want. If op->type 
 * is VALUE_STR and size is 10, enough memory will be allocated for a 
 * 10-character string (9 text and 1 null terminator).
 * 
 * Defined in value.c.
 */
void * value_malloc(value *op, size_t size);
void * value_realloc(value *op, size_t size);

/* 
 * Return Codes
 * 0: Success.
 * -1: ptr could not be freed.
 */
//int value_value_free(void *ptr);
#define value_free(ptr) free(ptr)

/* 
 * This function must be called for the other functions in tools.c to work.
 */
int init_tools();

int iscsymbol(char c);
#define isbracket(c) ((c) == '(' || (c) == ')' || (c) == '[' || (c) == ']' || (c) == '{' || (c) == '}')

int print_words(char *words[], int length);

int streq2(char *s, char *t);
int strne2(char *s, char *t);

/* 
 * Determines whether (str) is a string representation of a number. Returns 0 if not, 
 * returns 1 if (str) represents an integer, and returns 2 if (str) represents a 
 * float.
 */
int minus_isnumeric(char *str);
int isnumeric(char *str);

/* 
 * Determines whether (str) is a word -- that is, a letter followed by a series of 
 * letters or numbers.
 */
int isword(char *str);

int is_string_literal(char *str);
int is_regex_literal(char *str);

char * convert_to_literal(char *str);
char * convert_regex_to_literal(char *str);

/* 
 * Returns the first index of (word) in (words). If not found, returns -1.
 */
int array_index(char *words[], int length, char *word);

/* 
 * Returns the last index of (word) in (words). If not found, returns -1.
 */
int array_last_index(char *words[], int length, char *word);

/* 
 * Copies (source) into (target).
 */
int array_copy(char *target[], char *source[], int size);

/* 
 * Reports the number of errors found. If the error count is greater than 0, the program 
 * will exit.
 */ 
int fatal_error();
int handle_errors();

#define RESIZE_MIN 16

size_t next_power_of_2(size_t x);
size_t next_size(size_t x);
int is_power_of_2(size_t x);
int resize_p(size_t x);

