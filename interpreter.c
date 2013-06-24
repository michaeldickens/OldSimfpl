/*
 *  interpreter.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 5/8/10.
 *
 */



#include "interpreter.h"

int init_interpreter()
{
	same_type_determiner = SAME_TYPE_VALUE;
	print_interpreter_stuff = FALSE;
	is_eof = FALSE;
	previous_truth_value = FALSE;
	previous_was_if_statement;
	
	assume_first_is_function = TRUE;
	
	line_queue = value_init(VALUE_LST);
	line_queue_back = line_queue;
	
	input_stream = stdin;
//	input_stream = fopen("benchmarks/recursive benchmark 1.txt", "r");
	if (input_stream == NULL) {
		value_error(0, "IO Error: Unknown file stream.");
		input_stream = stdin;
	}
	
	stream_paren_balance = stream_bracket_balance = stream_curly_balance = stream_quote_balance = stream_regex_balance = 0;
		
	return 0;
}

/* Interprets an input stream.
 */
int run_interpreter()
{
	int i;
	print_interpreter_stuff = TRUE;
	
	value values, result = value_init_nil();
	for (i = 0; is_eof == FALSE; ++i) {
		if (input_stream == stdin && value_empty_p(line_queue))
			printf(">>> ");
		
		values = get_values();
		if (values.type == VALUE_ARY)
			result = interpret_values(&outer_variables, values.core.u_a.a, value_length(values));
		else result = value_init_nil();
				
		value_clear(&values);
		if (result.type == VALUE_STOP && result.core.u_stop.type == STOP_EXIT) {
			value_clear(&result);
			break;
		}
				
		if (input_stream == stdin && value_empty_p(line_queue)) {
			printf("==> ");
			value_printf("%s\n", result);
		} else if (result.type == VALUE_ERROR) {
			// If one of the statements is an error, stop running, but only if the input stream is a 
			// file and not stdin.
			value_clear(&result);
			return 1;
		}
		
		value_clear(&result);
	}
	
	return 0;
}

value interpret_given_statement(value *variables, char *statement)
{
	value holder = statement_to_values(statement);
	if (holder.type == VALUE_ARY) {
		value res = interpret_values(variables, holder.core.u_a.a, value_length(holder));
		value_clear(&holder);
		return res;
	} else return holder;
}

value compile_statement(char *statement)
{
	value values = statement_to_values(statement);
	if (values.type == VALUE_ARY) {
		value sexp = compile_values(values.core.u_a.a, values.core.u_a.length);
		value_clear(&values);
		return sexp;
	} else return values;
}

/* A return value of 1 indicates EOF.
 */
int get_line(char *str, int length, FILE *stream)
{
	char *ret = fgets(str, length, stream);
	++linenum;
	if (ret == NULL) {
		is_eof = TRUE;
		return 1;
	}
	
	return fix_up_line(str, TRUE);
}

int fix_up_line(char *str, int is_first)
{
	char *ptr;
	int is_prev_backslash = 0;
	int result = 0;
	
	ptr = str - 1;
	while (*(++ptr)) {
		// Remove comments.
		if (*ptr == '/' && *(ptr+1) == '/' && stream_quote_balance == 0 && stream_regex_balance == 0) {
			*ptr = '\n';
			*(ptr+1) = '\0';
			break;
		}
		
		switch(*ptr) {
		case '(':
			++stream_paren_balance; break;
		case ')':
			--stream_paren_balance; break;
		case '[':
			++stream_bracket_balance; break;
		case ']':
			--stream_bracket_balance; break;
		case '{':
			++stream_curly_balance; break;
		case '}':
			--stream_curly_balance; break;
		case '"':
			if (is_prev_backslash == 0)
				stream_quote_balance ^= 1;
			break;
		case '\'':
			if (is_prev_backslash == 0)
				stream_regex_balance ^= 1;
			break;
		case '\\':
			is_prev_backslash ^= 1;
			break;
		}
	}
	
	return result;
}

char * get_statement()
{
	char buffer[BIGBUFSIZE];
	size_t length = 0;

	stream_paren_balance = stream_bracket_balance = stream_curly_balance = stream_quote_balance = stream_regex_balance = 0;
	int incompleteness_type = STATEMENT_COMPLETE;
	
	do {
		if (incompleteness_type == STATEMENT_INCOMPLETE_CURLY) {
			buffer[length++] = ' ';
			buffer[length++] = 'n';
			buffer[length++] = 'l';
			buffer[length++] = ' ';
		}

		
		get_line(buffer + length, BIGBUFSIZE - length, input_stream);
		length += strlen(buffer + length);
		
		if (is_eof) break;
		
	} while ((incompleteness_type = is_statement_complete(buffer)) != STATEMENT_COMPLETE);
	
	char *res = value_malloc(NULL, length + 1);
	if (res == NULL) return NULL;
	strncpy(res, buffer, length);
	res[length] = '\0';
		
	return res;
}

value get_values()
{
	value x;
	
	if (value_empty_p(line_queue)) {
		char *str = get_statement();
		if (is_eof)
			return value_init_nil();
		x = statement_to_values(str);
		value_free(str);
		return x;
	} else {
		x = line_dequeue();
		return x;
	}
}

void statement_to_words(char *words[], size_t wordcount, char *statement)
{	
	int i;
	char *ptr = statement, *start;
	char *temp;
	
	ptr = skip_whitespace(ptr);
	for (i = 0; *ptr && i < wordcount; ++i) {
		start = ptr;
		ptr = end_of_word(start);
		
		temp = value_malloc(NULL, ptr-start+1);
		if (temp == NULL) return;
		strncpy(temp, start, ptr-start);
		temp[ptr-start] = '\0';
		
		words[i] = temp;
		ptr = skip_whitespace(ptr);
	}
	
	for (; i < wordcount; ++i)
		words[i] = NULL;
}

