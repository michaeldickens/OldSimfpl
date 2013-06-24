/*
 *  value_block.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 4/4/10.
 *
 */

/* 
 * A block is an array of values that can be called as if it were a function. 
 * It is internally represented by a struct containing an array and a 
 * description.
 * 
 * A built-in function contains a pointer to a C function. This is considerably 
 * more efficient than external functions.
 * 
 * A user-defined function contains an ID with the name of the function. When 
 * it is called, the function is retrieved from the variable list. It happens 
 * then and not earlier because it's possible for the function to be modified 
 * after compile time but before it's called.
 * 
 * An unnamed function contains a pointer to two values. One is the list of 
 * variable names, and the other is a block.
 */

#include "value.h"

value value_set_block(value array[], size_t length)
{
	if (length < 0)
		length = 0;
	value res;
	res.type = VALUE_BLK;
	res.core.u_blk.s = NULL;
	res.core.u_blk.a = value_malloc(NULL, sizeof(value) * next_size(length));
	return_if_null(res.core.u_blk.a);
	res.core.u_blk.length = length;
	
	unsigned long i;
	for (i = 0; i < length; ++i)
		res.core.u_blk.a[i] = value_set(array[i]);
	
	return res;
}

value value_set_fun(value (*fun)(int argc, value argv[]))
{
	value res;
	
	res.type = VALUE_BIF;
	res.core.u_bif = value_malloc(NULL, sizeof(struct value_bif));
	return_if_null(res.core.u_bif);
	res.core.u_bif->f = fun;
	res.core.u_bif->spec = value_nil_function_spec;
	return res;
}

int value_commutative_p(value fun)
{
	if (fun.type == VALUE_BIF) {
		value (*f)(int argc, value argv[]) = fun.core.u_bif->f;
		
		if (f == &value_add_arg || f == &value_mul_arg) {
			return TRUE;
		} else {
			return FALSE;
		}

	}
	
	return FALSE;
}

value value_call(value *variables, value func, int argc, value argv[])
{
	value res = value_init_nil();
	if (func.type == VALUE_BIF) {
		res = value_bifcall(func, argc, argv);
	} else if (func.type == VALUE_UDF) {
		res = value_udfcall(variables, func, argc, argv);
	} else if (func.type == VALUE_BLK) {
		res = eval(variables, func);
	} else {
		value_error(1, "Type Error: call() is undefined where op1 is %ts (function or block expected).", func);
		res = value_init_error();
	}

	return res;
}

