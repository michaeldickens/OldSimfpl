/*
 *  value_string.c
 *  Calculator
 *
 *  Created by Michael Dickens on 2/13/10.
 *
 */

#include "value.h"

value value_set_str(char *str)
{
	value res;
	
	res.type = VALUE_STR;
	size_t len = strlen(str);
	value_malloc(&res, len+1);
	return_if_error(res);
	strcpy(res.core.u_s, str);
	return res;
}

value value_set_str_length(char *str, size_t length)
{
	value res;
	
	res.type = VALUE_STR;
	value_malloc(&res, length + 1);
	return_if_error(res);
	strncpy(res.core.u_s, str, length);
	res.core.u_s[length] = '\0';
	return res;
}

value value_set_symbol(char *str)
{
	value res;
	
	res.type = VALUE_SYM;
	size_t len = strlen(str);
	res.core.u_s = value_malloc(NULL, len+1);
	return_if_null(res.core.u_s);
	strcpy(res.core.u_s, str);
	return res;
}

value value_capitalize(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: String operation capitalize() is undefined where op is not a string.");
		return value_init_error();
	}
	
	value res;
	
	res.type = VALUE_STR;
	value_malloc(&res, strlen(op.core.u_s) + 1);
	return_if_error(res);
	strcpy(res.core.u_s, op.core.u_s);
	char *ptr = res.core.u_s;
	*ptr = toupper(*ptr);
	while (*(++ptr) = tolower(*ptr))
		;
	
	return res;
}

value value_asc(value op)
{
	if (op.type == VALUE_STR) {
		if (op.core.u_s[0] != '\0' && op.core.u_s[1] == '\0') {
			return value_set_long(op.core.u_s[0]);
		} else {
			value_error(1, "Type Error: asc() is undefined where op is %s (it must contain only one character).", op);
		}
		
	} else {
		value_error(1, "Type Error: asc() is undefined where op is %ts (string expected).", op);
	}

	return value_init_error();
}

value value_chop(value op)
{
	if (op.type == VALUE_STR) {
		value res;
		
		res.type = VALUE_STR;
		size_t length = strlen(op.core.u_s);
		if (length == 0) {
			res = value_set_str("");
			return res;
		}
		value_malloc(&res, length);
		return_if_error(res);
		strncpy(res.core.u_s, op.core.u_s, length-1);
		res.core.u_s[length-1] = '\0';
		return res;
	} else if (op.type == VALUE_RGX) {
		value res;
		
		res.type = VALUE_RGX;
		size_t length = strlen(op.core.u_x);
		if (length == 0) {
			res.core.u_x = value_malloc(NULL, sizeof(char) * 1);
			return_if_null(res.core.u_x);
			res.core.u_x[0] = '\0';
			return res;
		}
		res.core.u_x = value_malloc(NULL, length);
		return_if_null(res.core.u_x);
		strncpy(res.core.u_x, op.core.u_x, length-1);
		res.core.u_s[length-1] = '\0';
		return res;
	} else {
		value_error(1, "Type Error: chop() is undefined where op is %ts (string expected).", op);
		return value_init_error();
	}
	
}

value value_chop_now(value *op)
{
	value res = value_init_nil();
	if (op->type == VALUE_STR) {
		size_t length = strlen(op->core.u_s);
		value_realloc(op, length);
		return_if_error(*op);
		op->core.u_s[length-1] = '\0';
	} else if (op->type == VALUE_RGX) {
		op->type = VALUE_STR;
		res = value_chop_now(op);
		op->type = VALUE_RGX;
	} else {
		value_error(1, "Type Error: chop!() is undefined where op is %ts (string expected).", *op);
		res = value_init_error();
	}
	
	return res;
}

value value_chr(value op)
{
	if (op.type == VALUE_MPZ) {
		value zero = value_set_long(0);
		value charmax = value_set_long(256);
		value res;
		if (value_ge(op, zero) && value_lt(op, charmax)) {
			char str[2];
			str[0] = value_get_ulong(op);
			str[1] = '\0';
			res = value_set_str(str);
		} else {
			value_error(1, "Argument Error: In chr(), %ts is out of the character range.", op);
			res = value_init_error();
		}
		
		value_clear(&zero);
		value_clear(&charmax);
		return res;
	} else {
		value_error(1, "Type Error: chr() is undefined where op is %ts (integer expected).", op);
		return value_init_error();
	}

}

int value_contains_p(value op1, value op2)
{
	value res = value_contains_p_std(op1, op2);
	if (res.type == VALUE_BOO)
		return res.core.u_b;
	else return FALSE;
}