value statement_to_values(char *statement)
{
	if (*(skip_whitespace(statement)) == '\0') {
		return value_init_nil();
	}
	
	size_t wordcount = count_words(statement);
	if (wordcount == 0)
		return value_init_nil();
	
	/* Stage 1: Read in (statement) and put each word into (words). */
	
	size_t i;
	char *words[wordcount];
	char *ptr = statement, *start;
	char *temp;
	
	ptr = skip_whitespace(ptr);
	for (i = 0; *ptr; ++i) {
		start = ptr;
		ptr = end_of_word(start);
		
		temp = value_malloc(NULL, ptr-start+1);
		if (temp == NULL) return value_init_error();
		strncpy(temp, start, ptr-start);
		temp[ptr-start] = '\0';
		
		words[i] = temp;
		ptr = skip_whitespace(ptr);
	}
	
	value values[wordcount];
	int error_p = words_to_values(values, words, wordcount);
	for (i = 0; i < wordcount; ++i) {
		value_free(words[i]);
	}
	if (error_p)
		return value_init_error();
	
	return value_set_ary_ref(values, wordcount);
}

char * values_to_statement(value values[], size_t length)
{
	char str[BIGBUFSIZE];
	char *ptr = str;
	size_t i;
	for (i = 0; i < length; ++i) {
		strcpy(ptr, values[i].core.u_s);
		ptr += strlen(ptr);
		*(ptr++) = ' ';
	}
	
	*ptr = '\0';
	return str;
}

int words_to_values(value values[], char *words[], size_t wordcount)
{
	size_t i;
	value x;
	
	for (i = 0; i < wordcount; ++i) {
		if ((values[i] = value_set_str_smart(words[i], 0)).type == VALUE_ERROR) {
			value_clear(&values[i]);
			values[i].type = VALUE_ID;
			values[i].core.u_id = value_malloc(NULL, strlen(words[i])+1);
			if (values[i].core.u_id == NULL) return VALUE_ERROR;
			strcpy(values[i].core.u_id, words[i]);
			
			/* Is values[i] a user-defined function? */
			values[i].type = VALUE_VAR;
			// Pull a copy of x out of ud_functions. It would be better if it could pull 
			// the value out instead of a copy, because that way UDF_SHELLs could be 
			// converted into UDFs and it would only have to be done one time.
			x = value_hash_get(ud_functions, values[i]);
			if (x.type != VALUE_NIL) {
				value_clear(&values[i]);
				values[i] = x;
				continue;			
			}
			
			/* Is values[i] a built-in function? */
			value_clear(&x);
			values[i].type = VALUE_ID;
			x = value_hash_get(primitive_funs, values[i]);
			if (x.type != VALUE_NIL) {
				value_clear(&values[i]);
				values[i] = x;					
				continue;
			}
			
			value_clear(&x);
			values[i].type = VALUE_ID;
			if (is_symbol(values[i].core.u_id)) {
				// Do nothing.
			} else {
				values[i].type = VALUE_VAR;
			}
		}
	}
		
	return 0;
}

/* 
 * Compiles a list of values into an S-expression. Does not guarantee that 
 * (words) will be intact -- some values may change.
 */
