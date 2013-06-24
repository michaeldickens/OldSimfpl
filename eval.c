/*
 *  eval.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 1/17/10.
 *  
 */

#include "eval.h"

int init_evaluator()
{
	print_info_p = FALSE;
	
	global_variables = value_hash_init();
	outer_variables = value_hash_init();
	ud_functions = value_hash_init();
	
	// This is a work in progress.
	value type;
	
	type.type = VALUE_TYP;
	
	type.core.u_type = VALUE_NIL;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_BOO;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_MPZ;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_MPF;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_STR;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_RGX;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_ARY;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_LST;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_HSH;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_TRE;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_PTR;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_BLK;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_RNG;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_NAN;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_INF;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_TYP;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	type.core.u_type = VALUE_ERROR;
	value_hash_put_var(&global_variables, type_to_string(type.core.u_type), type);
	
	
	primitive_funs = value_hash_init_capacity(500);
	function_funs = value_hash_init();
	
	primitive_names = value_hash_init_capacity(500);
	primitive_specs = value_hash_init_capacity(500);
	symbol_ids = value_hash_init_capacity(50);
	function_ids = value_hash_init();
		
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	/*
	 How the function's spec string works: 
	 
	 If it starts with an 'o', take the first number. That is the 
	 point after which arguments become optional. 0 means that all 
	 are optional, 1 means that all but the first are optional, etc.
	 
	 The first three characters are each either true ('t') or false 
	 ('f'). They denote whether (1) the function takes variables, 
	 (2) the function keeps the first argument if it is a variable, 
	 and (3) the function's arguments are not evaluated before the 
	 function is called. These are used for (1) control functions  
	 and iterators, (2) assignment, and (3) control functions. If (1) 
	 is 'u', ud_functions is passed in instead of the current variables.
	 
	 An x denotes that there may be any number of arguments greater than 
	 or equal to the given arg count.
	 
	 The first remaining  character denotes the number of arguments, 
	 the second denotes left or right precedence, and the rest 
	 denote what precedence the operator has.
	 */

	/*
	 Precedence Levels
	 
	 19: at.
	 18: Quote and eval.
	 17: Range operators (..) and (...).
	 16: Unary functions and user-defined unary functions.
	 15: Other functions and user-defined functions.
	 14: 
	 13: Multiplication, Division, Modulo: *, /, %
	 12: Addition and Subtraction: +, -
	 11: Shift: <<, >>
	 10: Comparison: <, <=, >=, >
	  9: Equality: ==, !=
	  8: Logical AND: &
	  7: Logical XOR: ^
	  6: Logical OR: |
	  5: Conditional AND: &&
	  4: Conditional OR: ||
	  3: Assignment functions: =, +=, *=, ...
	  2: Functions that usually have their own line: print, import, 
	     break, def, if, ...
	  1: Commas.
	  0: Semicolons.
	 */

	add_function("import", value_set_fun(&value_import_arg), "1r2");
	add_function("=", value_set_fun(&value_assign_arg), "ttf2r3");
	add_function("+=", value_set_fun(&value_assign_add_arg), "ttf2r3");
	add_function("-=", value_set_fun(&value_assign_sub_arg), "ttf2r3");
	add_function("*=", value_set_fun(&value_assign_mul_arg), "ttf2r3");
	add_function("/=", value_set_fun(&value_assign_div_arg), "ttf2r3");
	add_function("%=", value_set_fun(&value_assign_mod_arg), "ttf2r3");
	add_function("&=", value_set_fun(&value_assign_and_arg), "ttf2r3");
	add_function("^=", value_set_fun(&value_assign_xor_arg), "ttf2r3");
	add_function("|=", value_set_fun(&value_assign_or_arg), "ttf2r3");
	add_function("<<=", value_set_fun(&value_assign_shl_arg), "ttf2r3");
	add_function(">>=", value_set_fun(&value_assign_shr_arg), "ttf2r3");
	
	add_function("**", value_set_fun(&value_pow_arg), "2r16");
	add_function("!", value_set_fun(&value_not_p_arg), "1l16");
	add_function("~", value_set_fun(&value_not_arg), "1l16");
	add_function("--", value_set_fun(&value_uminus_arg), "1l16");
	add_function("++", value_set_fun(&value_uplus_arg), "1l16");
	add_function("abs", value_set_fun(&value_abs_arg), "1l16");
	add_function("exp", value_set_fun(&value_exp_arg), "1l16");
	add_function("log", value_set_fun(&value_log_arg), "1l16");
	add_function("log2", value_set_fun(&value_log2_arg), "1l16");
	add_function("log10", value_set_fun(&value_log10_arg), "1l16");
	add_function("sqrt", value_set_fun(&value_sqrt_arg), "1l16");
	add_function("factorial", value_set_fun(&value_factorial_arg), "1l16");
	add_function("choose", value_set_fun(&value_choose_arg), "2l15");
	add_function("sin", value_set_fun(&value_sin_arg), "1l16");
	add_function("cos", value_set_fun(&value_cos_arg), "1l16");
	add_function("tan", value_set_fun(&value_tan_arg), "1l16");
	add_function("csc", value_set_fun(&value_csc_arg), "1l16");
	add_function("sec", value_set_fun(&value_sec_arg), "1l16");
	add_function("cot", value_set_fun(&value_cot_arg), "1l16");
	add_function("asin", value_set_fun(&value_asin_arg), "1l16");
	add_function("acos", value_set_fun(&value_acos_arg), "1l16");
	add_function("atan", value_set_fun(&value_atan_arg), "1l16");
	add_function("sinh", value_set_fun(&value_sinh_arg), "1l16");
	add_function("cosh", value_set_fun(&value_cosh_arg), "1l16");
	add_function("tanh", value_set_fun(&value_tanh_arg), "1l16");
	add_function("csch", value_set_fun(&value_csch_arg), "1l16");
	add_function("sech", value_set_fun(&value_sech_arg), "1l16");
	add_function("coth", value_set_fun(&value_coth_arg), "1l16");
	add_function("asinh", value_set_fun(&value_asinh_arg), "1l16");
	add_function("acosh", value_set_fun(&value_acosh_arg), "1l16");
	add_function("atanh", value_set_fun(&value_atanh_arg), "1l16");
	add_function("deriv", value_set_fun(&value_deriv_arg), "1l16");
	add_function("probab_prime?", value_set_fun(&value_probab_prime_p_arg), "1l16");
	add_function("nextprime", value_set_fun(&value_nextprime_arg), "1l16");
	add_function("gcd", value_set_fun(&value_gcd_arg), "2l15");
	add_function("seconds", value_set_fun(&value_seconds_arg), "0l15");
	
	add_function("times", value_set_fun(&value_times_arg), "tff2r15");
	add_function("summation", value_set_fun(&value_summation_arg), "tff2r15");

	add_function("to_a", value_set_fun(&value_to_a_arg), "1l16");
	add_function("to_f", value_set_fun(&value_to_f_arg), "1l16");
	add_function("to_h", value_set_fun(&value_to_h_arg), "1l16");
	add_function("to_i", value_set_fun(&value_to_i_arg), "1l16");
	add_function("to_l", value_set_fun(&value_to_l_arg), "1l16");
	add_function("to_s", value_set_fun(&value_to_s_arg), "1l16");
	add_function("to_r", value_set_fun(&value_to_r_arg), "1l16");
	add_function("to_s_base", value_set_fun(&value_to_s_base_arg), "2l15");
	add_function("type", value_set_fun(&value_type_arg), "1l16");
	
	add_function("*", value_set_fun(&value_mul_arg), "2l13");
	add_function("/", value_set_fun(&value_div_arg), "2l13");
	add_function("%", value_set_fun(&value_mod_arg), "2l13");
	add_function("+", value_set_fun(&value_add_arg), "2l12");
	add_function("-", value_set_fun(&value_sub_arg), "2l12");
	add_function("<<", value_set_fun(&value_shl_arg), "2l11");
	add_function(">>", value_set_fun(&value_shr_arg), "2l11");
	add_function("<", value_set_fun(&value_lt_arg), "2l10");
	add_function("<=", value_set_fun(&value_le_arg), "2l10");
	add_function(">", value_set_fun(&value_gt_arg), "2l10");
	add_function(">=", value_set_fun(&value_ge_arg), "2l10");
	add_function("==", value_set_fun(&value_eq_arg), "2l9");
	add_function("!=", value_set_fun(&value_ne_arg), "2l9");
	add_function("&", value_set_fun(&value_and_arg), "2l8");
	add_function("^", value_set_fun(&value_xor_arg), "2l7");
	add_function("|", value_set_fun(&value_or_arg), "2l6");
	add_function("&&", value_set_fun(&value_and_p_arg), "2l5");
	add_function("||", value_set_fun(&value_or_p_arg), "2l4");
	
	add_function("set", value_set_fun(&value_set_arg), "ftt1l16");
	add_function("true?", value_set_fun(&value_true_p_arg), "1l16");
	add_function("..", value_set_fun(&value_range_to_arg), "2l17");
	add_function("...", value_set_fun(&value_range_until_arg), "2l17");
	add_function("rand", value_set_fun(&value_rand_arg), "1l16");
	add_function("array", value_set_fun(&value_array_arg), "x0r15");
	add_function("list", value_set_fun(&value_list_arg), "x0r15");
	add_function("hash", value_set_fun(&value_hash_arg), "x0r15");
	add_function("->", value_set_fun(&value_make_pair_arg), "2r15");
	add_function("print", value_set_fun(&value_print_arg), "1l2");
	add_function("println", value_set_fun(&value_println_arg), "1l2");	
	add_function("printf", value_set_fun(&value_printf_arg), "x0l2");
	
	add_function("gets", value_set_fun(&value_gets_arg), "0l16");
	
	add_function("asc", value_set_fun(&value_asc_arg), "1l16");
	add_function("capitalize", value_set_fun(&value_capitalize_arg), "1l16");
	add_function("chop", value_set_fun(&value_chop_arg), "1l16");
	add_function("chop!", value_set_fun(&value_chop_now_arg), "1l16");
	add_function("chr", value_set_fun(&value_chr_arg), "1l16");
	add_function("contains?", value_set_fun(&value_contains_p_arg), "2l15");
	add_function("ends_with?", value_set_fun(&value_ends_with_p_arg), "2l15");
	add_function("index", value_set_fun(&value_index_arg), "2l15");
	add_function("insert", value_set_fun(&value_insert_arg), "3l15");
	add_function("insert!", value_set_fun(&value_insert_now_arg), "3l15");
	add_function("alpha?", value_set_fun(&value_alpha_p_arg), "1l16");
	add_function("alnum?", value_set_fun(&value_alnum_p_arg), "1l16");
	add_function("num?", value_set_fun(&value_num_p_arg), "1l16");
	add_function("length", value_set_fun(&value_length_arg), "1l16");
	add_function("lstrip", value_set_fun(&value_lstrip_arg), "1l16");
	add_function("range", value_set_fun(&value_range_arg), "3l15");
	add_function("replace", value_set_fun(&value_replace_arg), "3l15");
	add_function("replace!", value_set_fun(&value_replace_now_arg), "3l15");
	add_function("reverse", value_set_fun(&value_reverse_arg), "1l16");
	add_function("reverse!", value_set_fun(&value_reverse_now_arg), "1l16");
	add_function("rstrip", value_set_fun(&value_rstrip_arg), "1l16");
	add_function("scan", value_set_fun(&value_scan_arg), "2l15");
	add_function("split", value_set_fun(&value_split_arg), "2l15");
	add_function("starts_with?", value_set_fun(&value_starts_with_p_arg), "2l15");
	add_function("strip", value_set_fun(&value_strip_arg), "1l16");
	add_function("strip!", value_set_fun(&value_strip_now_arg), "1l16");
	add_function("to_upper", value_set_fun(&value_to_upper_arg), "1l16");
	add_function("to_lower", value_set_fun(&value_to_lower_arg), "1l16");
	
	add_function("match?", value_set_fun(&value_match_p_arg), "2l15");
	add_function("match", value_set_fun(&value_match_arg), "2l15");
	
	add_function("append", value_set_fun(&value_append_arg), "2l15");
	add_function("append!", value_set_fun(&value_append_now_arg), "2l15");
	add_function("array_with_length", value_set_fun(&value_array_with_length_arg), "1l15");
	add_function("at", value_set_fun(&value_at_arg), "o1xl19");
	add_function("at_equals", value_set_fun(&value_at_assign_arg), "o3tffxl3");
	add_function("at_add_equals", value_set_fun(&value_at_assign_add_arg), "o3tffxl3");
	add_function("at_sub_equals", value_set_fun(&value_at_assign_sub_arg), "o3tffxl3");
	add_function("at_mul_equals", value_set_fun(&value_at_assign_mul_arg), "o3tffxl3");
	add_function("at_div_equals", value_set_fun(&value_at_assign_div_arg), "o3tffxl3");
	add_function("at_mod_equals", value_set_fun(&value_at_assign_mod_arg), "o3tffxl3");
	add_function("at_and_equals", value_set_fun(&value_at_assign_and_arg), "o3tffxl3");
	add_function("at_xor_equals", value_set_fun(&value_at_assign_xor_arg), "o3tffxl3");
	add_function("at_or_equals", value_set_fun(&value_at_assign_or_arg), "o3tffxl3");
	add_function("at_shl_equals", value_set_fun(&value_at_assign_shl_arg), "o3tffxl3");
	add_function("at_shr_equals", value_set_fun(&value_at_assign_shr_arg), "o3tffxl3");
	add_function("concat", value_set_fun(&value_concat_arg), "2l15");
	add_function("delete", value_set_fun(&value_delete_arg), "2l15");
	add_function("delete_all", value_set_fun(&value_delete_all_arg), "2l15");
	add_function("delete_at", value_set_fun(&value_delete_at_arg), "2l15");
	add_function("delete_at!", value_set_fun(&value_delete_at_now_arg), "2l15");
	add_function("each", value_set_fun(&value_each_arg), "tff2l15");
	add_function("each_index", value_set_fun(&value_each_index_arg), "tff2l15");
	add_function("empty?", value_set_fun(&value_empty_p_arg), "1l16");
	add_function("filter", value_set_fun(&value_filter_arg), "tff2l15");
	add_function("find", value_set_fun(&value_find_arg), "tff2l15");
	add_function("flatten", value_set_fun(&value_flatten_arg), "1l16");
	add_function("flatten!", value_set_fun(&value_flatten_now_arg), "1l16");
	add_function("fold", value_set_fun(&value_fold_arg), "tff3l15");
	add_function("join", value_set_fun(&value_join_arg), "2l15");
	add_function("last", value_set_fun(&value_last_arg), "1l16");
	add_function("map", value_set_fun(&value_map_arg), "tff2l15");
	add_function("map!", value_set_fun(&value_map_now_arg), "tff2l15");
	add_function("pop", value_set_fun(&value_pop_arg), "1l16");
	add_function("pop!", value_set_fun(&value_pop_now_arg), "1l16");
	add_function("shuffle", value_set_fun(&value_shuffle_arg), "1l16");
	add_function("shuffle!", value_set_fun(&value_shuffle_now_arg), "1l16");
	add_function("size", value_set_fun(&value_size_arg), "1l16");
	add_function("sort", value_set_fun(&value_sort_arg), "1l16");
	add_function("sort!", value_set_fun(&value_sort_now_arg), "1l16");
	add_function("uniq", value_set_fun(&value_uniq_arg), "1l16");
	add_function("uniq!", value_set_fun(&value_uniq_now_arg), "1l16");
	add_function("uniq_sort", value_set_fun(&value_uniq_sort_arg), "1l16");
	add_function("uniq_sort!", value_set_fun(&value_uniq_sort_now_arg), "1l16");
	
	add_function("cons", value_set_fun(&value_cons_arg), "2r15");
	add_function("cons!", value_set_fun(&value_cons_now_arg), "2r15");
	add_function("drop", value_set_fun(&value_drop_arg), "2l15");
	add_function("head", value_set_fun(&value_head_arg), "1l16");
	add_function("tail", value_set_fun(&value_tail_arg), "1l16");
	add_function("take", value_set_fun(&value_take_arg), "2l15");
	
	add_function("contains_value?", value_set_fun(&value_contains_value_arg), "2l15");
		
	add_function(";", value_set_fun(&value_do_both_arg), "tft2l0");
	add_function(",", value_set_fun(&value_comma_arg), "fff2l1");
	add_function("do_all", value_set_fun(&value_do_all_arg), "tftx0l15");
	add_function("if", value_set_fun(&value_if_arg), "o2ttt3l15");
	add_function("unless", value_set_fun(&value_unless_arg), "o2ttt3l15");
	add_function("while", value_set_fun(&value_while_arg), "ttt2l15");
	add_function("until", value_set_fun(&value_until_arg), "ttt2l15");
	add_function("switch", value_set_fun(&value_switch_arg), "o1ttt2l15");
	add_function("for", value_set_fun(&value_for_arg), "ttt2l15");
	
	add_function("break", value_set_fun(&value_break_arg), "fff0l2");
	add_function("continue", value_set_fun(&value_continue_arg), "fff0l2");
	add_function("yield", value_set_fun(&value_yield_arg), "fff1l2");
	add_function("return", value_set_fun(&value_return_arg), "fff1l2");
	add_function("exit", value_set_fun(&value_exit_arg), "fff0l2");
	
	add_function("def", value_set_fun(&value_def_arg), "uft3l2");
	add_function("lambda", value_set_fun(&value_lambda_arg), "uft2l2");
		
	add_function("quote", value_set_fun(&value_quote_all_arg), "ftt1l18");
	add_function("`", value_set_fun(&value_quote_arg), "ttt1l18");
	add_function("dq", value_set_fun(&value_dequote_arg), "tft1l18");
	add_function("dv", value_set_fun(&value_devar_arg), "tft1l18");
	add_function("eval", value_set_fun(&value_eval_arg), "tff1l18");

	add_function("optimize", value_set_fun(&value_optimize_arg), "o1fff2l16");

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	value_hash_put_str(&symbol_ids, "nl", value_init_nil()); // newline
	value_hash_put_str(&symbol_ids, "(", value_init_nil());
	value_hash_put_str(&symbol_ids, ")", value_init_nil());
	value_hash_put_str(&symbol_ids, "[", value_init_nil());
	value_hash_put_str(&symbol_ids, "]", value_init_nil());
	value_hash_put_str(&symbol_ids, "{", value_init_nil());
	value_hash_put_str(&symbol_ids, "}", value_init_nil());
	
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	
	return 0;
}