value value_contains_p_std(value op1, value op2)
{
	if (op1.type == VALUE_STR) {
		if (op2.type == VALUE_STR) {
			return value_set_bool(strstr(op1.core.u_s, op2.core.u_s) != NULL);
		} else if (op2.type == VALUE_RGX) {
			regex_t compiled;
			int r = compile_regex(&compiled, op2.core.u_x, 0);
			if (r != 0)
				return value_init_error();
			int match = regexec(&compiled, op1.core.u_s, 0, NULL, 0);
			if (match == REG_ESPACE) {
				value_error(1, "Memory Error: contains?() ran out of memory.");
				return value_init_error();
			}
			return value_set_bool(match == 0);			
		} else {
			value_error(1, "Type Error: contains?() is undefined where op1 is a string and op2 is %ts (string or regular expression expected).", op1);
			return value_init_error();
		}

	} else if (op1.type == VALUE_ARY) {
		int length = value_length(op1);
		int i;
		for (i = 0; i < length; ++i)
			if (value_eq(op1.core.u_a.a[i], op2))
				return value_set_bool(TRUE);
		return value_set_bool(FALSE);
		
	} else if (op1.type == VALUE_LST) {
		while (!value_empty_p(op1))
			if (value_eq(op1.core.u_l[0], op2))
				return value_set_bool(TRUE);
			else op1 = op1.core.u_l[1];
		return value_set_bool(FALSE);
		
	} else if (op1.type == VALUE_PAR) {
		while (op1.type == VALUE_PAR)
			if (value_eq(op1.core.u_l[0], op2))
				return value_set_bool(TRUE);
			else op1 = op1.core.u_p->tail;
		return value_set_bool(FALSE);
		
	} else if (op1.type == VALUE_HSH) {
		return value_set_bool(value_hash_exists(op1, op2));
	}
	
	value_error(1, "Type Error: contains?() is undefined where op1 is %ts (string or array expected).", op1);
	return value_init_error();
}

int value_ends_with_p(value op1, value op2)
{
	return value_ends_with_p_std(op1, op2).core.u_b;
}

value value_ends_with_p_std(value op1, value op2)
{
	int error_p = FALSE;
	if (op1.type != VALUE_STR) {
		value_error(1, "Type Error: ends_with() is undefined where op1 is %ts (string expected).", op1);
		error_p = TRUE;
	}
		
	if (op2.type != VALUE_STR) {
		value_error(1, "Type Error: ends_with() is undefined where op2 is %ts (string expected).", op2);
		error_p = TRUE;
	}
	
	if (error_p)
		return value_init_error();
	
	size_t addition = value_length(op1) - value_length(op2);
	return value_set_bool(strcmp(op1.core.u_s + addition, op2.core.u_s) == 0);
}

size_t value_index(value op1, value op2)
{	
	if (op1.type == VALUE_STR) {
		if (op2.type == VALUE_STR) {
			char *ptr = strstr(op1.core.u_s, op2.core.u_s);
			if (ptr == NULL)
				return -1;
			else return ptr - op1.core.u_s;
		} else if (op2.type == VALUE_RGX) {
			regex_t compiled;
			int r = compile_regex(&compiled, op2.core.u_x, 0);
			if (r != 0)
				return -2;
			
			// The first element of regexec() tells where the string matches. That's all we care about.
			regmatch_t matchptr[1];
			matchptr[0].rm_so = -1;
			int match = regexec(&compiled, op1.core.u_s, 1, matchptr, 0);
			if (match == REG_ESPACE) {
				value_error(1, "Memory Error: match() ran out of memory.");
				return -2;
			}
			if (matchptr[0].rm_so == -1)
				return -1;
			return (size_t) matchptr[0].rm_so;
			
		} else {
			value_error(1, "Type Error: index() is undefined where op1 is a string and op2 is %ts (string or regular expression expected).", op2);
			return -2;			
		}

	}
	
	if (op1.type == VALUE_ARY) {
		size_t length = value_length(op1);
		size_t i;
		for (i = 0; i < length; ++i)
			if (value_eq(op1.core.u_a.a[i], op2))
				return i;
		return -1;
	}
	
	if (op1.type == VALUE_LST) {
		size_t i;
		for (i = 0; !value_empty_p(op1); ++i)
			if (value_eq(op1.core.u_l[0], op2))
				return i;
			else op1 = op1.core.u_l[1];
		return -1;
	}
	
	if (op1.type == VALUE_PAR) {
		size_t i;
		for (i = 0; op1.type == VALUE_PAR; ++i)
			if (value_eq(op1.core.u_p->head, op2))
				return i;
			else op1 = op1.core.u_p->tail;
		return -1;
	}
	
	value_error(1, "Type Error: index() is undefined where op1 is %ts (string, array or list expected).", op1);
	return -2;
}

value value_index_std(value op1, value op2)
{
	if (op1.type == VALUE_STR) {
		if (op2.type == VALUE_STR) {
			char *ptr = strstr(op1.core.u_s, op2.core.u_s);
			if (ptr == NULL)
				return value_init_nil();
			else return value_set_long(ptr - op1.core.u_s);
		} else if (op2.type == VALUE_RGX) {
			regex_t compiled;
			int r = compile_regex(&compiled, op2.core.u_x, 0);
			if (r != 0)
				return value_init_error();
			
			// The first element of regexec() tells where the string matches. That's all we care about.
			regmatch_t matchptr[1];
			matchptr[0].rm_so = -1;
			int match = regexec(&compiled, op1.core.u_s, 1, matchptr, 0);
			if (match == REG_ESPACE) {
				value_error(1, "Memory Error: match() ran out of memory.");
				return value_init_error();
			}
			if (matchptr[0].rm_so == -1)
				return value_init_nil();
			return value_set_long((long) matchptr[0].rm_so);
			
		} else {
			value_error(1, "Type Error: index() is undefined where op1 is a string and op2 is %ts (string or regular expression expected).", op2);
			return value_init_error();			
		}

	} else if (op1.type == VALUE_ARY) {
		size_t length = value_length(op1);
		size_t i;
		for (i = 0; i < length; ++i)
			if (value_eq(op1.core.u_a.a[i], op2))
				return value_set_ulong(i);
		return value_init_nil();
		
	} else if (op1.type == VALUE_LST) {
		size_t i;
		for (i = 0; !value_empty_p(op1); ++i)
			if (value_eq(op1.core.u_l[0], op2))
				return value_set_long(i);
			else op1 = op1.core.u_l[1];
		return value_init_nil();
		
	} else if (op1.type == VALUE_PAR) {
		size_t i;
		for (i = 0; op1.type == VALUE_PAR; ++i)
			if (value_eq(op1.core.u_p->head, op2))
				return value_set_long(i);
			else op1 = op1.core.u_p->tail;
		return value_init_nil();
	}
	
	value_error(1, "Type Error: index() is undefined where op1 is %ts (string, array or list expected).", op1);
	return value_init_error();
}

