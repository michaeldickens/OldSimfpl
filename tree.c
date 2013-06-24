/*
 *  tree.c
 *  Calculator
 *
 *  Created by Michael Dickens on 6/7/10.
 *
 */

#include "tree.h"

/* 
 * 
 * Tree functions.
 * 
 */

tree tree_init()
{
	tree res;
	res.node = value_init_nil();
	res.lf = NULL;
	res.rt = NULL;
	return res;
}

tree tree_set(tree op)
{
	tree res;
	res.node = value_set(op.node);
	
	if (op.lf) {
		res.lf = malloc(sizeof(tree));
		*res.lf = tree_set(*op.lf);
	}

	if (op.rt) {
		res.rt = malloc(sizeof(tree));
		*res.rt = tree_set(*op.rt);
	}
	
	return res;
}

/* Notice that this assigns tree to a reference of op rather than 
 * a copy.
 */
tree tree_set_value(value op)
{
	tree res = tree_init();
	res.node = op;
	return res;
}

void tree_clear(tree *res)
{
	if (res == NULL)
		return;
	value_clear(&res->node);
	tree_clear(res->lf);
	tree_clear(res->rt);
	free(res->lf);
	free(res->rt);
	res->lf = NULL;
	res->rt = NULL;
}

tree tree_error()
{
	tree res;
	res.node = value_init_error();
	res.lf = NULL;
	res.rt = NULL;
	return res;
}

int tree_nil_p(tree op)
{
	return op.ode.type == VALUE_NIL && op.lf == NULL && op.rt == NULL;
}

/* If res.node is a tree, it still only counts one towards the size.
 */
size_t tree_size(tree op)
{
	size_t res = 1;
	if (op.lf)
		res += tree_size(*op.lf);
	if (op.rt)
		res += tree_size(*op.rt);
	return res;
}

void tree_print(tree op)
{
	printf("(");
	value_print(op.node);
	if (op.lf) {
		printf(" ");
		tree_print(*op.lf);
	}
	
	if (op.rt) {
		printf(" ");
		tree_print(*op.rt);
	}
	
	printf(")");
}

void tree_println(tree op)
{
	tree_print(op);
	printf("\n");
}

/* 
 * 
 * Tree functions for value.
 * 
 */

value value_set_tree(tree op)
{
	value res;
	res.type = VALUE_TRE;
	res.core.u_t = malloc(sizeof(tree));
	*res.core.u_t = tree_set(op);
	return res;
}

/* 
 * 
 * Interpreter functions.
 * 
 */

int init_tree_interpreter()
{
	tree_globals = value_hash_init_capacity(5); // Keep it small.
	
	same_type_determiner = SAME_TYPE_TREE;
	
	return 0;
}

value tree_given_statement(char *statement)
{
	size_t wordcount = count_words(statement);
	char *words[wordcount];
	statement_to_words(words, wordcount, statement);
	tree trunk = compile_to_tree(words, wordcount);
	value res = interpret_tree(&tree_globals, trunk);
	tree_clear(&trunk);
	size_t i;
	for (i = 0; i < wordcount; ++i)
		free(words[i]);
	return res;
}

tree compile_to_tree(char *words[], size_t wordcount)
{
	tree res = tree_init();
	
	int balance = 0, rep = 0, inx = -1;
	size_t i, new_wordcount, value_count = 0;
	for (i = 0; i < wordcount && words[i]; ++i) {
		if (streq(words[i], "(")) {
			if (balance == 0)
				inx = i;
			++balance;
		} else if (streq(words[i], ")")) {
			--balance;
			if (balance < 0 || inx == -1) {
				value_error(1, "Syntax Error: Opening parenthesis not found.");
				tree_clear(&res);
				return tree_error();
			}
		}
		
		if (inx == -1)
			new_wordcount = 1;
		else new_wordcount = i-inx-1;
			
		if (balance == 0 && new_wordcount != 1) {
			// Do this only if the new word count is greater than 1.
			
			++value_count;
			tree parenres = compile_to_tree(words+inx+1, new_wordcount);
			if (rep == 0) {
				if (parenres.lf == NULL && parenres.rt == NULL) {
					res = parenres;
				} else {
					res.ode.type = VALUE_TRE;
					res.node.core.u_t = malloc(sizeof(tree));
					*res.node.core.u_t = parenres;
				}

			} else if (rep == 1) {
				res.lf = malloc(sizeof(tree));
				*res.lf = parenres;
			} else if (rep == 2) {
				res.rt = malloc(sizeof(tree));
				*res.rt = parenres;
			} else {
				value_error(1, "Syntax Error: Too many values in tree expression.");
				tree_clear(&res);
				tree_clear(&parenres);
				return tree_error();
			}
			inx = -1;
			++rep;
		} else if (balance == 0) {
			// If the new word count equals 1, evaluate the one value.
			
			++value_count;
			int saved_print_errors_p = print_errors_p;
			print_errors_p = FALSE;
			size_t pos = i;
			if (streq(words[i], ")"))
				--pos;
			value parenres = value_set_str_smart(words[pos], 0);
			print_errors_p = saved_print_errors_p;
			if (arenres.type == VALUE_ERROR) {
				arenres.type = VALUE_ID;
				parenres.core.u_id = malloc(strlen(words[pos])+1);
				strcpy(parenres.core.u_id, words[pos]);
			}
			
			if (rep == 0) {
				res.node = parenres;
			} else if (rep == 1) {
				res.lf = malloc(sizeof(tree));
				*res.lf = tree_set_value(parenres);
			} else if (rep == 2) {
				res.rt = malloc(sizeof(tree));
				*res.rt = tree_set_value(parenres);
			} else {
				char buf[20];
				sprintf(buf, "%ld", (long) value_count);
				value_error(1, "Syntax Error: Too many values in tree expression (3 expected, %s found).", buf);
				tree_clear(&res);
				value_clear(&parenres);
				return tree_error();
			}
			inx = -1;
			++rep;
		}
	}
	
	if (balance > 0) {
		value_error(1, "Syntax Error: Closing parenthesis not found.");
		tree_clear(&res);
		return tree_error();
	}
	
	return res;
}