void add_function(char *name, value fun, char *spec)
{
	value vname = value_set_id(name);
	value vspec;
	
	vspec.type = VALUE_SPEC;
	vspec.core.u_spec = compile_spec(spec);
	fun.core.u_bif->spec = vspec.core.u_spec;
	value_hash_put_refs(&primitive_funs, &vname, &fun);
	value_hash_put_refs(&primitive_specs, &fun, &vspec);
	vname.type = VALUE_VAR;
	value_hash_put_refs(&primitive_names, &fun, &vname);
}

/* 
 * The new evaluation function. Takes an S-expression and evaluates it.
 */

value eval_generic(value *variables, value sexp, int outer_was_block_p)
{	
	if (sexp.type == VALUE_VAR) {
		if (value_hash_exists(*variables, sexp))
			return value_hash_get(*variables, sexp);
		else if (value_hash_exists(global_variables, sexp))
			return value_hash_get(global_variables, sexp);
		else {
			value_error(1, "Error: Unrecognized function or value %s.", sexp);
			return value_init_error();
		}
	} else if (sexp.type != VALUE_BLK)
		return value_set(sexp);
				
	size_t length = sexp.core.u_blk.length;
	value res = value_init_nil();
	
	int error_p = FALSE;
	
	if (sexp.core.u_blk.a[0].type == VALUE_UDF_SHELL) {
		// Fix the code so that this conversion only happens one time, and after that it's 
		// remembered. This is actually taking up a pretty big chunk of the running time.
		value name = value_set_str(sexp.core.u_blk.a[0].core.u_udf->name);
		name.type = VALUE_VAR;
		value_clear(&sexp.core.u_blk.a[0]);
		sexp.core.u_blk.a[0] = value_hash_get(ud_functions, name);
		
		// This ought to fix the repeated-conversion bug. I don't think it does but I need 
		// to do more testing.
//		value_hash_put(&ud_functions, name, sexp.core.u_blk.a[0]);

		value_clear(&name);
	}
	
	value saved = sexp.core.u_blk.a[0];
	if (sexp.core.u_blk.a[0].type == VALUE_BLK) {
		sexp.core.u_blk.a[0] = eval(variables, sexp.core.u_blk.a[0]);
	}
	
	if (sexp.core.u_blk.a[0].type == VALUE_BIF) {
		
		res = value_bifcall_sexp(variables, &ud_functions, sexp);
		
	} else if (sexp.core.u_blk.a[0].type == VALUE_UDF) {
		
		// Evaluate the function.
		res = value_udfcall(variables, sexp.core.u_blk.a[0], sexp.core.u_blk.length - 1, sexp.core.u_blk.a + 1);
		
		// This fixes the problem where a UDF_SHELL has to be repeatedly set to a UDF. But since this happens 
		// every time a UDF is called, I doubt that it's any faster. At least it's a solution, but I need an 
		// elegant solution.
		// Later: I put this code in the place where a UDF_SHELL is found.
//		value name = value_set_str(sexp.core.u_blk.a[0].core.u_udf->name);
//		name.type = VALUE_VAR;
//		value_hash_put(&ud_functions, name, sexp.core.u_blk.a[0]);
		
	} else if (length == 1) {
		if (sexp.core.u_blk.a[0].type == VALUE_VAR) {
			if (value_hash_exists(*variables, sexp.core.u_blk.a[0]))
				return value_hash_get(*variables, sexp.core.u_blk.a[0]);
			else if (value_hash_exists(global_variables, sexp.core.u_blk.a[0]))
				return value_hash_get(global_variables, sexp.core.u_blk.a[0]);
			else {
				value_error(1, "Error: Unrecognized function or value %s.", sexp.core.u_blk.a[0]);
				return value_init_error();
			}
		} else
			return value_set(sexp.core.u_blk.a[0]);
	} else {
		value_error(1, "Syntax Error: S-expressions are undefined where the first element is %ts.", sexp.core.u_blk.a[0]);
		res = value_init_error();
	}
	
	if (saved.type == VALUE_BLK) {
		value_clear(&sexp.core.u_blk.a[0]);
		sexp.core.u_blk.a[0] = saved;
	}
	
	if (print_errors_p && outer_was_block_p == FALSE && res.type == VALUE_ERROR) {
		value_printf("\tin %s\n", sexp);
	}
	
	return res;
}

