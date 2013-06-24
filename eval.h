/*
 *  eval.h
 *  Simfpl
 *
 *  Created by Michael Dickens on 1/17/10.
 *  
 *  This file is used for evaluating s-expressions.
 *  
 */

#include "value.h"

int print_info_p;


// global_variables declared in values.h.
value outer_variables;
value ud_functions;

// These are initialized in init_interpreter().
value function_funs;

int init_evaluator();
void add_function(char *name, value fun, char *spec);

int is_primitive(value id);
int is_function(value id);
int is_symbol(char *id);

// See value.h for declaration of compile_spec().

struct value_spec get_spec(value id);
int primitive_needs_variables(value id);
int primitive_keep_arg(value id);
int primitive_delay_eval(value id);
size_t arg_count(value id);
char primitive_associativity(value id);
int primitive_precedence(value id);

