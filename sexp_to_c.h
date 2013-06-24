/*
 *  sexp_to_c.h
 *  Simfpl
 *
 *  Created by Michael Dickens on 8/22/10.
 *
 */

#include "tests.h"

#define S2C_IF 1
#define S2C_TIMES 2

int init_sexp_to_c();

value create_associated_numbers(value sexp);
value create_associated_numbers_recursive(value sexp, value start);
value get_count(value counts);

int sexp_to_c(FILE *stream, value sexp);
int sexp_to_c_recursive(FILE *stream, value sexp, value counts);