value compile_values(value words[], size_t wordcount)
{
	if (wordcount == 0) {
		previous_truth_value = FALSE;
		previous_was_if_statement = FALSE;
		return value_init_nil();
	}
	
	value sexp = value_init_nil();
	
	size_t i, j, inx = -1, binx = -1, cinx = -1;
	int parens = 0, brackets = 0, curlies = 0;
	
	// Do a preliminary compilation. Find IDs that look like functions and make them  
	// into UDF shells.
	for (i = 0; i < wordcount; ++i) {
		if (words[i].type == VALUE_BIF && words[i].core.u_bif->f == &value_def_arg) {
			assume_first_is_function = FALSE;
			
			// If the function was previously defined, delete the old definition.
			if (words[i+1].type == VALUE_UDF) {
				value temp = words[i+1];
				words[i+1] = value_set_str(temp.core.u_udf->name);
				words[i+1].type = VALUE_VAR;
				value_clear(&temp);
			}
			
			// A function is being defined. Find the name of the function and the 
			// number of arguments.
			value name = words[i+1];
			size_t argc = 0;
			for (j = i+3; j < wordcount; ++j)
				if (words[j].type == VALUE_ID && streq(words[j].core.u_id, ")"))
					break;
				else if (words[j].type == VALUE_VAR) ++argc;
									
			value shell = value_init(VALUE_UDF_SHELL);
			if (name.type == VALUE_VAR) {
				shell.core.u_udf->name = value_malloc(NULL, strlen(name.core.u_var) + 1);
				return_if_null(shell.core.u_udf->name);
				strcpy(shell.core.u_udf->name, name.core.u_var);
			} else {
				shell.core.u_udf->name = NULL;
			}

			shell.core.u_udf->spec.associativity = 'l';
			shell.core.u_udf->spec.argc = argc;
			if (argc == 1)
				shell.core.u_udf->spec.precedence = 16;
			else shell.core.u_udf->spec.precedence = 15;
			
			// Find every call to the defined function, and convert it to a 
			// function shell.
			i = j - 1;
			for (; j < wordcount; ++j) {
				var_to_shell(&words[j], name, shell);
			}
			
			value_clear(&shell);
		}
	}
	
	value res;
	
	// Find parentheses and brackets. Also determine if the expression contains any functions.
	int first_function_index = -1;
	int scope_assume_first_is_function = assume_first_is_function;
	for (i = 0; i < wordcount; ++i) {
		if (parens == 0 && brackets == 0 && curlies == 0 && 
				(words[i].type == VALUE_BIF || words[i].type == VALUE_UDF || words[i].type == VALUE_UDF_SHELL))
			if (first_function_index == -1)
				first_function_index = i;
		
		if (words[i].type == VALUE_BIF && words[i].core.u_bif->f == &value_for_arg)
			assume_first_is_function = FALSE;

		
		if (words[i].type != VALUE_ID) {
//			continue;
				
		} else if (streq(words[i].core.u_id, "(")) {
			if (parens == 0)
				inx = i;
			++parens;
		} else if (streq(words[i].core.u_id, ")")) {
			--parens;
			if (parens < 0) {
				value_error(1, "Syntax Error: Opening parenthesis not found.");
				value_clear(&sexp);
				return value_init_error();

			} else if (parens == 0 && brackets == 0 && curlies == 0) {
				// If we are inside brackets, don't evaluate the statement in parentheses just yet.
				
				if (inx == -1) {
					value_error(1, "Syntax Error: Opening parenthesis not found.");
					value_clear(&sexp);
					return value_init_error();
				}
				
				res = compile_values(words+inx+1, i-inx-1);
								
				if (res.type == VALUE_ERROR)
					break;
				
				// 
				// This code removes all the words that have been evaluated, 
				// i.e. the words in between the parentheses and the 
				// parentheses themselves.
				// 
				wordcount = delete_from_words(words, wordcount, res, i, inx);
								
				// Move (i) back to the beginning of the parentheses.
				i = inx;
				
			} // if (paren_balance)
			
		} else if (streq(words[i].core.u_id, "[")) {
			if (brackets == 0)
				binx = i;
			++brackets;
		} else if (streq(words[i].core.u_id, "]")) {
			--brackets;
			if (curlies < 0) {
				value_error(1, "Syntax error: Opening square bracket not found.");
				value_clear(&sexp);
				return value_init_error();
				
			} else if (parens == 0 && brackets == 0 && curlies == 0) {
				
				if (binx == -1) {
					value_error(1, "Syntax error: Opening square bracket not found.");
					value_clear(&sexp);
					return value_init_error();
				}
				
				// res = the value inside the brackets.
				res = compile_values(words+binx+1, i-binx-1);				
				if (res.type == VALUE_ERROR)
					break;
				
				int is_infix = infix_p(words, wordcount);
				
				value_clear(&words[binx]);
								
				int offset = 0;
				int at_assign = 0;
				int word_i = -1;
				
				// Set words[binx] to the function, be it at(), at=(), at+=(), etc.
				if (is_infix && words[word_i = i+1].type == VALUE_BIF || 
						!is_infix && binx >= 2 && words[word_i = binx-2].type == VALUE_BIF) {
					// If there is an assignment operator nearby, convert this from at() to at=() 
					// or something similar.
					
					at_assign = 1;
					if (words[word_i].core.u_bif->f == &value_assign_arg)
						words[binx] = value_set_fun(&value_at_assign_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_add_arg)
						words[binx] = value_set_fun(&value_at_assign_add_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_sub_arg)
						words[binx] = value_set_fun(&value_at_assign_sub_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_mul_arg)
						words[binx] = value_set_fun(&value_at_assign_mul_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_div_arg)
						words[binx] = value_set_fun(&value_at_assign_div_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_mod_arg)
						words[binx] = value_set_fun(&value_at_assign_mod_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_and_arg)
						words[binx] = value_set_fun(&value_at_assign_and_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_xor_arg)
						words[binx] = value_set_fun(&value_at_assign_xor_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_or_arg)
						words[binx] = value_set_fun(&value_at_assign_or_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_shl_arg)
						words[binx] = value_set_fun(&value_at_assign_shl_arg);
					else if (words[word_i].core.u_bif->f == &value_assign_shr_arg)
						words[binx] = value_set_fun(&value_at_assign_shr_arg);
					else {
						at_assign = 0;
						words[binx] = value_set_fun(&value_at_arg);
					}
					
				} else {
					words[binx] = value_set_fun(&value_at_arg);
				}
				
				offset += at_assign;
				
				value *specptr = value_hash_get_ref(primitive_specs, words[binx]);
				if (specptr == NULL) {
					value_error(1, "Error: Undefined value %ts. Probably you tried to call a function internally which does not exist.", words[binx]);
				}
				words[binx].core.u_bif->spec = specptr->core.u_spec;
				
				// Put res into words[binx+1].
				value_clear(&words[binx+1]);
				words[binx+1] = res;
				res.type = VALUE_NIL; // Now res can't be accidentally cleared.
				
				// If the statement is in prefix notation don't do (a at i), instead do (at a i).
				if (!is_infix && binx >= 1) {
					value temp = words[binx];
					words[binx] = words[binx-1];
					words[binx-1] = temp;
				}
				
				// See if there are multiple brackets in a row.
//				j = i;
//				while (words[j+1].type == VALUE_ID && streq(words[j+1].core.u_id, "[")) {
//					res = compile_values(...stuff...);
//				}
				
				if (is_infix) {
					// If at_assign is 0, then the function is at(). Otherwise, it is at=(), at+=(), etc.
					if (at_assign != 0) {
						// Flip the (at) with the value inside the brackets, so (a[0] = x) will compile to 
						// (a 0 at_equals x) instead of (a at_equals 0 x). This will make it compile to 
						// prefix as expected.
						value temp = words[binx];
						words[binx] = words[binx+1];
						words[binx+1] = temp;
					}
					
					// The statement is in infix notation. Remove any stray words inside the brackets, 
					// the closing bracket, and possibly the assignment operator if offset == 1.
					for (j = 1; i+j+offset < wordcount; ++j) {
						value_clear(&words[binx+1+j]);
						words[binx+1+j] = words[i+j+offset];
						words[i+j+offset].type = VALUE_NIL;
					}
					wordcount = binx + 1 + j;
				
				} else if (!is_infix && offset) {
					// If the expression is in prefix notation and the array index is being assigned, 
					// the assignment operator still needs to be deleted.
					for (j = 1; i+j < wordcount; ++j) {
						value_clear(&words[binx+1+j]);
						words[binx+1+j] = words[i+j];
						words[i+j].type = VALUE_NIL;
					}
					
					wordcount = binx + 1 + j;
					for (j = binx-2; j+1 < wordcount; ++j)
						words[j] = words[j+1];
					words[j].type = VALUE_NIL;
					--wordcount;
				}
				
				i = binx + 1;
				
			}
			
		} else if (streq(words[i].core.u_id, "{")) {
			if (curlies == 0)
				cinx = i;
			++curlies;
		} else if (streq(words[i].core.u_id, "}")) {
			--curlies;
			if (curlies < 0) {
				value_error(1, "Syntax Error: Opening curly bracket was not found.");
				value_clear(&sexp);
				return value_init_error();

			} else if (parens == 0 && brackets == 0 && curlies == 0) {
				// If we are inside brackets, don't evaluate the statement in parentheses just yet.
				
				if (cinx == -1) {
					// Probably, this code never executes because curlies would be < 0 so the error 
					// will be caught earlier. It's still here just in case.
					value_error(1, "Syntax Error: Opening curly bracket not found.");
					value_clear(&sexp);
					return value_init_error();
				}
								
				// Newlines that aren't near any expressions are ignored. Others are converted to 
				// semicolons.
				size_t prev_nl_inx = cinx;
				int inner_curlies = 0;
				for (j = cinx+1; j < i; ++j) {
					if (words[j].type != VALUE_ID)
						continue;
					
					// Keep track of whether there are any curly brackets inside of the block. If there are, 
					// don't delete their nl's because those curly brackets will need them later. For example, 
					// if you have { { 3 nl 2 } } it's supposed to compile to (; 3 2), but if the outer curly 
					// brackets remove the nl, then the inner curly brackets will see { 3 2 } which won't 
					// compile correctly.
					if (streq(words[j].core.u_id, "{")) {
						++inner_curlies;
					} else if (streq(words[j].core.u_id, "}")) {
						--inner_curlies;
					
					} else if (inner_curlies == 0 && streq(words[j].core.u_id, "nl")) {
						if (words[j-1].type == VALUE_ID && streq(words[j-1].core.u_id, "{")	|| // previous was a curly bracket
							words[j-1].type == VALUE_BIF && words[j-1].core.u_bif->f == &value_do_both_arg || // previous was a semicolon
							words[j+1].type == VALUE_ID && (streq(words[j+1].core.u_id, "}") || // next is a curly bracket or newline
															streq(words[j+1].core.u_id, "nl"))) {
							// The nl is not surrounded by expressions. Delete it.
							value_clear(&words[j]);
							size_t k;
							for (k = j; k < wordcount-1; ++k)
								words[k] = words[k+1];
							words[k].type = VALUE_NIL;
							--wordcount;
							--i;
							--j;
						} else {
							// The nl is surrounded by expressions. Do something with it.
							
							res = compile_values(words+prev_nl_inx+1, j-prev_nl_inx-1);
							if (res.type == VALUE_ERROR)
								break;
							
							size_t old_wordcount = wordcount;
							wordcount = delete_from_words(words, wordcount, res, j-1, prev_nl_inx+1);
							i -= old_wordcount - wordcount;
							j -= old_wordcount - wordcount;
														
							// Convert the nl to a semicolon. It has to pull the function from (primitive_funs) 
							// rather than just using value_set_fun() because it has to get the function spec 
							// correct.
							value_clear(&words[j]);
							value key = value_set_id(";");
							words[j] = value_hash_get(primitive_funs, key);
							value_clear(&key);
							
							prev_nl_inx = j;
						}

					}
				}
				
				if (prev_nl_inx != cinx) {
					res = compile_values(words+prev_nl_inx+1, j-prev_nl_inx-1);
					if (res.type == VALUE_ERROR)
						break;
					
					size_t old_wordcount = wordcount;
					wordcount = delete_from_words(words, wordcount, res, j-1, prev_nl_inx+1);
					i -= old_wordcount - wordcount;
					j -= old_wordcount - wordcount;
				}
				
				res = compile_values(words+cinx+1, i-cinx-1);
								
				if (res.type == VALUE_ERROR)
					break;
				
				// 
				// This code removes all the words that have been evaluated, 
				// i.e. the words in between the brackets and the brackets
				// themselves.
				// 
				value_clear(&words[cinx]);
				words[cinx] = res;
				res.type = VALUE_NIL;
				for (j = 1; i+j < wordcount; ++j) {
					value_clear(&words[cinx+j]);
					words[cinx+j] = words[i+j];
					words[i+j].type = VALUE_NIL;
				}
				
				wordcount = cinx + j;
				
				// Move (i) back to the beginning of the brackets.
				i = cinx;
				
			} // if (paren_balance)
			
		}

		
	} // find parentheses and brackets
	
	if (sexp.type == VALUE_ERROR)
		return sexp;
	if (res.type == VALUE_ERROR)
		return res;
	
	// If there are more values before the first function than that function has arguments, 
	// there is probably a mistake in the syntax.
	if (scope_assume_first_is_function && first_function_index != -1 && 
			words[first_function_index].type != VALUE_UDF_SHELL) {
		
		struct value_spec wspec = get_spec(words[first_function_index]);
		if (wspec.argc > 0 && wspec.argc < first_function_index && wspec.optional >= wspec.argc) {
			value tmpexp;
			tmpexp.type = VALUE_BLK;
			tmpexp.core.u_blk.a = words;
			tmpexp.core.u_blk.length = wordcount;
			tmpexp.core.u_blk.s = NULL;
			value_error(0, "Warning: Expression %s will probably not evaluate as expected (function at index %d has arg count %d).", tmpexp, first_function_index, wspec.argc);
		}
	}
	
	// Convert to prefix notation.
	
	if (infix_p(words, wordcount)) {
		value old_words[wordcount];
		
		for (i = 0; i < wordcount; ++i) {
			old_words[i] = words[i];
			words[i].type = VALUE_NIL;
		}
		
		int error_p = infix_to_prefix(words, old_words, wordcount);

		if (error_p) {
			value_clear(&sexp);
			sexp = value_init_error();
		}
	}

	if (sexp.type == VALUE_ERROR)
		return sexp;
	
	// Fix things like (a[i][j] = x). It compiles to (at_equals at a i j x) so 
	// change it to (at_equals a i j x). This is an inelegant solution because 
	// it should be changed earlier, but at least it works.
	value func_at = value_set_fun(&value_at_arg);
	for (i = 0; i < wordcount-1; ++i) {
		if (value_eq(words[i+1], func_at) && words[i].type == VALUE_BIF) {
			if (
			words[i].core.u_bif->f == &value_at_assign_arg || 
			words[i].core.u_bif->f == &value_at_assign_add_arg || 
			words[i].core.u_bif->f == &value_at_assign_sub_arg || 
			words[i].core.u_bif->f == &value_at_assign_mul_arg || 
			words[i].core.u_bif->f == &value_at_assign_div_arg || 
			words[i].core.u_bif->f == &value_at_assign_mod_arg || 
			words[i].core.u_bif->f == &value_at_assign_and_arg || 
			words[i].core.u_bif->f == &value_at_assign_xor_arg || 
			words[i].core.u_bif->f == &value_at_assign_or_arg || 
			words[i].core.u_bif->f == &value_at_assign_shl_arg || 
			words[i].core.u_bif->f == &value_at_assign_shr_arg
			) {
				value_clear(&words[i+1]);
				for (j = i+1; j < wordcount-1; ++j) {
					words[j] = words[j+1];
					words[j+1].type = VALUE_NIL;
				}
				--wordcount;
			}
		}
	}
	value_clear(&func_at);
	
	// Convert to an S-expression and return.
	
	value_clear(&sexp);
	size_t it = 0;
	sexp = prefix_words_to_sexp(words, &it, wordcount);
		
	assume_first_is_function = TRUE;
		
	return sexp;
}

