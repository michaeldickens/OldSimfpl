/*
 *  value_regex.c
 *  Calculator
 *
 *  Created by Michael Dickens on 6/1/10.
 * 
 *  Uses POSIX regular expressions from the GNU C Library.
 *  GNU Regular Expressions: http://www.gnu.org/s/libc/manual/html_node/Regular-Expressions.html
 *  POSIX: http://www.regular-expressions.info/posix.html
 */

#include "value.h"

int compile_regex(regex_t *compiled, char *regex, int flags)
{
	int r = regcomp(compiled, regex, flags | REG_EXTENDED);
	if (r == REG_BADBR)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; invalid construct between curly brackets.", regex);
	else if (r == REG_BADPAT)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; syntax error.", regex);
	else if (r == REG_BADRPT)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; repetition operator (?, *, +) appeared in a bad position.", regex);
	else if (r == REG_ECOLLATE)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; referred to an invalid collating element.", regex);
	else if (r == REG_ECTYPE)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; referred to an invalid character class name.", regex);
	else if (r == REG_EESCAPE)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; ended with '\\'.", regex);
	else if (r == REG_ESUBREG)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; invalid number in \\digit construct.", regex);
	else if (r == REG_EBRACK)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; unbalanced square brackets.", regex);
	else if (r == REG_EPAREN)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; unbalanced parentheses.", regex);
	else if (r == REG_EBRACE)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; unbalanced curly brackets.", regex);
	else if (r == REG_ERANGE)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; one of the enpoints in a range expression was invalid.", regex);
	else if (r == REG_ESPACE)
		value_error(1, "Syntax Error: Cannot compile regular expression '%s'; compiler ran out of memory.", regex);
	
	return r;
}

int value_match_p(value regex, value str)
{
	int error_p = FALSE;
	if (regex.type != VALUE_RGX && regex.type != VALUE_STR) {
		value_error(1, "Type Error: match?() is undefined where regex is %ts (string or regular expression expected).", regex);
		error_p = TRUE;
	}
	
	if (str.type != VALUE_STR) {
		value_error(1, "Type Error: match?() is undefined where str is %ts (string expected).", str);
		error_p = TRUE;
	}
	
	if (error_p)
		return -1;
	
	regex_t compiled;
	int r = compile_regex(&compiled, regex.core.u_x, 0);
	if (r != 0)
		return -2;
	int match = regexec(&compiled, str.core.u_s, 0, NULL, 0);
	if (match == REG_ESPACE) {
		value_error(1, "Memory Error: match?() ran out of memory.");
		return -3;
	}
	
	return match == 0;
}

value value_match_p_std(value regex, value str)
{
	int match = value_match_p(regex, str);
	if (match < 0)
		return value_init_error();
	return value_set_bool(match);
}

/* 
 * Return Codes (contained in res.rm_so)
 * 
 * -1: No match.
 * -2: Memory error.
 * -3: Compile error.
 * -4: Type error.
 */
regmatch_t value_match(value regex, value str)
{
	int error_p = FALSE;
	if (regex.type != VALUE_RGX && regex.type != VALUE_STR) {
		value_error(1, "Type Error: match() is undefined where regex is %ts (string or regular expression expected).", regex);
		error_p = TRUE;
	}
	
	if (str.type != VALUE_STR) {
		value_error(1, "Type Error: match() is undefined where str is %ts (string expected).", str);
		error_p = TRUE;
	}
	
	regmatch_t matchptr[1];
	matchptr[0].rm_so = -1; // -1 indicates that there was no match.
	
	if (error_p) {
		matchptr[0].rm_so = -4;
		return matchptr[0];
	}
	
	regex_t compiled;
	int r = compile_regex(&compiled, regex.core.u_x, 0);
	if (r != 0) {
		matchptr[0].rm_so = -3;
		return matchptr[0];
	}
	
	int match = regexec(&compiled, str.core.u_s, 1, matchptr, 0);
	if (match == REG_ESPACE) {
		value_error(1, "Memory Error: match() ran out of memory.");
		matchptr[0].rm_so = -2;
		return matchptr[0];
	}

	return matchptr[0];	
}

regmatch_t value_match_str(value regex, char *str)
{
	value x;
	x.type = VALUE_STR;
	x.core.u_s = str;
	return value_match(regex, x);
}

value value_match_std(value regex, value str)
{
	regmatch_t match = value_match(regex, str);
	
	if (match.rm_so < -1)
		return value_init_error();
	if (match.rm_so == -1)
		return value_init_nil();
	return value_set_long((long) match.rm_so);	
}

value value_match_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "match?()") ? value_init_error() : value_match_p_std(argv[0], argv[1]);
}

value value_match_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "match()") ? value_init_error() : value_match_std(argv[0], argv[1]);
}