value value_insert(value op1, value index, value op3)
{
	long inx;
	
	if (op1.type == VALUE_STR && op3.type == VALUE_STR) {
		if (index.type == VALUE_MPZ) {
			if (value_lt(index, value_zero)) {
				value_error(1, "Domain Error: insert() is undefined where index is %s (>= 0 expected).", index);
				return value_init_error();
			} else if (value_gt(index, value_int_max)) {
				value_error(1, "Domain Error: insert() is undefined where index is %s (<= %s expected).", index, value_int_max);
				return value_init_error();
			} else inx = value_get_long(index);
		} else {
			value_error(1, "Type Error: insert() is undefined where op1 is a string and index is %ts (integer expected).", index);
			return value_init_error();
		}
		
		size_t length = strlen(op1.core.u_s);
		if (inx > length)
			inx = length;
		
		value res;
		res.type = VALUE_STR;
		value_malloc(&res, length + strlen(op3.core.u_s) + 1);
		return_if_error(res);
		strncpy(res.core.u_s, op1.core.u_s, inx);
		strcpy(res.core.u_s + inx, op3.core.u_s);
		strcpy(res.core.u_s + inx + strlen(op3.core.u_s), op1.core.u_s + inx);
		return res;
	} else if (op1.type == VALUE_ARY) {
		if (index.type == VALUE_MPZ) {
			if (value_lt(index, value_zero)) {
				value_error(1, "Domain Error: insert() is undefined where index is %s (>= 0 expected).", index);
				return value_init_error();
			} else if (value_gt(index, value_int_max)) {
				value_error(1, "Domain Error: insert() is undefined where index is %s (<= %s expected).", index, value_int_max);
				return value_init_error();
			} else inx = value_get_long(index);
		} else {
			value_error(1, "Type Error: insert() is undefined where op1 is an array and index is %ts (integer expected).", index);
			return value_init_error();
		}
		
		value res;
		res.type = VALUE_ARY;
		size_t length = value_length(op1);
		if (inx > length)
			inx = length;
		
		res.core.u_a.a = value_malloc(NULL, sizeof(value) * next_size(op1.core.u_a.length + 1));
		return_if_null(res.core.u_a.a);
		res.core.u_a.length = op1.core.u_a.length + 1;
		size_t i;
		for (i = 0; i < inx; ++i)
			res.core.u_a.a[i] = value_set(op1.core.u_a.a[i]);
		res.core.u_a.a[inx] = value_set(op3);
		for (i = inx; i < length; ++i)
			res.core.u_a.a[i+1] = value_set(op1.core.u_a.a[i]);
		
		return res;
	} else if (op1.type == VALUE_LST) {
		if (index.type == VALUE_MPZ) {
			if (value_lt(index, value_zero)) {
				value_error(1, "Domain Error: insert() is undefined where index is %s (>= 0 expected).", index);
				return value_init_error();
			} else if (value_gt(index, value_int_max)) {
				value_error(1, "Domain Error: insert() is undefined where index is %s (<= %s expected).", index, value_int_max);
				return value_init_error();
			} else inx = value_get_long(index);
		} else {
			value_error(1, "Type Error: insert() is undefined where op1 is a list and index is %ts (integer expected).", index);
			return value_init_error();
		}

		value res = value_init_nil();
		value optr = op1;
		value *rptr = &res;
		
		size_t i;
		for (i = 0; optr.type == VALUE_LST; ++i) {
			if (i == inx) {
				*rptr = value_init(VALUE_LST);
				rptr->core.u_l[0] = value_set(op3);
			} else {
				*rptr = value_init(VALUE_LST);
				rptr->core.u_l[0] = value_set(optr.core.u_l[0]);
				optr = optr.core.u_l[1];
			}
			
			rptr->core.u_l[1] = value_init_nil();
			rptr = &rptr->core.u_l[1];
		}
		
		if (i < inx) {
			value_error(1, "Domain Error: insert() is undefined where index is %s (<= %ld expected).", index, i);
			value_clear(&res);
			res = value_init_error();
		}
				
		return res;
		
	} else if (op1.type == VALUE_PAR) {
		if (index.type == VALUE_MPZ) {
			if (value_lt(index, value_zero)) {
				value_error(1, "Domain Error: insert() is undefined where index is %s (>= 0 expected).", index);
				return value_init_error();
			} else if (value_gt(index, value_int_max)) {
				value_error(1, "Domain Error: insert() is undefined where index is %s (<= %s expected).", index, value_int_max);
				return value_init_error();
			} else inx = value_get_long(index);
		} else {
			value_error(1, "Type Error: insert() is undefined where op1 is a list and index is %ts (integer expected).", index);
			return value_init_error();
		}

		value res = value_init_nil();
		value optr = op1;
		value *rptr = &res;
		
		size_t i;
		for (i = 0; optr.type == VALUE_PAR; ++i) {
			if (i == inx) {
				*rptr = value_init(VALUE_PAR);
				rptr->core.u_p->head = value_set(op3);
			} else {
				*rptr = value_init(VALUE_PAR);
				rptr->core.u_p->head = value_set(optr.core.u_p->head);
				optr = optr.core.u_p->tail;
			}
			
			rptr->core.u_p->tail = value_init_nil();
			rptr = &rptr->core.u_p->tail;
		}
		
		if (i < inx) {
			value_error(1, "Domain Error: insert() is undefined where index is %s (<= %ld expected).", index, i);
			value_clear(&res);
			res = value_init_error();
		}
				
		return res;
		
	} else if (op1.type == VALUE_HSH) {
		value res = value_set(op1);
		value_hash_put(&res, index, op3);
		return res;
	}
	
	if (op1.type != VALUE_STR && op1.type != VALUE_ARY)
		value_error(1, "Type Error: insert() is undefined where op1 is %ts (array or string expected).", op1);
	if (op1.type == VALUE_STR && op3.type != VALUE_STR)
		value_error(1, "Type Error: insert() is undefined where op1 is a string and op3 is %ts (string expected).", op3);
	return value_init_error();
}

