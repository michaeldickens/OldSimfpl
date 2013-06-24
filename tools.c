/*
 *  tools.c
 *  Calculator
 *
 *  Created by Michael Dickens on 1/9/10.
 *
 */

#include "tools.h"

int init_tools()
{
	print_errors_p = TRUE;
	error_count = 0;
	linenum = 0;
		
	// Initialize the random number generator.
	init_genrand((unsigned long) time(NULL));

	return 0;
}

/* 
 * Not a symbol: 
 * 
 * ( ) [ ] { } $ ;
 */
int iscsymbol(char c)
{
	switch (c) {
		case '!': case '#': case '%': case '&': 
		case '*': case '+': case '-': case '.': case '/': 
		case ':': case '<': case '=': case '>': case '?': 
		case '@': case '\\': case '^': case '_': case '`': 
		case '|': case '~': 
			return TRUE;
		default:
			return FALSE;
	}
}

int streq2(char *s, char *t)
{
	--s; --t;
	while (*(++s))
		if (*s != *(++t))
			return FALSE;
	return TRUE;
}

int strne2(char *s, char *t)
{
	--s; --t;
	while (*(++s) && *(++t))
		if (*s == *t)
			return FALSE;
	return TRUE;
}

int print_words(char *words[], int length)
{
	int i;
	for (i = 0; i < length; ++i) {
		printf("%s ", words[i]);
	} printf("\n");
	return 0;
}

/*
 * Takes minus signs into account.
 */
int minus_isnumeric(char *str)
{
	if (str == NULL) {
		value_error(0, "Compiler warning: Null string passed to isnumeric().");
		return FALSE;
	}
		
	int pointcount = 0; // There can only be one decimal point in a number, so keep track.
	int xcount = 0; // Same deal as pointcount.
	int is_hex = FALSE;
	int isbeforedigits = TRUE;
	
	while (isspace(*str))
		++str;
	
	if (*str == '-' && *(str+1) == '\0')
		return FALSE;
	
	if (*str == '-' || isdigit(*str)) {
		if (*str == '-')
			++str;
		
		if (isdigit(*str))
			isbeforedigits = FALSE;
		while (*(++str) != '\0') {
			if (!isdigit(*str))
				if (is_hex && *str >= 'a' && *str <= 'f')
					;
				else if (xcount == 0 && isbeforedigits == FALSE && *str == 'x') {
					++xcount;
					is_hex = TRUE;
				} else if (pointcount == 0 && isbeforedigits == FALSE && *str == '.')
					++pointcount;
				else return FALSE;
			else isbeforedigits = FALSE;
		}
		if (*str == '.')
			return FALSE; // the point cannot be the last character
		return pointcount + 1; // 1 for integer, 2 for float
	} else return FALSE;
}

/*
 * Does not consider minus signs to be a part of the number.
 */
int isnumeric(char *str)
{
	if (str == NULL) {
		value_error(0, "Compiler warning: Null string passed to isnumeric().");
		return -1;
	}
		
	int pointcount = 0; // There can only be one decimal point in a number, so keep track.
	int xcount = 0; // Same deal as pointcount.
	int is_hex = FALSE;
		
	while (isspace(*str))
		++str;
	
	int scientific_p = FALSE;
	
	if (*str == '-')
		++str;
	
	if (isdigit(*str)) {
		
		while (*(++str)) {
			if (!isdigit(*str))
				if (is_hex && *str >= 'a' && *str <= 'f')
					;
				else if (*str == 'e') {
					if (scientific_p) {
						value_error(1, "Syntax Error: Illegal number syntax %c; numbers in scientific notation may not have more than one 'e'.", str);
						return -1;
					} else {
						scientific_p = TRUE;
						if (pointcount == 0) pointcount = 1;
					}
				} else if (xcount == 0 && *str == 'x') {
					++xcount;
					is_hex = TRUE;
				} else if (pointcount == 0 && *str == '.')
					++pointcount;
				else return FALSE;
		}
		if (*str == '.')
			return FALSE; // the point cannot be the last character
		return pointcount + 1; // 1 for integer, 2 for float
	} else return FALSE;
}

int isword(char *str)
{
	if (str == NULL) {
		value_error(0, "Compiler warning: Null string passed to isnumeric().");
		return FALSE;
	}
	
	if (!isalpha(*str))
		return FALSE;
	while (*(++str))
		if (!isalnum(*str))
			return FALSE;
	return TRUE;
}

int is_string_literal(char *str)
{
	if (*str != '"')
		return FALSE;
	int is_prev_backslash = FALSE;
	int is_end = FALSE;
	while (*(++str)) {
		if (is_end && !isspace(*str)) {
			// We are off the end of the string.
			return FALSE;
		}
		
		
		if (*str == '\\')
			is_prev_backslash = !is_prev_backslash;
		else {
			if (*str == '"' && !is_prev_backslash)
				is_end = TRUE;
			is_prev_backslash = FALSE;
		}
	}
	
	return is_end;
}