value value_bifcall_sexp(value *variables, value *ud_functions, value sexp)
{
	size_t length = sexp.core.u_blk.length;
	int error_p = FALSE;
	value res = value_init_nil();
	
	struct value_spec spec = sexp.core.u_blk.a[0].core.u_bif->spec;

	// Call the function that's pointed to by the VALUE_BIF.
	
	int argc = spec.argc;
	if (argc < length && spec.rest_p)
		argc = length - 1;
			
	int args_length = argc+1;
	value args[args_length];
	int clear_args_p[args_length];
	value *vptrs[args_length]; // holds pointers to variables inside of a variable list
	memset(clear_args_p, 0, sizeof(int) * (args_length)); // the default for clear_args_p is FALSE; 
														  // that is, don't clear unless specifically told to
	memset(vptrs, 0, sizeof(value) * (args_length));
	
	// The reason for vptrs is because of how value copying works. If the function to be called 
	// takes a pointer, the pointer has to be a reference to the correct value. (args) will not 
	// reference the correct value because the value is copied from (sexp). (vptrs) therefore 
	// holds pointers to the actual values. After the s-expression is evaluated, (vptrs) is set 
	// to (args), thus making it point to the new values.
	
	size_t i, j = 0;
	if (spec.needs_variables_p == NEEDS_UD_FUNCTIONS)
		args[j++] = value_refer(ud_functions);
	else if (spec.needs_variables_p == TRUE)
		args[j++] = value_refer(variables);
	
	// Move the contents of (sexp) over to (args), which will be passed into the called function 
	// as the argument list.
	for (i = 1; i < length; ++i, ++j) {
		if (spec.delay_eval_p) {
			args[j] = sexp.core.u_blk.a[i];
		} else if (sexp.core.u_blk.a[i].type == VALUE_VAR && !(i == 1 && spec.keep_arg_p)) {
			// sexp.core.u_blk.a[i] is a variable. Find its value and put the value in its place. 
			// Finds a reference to the value, not a copy.
			
			if (value_hash_exists(*variables, sexp.core.u_blk.a[i]))
				vptrs[j] = value_hash_get_ref(*variables, sexp.core.u_blk.a[i]);
			else if (value_hash_exists(global_variables, sexp.core.u_blk.a[i]))
				vptrs[j] = value_hash_get_ref(global_variables, sexp.core.u_blk.a[i]);
			else {
				value_error(1, "Error: Unrecognized function or value %s.", sexp.core.u_blk.a[i]);
				error_p = TRUE;
				// Don't break yet; keep executing to see if there are more errors.
			}
			if (vptrs[j])
				args[j] = *vptrs[j];
			
		} else if (sexp.core.u_blk.a[i].type == VALUE_BLK) {
			// sexp.core.u_blk.a[i] is an s-expression. Evaluate it.
			args[j] = eval_generic(variables, sexp.core.u_blk.a[i], TRUE);
			if (args[j].type == VALUE_ERROR)
				error_p = TRUE;
			clear_args_p[j] = TRUE;
		} else {
			args[j] = sexp.core.u_blk.a[i];
		}
	}
	
	int necessary_length = argc;
	if (spec.needs_variables_p)
		++necessary_length;
	if (length - 1 < necessary_length) {
		for (; j < necessary_length; ++j)
			if (j < spec.optional) args[j].type = VALUE_MISSING_ARG;
			else args[j].type = VALUE_NIL;
	}
	
	// Missing and extra arguments can't be identified at compile time because then (quote) 
	// won't work.
	if (spec.rest_p == FALSE && j < necessary_length) {
		value_error(1, "Argument Error: In %s, %d extra arguments (%d expected, %d found).", sexp.core.u_blk.a[0], args_length - j, j, args_length);
		error_p = TRUE;
	}
	
	if (error_p) {
		res = value_init_error();
	} else res = (*sexp.core.u_blk.a[0].core.u_bif->f)(j, args);
	
	i = 0;
	if (spec.needs_variables_p)
		++i; // We don't want to accidentally clear the variables, which are 
			 // held in args[0].
	for ( ; i < j; ++i) {
		if (clear_args_p[i])
			value_clear(&args[i]);
		else if (vptrs[i])
			*vptrs[i] = args[i];
	}
	
	return res;
}

value value_bifcall(value op, int argc, value argv[])
{
	if (op.type == VALUE_BIF) {
		return (*(op.core.u_bif->f))(argc, argv);
	} else {
		value_error(1, "Type Error: bifcall() is undefined where op is %ts (function expected).", op);
		return value_init_error();
	}
}

value value_udfcall(value *variables, value op, int argc, value argv[])
{
	if (op.type != VALUE_UDF) {
		value_error(1, "Type Error: udfcall() is undefined where op is %ts (user-defined function expected).", op);
		return value_init_error();
	}
			
	int fargc = op.core.u_udf->spec.argc;
	if (fargc > argc) {
		value_error(1, "Argument Error: In %s, %d missing arguments (%d expected, %d found).", op, fargc - argc, fargc, argc);
		return value_init_error();
	}
	
	int change_scope_p = op.core.u_udf->spec.change_scope_p;
	int delay_eval_p = op.core.u_udf->spec.delay_eval_p;
			
	value varkeys = op.core.u_udf->vars;
	value vnil = value_init_nil();
		
	size_t i;
	
	value one_array[1];
	value *new_vars = one_array;
	
	// Add variables from the function call to the list of variables.
	if (change_scope_p) {
		*new_vars = value_hash_init();
		return_if_error(*new_vars);
	} else {
		// When the scope is not changed, add the function variables to the 
		// variable list and remove them afterwards.
		
		new_vars = variables;
	}
		
	size_t length = varkeys.type == VALUE_BLK ? varkeys.core.u_blk.length : 1;
	
	int existed[length];

	if (varkeys.type == VALUE_BLK) {
		for (i = 0; i < varkeys.core.u_blk.length; ++i) {
			value key = value_set(varkeys.core.u_blk.a[i]);
			
			if (change_scope_p == FALSE)
				if (value_hash_exists(*new_vars, key))
					existed[i] = TRUE;
				else existed[i] = FALSE;
			
			if (i >= argc) {
				value_hash_put_refs(new_vars, &key, &vnil);
			} else {
				value x;
				if (delay_eval_p)
					x = value_set(argv[i]);
				else x = eval(variables, argv[i]);
				value_hash_put_refs(new_vars, &key, &x);
			}
		}
	} else if (varkeys.type == VALUE_VAR) {
		value key = value_set(varkeys);
		if (change_scope_p == FALSE && value_hash_exists(*new_vars, key))
			existed[0] = TRUE;
		else existed[0] = FALSE;
		
		if (0 >= argc) {
			value_hash_put_refs(new_vars, &key, &vnil);
		} else {
			value x;
			if (delay_eval_p)
				x = value_set(argv[0]);
			else x = eval(variables, argv[0]);
			value_hash_put_refs(new_vars, &key, &x);
		}		
	}

	value res = eval(new_vars, op.core.u_udf->body);
	
	// Remove the (return) wrapper.
	if (op.core.u_udf->spec.not_stop_p == FALSE && res.type == VALUE_STOP && res.core.u_stop.type == STOP_RETURN) {
		value temp = *res.core.u_stop.core;
		value_free(res.core.u_stop.core);
		res = temp;
	}
	
	// If the scope didn't change, remove any variables that were added.
	if (change_scope_p == FALSE) {
		if (varkeys.type == VALUE_BLK) {
			for (i = 0; i < length; ++i) {
				if (existed[i] == FALSE) {
					value_hash_delete_at_void(new_vars, varkeys.core.u_blk.a[i]);
				}
			}
		} else {
			if (existed[0] == FALSE) {
				value_hash_delete_at_void(new_vars, varkeys);
			}
		}

	} else value_clear(new_vars);
	
	return res;
}