value value_insert_now(value *op1, value index, value op3)
{
	value res = value_init_nil();
	long inx;
	
	if (op1->type == VALUE_ARY) {
		if (index.type == VALUE_MPZ) {
			if (value_lt(index, value_zero)) {
				value_error(1, "Type Error: insert!() is undefined where index is %s (>= 0 expected).", index);
				return value_init_error();
			} else if (value_gt(index, value_int_max)) {
				value_error(1, "Type Error: insert!() is undefined where index is %s (<= %s expected).", index, value_int_max);
				return value_init_error();
			} else inx = value_get_long(index);
		} else {
			value_error(1, "Type Error: insert!() is undefined where op1 is an array and index is %ts (integer expected).", index);
			return value_init_error();
		}
		
		if (resize_p(op1->core.u_a.length)) {
			value_realloc(op1, next_size(op1->core.u_a.length));
			return_if_error(*op1);
		}
				
		long i;
		for (i = op1->core.u_a.length; inx < i; )
			op1->core.u_a.a[i] = op1->core.u_a.a[--i];
		
		++op1->core.u_a.length;
		op1->core.u_a.a[inx] = value_set(op3);
	
	} else if (op1->type == VALUE_LST) {
		if (index.type == VALUE_MPZ) {
			if (value_lt(index, value_zero)) {
				value_error(1, "Domain Error: insert!() is undefined where index is %s (>= 0 expected).", index);
				return value_init_error();
			} else if (value_gt(index, value_int_max)) {
				value_error(1, "Domain Error: insert!() is undefined where index is %s (<= %s expected).", index, value_int_max);
				return value_init_error();
			} else inx = value_get_long(index);
		} else {
			value_error(1, "Type Error: insert!() is undefined where op1 is a list and index is %ts (integer expected).", index);
			return value_init_error();
		}

		value *optr = op1;
		
		size_t i;
		for (i = 0; optr->type == VALUE_LST; ++i) {
			if (i == inx) {
				value added = value_cons_now(op3, optr);
				optr = &added;
			}
			
			optr = &optr->core.u_l[1];
		}
		
		if (i < inx) {
			value_error(1, "Domain Error: insert!() is undefined where index is %s (<= %ld expected).", index, i);
			res = value_init_error();
		} else if (i == inx) {
			*optr = value_init(VALUE_LST);
			optr->core.u_l[0] = value_set(op3);
		}
		
	} else if (op1->type == VALUE_PAR) {
		if (index.type == VALUE_MPZ) {
			if (value_lt(index, value_zero)) {
				value_error(1, "Domain Error: insert!() is undefined where index is %s (>= 0 expected).", index);
				return value_init_error();
			} else if (value_gt(index, value_int_max)) {
				value_error(1, "Domain Error: insert!() is undefined where index is %s (<= %s expected).", index, value_int_max);
				return value_init_error();
			} else inx = value_get_long(index);
		} else {
			value_error(1, "Type Error: insert!() is undefined where op1 is a list and index is %ts (integer expected).", index);
			return value_init_error();
		}

		value *optr = op1;
		
		size_t i;
		for (i = 0; optr->type == VALUE_PAR; ++i) {
			if (i == inx) {
				value added = value_cons_now(op3, optr);
				optr = &added;
			}
			
			optr = &optr->core.u_p->tail;
		}
		
		if (i < inx) {
			value_error(1, "Domain Error: insert!() is undefined where index is %s (<= %ld expected).", index, i);
			res = value_init_error();
		} else if (i == inx) {
			*optr = value_init(VALUE_PAR);
			optr->core.u_p->head = value_set(op3);
		}
		
	} else if (op1->type == VALUE_HSH) {
		res = value_hash_put(op1, index, op3);
	} else {
		value tmp = value_insert(*op1, index, op3);
		return_if_error(tmp);
		value_clear(op1);
		*op1 = tmp;
	}

	
	return res;
}