value prefix_words_to_sexp(value words[], size_t *i, size_t length)
{
	// If the expression contains only one argument, place the argument inside a block.
	if (length == 1 && words[*i].type != VALUE_BIF && words[*i].type != VALUE_UDF && words[*i].type != VALUE_UDF_SHELL) {
		if (words[*i].type == VALUE_BLK)
			return value_set(words[*i]);
		value sexp;
		
		sexp.type = VALUE_BLK;
		value_malloc(&sexp, next_size(1));
		return_if_error(sexp);
		sexp.core.u_blk.s = NULL;
		sexp.core.u_blk.a[0] = value_set(words[*i]);
		sexp.core.u_blk.length = 1;
		return sexp;
	}
	
	// If the s-expression does not start with a function name, just put every 
	// word into a block and return it.
	if (words[*i].type != VALUE_BIF && words[*i].type != VALUE_UDF && words[*i].type != VALUE_UDF_SHELL) {
		value sexp;
		
		sexp.type = VALUE_BLK;
		value_malloc(&sexp, next_size(length));
		return_if_error(sexp);
		sexp.core.u_blk.length = length;
		sexp.core.u_blk.s = NULL;
		size_t j;
		for (j = *i; j < length; ++j)
			sexp.core.u_blk.a[j] = value_set(words[j]);
		return sexp;
	}
	
	struct value_spec spec;
	if (words[*i].type == VALUE_BIF)
		spec = words[*i].core.u_bif->spec;
	else if (words[*i].type == VALUE_UDF)
		spec = words[*i].core.u_udf->spec;
	else if (words[*i].type == VALUE_UDF_SHELL)
		spec = words[*i].core.u_udf->spec;
	
	// For a function that takes any number of arguments, set the number of arguments 
	// to the total length of the given words so that they'll all be passed in.
	int argc = spec.argc;
	if (spec.rest_p) {
		argc = length - 1;
	}
		
	value sexp;
	sexp.type = VALUE_BLK;
	value_malloc(&sexp, next_size(argc+1));	
	return_if_error(sexp);	
	sexp.core.u_blk.s = NULL;
	sexp.core.u_blk.a[0] = value_set(words[*i]);
	
	int prev_quote_p = words[*i].type == VALUE_BIF && words[*i].core.u_bif->f == &value_quote_arg;
	int prev_def_p = words[*i].type == VALUE_BIF && words[*i].core.u_bif->f == &value_def_arg;
	size_t first = *i;
	++*i;
	size_t j;
	for (j = 1; j <= argc && *i < length; ++j) {
		if (!prev_def_p && !prev_quote_p && (words[*i].type == VALUE_BIF || words[*i].type == VALUE_UDF || words[*i].type == VALUE_UDF_SHELL)) {
			value temp = prefix_words_to_sexp(words, i, length);
			if (temp.type == VALUE_ERROR) {
				value_clear(&sexp);
				return temp;
			}
			sexp.core.u_blk.a[j] = temp;
		} else {
			sexp.core.u_blk.a[j] = value_set(words[(*i)++]);
		}
	}
	
	// If this is the outermost sexp and there are too many arguments, return an error. It can tell 
	// if it's the outermost sexp if (first) is equal to 0, which means the sexp starts at the 
	// beginning of (words).
	if (first == 0 && *i < length) {
		// This should be done at runtime.
		value_error(1, "Argument Error: Too many arguments in function %s (%d expected, at least %ld found).", words[first], argc, j);
		value_clear(&sexp);
		return value_init_error();
	}
	
	// If the sexp is missing some arguments, unless the function takes any number of 
	// arguments, set all the missing arguments to nil.
//	if (!argc_changed)
//		for (; j <= argc; ++j)
//			sexp.core.u_blk.a[j] = value_init_nil();
	
	sexp.core.u_blk.length = j;
	
	return sexp;
}