value value_def(value *variables, value name, value vars, value body)
{
	int error_p = FALSE;
	int named_p = TRUE;
	
	if (name.type == VALUE_BIF) {
		value *ptr = value_hash_get_ref(primitive_names, name);
		name = *ptr;
	} else if (name.type == VALUE_UDF) {
		value temp;
		temp.type = VALUE_VAR;
		temp.core.u_var = name.core.u_udf->name;
		name = temp;
	} else if (name.type == VALUE_NIL) {
		named_p = FALSE;
	}
	
	if (named_p && name.type != VALUE_VAR) {
		value_error(1, "Type Error: def() is undefined where name is %ts (variable expected).", name);
		error_p = TRUE;
	}
	
	if (vars.type == VALUE_VAR) {
		value temp = value_init(VALUE_BLK);
		value_append_now2(&temp, &vars);
		vars = temp;
	}
	
	if (vars.type != VALUE_BLK && vars.type != VALUE_NIL) {
		value_error(1, "Type Error: def() is undefined where vars is %ts (block expected).", vars);
		error_p = TRUE;
	}
	
	if (vars.type == VALUE_NIL)
		vars = value_init(VALUE_BLK);
	
	struct value_spec spec = compile_spec("0l15");
	
	value fvars = value_init(VALUE_BLK);

	// Add variables to the function definition.
	size_t i;
	for (i = 0; i < vars.core.u_blk.length; ++i) {
		if (vars.core.u_blk.a[i].type == VALUE_VAR) {
			++spec.argc;
			value_append_now(&fvars, vars.core.u_blk.a[i]);
		} else if (vars.core.u_blk.a[i].type == VALUE_SYM) {
			if (streq(vars.core.u_blk.a[i].core.u_s, "delay_eval")) {
				spec.delay_eval_p = TRUE;
			} else if (streq(vars.core.u_blk.a[i].core.u_s, "keep_scope")) {
				spec.change_scope_p = FALSE;
//			} else if (streq(vars.core.u_blk.a[i].core.u_s, "rest")) {
				// This is not fully implemented yet. The UDF has to have a way to access the argument list.
//				rest_p = TRUE;
			} else {
				value_error(1, "Type Error: in def() variable list, undefined symbol %s.", vars.core.u_blk.a[i]);
				error_p = TRUE;
			}
		} else {
			value_error(1, "Type Error: def() is undefined where one of the function arguments is %ts (variable or symbol expected).", vars.core.u_blk.a[i]);
			error_p = TRUE;
		}
	}
	
	if (error_p) {
		return value_init_error();
	}
	
	value fun;
	
	if (named_p) {
		// Create a user-defined function.
		
		fun.type = VALUE_UDF;
		fun.core.u_udf = value_malloc(NULL, sizeof(struct value_function));
		return_if_null(fun.core.u_udf);
		fun.core.u_udf->name = value_malloc(NULL, strlen(name.core.u_var) + 1);
		return_if_null(fun.core.u_udf->name);
		strcpy(fun.core.u_udf->name, name.core.u_var);
		
		fun.core.u_udf->spec = spec;
		
		fun.core.u_udf->vars = fvars;
		fun.core.u_udf->body = value_set(body);
			
		value_hash_put(variables, name, fun);
	} else {
		// Create an unnamed function.

		fun.type = VALUE_UDF;
		fun.core.u_udf = value_malloc(NULL, sizeof(struct value_function));
		return_if_null(fun.core.u_udf);
		fun.core.u_udf->name = NULL;
		
		fun.core.u_udf->spec = spec;
		
		fun.core.u_udf->vars = fvars;
		fun.core.u_udf->body = value_set(body);
	}

			
	return fun;
}