int value_alpha_p(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Syntax Error: String operation alpha?() is undefined where op is %ts (string expected).", op);
		return VALUE_ERROR;
	}
	
	char *ptr = op.core.u_s - 1;
	while (*(++ptr) != '\0')
		if (!isalpha(*ptr))
			return FALSE;
	return TRUE;
}

value value_alpha_p_std(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Syntax Error: String operation alpha?() is undefined where op is %ts (string expected).", op);
		return value_init_error();
	}
	
	char *ptr = op.core.u_s - 1;
	while (*(++ptr) != '\0')
		if (!isalpha(*ptr))
			return value_set_bool(FALSE);
	return value_set_bool(TRUE);
}

int value_alnum_p(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: String operation alnum?() is undefined where op is not a string.");
		return VALUE_ERROR;
	}
	
	char *ptr = op.core.u_s - 1;
	while (*(++ptr) != '\0')
		if (!isalnum(*ptr))
			return FALSE;
	return TRUE;
}

value value_alnum_p_std(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: String operation alnum?() is undefined where op is not a string.");
		return value_init_error();
	}
	
	char *ptr = op.core.u_s - 1;
	while (*(++ptr) != '\0')
		if (!isalnum(*ptr))
			return value_set_bool(FALSE);
	return value_set_bool(TRUE);
}

int value_num_p(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: String operation num?() is undefined where op is %ts (string expected).", op);
		return VALUE_ERROR;
	}
	
	char *ptr = op.core.u_s - 1;
	while (*(++ptr) != '\0')
		if (!isdigit(*ptr))
			return FALSE;
	return TRUE;
}

value value_num_p_std(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: String operation num?() is undefined where op is %ts (string expected).", op);
		return value_init_error();
	}
	
	char *ptr = op.core.u_s - 1;
	while (*(++ptr) != '\0')
		if (!isdigit(*ptr))
			return value_set_bool(FALSE);
	return value_set_bool(TRUE);
}

size_t value_length(value op)
{
	if (op.type == VALUE_NIL)
		return 0;
	else if (op.type == VALUE_STR || op.type == VALUE_RGX)
		return strlen(op.core.u_s);
	else if (op.type == VALUE_ARY)
		return op.core.u_a.length;
	else if (op.type == VALUE_LST) {
		size_t length = 0;
		while (op.type == VALUE_LST) {
			++length;
			op = op.core.u_l[1];
		}
		return length;
	} else if (op.type == VALUE_PAR) {
		size_t length = 0;
		while (op.type == VALUE_PAR) {
			++length;
			op = op.core.u_p->tail;
		}
		return length;
	} else if (op.type == VALUE_BLK)
		return op.core.u_blk.length;
	else {
		value_error(1, "Type Error: length() is undefined where op is %ts (linear container expected).", op);
		return 0;
	}
}

value value_length_std(value op)
{
	if (op.type == VALUE_NIL)
		return value_set_long(0);
	else if (op.type == VALUE_STR || op.type == VALUE_RGX)
		return value_set_long(strlen(op.core.u_s));
	else if (op.type == VALUE_ARY)
		return value_set_long((long) op.core.u_a.length);
	else if (op.type == VALUE_LST || op.type == VALUE_PAR)
		return value_set_long(value_length(op));
	else if (op.type == VALUE_BLK)
		return value_set_long(op.core.u_blk.length);
	else {
		value_error(1, "Type Error: length() is undefined where op is %ts (linear container expected).", op);
		return value_init_error();
	}
}

value value_lstrip(value op)
{
	value res;
	if (op.type == VALUE_STR) {
		char *ptr = op.core.u_s;
		while (isspace(*ptr))
			++ptr;
		res.type = VALUE_STR;
		res.core.u_s = value_malloc(NULL, strlen(ptr));
		return_if_null(res.core.u_s);
		strcpy(res.core.u_s, ptr);
	} else {
		value_error(1, "Type Error: lstrip() is undefined where op is %ts (string expected).", op);
		res = value_init_error();
	}
	
	return res;
}

value value_range(value op, value start, value end)
{
	long istart, iend;
	int error_p = FALSE;
	
	if (start.type == VALUE_MPZ) {
		istart = value_get_long(start);
	} else {
		value_error(1, "Type Error: range() is undefined where start is %ts (integer expected).", start);
		error_p = TRUE;
	}
	
	if (end.type == VALUE_MPZ) {
		iend = value_get_long(end);
	} else {
		value_error(1, "Type Error: range() is undefined where end is %ts (integer expected).", end);
		error_p = TRUE;
	}
	
	if (op.type == VALUE_STR || op.type == VALUE_RGX) {
		if (error_p)
			return value_init_error();
		
		if (iend <= istart)
			return value_init_nil();
		
		size_t length = strlen(op.core.u_s);
		if (istart >= length)
			return value_init_nil();
		if (iend > length)
			iend = length;
		
		value res;
		res.type = VALUE_STR;
		value_malloc(&res, iend - istart + 1);
		return_if_error(res);
		strncpy(res.core.u_s, op.core.u_s+istart, iend - istart);
		res.core.u_s[iend - istart] = '\0';
		return res;
	}
	
	if (op.type == VALUE_ARY) {
		if (error_p)
			return value_init_error();

		if (iend <= istart)
			return value_init_nil();
		
		size_t length = value_length(op);

		if (istart >= length)
			return value_init_nil();
				
		if (iend > length)
			iend = length;
		
		value res;
		res.type = VALUE_ARY;
		res.core.u_a.a = value_malloc(NULL, sizeof(value) * (iend - istart + 1));
		return_if_null(res.core.u_a.a);

		size_t i;
		for (i = istart; i < iend; ++i)
			res.core.u_a.a[i-istart] = value_set(op.core.u_a.a[i]);
		res.core.u_a.length = iend - istart;
		
		return res;
	}
	
	value_error(1, "Type Error: Operation range() is undefined where op is %ts (string or array expected).", op);
	return value_init_error();
}