value interpret_values(value *variables, value words[], size_t wordcount)
{
	if (wordcount == 0) {
		return value_init_nil();
	}
	value sexp = compile_values(words, wordcount);
	if (sexp.type == VALUE_ERROR) {
		printf("\tin statement: ");
		print_values(words, wordcount);
		return sexp;
	}
	
	value res = eval(variables, sexp);
	value_clear(&sexp);
	return res;
}

/* 
 * Determines whether (words) is infix or prefix. Looks at the first word that's 
 * not a VALUE_ID. If it's a function call, then the words are in prefix notation. 
 * If it is a value, then they are in infix notation. If it is a function call with 
 * only one argument, then it's impossible to tell yet, so go to the next word.
 */
int infix_p(value words[], size_t wordcount)
{
	int is_infix = FALSE;
	size_t i;
	for (i = 0; i < wordcount; ++i)
		if (words[i].type != VALUE_ID && words[i].type != VALUE_BIF && words[i].type != VALUE_UDF && words[i].type != VALUE_UDF_SHELL) {
			if (i + 1 != wordcount) {
				is_infix = TRUE;
				break;
			}
		} else if (words[i].type != VALUE_ID) { 
			// If arg count is 2 or greater, the expression is prefi.
			// If arg count is 1, we don't know yet if the statement is prefix or infix.
			if (arg_count(words[i]) != 1)
				break;
		}
	
	return is_infix;
}