int is_primitive(value id)
{
	if (id.type == VALUE_BIF)
		return value_hash_exists(primitive_specs, id);
	return value_hash_exists(primitive_funs, id);
}

int is_function(value id)
{
	if (id.type == VALUE_BIF)
		return value_hash_exists(primitive_specs, id);
	if (id.type == VALUE_UDF)
		return value_hash_exists(ud_functions, id);
	if (id.type == VALUE_UDF_SHELL)
		; // Do something.
	return value_hash_exists(primitive_funs, id);
}

int is_symbol(char *id)
{
	return value_hash_exists_str(symbol_ids, id);
}

struct value_spec compile_spec(char *str)
{
	struct value_spec spec;
	
	if (str == NULL) {
		spec.change_scope_p = 0;
		spec.needs_variables_p = 0;
		spec.keep_arg_p = 0;
		spec.delay_eval_p = 0;
		spec.argc = 0;
		spec.optional = 0;
		spec.rest_p = 0;
		spec.associativity = 0;
		spec.precedence = 0;
		spec.not_stop_p = 0;
		return spec;
	}
	
	char *ptr = str;
	
	spec.optional = INT_MAX;
	if (*ptr == 'o') {
		spec.optional = 0;
		while (isdigit(*(++ptr)))
			spec.optional = (10 * spec.optional) + (*ptr - '0');
	}
	