value value_lambda(value *variables, value vars, value body)
{
	return value_def(variables, value_nil, vars, body);
}

value value_defmacro(value *variables, value name, value vars, value body)
{
	return value_init_error();
}

value value_def_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "def()") ? value_init_error() : value_def(tmp, argv[1], argv[2], argv[3]);
}

value value_lambda_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "lambda()") ? value_init_error() : value_lambda(tmp, argv[1], argv[2]);
}

value value_quote(value *variables, value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_BLK) {
		if (op.core.u_blk.a[0].type == VALUE_BIF && (op.core.u_blk.a[0].core.u_bif->f == &value_dequote_arg || 
			op.core.u_blk.a[0].core.u_bif->f == &value_devar_arg)) {
			res = eval(variables, op);
			
		} else {
			res = value_init(VALUE_BLK);
			value_malloc(&res, next_size(op.core.u_blk.length));
			return_if_error(res);
			res.core.u_blk.length = op.core.u_blk.length;
			
			size_t i;
			for (i = 0; i < op.core.u_blk.length; ++i) {
				res.core.u_blk.a[i] = value_quote(variables, op.core.u_blk.a[i]);
			}
		}

		
	} else {
		res = value_set(op);
	}
	
	return res;
}

value value_quote_all_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "quote()") ? value_init_error() : value_set(argv[0]);
}

value value_quote_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "`()") ? value_init_error() : value_quote(tmp, argv[1]);
}

value value_dequote_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "dq()") ? value_init_error() : eval(tmp, argv[1]);
}

value value_devar_arg(int argc, value argv[])
{
	value *variables = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "dv()"))
		return value_init_error();
	
	if (argv[1].type == VALUE_VAR) {
		return value_hash_get(*variables, argv[1]);
	}
	
	return value_set(argv[1]);
}

value value_eval_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "eval()") ? value_init_error() : eval(tmp, argv[1]);
}

value value_break_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "break()"))
		return value_init_error();
		
	value res;
	res.type = VALUE_STOP;
	res.core.u_stop.type = STOP_BREAK;
	res.core.u_stop.core = NULL;
	return res;
}

value value_continue_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "continue()"))
		return value_init_error();
		
	value res;
	res.type = VALUE_STOP;
	res.core.u_stop.type = STOP_CONTINUE;
	res.core.u_stop.core = NULL;
	return res;
}

value value_yield_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "yield()"))
		return value_init_error();
		
	value res;
	res.type = VALUE_STOP;
	res.core.u_stop.type = STOP_YIELD;
	res.core.u_stop.core = value_malloc(NULL, sizeof(value));
	return_if_null(res.core.u_stop.core);
	*res.core.u_stop.core = value_set(argv[0]);
	return res;
}

value value_return_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "return()"))
		return value_init_error();
		
	value res;
	res.type = VALUE_STOP;
	res.core.u_stop.type = STOP_RETURN;
	res.core.u_stop.core = value_malloc(NULL, sizeof(value));
	return_if_null(res.core.u_stop.core);
	*res.core.u_stop.core = value_set(argv[0]);
	return res;
}

value value_exit_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "exit()"))
		return value_init_error();
	
	value res;
	res.type = VALUE_STOP;
	res.core.u_stop.type = STOP_EXIT;
	res.core.u_stop.core = NULL;
	return res;
}