/* 
 * Converts an infix expression to prefix based on the Shunting Yard Algorithm.
 * http://en.wikipedia.org/wiki/Shunting_yard_algorithm#The_algorithm_in_detail 
 * 
 * outwords: Destination array, (length) elements long. Holds the prefix 
 *   expression.
 * inwords: Source array, (length) elements along. Holds an infix expression.
 * length: Length of (outwords) and (inwords).
 * 
 * This is not exactly like the Shunting Yard Algorithm, as it has been modified 
 * to properly handle unary operators. In pseudo-code, the algorithm looks like: 

pstack = new stack

for i from length downto 0: 
  if inwords[i] is a value:
    outwords.prepend(j)
  else if inwords[i] is a function: 
    while pstack.top is a function: 
	  if inwords[i] has only one argument: 
	    switch associativity from left to right or vice versa
		
	  unless (inwords[i] is left-associative and inwords[i].precedence <= pstack.top.precedence)
	      OR (inwords[i] is right-associative and inwords[i].precedence < pstack.top.precedence): 
	    break
	  outwords.prepend(pstack.pop())
	pstack.push(inwords[i])

 * 
 */
int infix_to_prefix(value outwords[], value inwords[], size_t length)
{
	value pstack[length];
	
	// The index of the first empty value in (pstack).
	int pindex = 0;
		
	size_t i, j = length;
	
	
	// This loop stops when -1 == i, and not when 0 > i, because if size_t is unsigned then 0 will always be <= i.
	for (i = length-1; -1 != i; --i) {
		if (inwords[i].type != VALUE_ID && inwords[i].type != VALUE_BIF && inwords[i].type != VALUE_UDF && inwords[i].type != VALUE_UDF_SHELL) {
			outwords[--j] = inwords[i];
		} else if (inwords[i].type == VALUE_BIF || inwords[i].type == VALUE_UDF || inwords[i].type == VALUE_UDF_SHELL) {
			
			while (pindex > 0) {
				
				struct value_spec spec1;
				if (inwords[i].type == VALUE_BIF) spec1 = inwords[i].core.u_bif->spec;
				else if (inwords[i].type == VALUE_UDF || inwords[i].type == VALUE_UDF_SHELL) spec1 = inwords[i].core.u_udf->spec;
				
				struct value_spec spec2;
				if (pstack[pindex-1].type == VALUE_BIF) spec2 = pstack[pindex-1].core.u_bif->spec;
				else if (pstack[pindex-1].type == VALUE_UDF || pstack[pindex-1].type == VALUE_UDF_SHELL) spec2 = pstack[pindex-1].core.u_udf->spec;
								
				if (spec1.argc == 1 && !((spec2.associativity == 'l' && spec1.precedence <= spec2.precedence) ||
					(spec2.associativity == 'r' && spec1.precedence < spec2.precedence) ))
					break;
				
				// Determine if the function is left- or right-associative, and behave accordingly.
				if (!((spec2.associativity == 'r' && spec1.precedence <= spec2.precedence) ||
					(spec2.associativity == 'l' && spec1.precedence < spec2.precedence) ))
					break;
								
				outwords[--j] = pstack[--pindex];
			}
			
			pstack[pindex++] = inwords[i];
		} else {
			if (is_symbol(inwords[i].core.u_id))
				value_error(1, "Syntax Error: Unexpected symbol %s.", inwords[i]);
			else value_error(1, "Compiler Error: In to_postfix(), undefined id %s.", inwords[i]);
			return VALUE_ERROR;
		}
	}
	
	while (pindex > 0) {
		outwords[--j] = pstack[--pindex];
	}
	
	return 0;
}