	spec.change_scope_p = TRUE;
	
	if (*ptr == 't')
		spec.needs_variables_p = TRUE;
	else if (*ptr == 'f')
		spec.needs_variables_p = FALSE;
	else if (*ptr == 'u')
		spec.needs_variables_p = NEEDS_UD_FUNCTIONS;
	else {
		spec.needs_variables_p = FALSE;
		--ptr;
	}
	++ptr;

	if (*ptr == 't')
		spec.keep_arg_p = TRUE;
	else if (*ptr == 'f')
		spec.keep_arg_p = FALSE;
	else {
		spec.keep_arg_p = FALSE;
		--ptr;
	}
	++ptr;

	if (*ptr == 't')
		spec.delay_eval_p = TRUE;
	else if (*ptr == 'f')
		spec.delay_eval_p = FALSE;
	else {
		spec.delay_eval_p = FALSE;
		--ptr;
	}
	++ptr;
	
	spec.argc = 0;
	if (*ptr == 'x') {
		spec.rest_p = TRUE;
		++ptr;
	} else {
		spec.rest_p = FALSE;
	}

	while (isdigit(*ptr))
		spec.argc = (10 * spec.argc) + (*(ptr++) - '0');
	
	if (*ptr == 'l')
		spec.associativity = 'l';
	else if (*ptr == 'r')
		spec.associativity = 'r';
	else {
		value_error(1, "Internal error: In spec string \"%s\", associativity has an undefined value.", str);
		spec = value_nil_function_spec;
		return spec;
	}
	