value value_if(value *variables, value condition, value body, value else_body, int reverse)
{
	value res;
	if (value_true_p(res = eval(variables, condition)) ^ reverse) {
		if (res.type == VALUE_ERROR) return res;
		value_clear(&res);
		res = eval(variables, body);
	} else {
		if (res.type == VALUE_ERROR) return res;
		value_clear(&res);
		res = eval(variables, else_body);
	}
	
	return res;
}

value value_while(value *variables, value condition, value body, int reverse)
{
	value clr, res = value_init(VALUE_ARY);
	while (value_true_p(clr = eval(variables, condition)) ^ reverse) {
		if (clr.type == VALUE_ERROR)
			return clr;
		value_clear(&clr);
		clr = eval(variables, body);
		
		if (clr.type == VALUE_ERROR)
			return clr;
		else if (clr.type == VALUE_STOP && clr.core.u_stop.type == STOP_BREAK)
			break;
		else if (clr.type == VALUE_STOP && clr.core.u_stop.type == STOP_YIELD)
			value_append_now2(&res, clr.core.u_stop.core);
		else if (clr.type == VALUE_STOP && (clr.core.u_stop.type == STOP_RETURN || clr.core.u_stop.type == STOP_EXIT)) {
			value_clear(&res);
			return clr;
		} else value_clear(&clr);
	}
	
	if (value_length(res) == 0)
		value_clear(&res);
	return res;
}

value value_switch(value *variables, value val, value body)
{
	int compare_p = TRUE;
	if (body.type == VALUE_NIL) {
		compare_p = FALSE;
		body = val;
		val.type = VALUE_NIL;
	}
	
	if (body.type != VALUE_BLK) {
		value_error(1, "Type Error: switch() is undefined where body is %ts (block expected).", body);
		return value_init_error();
	}
	
	value cmp = value_init_nil();
	if (compare_p)
		cmp = eval(variables, val);
	value res = value_init_nil();
	value vdefault = value_init_nil();
	int default_defined = FALSE;
	
	size_t i;
	for (i = 0; i < body.core.u_blk.length; ++i) {
		if (body.core.u_blk.a[i].type == VALUE_SYM && streq(body.core.u_blk.a[i].core.u_s, "if")) {
			if (++i >= body.core.u_blk.length) break;
			if (compare_p) {
				if (value_eq(body.core.u_blk.a[i], cmp)) {
					if (++i >= body.core.u_blk.length) break;
					res = eval(variables, body.core.u_blk.a[i]);
					break;
				}
			} else {
				value ev = eval(variables, body.core.u_blk.a[i]);
				if (value_true_p(ev)) {
					if (++i >= body.core.u_blk.length) break;
					res = eval(variables, body.core.u_blk.a[i]);
					break;					
				}
			}

		} else if (body.core.u_blk.a[i].type == VALUE_SYM && streq(body.core.u_blk.a[i].core.u_s, "else")) {
			if (default_defined == FALSE) {
				default_defined = TRUE;
				if (++i >= body.core.u_blk.length) break;
				vdefault = body.core.u_blk.a[i];
			} else {
				value_error(1, "Error: In switch(), multiple :else symbols found.");
				res = value_init_error();
				break;
			}
		}
	}
	
	if (i == body.core.u_blk.length) {
		res = eval(variables, vdefault);
	}
	
	value_clear(&cmp);
	return res;
}

value value_do(value *variables, value todo[], size_t length)
{
	 size_t i;
	 value res = value_init_nil();
	 for (i = 0; i < length; ++i) {
		 value_clear(&res);
		 res = eval(variables, todo[i]);
		 if (res.type == VALUE_ERROR || res.type == VALUE_STOP)
			return res;
	 }
	 
	 return res;
}

value value_comma(value op1, value op2)
{
	value res;
	if (op1.type == VALUE_ARY) {
		res = value_append(op1, op2);
	} else if (op1.type == VALUE_HSH && op2.type == VALUE_PAR) {
		res = value_set(op1);
		value_hash_put(&res, op2.core.u_p->head, op2.core.u_p->tail);				
	} else if (op1.type == VALUE_PAR && op2.type == VALUE_PAR) {
		// If op1 and op2 are both pairs, construct a hash.
		res = value_init(VALUE_HSH);
		value_hash_put(&res, op1.core.u_p->head, op1.core.u_p->tail);
		value_hash_put(&res, op2.core.u_p->head, op2.core.u_p->tail);		
	} else {
		res = value_init(VALUE_ARY);
		value_append_now(&res, op1);
		value_append_now(&res, op2);
	}
	
	return res;
}