value value_replace(value op1, value op2, value op3)
{
	int error_p = FALSE;
	if (op1.type != VALUE_STR) {
		value_error(1, "Type Error: String operation replace() is undefined where op1 is %ts (string expected).", op1);
		error_p = TRUE;
	}
	
	if (op2.type != VALUE_STR && op2.type != VALUE_RGX) {
		value_error(1, "Type Error: String operation replace() is undefined where op2 is %ts (string or regex expected).", op2);
		error_p = TRUE;
	}
	
	if (op3.type != VALUE_STR) {
		value_error(1, "Type Error: String operation replace() is undefined where op3 is %ts (string expected).", op3);
		error_p = TRUE;
	}
	
	if (error_p)
		return value_init_error();
	
	value res = value_init_nil();
	
	if (op2.type == VALUE_STR) {
		size_t length = strlen(op1.core.u_s);
		size_t length2 = strlen(op2.core.u_s);
		size_t length3 = strlen(op3.core.u_s);
		
		// (buflen) is the longest that the resulting string can be. If length3 > length2, then 
		// the longest possible resulting string would be if op1 is entirely made up of N 
		// repetitions of op2, in which case the result would be N repetitions of op3.
		size_t buflen = length3 > length2 ? (length / length2 + 1) * length3 : length;
		char buffer[buflen];
		
		size_t bi = 0;
		size_t i, j;
		for (i = 0; i < length; ) {
			int replace_p = TRUE;
			for (j = 0; j < length2; ++j) {
				if (i+j >= length || op1.core.u_s[i+j] != op2.core.u_s[j]) {
					replace_p = FALSE;
					break;
				}
			}
			
			if (replace_p) {
				strcpy(buffer+bi, op3.core.u_s);
				bi += length3;
			} else {
				++j;
				strncpy(buffer+bi, op1.core.u_s+i, j);
				buffer[bi+j] = '\0';
				bi += strlen(buffer+bi);
			}
			
			i += j;
		}
		
		res = value_set_str(buffer);
	} else {
		size_t length = strlen(op1.core.u_s);
		size_t length3 = strlen(op3.core.u_s);
		
		// This is the largest that the result can possibly be, but it takes O(n^2) space. 
		// I need to find a more efficient way to do this.
		size_t buflen = length * length3;
		char buffer[buflen];
		
		size_t i, bi = 0;
		regmatch_t match;
		for (i = 0; i < length; ) {
			if ((match = value_match_str(op2, op1.core.u_s + i)).rm_so != -1) {
				// Copy the part of the string that came before the match.
				strncpy(buffer+bi, op1.core.u_s + i, match.rm_so);
				bi += match.rm_so;
				
				// Copy the match.
				strcpy(buffer+bi, op3.core.u_s);
				bi += length3;
				i += match.rm_eo;
			} else {
				buffer[bi++] = op1.core.u_s[i++];
			}
		}
		
		buffer[bi] = '\0';
		res = value_set_str(buffer);
	}

	
	return res;
}

value value_replace_now(value *op1, value op2, value op3)
{
	value res = value_replace(*op1, op2, op3);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op1);
	*op1 = res;
	return value_init_nil();
}

value value_reverse(value op)
{
	value res = value_init_nil();
	
	if (op.type == VALUE_NIL) {
		
	} else if (op.type == VALUE_STR) {
		res = value_set(op);
		size_t i, len = strlen(res.core.u_s), max = len >> 1;
		char temp;
		for (i = 0; i < max; ++i) {
			temp = res.core.u_s[i];
			res.core.u_s[i] = res.core.u_s[len-i-1];
			res.core.u_s[len-i-1] = temp;
		}
	} else if (op.type == VALUE_ARY) {
		res = value_set(op);
		value temp;
		size_t length = value_length(op);
		size_t length2 = length >> 1;
		int i;
		for (i = 0; i < length2; ++i) {
			temp = res.core.u_a.a[i];
			res.core.u_a.a[i] = res.core.u_a.a[length-i-1];
			res.core.u_a.a[length-i-1] = temp;
		}
	} else if (op.type == VALUE_LST) {
		res = value_init_nil();
		
		value ptr = op;
		while (ptr.type == VALUE_LST) {
			value_cons_now(ptr.core.u_l[0], &res);
			ptr = ptr.core.u_l[1];
		}		
		
	} else if (op.type == VALUE_PAR) {
		res = value_init_nil();
		
		value ptr = op;
		while (ptr.type == VALUE_PAR) {
			value_cons_now(ptr.core.u_p->head, &res);
			ptr = ptr.core.u_p->tail;
		}		
		
	} else {
		value_error(1, "Type Error: reverse() is undefined where op is %ts (linear container expected).", op);
		res = value_init_error();
	}

	return res;
}