size_t count_words(char *statement)
{
	size_t wordcount = 0;
	statement = skip_whitespace(statement);
	while (*statement != '\0') {
		statement = end_of_word(statement);
		statement = skip_whitespace(statement);
		++wordcount;

	}
	
	return wordcount;
}

size_t delete_from_words(value words[], size_t wordcount, value res, size_t i, size_t inx)
{
	value_clear(&words[inx]);
	words[inx] = res;
	
	if (i <= inx)
		return wordcount;
	size_t j;
	for (j = 1; i+j < wordcount; ++j) {
		value_clear(&words[inx+j]);
		words[inx+j] = words[i+j];
		words[i+j].type = VALUE_NIL;
	}
	
	return inx + j;
}

void var_to_shell(value *body, value name, value shell)
{
	if (body->type == VALUE_BLK) {
		size_t i;
		for (i = 0; i < body->core.u_blk.length; ++i)
			var_to_shell(&body->core.u_blk.a[i], name, shell);
		
	} else if (body->type == VALUE_UDF) {
		// When redefining recursive functions, we can't use the old definition 
		// of the function. Delete it and add in the new function shell.
		if (streq(body->core.u_udf->name, name.core.u_var)) {
			value_clear(body);
			*body = value_set(shell);
		}
		
	} else if (value_eq(*body, name)) {
		value_clear(body);
		*body = value_set(shell);
	}
}

value get_next_word(value words[], size_t length)
{
	value res = value_init_nil();
	if (length <= 0) {
		// Return nil.
		
	} else if (words[0].type == VALUE_ID && streq(words[0].core.u_id, "(")) {
		size_t i;
		int balance = 1;
		for (i = 1; i < length; ++i)
			if (words[i].type == VALUE_ID && streq(words[i].core.u_id, "("))
				++balance;
			else if (words[i].type == VALUE_ID && streq(words[i].core.u_id, ")"))
				if (--balance == 0)
					break;
		
		if (balance == 0)
			res = value_set_block(words, i+1);
		
	} else if (words[0].type == VALUE_ID && streq(words[0].core.u_id, "{")) {
		size_t i;
		int balance = 1;
		for (i = 1; i < length; ++i)
			if (words[i].type == VALUE_ID && streq(words[i].core.u_id, "{"))
				++balance;
			else if (words[i].type == VALUE_ID && streq(words[i].core.u_id, "}"))
				if (--balance == 0)
					break;
		
		if (balance == 0)
			res = value_set_block(words+1, i-1);
				
	} else {
		res = value_set_block(words, 1);
	}
	
	return res;
}

char * end_of_word(char *statement)
{
 	char orig = *statement;
	if (orig == '\0')
		return statement;
	
	// Hex numbers are stated like "0x123", so that needs to be read as legal.
	if (orig == '0' && *(statement+1) == 'x') {
		++statement;
		while (isdigit(*(++statement)) || *statement >= 'a' && *statement <= 'f')
			;
		return statement;
	}
	
	
	// A symbol starts with a ':'.
	if (orig == ':' && isalpha(*(statement+1)))
		orig = *(++statement);
	else
		// A variable can start with any number of $ symbols.
		while (orig == '$' && (*(statement+1) == '$' || isalpha(*(statement+1))))
			orig = *(++statement);
	
	// In a short function name, such as 'x!', the x will just get skipped over if 
	// we go straight to the while loop. So do this part first.
	if (isalpha(orig) && (*(statement+1) == '!' || *(statement+1) == '?'))
		return statement += 2;
	
	int contains_dot_p = FALSE;
		
	while (is_same_type(*(++statement), orig) || 
				// The word can start with a letter and contain numbers.
				((isalpha(orig) || orig == '_') && isdigit(*statement)) || 
				
				// A number can contain a single 'e' to indicate scientific notation.
				isdigit(orig) && *statement == 'e' || 
				
				// A word can contain a dot to indicate a call to a class's method or variable.
				isalpha(orig) && *statement == '.' ||
				
				// Strings only terminate in a special case -- see bottom of loop.
				orig == '"' || orig == '\''
			) {
		
		if (*statement == '.' && isalnum(orig)) {
			if (contains_dot_p) {
				--statement;
				break;
			} else contains_dot_p = TRUE;
		}
				
		if (*statement == '\0')
			break;
		
		// A word can end in an exclamation mark or a question mark.
		if (isalpha(orig) && (*(statement+1) == '!' || *(statement+1) == '?')) {
			statement += 2;
			break;
		}
		
		if (orig == '"' && *statement == '"') {
			char *back = statement;
			while (*(--back) == '\\')
				;
			++back;
			if ((statement-back & 1) == 0) {
				++statement;
				break;
			}
		} else if (*statement == '"' && orig != '\'') {
			break;
		}
		
		if (orig == '\'' && *statement == '\'') {
			char *back = statement;
			while (*(--back) == '\\')
				;
			++back;
			if ((statement-back & 1) == 0) {
				++statement;
				break;
			}
		} else if (*statement == '\'' && orig != '"') {
			break;
		}
		
	}
		
	return statement;
}