int is_regex_literal(char *str)
{
	if (*str != '\'')
		return FALSE;
	int is_prev_backslash = FALSE;
	int is_end = FALSE;
	while (*(++str)) {
		if (is_end && !isspace(*str)) {
			// We are off the end of the string.
			return FALSE;
		}
		
		
		if (*str == '\\')
			is_prev_backslash = !is_prev_backslash;
		else {
			if (*str == '\'' && !is_prev_backslash)
				is_end = TRUE;
			is_prev_backslash = FALSE;
		}
	}
	
	return is_end;
}

/* Since GNU POSIX does not support shorthand character classes, this creates them.
 * Supports \w, \d, \s and the corresponding uppercase versions.
 */
char * convert_regex_to_literal(char *str)
{
	size_t len = strlen(str);
	char tmp[len * 3];
	char *ptr = tmp;
	while (*str) {
		if (*str == '\\') {
			++str;
			if (*str == 'w') {
				strcpy(ptr, "[A-Za-z0-9_]");
				ptr += strlen(ptr);
			} else if (*str == 'd') {
				strcpy(ptr, "[0-9]");
				ptr += strlen(ptr);
			} else if (*str == 's') {
				strcpy(ptr, "[ \t\n\f\r]");
				ptr += strlen(ptr);
			} else if (*str == 'W') {
				strcpy(ptr, "[^A-Za-z0-9_]");
				ptr += strlen(ptr);
			} else if (*str == 'D') {
				strcpy(ptr, "[^0-9]");
				ptr += strlen(ptr);
			} else if (*str == 'S') {
				strcpy(ptr, "[^ \t\n\f\r]");
				ptr += strlen(ptr);
			} else if (*str != '\'') {
				*(ptr++) = '\\';
				*(ptr++) = *str;
			}
		} else {
			*ptr = *str;
			++ptr;
		}
		++str;
	}
	
	*ptr = '\0';
	
	char *res = value_malloc(NULL, sizeof(char) * (strlen(tmp) + 1));
	if (res) strcpy(res, tmp);
	return res;
}

/* Converts "\n" into a newline, "\\" into a backslash, etc.
 */
char * convert_to_literal(char *str)
{
	char *res = value_malloc(NULL, strlen(str) + 1);
	if (res == NULL) return NULL;
	char *ptr = res;
	while (*str) {
		if (*str == '\\') {
			++str;
			switch (*str) {
				case '\\':
					*ptr = '\\';
					break;
				case '"':
					*ptr = '"';
					break;
				case 'a':
					*ptr = '\a';
					break;
				case 'b':
					*ptr = '\b';
					break;
				case 'f':
					*ptr = '\f';
					break;
				case 'n':
					*ptr = '\n';
					break;
				case 'r':
					*ptr = '\r';
					break;
				case 't': 
					*ptr = '\t';
					break;
				case 'v':
					*ptr = '\v';
					break;
				default:
					*ptr = *str; // Just skip the backslash.
					break;
			}
		} else *ptr = *str;
		++ptr; ++str;
	}
	
	*ptr = '\0';
	res = realloc(res, strlen(res) + 1);
	return res;
}

int array_index(char *words[], int length, char *word)
{
	int i;
	for (i = 0; i < length; ++i) {
		if (strcmp(words[i], word) == 0)
			return i;
	}
	
	return -1;
}


int array_last_index(char *words[], int length, char *word)
{
	int i;
	for (i = length-1; 0 <= i; --i) {
		if (strcmp(words[i], word) == 0)
			return i;
	}
	
	return -1;
}

int array_copy(char *target[], char *source[], int size)
{
	int i;
	for (i = 0; i < size; ++i) {
		target[i] = value_malloc(NULL, strlen(source[i])+1);
		if (target[i] == NULL) return -1;
		strcpy(target[i], source[i]);
	}
	return 0;
}

int fatal_error()
{
	if (error_count == 1) {
		fprintf(stderr, "Build failed. At least 1 error found.\n");
		exit(1);
	} else if (error_count > 1) {
		fprintf(stderr, "Build failed. At least %d errors found.\n", error_count);
		exit(1);
	}
	
	return 0;
}

int handle_errors()
{
	if (error_count == 1) {
		fprintf(stderr, "Build failed. 1 error found.\n");
		exit(1);
	} else if (error_count > 1) {
		fprintf(stderr, "Build failed. %d errors found.\n", error_count);
		exit(1);
	} else fprintf(stderr, "Build succeeded.\n");
	
	return 0;
}

/* Returns the next power of 2. In the case in which x is already a power of 2, 
 * it will return x << 1. 0 returns 0.
 */
size_t next_power_of_2(size_t x)
{
	// This ORs x with every possible 1 bit up to its length, so every bit will 
	// be filled. Then it adds 1 which for instance turns 111 into 1000.
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x |= x >> 32;
	return x;
}

size_t next_size(size_t x)
{
	return x < RESIZE_MIN ? RESIZE_MIN : next_power_of_2(x);
}

int is_power_of_2(size_t x)
{
	// Iff x is a power of 2, then x and (x-1) will share no digits. For instance, 
	// 1000 & 111 = 0.
	return !(x & (x - 1));	
}

int resize_p(size_t x)
{
	// Iff x is a power of 2, then x and (x-1) will share no digits. For instance, 
	// 1000 & 111 = 0. Do not resize unless x is greater than a certain minimum.
	return x == 0 || x >= RESIZE_MIN && !(x & (x - 1));	
}