	spec.precedence = 0;
	while (isdigit(*(++ptr)))
		spec.precedence = (10 * spec.precedence) + *ptr - '0';
	
	spec.not_stop_p = FALSE;
	
	return spec;
}

struct value_spec get_spec(value id)
{
	if (id.type == VALUE_BIF) {
		return id.core.u_bif->spec;
	} else if (id.type == VALUE_UDF) {
		return id.core.u_udf->spec;
	}
	
	return compile_spec(NULL);
}

int primitive_needs_variables(value id)
{
	value *refp = value_hash_get_ref(primitive_specs, id);
	return refp->core.u_spec.needs_variables_p;	
}

int primitive_keep_arg(value id)
{
	value *refp = value_hash_get_ref(primitive_specs, id);
	return refp->core.u_spec.keep_arg_p;	
}

int primitive_delay_eval(value id)
{
	value *refp = value_hash_get_ref(primitive_specs, id);
	return refp->core.u_spec.delay_eval_p;	
}

size_t arg_count(value id)
{
	if (id.type == VALUE_BIF) {	
		return id.core.u_bif->spec.argc;
	} else if (id.type == VALUE_UDF) {
		return id.core.u_udf->vars.core.u_a.length;
	} else if (id.type == VALUE_UDF_SHELL) {
		return id.core.u_udf->spec.argc;
	} else {
		return -1;
	}

}

char primitive_associativity(value id)
{
	if (!is_primitive(id))
		return 1;

	value *val = value_hash_get_ref(primitive_specs, id);
	return val->core.u_spec.associativity;
}

int primitive_precedence(value id)
{
	if (!is_primitive(id)) {
		value_error(1, "Compiler Error: unknown id %ts.", id);
		return -1;
	}

	value *val = value_hash_get_ref(primitive_specs, id);
	return val->core.u_spec.precedence;
}