value value_for(value *variables, value condition, value body)
{
	int error_p = FALSE;
	
	if (condition.type != VALUE_BLK) {
		value_error(1, "Type Error: for() is undefined where condition is %ts (block expected).", condition);
		error_p = TRUE;
	}
	
	// condition.core.u_blk.a[0] contains the variable name; 
	// condition.core.u_blk.a[1] contains a symbol; 
	// condition.core.u_blk.a[2] contains the iterable
	
	if (error_p) return value_init_error();
	
	value res;
	
	size_t length = condition.core.u_blk.length;
	if (length < 3) {
		value_error(1, "Error: for() is undefined where the condition contains fewer than three words.");
		return value_init_error();
	}
		
	if (condition.core.u_blk.a[0].type == VALUE_VAR || condition.core.u_blk.a[0].type == VALUE_BLK) {
		if (condition.core.u_blk.a[1].type != VALUE_SYM) {
			value_error(1, "Error: Undefined syntax in for loop's condition %s. No symbol found.", condition);
			return value_init_error();
		}
		
		value iter = eval(variables, condition.core.u_blk.a[2]);
		
		size_t ifs_position = length;
		while (ifs_position >= 5 && value_eq(condition.core.u_blk.a[ifs_position-2], value_symbol_if)) {
			value func = value_init(VALUE_UDF);
			func.core.u_udf->vars = value_set(condition.core.u_blk.a[0]);
			func.core.u_udf->body = value_set(condition.core.u_blk.a[ifs_position-1]);
			func.core.u_udf->spec.change_scope_p = FALSE;
			func.core.u_udf->spec.not_stop_p = TRUE;
			func.core.u_udf->spec.argc = 1;
			
			value tmp_iter = value_filter(variables, iter, func);
			value_clear(&iter);
			value_clear(&func);
			iter = tmp_iter;
			
			ifs_position -= 2;
		}
		
		if (value_eq(condition.core.u_blk.a[1], value_symbol_in)) {
			value func = value_init(VALUE_UDF);
			func.core.u_udf->vars = value_set(condition.core.u_blk.a[0]);
			func.core.u_udf->body = value_set(body);
			func.core.u_udf->spec.change_scope_p = FALSE;
			func.core.u_udf->spec.not_stop_p = TRUE;
			func.core.u_udf->spec.argc = 1;
			
			res = value_each(variables, iter, func);
			value_clear(&iter);
			value_clear(&func);
			return res;
		} else if (value_eq(condition.core.u_blk.a[1], value_symbol_dotimes)) {			
			value func = value_init(VALUE_UDF);
			func.core.u_udf->vars = value_set(condition.core.u_blk.a[0]);
			func.core.u_udf->body = value_set(body);
			func.core.u_udf->spec.change_scope_p = FALSE;
			func.core.u_udf->spec.not_stop_p = TRUE;
			func.core.u_udf->spec.argc = 1;
			
			res = value_times(variables, iter, func);
			value_clear(&iter);
			value_clear(&func);
			return res;			
		}
	} else {
		value_error(1, "Error: Undefined syntax for loop's condition %s. No variable found.", condition);
		return value_init_error();
	}

	
	return value_init_nil();
}

value value_if_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "if()") ? value_init_error() : value_if(tmp, argv[1], argv[2], argv[3], FALSE);
}

value value_unless_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "unless()") ? value_init_error() : value_if(tmp, argv[1], argv[2], argv[3], TRUE);
}

value value_while_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "while()") ? value_init_error() : value_while(tmp, argv[1], argv[2], FALSE);
}

value value_until_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "until()") ? value_init_error() : value_while(tmp, argv[1], argv[2], TRUE);
}

value value_switch_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "switch()") ? value_init_error() : value_switch(tmp, argv[1], argv[2]);
}

value value_for_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "for()") ? value_init_error() : value_for(tmp, argv[1], argv[2]);
}

value value_do_both_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, ";()") ? value_init_error() : value_do(tmp, argv+1, (size_t) argc-1);
}

value value_do_all_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "do_all()") ? value_init_error() : value_do(tmp, argv+1, (size_t) argc-1);
}

value value_comma_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, ",()") ? value_init_error() : value_comma(argv[0], argv[1]);
}