value value_reverse_now(value *op)
{
	if (op->type == VALUE_NIL) {
		
	} else if (op->type == VALUE_STR) {
		size_t i, len = strlen(op->core.u_s), max = len >> 1;
		char temp;
		for (i = 0; i < max; ++i) {
			temp = op->core.u_s[i];
			op->core.u_s[i] = op->core.u_s[len-i-1];
			op->core.u_s[len-i-1] = temp;
		}
	} else if (op->type == VALUE_ARY) {
		size_t i, len = value_length(*op), max = len >> 1;
		value temp;
		for (i = 0; i < max; ++i) {
			temp = op->core.u_a.a[i];
			op->core.u_a.a[i] = op->core.u_a.a[len-i-1];
			op->core.u_a.a[len-i-1] = temp;
		}
	} else if (op->type == VALUE_LST || op->type == VALUE_PAR) {
		// I'm not entirely sure if the memory management is correct here. There 
		// may be some leakage.
		
		value res = value_reverse(*op);
		return_if_error(res);
		value_clear(op);
		*op = res;
		
	} else {
		value_error(1, "Type Error: reverse!() is undefined where op is %ts (linear container expected).", *op);
		return value_init_error();
	}

	
	return value_init_nil();
}

value value_rstrip(value op)
{
	value res;
	if (op.type == VALUE_STR) {
		char *end = op.core.u_s + strlen(op.core.u_s) - 1;
		while (end >= op.core.u_s && isspace(*end))
			--end;
		char saved = *(end+1);
		*(end+1) = '\0';
		res.type = VALUE_STR;
		value_malloc(&res, strlen(op.core.u_s));
		return_if_error(res);
		strcpy(res.core.u_s, op.core.u_s);
		*(end+1) = saved;
	} else {
		value_error(1, "Type Error: rstrip() is undefined where op is %ts (string expected).", op);
		res = value_init_error();
	}

	return res;
}

value value_scan(value op1, value op2)
{
	value res = value_init_nil();
	
	if (op1.type == VALUE_STR) {
				
		if (op2.type == VALUE_STR || op2.type == VALUE_RGX) {
			
			res = value_init(VALUE_ARY);
			
			regex_t compiled;
			regmatch_t match;
			
			compile_regex(&compiled, op2.core.u_x, 0);
			
			char *ptr = op1.core.u_s;
			while (*ptr) {
				match = value_match_str(op2, ptr);
				
				if (match.rm_so == -1) {
					++ptr;
				} else if (match.rm_so < -1) {
					value_clear(&res);
					return value_init_error();
				} else {
					char saved = ptr[match.rm_eo];
					ptr[match.rm_eo] = '\0';
					value vstr = value_set_str(ptr + match.rm_so);
					value_append_now2(&res, &vstr);
					ptr[match.rm_eo] = saved;
					ptr += match.rm_eo;
					if (match.rm_so == match.rm_eo)
						++ptr;
				}

			}
			
		} else {
			value_error(1, "Type Error: scan() is undefined where op2 is %ts (string or regular expression expected).", op2);
			res = value_init_error();
		}

		
	} else {
		value_error(1, "Type Error: scan() is undefined where op1 is %ts (string expected).", op1);
		if (op2.type != VALUE_STR && op2.type != VALUE_RGX)
			value_error(1, "Type Error: scan() is undefined where op2 is %ts (string or regular expression expected).", op2);
		res = value_init_error();
	}
	
	return res;
}

value value_split(value op1, value op2)
{
	int error_p = FALSE;
	if (op1.type != VALUE_STR) {
		value_error(1, "Type Error: split() is undefined where op1 is %ts (string expected).", op1);
		error_p = TRUE;
	}
		
	if (op2.type != VALUE_STR) {
		value_error(1, "Type Error: split() is undefined where op2 is %ts (string expected).", op2);
		error_p = TRUE;
	}
	
	if (error_p)
		return value_init_error();
	
	char *str = op2.core.u_s;
	size_t len = strlen(str);
	
	value res = value_init(VALUE_ARY);
	char *start = op1.core.u_s;
	char *ptr = start;
	
	if (len == 0) {
		while (*ptr) {
			value tmp = value_set_str_length(ptr, 1);
			value_append_now2(&res, &tmp);
			++ptr;
		}
		return res;
	}
	
	char saved;
	value temp;
	while (*ptr != '\0') {
		if (strncmp(ptr, str, len) == 0) {
			saved = *ptr;
			*ptr = '\0';
			if (strlen(start) > 0) {
				temp = value_set_str(start);
				value_append_now(&res, temp);
				value_clear(&temp);
			}
			*ptr = saved;
			ptr += len;
			start = ptr;
		} else ++ptr;
	}
	
	temp = value_set_str(start);
	value_append_now(&res, temp);
	value_clear(&temp);
	
	return res;
}

int value_starts_with_p(value op1, value op2)
{
	int error_p = FALSE;
	if (op1.type != VALUE_STR) {
		value_error(1, "Type Error: starts_with() is undefined where op1 is %ts (string expected).", op1);
		error_p = TRUE;
	}
		
	if (op2.type != VALUE_STR) {
		value_error(1, "Type Error: starts_with() is undefined where op2 is %ts (string expected).", op2);
		error_p = TRUE;
	}
	
	if (error_p)
		return VALUE_ERROR;
	
	return strncmp(op1.core.u_s, op2.core.u_s, value_length(op2)) == 0;
}

