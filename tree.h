/*
 *  tree.h
 *  Calculator
 *
 *  Created by Michael Dickens on 6/7/10.
 *  
 *  This is a little side project that's sort of an experimental programming language based 
 *  on Lisp. It is put with these other files because then it will be able to call on them.
 * 
 *  tree.c uses some of the functions found in interpreter.c, but runs an entirely different 
 *  language. While interpreter.c uses interpret_values() as the core function, tree.c first 
 *  compiles the input into trees and then evaluates the trees.
 *
 */


/* 
 * BUGS
 * 
 * Sometimes when it tries to print a tree, it goes into an infinite loop. I'm not sure why 
 * and it looks like it's not happening anywhere right now, but it may still be possible.
 * 
 */

#include "interpreter.h"

#define TREE_TREE	0
#define TREE_VALUE	1

typedef struct tree_struct {
	value node;	
	struct tree_struct *lf; // pointer to left branch
	struct tree_struct *rt; // pointer to right branch
} tree;

value tree_globals;

/* 
 * Tree functions.
 */

tree tree_init();
tree tree_set(tree op);
tree tree_set_value(value op);
void tree_clear(tree *res);
tree tree_error();
int tree_nil_p(tree op);
size_t tree_size(tree op);
void tree_print(tree op);
void tree_println(tree op);

/* 
 * Tree functions for value.
 */
value value_set_tree(tree op);

/* 
 * Interpreter functions.
 */

int init_tree_interpreter();
value tree_given_statement(char *statement);
tree compile_to_tree(char *words[], size_t wordcount);
value interpret_tree(value *variables, tree trunk);

int test_tree_inputs();