char * skip_whitespace(char *statement)
{
	while (isspace(*statement))
		++statement;
	return statement;
}

int is_same_type(char c, char orig)
{
	if (same_type_determiner == SAME_TYPE_VALUE) {
		if (isbracket(c))
			return FALSE;
		if (isalpha(orig))
			if (isalpha(c) || c == '_') return TRUE;
			else return FALSE;
		if (isdigit(orig))
			if (isdigit(c) || c == '.') return TRUE;
			else return FALSE;
		if (orig == '"')
			if (c == '"') return TRUE;
			else return FALSE;
		if (orig == '\'')
			if (c == '\'') return TRUE;
			else return FALSE;
		if (isbracket(orig))
			return FALSE;
		if (iscsymbol(orig))
			if (iscsymbol(c)) return TRUE;
			else return FALSE;
		else return FALSE; // Returns FALSE even if (c) and (orig) are the same character.
	} else if (same_type_determiner == SAME_TYPE_TREE) {
		if (isbracket(c))
			return FALSE;
		if (isalpha(orig))
			if (isalpha(c) || c == '_' || iscsymbol(c) || isdigit(c)) return TRUE;
			else return FALSE;
		if (isdigit(orig) || orig == '-')
			if (isdigit(c) || c == '.') return TRUE;
			else return FALSE;
		if (orig == '"')
			if (c == '"') return TRUE;
			else return FALSE;
		if (orig == '\'')
			if (c == '\'') return TRUE;
			else return FALSE;
		if (isbracket(orig))
			return FALSE;
		if (iscsymbol(orig))
			if (iscsymbol(c)) return TRUE;
			else return FALSE;
		else return FALSE; // Returns FALSE even if (c) and (orig) are the same character.
	}
}

int char_type(char c)
{
	return isalpha(c) ? CHARTYPE_ALPHA : isdigit(c) ? CHARTYPE_NUMBER : isspace(c) ? CHARTYPE_SPACE 
			: isbracket(c) ? CHARTYPE_BRACKET
			: iscsymbol(c) ? CHARTYPE_SYMBOL
			: CHARTYPE_OTHER;
}

/* 
 * Return Values
 * 0: incomplete because of quotes, parentheses or brackets
 * 1: complete
 * 2: incomplete because of curly brackets
 */
int is_statement_complete(char *statement)
{
	char match_stack[1024];
	size_t i = 0;

	char *ptr = statement - 1;
	int parens = 0, brackets = 0, curlies = 0, quotes = 0, regexes = 0;
	int is_prev_backslash = FALSE;
	while (*(++ptr)) {
		switch (*ptr) {
			case '(':
				if (quotes == 0 && regexes == 0) match_stack[i++] = '(';
				break;
			case ')':
				if (quotes == 0 && regexes == 0)
					if (match_stack[--i] != '(') {
						value_error(1, "Syntax error: Mismatched control characters.");
						return STATEMENT_COMPLETE;
					}
				break;
			case '[':
				if (quotes == 0 && regexes == 0) match_stack[i++] = '[';
				break;
			case  ']':
				if (quotes == 0 && regexes == 0)
					if (match_stack[--i] != '[') {
						value_error(1, "Syntax error: Mismatched control characters.");
						return STATEMENT_COMPLETE;
					}				
				break;
			case '{':
				if (quotes == 0 && regexes == 0) {
					++curlies;
					match_stack[i++] = '{';
				}
				break;
			case '}':
				if (quotes == 0 && regexes == 0) 
					if (match_stack[--i] != '{') {
						value_error(1, "Syntax error: Mismatched control characters.");
						return STATEMENT_COMPLETE;
					} else --curlies;	
				break;
			case '"':
				if (regexes == 0 && !is_prev_backslash)
					quotes ^= 1;
				break;
			case '\'':
				if (quotes == 0 && !is_prev_backslash)
					regexes ^= 1;
			case '\\':
				is_prev_backslash = !is_prev_backslash;
				break;
			default:
				break;
		}
		
		if (*ptr != '\\')
			is_prev_backslash = FALSE;
	}
	
	return i == 0 ? STATEMENT_COMPLETE : match_stack[i-1] == '{' ? STATEMENT_INCOMPLETE_CURLY : STATEMENT_INCOMPLETE_OTHER;
		
	return curlies > 0 ? STATEMENT_INCOMPLETE_CURLY : parens <= 0 && brackets <= 0 && 
			quotes == 0 && regexes == 0 ? STATEMENT_COMPLETE : STATEMENT_INCOMPLETE_OTHER;
}

void line_enqueue(value op)
{
	line_queue_back.core.u_l[0] = op;
	line_queue_back = line_queue_back.core.u_l[1] = value_init(VALUE_LST);
}

void line_enqueue_front(value op)
{
	value old = line_queue;
	line_queue = value_init(VALUE_LST);
	line_queue.core.u_l[0] = op;
	line_queue.core.u_l[1] = old;
}

value line_dequeue()
{
	value res = line_queue.core.u_l[0];
	value old = line_queue;
	line_queue = line_queue.core.u_l[1];
	value_free(old.core.u_l);
	return res;
}