value interpret_tree(value *variables, tree trunk)
{
//	value new_variables = value_set(*variables);
	value res = value_init_nil();
	int is_error = FALSE;
	if (trunk.ode.type == VALUE_BIF) {
		// Do something special.
		if (streq(trunk.node.core.u_id, "grow")) // This line is a bad idea because trunk.node is BIF, not ID.
		{
			if (trunk.lf == NULL) {
//				value_clear(&new_variables);
				return value_init_nil();
			} else if (trunk.lf->node.type != VALUE_ID) {
				value_error(1, "Type Error: grow is undefined where lf is %t (id expected).", trunk.lf->node);
//				value_clear(&new_variables);
				return value_init_error();
			}
			
			value lf = value_set(trunk.lf->node); // This line is causing memory leakage.
			value rt = value_init_nil();
			if (trunk.rt) {
//				rt = interpret_tree(&new_variables, *trunk.rt);
				rt = interpret_tree(variables, *trunk.rt);
			}
			
			value_hash_put_refs(variables, &lf, &rt);
			res = value_set(rt);
		} else if (is_primitive(trunk.node)) {
			if (trunk.lf == NULL) {
				value_error(1, "Type Error: %s is undefined where lf is nil.", trunk.node);
				is_error = TRUE;
			} else if (tree_size(*trunk.lf) > 1) {
				value_error(1, "Error: %s is undefined where lf is a tree with size greater than 1.", trunk.node);
				is_error = TRUE;
			}
			if (trunk.rt == NULL) {
				value_error(1, "Type Error: %s is undefined where rt is nil.", trunk.node);
				is_error = TRUE;
			} else if (tree_size(*trunk.rt) > 1) {
				value_error(1, "Error: %s is undefined where rt is a tree with size greater than 1.", trunk.node);
				is_error = TRUE;
			}
			if (is_error) {
//				value_clear(&new_variables);
				return value_init_error();
			}
			
			value ops[3];
			ops[2].type = VALUE_BIF;
			ops[2].core.u_bif = trunk.node.core.u_bif;
			ops[1] = trunk.lf->node;
			ops[0] = trunk.rt->node;
			// Evaluate somehow.
			
		} else if (value_hash_exists(*variables, trunk.node)) {
			res = value_hash_get(*variables, trunk.node);
		} else {
			value_error(1, "Error: Undefined id %t.", trunk.node);
			res = value_init_error();
		}

		
	} else if (trunk.lf == NULL && trunk.rt == NULL) {
		res = value_set(trunk.node);
	} else {
		int which = value_true_p(trunk.node);
		if (which) {
			if (trunk.lf)
//				res = interpret_tree(&new_variables, *trunk.lf);
				res = interpret_tree(variables, *trunk.lf);
		} else {
			if (trunk.rt)
//				res = interpret_tree(&new_variables, *trunk.rt);
				res = interpret_tree(variables, *trunk.rt);
		}
	}
	
//	value_clear(&new_variables);
	
	return res;
}

int test_tree_inputs()
{
	// I should really implement this function. When I do, I'll test these: 
	// 
	// false (100 \"hello\" 6.5) -3 ==> -3
	// true  (100 \"hello\" 6.5) -3 ==> "hello"
	// true  (  0 \"hello\" 6.5) -3 ==> 6.5
	// false (100 \"hello\" 6.5) (-3 2) ==> 2
	// false (100 \"hello\" 6.5) (-0 2) ==> nil
	// grow x 100 ==> 100
	// x ==> 100
	
	return 0;
}