value value_starts_with_p_std(value op1, value op2)
{
	int error_p = FALSE;
	if (op1.type != VALUE_STR) {
		value_error(1, "Type Error: starts_with() is undefined where op1 is %ts (string expected).", op1);
		error_p = TRUE;
	}
		
	if (op2.type != VALUE_STR) {
		value_error(1, "Type Error: starts_with() is undefined where op2 is %ts (string expected).", op2);
		error_p = TRUE;
	}
	
	if (error_p)
		return value_init_error();
	
	return value_set_bool(strncmp(op1.core.u_s, op2.core.u_s, value_length(op2)) == 0);
}

value value_strip(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: strip() is undefined where op is %ts (string expected).", op);
		return value_init_error();
	}
	
	value res;
	char *ptr = op.core.u_s;
	while (isspace(*ptr))
		++ptr;
	char *end = ptr + strlen(ptr) - 1;
	while (end >= ptr && isspace(*end))
		--end;
	char saved = *(end+1);
	*(end+1) = '\0';
	res.type = VALUE_STR;
	value_malloc(&res, strlen(ptr));
	return_if_error(res);
	strcpy(res.core.u_s, ptr);
	*(end+1) = saved;
	return res;	
}

value value_strip_now(value *op)
{
	if (op->type != VALUE_STR) {
		value_error(1, "Type Error: strip() is undefined where op is %ts (string expected).", op);
		return value_init_error();
	}
	
	char *ptr = op->core.u_s;
	while (isspace(*ptr))
		++ptr;
	
	char *end = ptr + strlen(ptr) - 1;
	while (end > ptr && isspace(*end))
		--end;
	++end;
	
	memmove(op->core.u_s, ptr, end - ptr);
	*(end - (ptr - op->core.u_s)) = '\0';
	
	return value_init_nil();
}

value value_to_upper(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: to_upper() is undefined where op is %ts (string expected).", op);
		return value_init_error();
	}
	
	value res = value_set_str(op.core.u_s);
	char *ptr = res.core.u_s - 1;
	while (*(++ptr) = toupper(*ptr))
		;
	
	return res;
}

value value_to_lower(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: to_upper() is undefined where op is %ts (string expected).", op);
		return value_init_error();
	}
	
	value res = value_set_str(op.core.u_s);
	char *ptr = res.core.u_s - 1;
	while (*(++ptr) = tolower(*ptr))
		;
	
	return res;
}

value value_asc_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "asc()") ? value_init_error() : value_asc(argv[0]);
}

value value_capitalize_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "capitalize()") ? value_init_error() : value_capitalize(argv[0]);
}

value value_chop_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "chop()") ? value_init_error() : value_chop(argv[0]);
}

value value_chop_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "chop!()") ? value_init_error() : value_chop_now(&argv[0]);
}

value value_chr_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "chr()") ? value_init_error() : value_chr(argv[0]);
}

value value_contains_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "contains?()") ? value_init_error() : value_contains_p_std(argv[0], argv[1]);
}

value value_ends_with_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "ends_with?()") ? value_init_error() : value_ends_with_p_std(argv[0], argv[1]);
}

value value_index_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "index()") ? value_init_error() : value_index_std(argv[0], argv[1]);
}

value value_insert_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "insert()") ? value_init_error() : value_insert(argv[0], argv[1], argv[2]);
}

value value_insert_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "insert()") ? value_init_error() : value_insert_now(&argv[0], argv[1], argv[2]);
}

value value_alpha_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "is_alpha?()") ? value_init_error() : value_alpha_p_std(argv[0]);
}

value value_alnum_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "is_alnum?()") ? value_init_error() : value_alnum_p_std(argv[0]);
}

value value_num_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "is_num?()") ? value_init_error() : value_num_p_std(argv[0]);
}

value value_length_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "length()") ? value_init_error() : value_length_std(argv[0]);
}

value value_lstrip_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "lstrip()") ? value_init_error() : value_lstrip(argv[0]);
}

value value_range_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "range()") ? value_init_error() : value_range(argv[0], argv[1], argv[2]);
}

value value_replace_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "replace()") ? value_init_error() : value_replace(argv[0], argv[1], argv[2]);
}

value value_replace_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "replace!()") ? value_init_error() : value_replace_now(&argv[0], argv[1], argv[2]);
}

value value_reverse_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "reverse()") ? value_init_error() : value_reverse(argv[0]);
}

value value_reverse_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "reverse!()") ? value_init_error() : value_reverse_now(&argv[0]);
}

value value_rstrip_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "rstrip()") ? value_init_error() : value_rstrip(argv[0]);
}

value value_scan_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "scan()") ? value_init_error() : value_scan(argv[0], argv[1]);
}

value value_split_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "split()") ? value_init_error() : value_split(argv[0], argv[1]);
}

value value_starts_with_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "starts_with?()") ? value_init_error() : value_starts_with_p_std(argv[0], argv[1]);
}

value value_strip_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "strip()") ? value_init_error() : value_strip(argv[0]);
}

value value_strip_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "strip!()") ? value_init_error() : value_strip_now(&argv[0]);
}

value value_to_upper_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_upper()") ? value_init_error() : value_to_upper(argv[0]);
}

value value_to_lower_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_lower()") ? value_init_error() : value_to_lower(argv[0]);
}



