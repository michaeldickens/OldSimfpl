/*
 *  value_dataset.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 2/13/10.
 *
 */

#include "value.h"

/* 
 * Contains functions for manipulating arrays. Notice that all of 
 * these functions work on lists as well.
 */

/* 
 * Array Implementation
 * 
 * At any given time, the amount of memory allocated for the array is 
 * always a power of 2. This keeps insertion relatively efficient 
 * while also not using too much extra memory. Technically, it's 
 * always a power of 2 plus one, because of the extra element that 
 * represents the length. This implementation has not been tested 
 * very thoroughly, so there may be bugs.
 */

value block_array_cast(value op)
{
	if (op.type == VALUE_BLK) {
		value old_op = op;
		op.type = VALUE_ARY;
		op.core.u_a.length = old_op.core.u_blk.length;
		op.core.u_a.a = old_op.core.u_blk.a;
	}
	
	return op;
}

value value_set_ary(value array[], size_t length)
{
	if (length < 0)
		length = 0;
	value res;
	
	res.type = VALUE_ARY;
	value_malloc(&res, next_size(length));
	return_if_error(res);
	res.core.u_a.length = length;
	
	unsigned long i;
	for (i = 0; i < length; ++i)
		res.core.u_a.a[i] = value_set(array[i]);
	
	return res;
}

value value_set_ary_ref(value array[], size_t length)
{
	value res;
	
	res.type = VALUE_ARY;
	value_malloc(&res, next_size(length));
	return_if_error(res);
	res.core.u_a.length = length;
	
	memcpy(res.core.u_a.a, array, sizeof(value) * length);
	
	return res;
}

value value_set_ary_bool(int array[], size_t length)
{
	value res;
	
	res.type = VALUE_ARY;
	value_malloc(&res, next_size(length));
	return_if_error(res);
	res.core.u_a.length = length;
	
	unsigned long i;
	for (i = 0; i < length; ++i)
		res.core.u_a.a[i] = value_set_bool(array[i]);
	
	return res;
}

value value_set_ary_long(long array[], size_t length)
{
	value res;
	
	res.type = VALUE_ARY;
	value_malloc(&res, next_size(length));
	return_if_error(res);
	res.core.u_a.length = length;
	
	unsigned long i;
	for (i = 0; i < length; ++i)
		res.core.u_a.a[i] = value_set_long(array[i]);
	
	return res;
}

value value_set_ary_ulong(unsigned long array[], size_t length)
{
	value res;
	
	res.type = VALUE_ARY;
	value_malloc(&res, next_size(length));
	return_if_error(res);
	res.core.u_a.length = length;
	
	unsigned long i;
	for (i = 0; i < length; ++i)
		res.core.u_a.a[i] = value_set_ulong(array[i]);
	
	return res;
}

value value_set_ary_double(double array[], size_t length)
{
	value res;
	
	res.type = VALUE_ARY;
	value_malloc(&res, next_size(length));
	return_if_error(res);
	res.core.u_a.length = length;
	
	unsigned long i;
	for (i = 0; i < length; ++i)
		res.core.u_a.a[i] = value_set_double(array[i]);
	
	return res;
}

value value_set_ary_str(char *array[], size_t length)
{
	value res;
	
	res.type = VALUE_ARY;
	value_malloc(&res, next_size(length));
	return_if_error(res);
	res.core.u_a.length = length;
		
	unsigned long i;
	for (i = 0; i < length; ++i)
		res.core.u_a.a[i] = value_set_str(array[i]);
	
	return res;
}

value value_append(value op1, value op2)
{
	if (op1.type == VALUE_ARY) {
		size_t length = value_length(op1);
		
		value res;
		res.type = VALUE_ARY;
		value_malloc(&res, next_size(length+1) + 1);
		return_if_error(res);
		int i;
		for (i = 0; i < length; ++i)
			res.core.u_a.a[i] = value_set(op1.core.u_a.a[i]);
		res.core.u_a.length = op1.core.u_a.length + 1;
		res.core.u_a.a[length] = value_set(op2);
		
		return res;
	} else if (op1.type == VALUE_LST) {
		value res = value_init(VALUE_LST);
		value ptr = res;
		while (op1.type == VALUE_LST) {
			ptr.core.u_l[0] = value_set(op1.core.u_l[0]);
			op1 = op1.core.u_l[1];
			ptr = ptr.core.u_l[1] = value_init(VALUE_LST);
		}
		
		ptr.core.u_l[0] = value_set(op2);
		return res;
	} else if (op1.type == VALUE_PAR) {
		value res = value_init(VALUE_PAR);
		value ptr = res;
		while (op1.type == VALUE_PAR) {
			ptr.core.u_p->head = value_set(op1.core.u_p->head);
			op1 = op1.core.u_p->tail;
			ptr = ptr.core.u_p->tail = value_init(VALUE_PAR);
		}
		
		ptr.core.u_p->head = value_set(op2);
		return res;		
	}
	
	value_error(1, "Type Error: append() is undefined where op1 is %ts (array or list expected).", op1);
	return value_init_error();	
}

value value_append_now(value *op1, value op2)
{
	value set = value_set(op2);
	if (set.type == VALUE_ERROR)
		return set;
	return value_append_now2(op1, &set);
}

value value_append_now2(value *op1, value *op2)
{
	if (op1->type == VALUE_ARY) {
		size_t length = op1->core.u_a.length;
		if (resize_p(length)) {
			value_realloc(op1, next_size(length));
			return_if_error(*op1);
		}
		
		op1->core.u_a.a[length] = *op2;
		++op1->core.u_a.length;
		
	} else if (op1->type == VALUE_LST) {
		value ptr = *op1;
		while (!value_empty_p(ptr))
			ptr = ptr.core.u_l[1];
		ptr = value_init(VALUE_LST);
		ptr.core.u_l[0] = *op2;

	} else if (op1->type == VALUE_PAR) {
		value ptr = *op1;
		while (ptr.type == VALUE_PAR)
			ptr = ptr.core.u_p->tail;
		value_clear(&ptr);
		ptr = value_init(VALUE_PAR);
		ptr.core.u_l[0] = *op2;

	} else if (op1->type == VALUE_BLK) {
		size_t length = value_length(*op1);
		if (resize_p(length)) {
			value_realloc(op1, next_size(length));
			if (op1->type == VALUE_ERROR)
				return value_init_error();
		}
		
		op1->core.u_blk.a[length] = *op2;
		++op1->core.u_blk.length;
		
	} else {
		value_error(1, "Type Error: append() is undefined where op1 is %ts (array, list or block expected).", *op1);
		return value_init_error();
	}
	
	return value_init_nil();
}

value value_array_with_length(value op)
{
	value res = value_init_nil();
	
	if (op.type == VALUE_MPZ) {
		if (value_lt(op, value_zero)) {
			value_error(1, "Domain Error: array_with_length() is undefined where op is %s (greater than or equal to 0 expected).", op);
			return value_init_error();
		} else if (value_gt(op, value_int_max)) {
			value_error(1, "Domain error: array_with_length() is undefined where op is %s (less than %s expected).", op, value_int_max);
			return value_init_error();
		}
		
		res.type = VALUE_ARY;
		res.core.u_a.length = value_get_long(op);
		value_malloc(&res, next_size(res.core.u_a.length));
		return_if_error(res);
		memset(res.core.u_a.a, 0, sizeof(value) * res.core.u_a.length); // A value set to 0 will have type 0, which is VALUE_NIL.
		
	} else {
		value_error(1, "Type Error: array_with_length() is undefined where op is %ts (integer expected).", op);
		res = value_init_error();
	}
	
	return res;
}

value value_at(value op, value index, value more[], size_t length)
{
	value res = value_init_nil();
	
	if (index.type == VALUE_NIL)
		if (length)
			return value_at(op, more[0], more+1, length-1);
		else return value_set(op);
	
	if (op.type == VALUE_STR) {
		if (index.type == VALUE_MPZ) {
			value len = value_set_long(strlen(op.core.u_s));
			if (value_ge(index, len) || value_lt(index, value_zero)) {
				value_error(1, "Domain Error: in at(), index %s is beyond the bounds of string %s.", index, op);
				return value_init_error();
			}
			value_clear(&len);
			
			char str[2];
			str[0] = op.core.u_s[value_get_ulong(index)];
			str[1] = '\0';
			res = value_set_str(str);
			
		} else if (index.type == VALUE_RNG) {
			size_t i, start = (size_t) value_get_long(index.core.u_r->min);
			size_t finish = (size_t) value_get_long(index.core.u_r->max);
			
			char *str = value_malloc(NULL, finish - start + 1);
			return_if_null(str);
			if (index.core.u_r->inclusive_p)
				++finish;
			for (i = 0; start < finish; ++start, ++i)
				str[i] = op.core.u_s[start];
			str[i] = '\0';
			res.type = VALUE_STR;
			res.core.u_s = str;
		} else {
			value_error(1, "Type Error: at() is undefined where op is %ts and index is %ts (integer or range expected).", op, index);
			return value_init_error();
		}
				
	} else if (op.type == VALUE_ARY || op.type == VALUE_BLK) {
		value *ptr = op.type == VALUE_ARY ? op.core.u_a.a : op.core.u_blk.a;
		
		if (index.type == VALUE_MPZ) {
			long lindex = value_get_long(index);
			if (lindex >= value_length(op) || value_lt(index, value_zero) || value_gt(index, value_int_max)) {
				value_error(1, "Domain Error: in at(), index %s is beyond the bounds of array %s.", index, op);
				return value_init_error();
			}
			
			res = value_set(ptr[lindex]);
		
		} else if (index.type == VALUE_ARY) {
			size_t i, length = value_length(index);
			value ary[length];
			
			for (i = 0; i < length; ++i) {
				ary[i] = value_at(op, index.core.u_a.a[i], NULL, 0);
				if (ary[i].type == VALUE_ERROR)
					return ary[i];
			}
			
			res = value_set_ary_ref(ary, length);
		
		} else if (index.type == VALUE_RNG) {
			if (value_gt(index.core.u_r->min, index.core.u_r->max)) {
				value_error(1, "Domain Error: at() is undefined for range %ts where min is greater than max.", index);
				return value_init_error();
			}
			value diff = value_sub(index.core.u_r->max, index.core.u_r->min);
			size_t i, length = (size_t) value_get_long(diff), start = (size_t) value_get_long(index.core.u_r->min);
			value_clear(&diff);
			if (index.core.u_r->inclusive_p)
				++length;
			
			value ary[length];
			for (i = 0; i < length; ++i, ++start)
				ary[i] = value_set(ptr[start]);
			
			res = value_set_ary_ref(ary, length);
			
		} else {
			value_error(1, "Type Error: at() is undefined where op is %ts and index is %ts (integer, array, or range expected).", op, index);
			return value_init_error();
		}
		
		
	} else if (op.type == VALUE_LST) {
		if (index.type != VALUE_MPZ) {
			value_error(1, "Type Error: at() is undefined where op is %ts and index is %ts (integer expected).", op, index);
			return value_init_error();
		}
		
		if (value_gt(index, value_int_max) || value_lt(index, value_zero)) {
			value_error(1, "Domain Error: in at(), index %s is beyond the bounds of list %s.", index, op);
			return value_init_error();
		}
		
		long inx = value_get_long(index);
		
		value ptr = op;
		while (ptr.type != VALUE_NIL) {
			if (inx == 0) {
				res = value_set(ptr.core.u_l[0]);
				break;
			}
			ptr = ptr.core.u_l[1];
			--inx;
		}
		
		// res is nil because its value was never found, i.e. the given index was beyond the bounds of the list.
		if (res.type == VALUE_NIL) {
			value_error(1, "Domain Error: in at(), index %s is beyond the bounds of list %s.", index, op);
			return value_init_error();
		}
		
	} else if (op.type == VALUE_PAR) {
		if (index.type != VALUE_MPZ) {
			value_error(1, "Type Error: at() is undefined where op is a %ts and index is %ts (integer expected).", op, index);
			return value_init_error();
		}
		
		if (value_gt(index, value_int_max) || value_lt(index, value_zero)) {
			value_error(1, "Domain Error: in at(), index %s is beyond the bounds of list %s.", index, op);
			return value_init_error();
		}
		
		long inx = value_get_long(index);
		
		value ptr = op;
		while (ptr.type == VALUE_PAR) {
			if (inx == 0) {
				res = value_set(ptr.core.u_p->head);
				break;
			}
			ptr = ptr.core.u_p->tail;
			--inx;
		}
		
		// res is nil because its value was never found, i.e. the given index was beyond the bounds of the list.
		if (res.type == VALUE_NIL) {
			value_error(1, "Domain Error: in at(), index %s is beyond the bounds of list %s.", index, op);
			return value_init_error();
		}
		
	} else if (op.type == VALUE_HSH) {
		res = value_hash_get(op, index);
	
	} else {
		value_error(1, "Type Error: at() is undefined where op is %ts (string or container expected).", op);
		return value_init_error();
	}
	
	if (res.type == VALUE_ERROR)
		return res;
	
	if (length) {
		// (more) contains elements. Recursively call value_at() until there are no more elements.
		value real_res = value_at(res, more[0], more+1, length-1);
		value_clear(&res);
		return real_res;
	}
	
	return res;
}

value * value_at_ref(value op, value index, value more[], size_t length)
{
	if (index.type == VALUE_NIL)
		if (length)
			return value_at_ref(op, more[0], more+1, length-1);
		else return &op;
	
	if (op.type == VALUE_ARY) {
		if (index.type == VALUE_MPZ) {
			long lindex = value_get_long(index);
			if (lindex >= op.core.u_a.length || value_lt(index, value_zero) || value_gt(index, value_int_max)) {
				value_error(1, "Domain Error: in at_ref(), index %s is beyond the bounds of array %s.", index, op);
				return NULL;
			}
			
			if (length)
				return value_at_ref(op.core.u_a.a[lindex], more[0], more+1, length-1);
			else return &op.core.u_a.a[lindex];
					
		} else {
			value_error(1, "Type Error: at_ref() is undefined where op is %ts and index is %ts (integer expected).", op, index);
			return NULL;
		}
		
		
	} else if (op.type == VALUE_LST) {
		if (index.type != VALUE_MPZ) {
			value_error(1, "Type Error: at_ref() is undefined where op is %ts and index is %ts (integer expected).", op, index);
			return NULL;
		}
		
		if (value_gt(index, value_int_max) || value_lt(index, value_zero)) {
			value_error(1, "Domain Error: in at_ref(), index %s is beyond the bounds of list %s.", index, op);
			return NULL;
		}
		
		long inx = value_get_long(index);
		
		value ptr = op;
		while (ptr.type != VALUE_NIL) {
			if (inx == 0)
				if (length)
					return value_at_ref(ptr.core.u_l[0], more[0], more+1, length-1);
				else return &ptr.core.u_l[0];
			ptr = ptr.core.u_l[1];
			--inx;
		}
		
		value_error(1, "Domain Error: in at_ref(), index %s is beyond the bounds of list %s.", index, op);
		return NULL;
		
	} else if (op.type == VALUE_PAR) {
		if (index.type != VALUE_MPZ) {
			value_error(1, "Type Error: at_ref() is undefined where op is a list and index is %ts (integer expected).", index);
			return NULL;
		}
		
		if (value_gt(index, value_int_max) || value_lt(index, value_zero)) {
			value_error(1, "Domain Error: in at_ref(), index %s is beyond the bounds of list %s.", index, op);
			return NULL;
		}
		
		long inx = value_get_long(index);
		
		value ptr = op;
		while (ptr.type == VALUE_PAR) {
				if (length)
					return value_at_ref(ptr.core.u_p->head, more[0], more+1, length-1);
				else return &ptr.core.u_p->head;
			--inx;
		}
		
		value_error(1, "Domain Error: in at_ref(), the index %s is beyond the bounds of list %s.", index, op);
		return NULL;
		
	} else if (op.type == VALUE_HSH) {
		return value_hash_get_ref(op, index);
	} else {
		value_error(1, "Type Error: at_ref() is undefined where op is %ts (container expected).", op);
		return NULL;
	}
}

value value_at_assign_do(value *variables, value *op1, value index, value more[], size_t length, value func, value op2)
{
	value *modify;
	if (op1->type == VALUE_VAR) {
		value *data = value_hash_get_ref(*variables, *op1);
		if (data == NULL) {
			value_error(1, "Error: In at=(), undefined variable %s.", *op1);
			return value_init_error();
		}
		modify = value_at_ref(*data, index, more, length);
	} else {
		modify = value_at_ref(*op1, index, more, length);
	}

	if (modify && modify->type == VALUE_ERROR)
		return value_set(*modify);
	
	if (func.type != VALUE_NIL) {
		// Do at=, at+=, etc.
		
		if (modify == NULL) {
			value_error(1, "Error: In atf=(), value at index %s not found.", index);
			return value_init_error();
		}
		value ary[] = { *modify, op2 };
		value temp = value_call(variables, func, 2, ary);
		value_clear(modify);
		*modify = temp;
	} else {
		// Don't calculate a function, just find the value in op1. 
		
		if (modify == NULL) {
			modify = &op2;
			value_hash_put(op1, index, op2);
		} else {
			value_clear(modify);
			*modify = value_set(op2);
		}
	}
	
	if (modify->type == VALUE_ERROR) {
		return value_init_error();
	}

	return value_set(*modify);
}

value value_concat(value op1, value op2)
{
	value set1 = value_set(op1);
	value set2 = value_set(op2);
	value_concat_now2(&set1, &set2);
	return set1;
}

value value_concat_now(value *op1, value op2)
{
	value set = value_set(op2);
	if (set.type == VALUE_ERROR)
		return set;
	return value_concat_now2(op1, &set);
}

value value_concat_now2(value *op1, value *op2)
{
	value res = value_init_nil();
	
	if (op1->type == VALUE_NIL && (op2->type == VALUE_LST || op2->type == VALUE_PAR)) {
		*op1 = *op2;
		
	} else if (op1->type == VALUE_ARY) {

		if (op2->type == VALUE_ARY) {
			size_t len1 = value_length(*op1), 
				   len2 = value_length(*op2), 
				   length = len1 + len2;
			
			if (len2 == 0)
				return res;
			if (len1 == 0) {
				*op1 = *op2;
				return res;
			}
			
			value_realloc(op1, next_size(length));
			if (op1->type == VALUE_ERROR)
				return value_init_error();
				
			size_t i;
			for (i = 0; i < len2; ++i)
				op1->core.u_a.a[i+len1] = op2->core.u_a.a[i];
			
			op1->core.u_a.length = length;
		} else {
			size_t length = value_length(*op1);
			if (resize_p(length+1)) {
				value_realloc(op1, next_size(length+1));
				if (op1->type == VALUE_ERROR)
					return value_init_error();
			}
			op1->core.u_a.a[length] = *op2;
			++op1->core.u_a.length;
		}

		
	} else if (op1->type == VALUE_LST) {
		if (op2->type == VALUE_NIL || op2->type == VALUE_LST) {
			value ptr = *op1;
			
			while (ptr.core.u_l[1].type == VALUE_LST)
				ptr = ptr.core.u_l[1];
			ptr.core.u_l[1] = *op2;
		} else {
			value_cons_now2(op1, op2);
			*op1 = *op2;
		}

	} else if (op1->type == VALUE_PAR) {
		value *ptr = op1;
		
		while (ptr->type == VALUE_PAR)
			ptr = &ptr->core.u_p->tail;
		*ptr = *op2;
	
	} else {
		value_error(1, "Type Error: concat() is undefined where op1 is %ts (array or list expected).", *op1);
		res = value_init_error();
	}	

	return res;
}

value value_delete(value op1, value op2)
{
	if (op1.type == VALUE_ARY) {
		size_t length = value_length(op1);
		value arr[length];
		
		size_t i, j;
		int are_none_deleted = TRUE;
		for (i = 0, j = 0; i < length; ++i)
			if (are_none_deleted && value_eq(op1.core.u_a.a[i], op2))
				are_none_deleted = FALSE;
			else
				arr[j++] = value_set(op1.core.u_a.a[i]);
		
		return value_set_ary_ref(arr, j);
	} else if (op1.type == VALUE_LST) {
		value ptr = op1;
		value res = value_init(VALUE_LST);
		value resptr = res;
		value *clrptr = &res;
		int are_none_deleted = TRUE;
		while (ptr.type == VALUE_LST) {
			if (are_none_deleted && value_eq(ptr.core.u_l[0], op2))
				are_none_deleted = FALSE;
			else {
				resptr.core.u_l[0] = value_set(ptr.core.u_l[0]);
				resptr.core.u_l[1] = value_init(VALUE_LST);
				clrptr = &resptr.core.u_l[1];
				resptr = resptr.core.u_l[1];
			}
			ptr = ptr.core.u_l[1];
		}
		
		value_clear(clrptr);
				
		return res;
	} else {
		value_error(1, "Type Error: delete() is undefined where op1 is %ts (array or list expected).", op1);
		return value_init_error();
	}
}

value value_delete_now(value *op1, value op2)
{
	if (op1->type == VALUE_ARY) {
		size_t length = value_length(*op1);
		size_t i;
		for (i = 0; i < length; ++i)
			if (value_eq(op1->core.u_a.a[i], op2)) {
				value vi = value_set_long(i-1);
				value clr = value_delete_at_now(op1, vi);
				value_clear(&vi);
				value_clear(&clr);
				return value_set_bool(TRUE);
			}

		return value_set_bool(FALSE);
	} else if (op1->type == VALUE_LST) {
		value *ptr = op1;
		while (!value_empty_p(*ptr)) {
			if (value_eq(ptr->core.u_l[0], op2)) {
				value_clear(&ptr->core.u_l[0]);
				*ptr = ptr->core.u_l[1];
				break;
			}
			
			ptr = &ptr->core.u_l[1];
		}
		
		return value_set_bool(FALSE);
	}
	
	value_error(1, "Type Error: delete() is undefined where op1 is %ts (array or list expected).", *op1);
	return value_init_error();
}

value value_delete_all(value op1, value op2)
{
	if (op1.type == VALUE_ARY) {
		size_t length = value_length(op1);
		value arr[length];
		
		size_t i, j;
		for (i = 0, j = 0; i < length; ++i)
			if (value_eq(op1.core.u_a.a[i], op2))
				;
			else
				// Because this is a direct assignment, arr does not need 
				// to be cleared later on.
				arr[j++] = op1.core.u_a.a[i];
		
		return value_set_ary(arr, j);
	} else if (op1.type == VALUE_LST) {
		value ptr = op1;
		value res = value_init(VALUE_LST);
		value resptr = res;
		value *clrptr = &res;
		while (!value_empty_p(ptr)) {
			if (value_eq(ptr.core.u_l[0], op2))
				;
			else {
				resptr.core.u_l[0] = value_set(ptr.core.u_l[0]);
				resptr.core.u_l[1] = value_init(VALUE_LST);
				clrptr = &resptr.core.u_l[1];
				resptr = resptr.core.u_l[1];
			}
			ptr = ptr.core.u_l[1];
		}
		
		value_clear(clrptr);

		return res;
	} else {
		value_error(1, "Type Error: delete_all() is undefined where op1 is %ts (array or list expected).", op1);
		return value_init_error();
	}
}

/* 
 * Could use two different methods. First, it could delete each element individually; 
 * this is fast for only deleting a small number of elements, but is O(n^2) to delete 
 * every element. The second method is to move each element into a new array; this is 
 * linearly slower, but is still O(n). This is the method that is used.
 */
value value_delete_all_now(value *op1, value op2)
{
	if (op1->type == VALUE_ARY) {
		int count = 0;
		
		size_t length = value_length(*op1);
		
		size_t i, j;
		for (i = 0, j = 0; i < length; ++i) {
			value temp = value_set(op1->core.u_a.a[i]);
			if (temp.type == VALUE_ERROR)
				return temp;
			value_clear(&(op1->core.u_a.a[j]));
			op1->core.u_a.a[j] = value_set(temp);
			if (op1->core.u_a.a[j].type == VALUE_ERROR)
				return value_init_error();
			value_clear(&temp);
		
			if (value_ne(op1->core.u_a.a[i], op2))
				++j;
		}
		
		op1->core.u_a.length = j - 1;
		
		while (j < i)
			value_clear(&(op1->core.u_a.a[j++]));
		
	} else if (op1->type == VALUE_LST) {
		value *ptr = op1;
		while (!value_empty_p(*ptr)) {
			if (value_eq(ptr->core.u_l[0], op2)) {
				value_clear(&ptr->core.u_l[0]);
				*ptr = ptr->core.u_l[1];
			} else ptr = &ptr->core.u_l[1];
		}
			
	} else {
		value_error(1, "Type Error: delete_all() is undefined where op1 is %ts (array expected).", *op1);
		return value_init_error();
	}
	
	return value_init_nil();
}

value value_delete_at(value op, value index)
{
	if (op.type == VALUE_ARY) {
		if (index.type != VALUE_MPZ) {
			value_error(1, "Type Error: delete_at() is undefined where index is %ts (integer expected).", index);
			return value_init_error();
		}
		
		long lindex = value_get_long(index);
		if (lindex >= op.core.u_a.length || value_lt(index, value_zero) || value_ge(index, value_int_max)) {
			value_error(1, "Argument Error: in delete_at(), index %s is out of bounds of the array.", index);
			return value_init_error();
		}
				
		size_t length = value_length(op);
		value arr[length];
		
		size_t i;
		for (i = 0; i < lindex; ++i)
			arr[i] = value_set(op.core.u_a.a[i]);
		for (i = lindex+1; i < length; ++i)
			arr[i-1] = value_set(op.core.u_a.a[i]);
				
		return value_set_ary_ref(arr, length-1);
	} else {
		value res = value_set(op);
		value clr = value_delete_at_now(&res, index);
		value_clear(&clr);
		return res;
	}
}

value value_delete_at_now(value *op, value index)
{	
	if (op->type == VALUE_ARY) {
		if (index.type != VALUE_MPZ) {
			value_error(1, "Type Error: delete_at() is undefined where index is %ts (integer expected).", index);
			return value_init_error();
		}
		
		long lindex = value_get_long(index);
		
		if (lindex >= op->core.u_a.length || value_lt(index, value_zero) || value_gt(index, value_int_max)) {
			value_error(1, "Argument Error: in delete_at(), index %s is out of bounds of the array.", index);
			return value_init_error();
		}
		
		value res = op->core.u_a.a[lindex];
		
		size_t length = op->core.u_a.length - 1;
		
		// Remove (res) from the array by taking it out of the block of memory. This memmove() call might be 
		// a little obscure, but the speed makes it worth it, especially when you're deleting an element from 
		// near the beginning of a very long array.
		memmove((void *) (op->core.u_a.a + lindex), (void *) (op->core.u_a.a + lindex + 1), sizeof(value) * (length - lindex + 1));
		
		if (resize_p(length))
			value_realloc(op, next_size(length));
		
		--op->core.u_a.length;
				
		return res;
	
	} else if (op->type == VALUE_HSH) {
		
		return value_hash_delete_at(op, index);
		
	} else {
		value_error(1, "Type Error: delete_at() is undefined where op is %ts (array or hash expected).", *op);
	}
	
	return value_init_error();
}

value value_each(value *variables, value op, value func)
{
	value res = value_init_nil();
	if (op.type == VALUE_ARY) {
		size_t i;
		for (i = 0; i < op.core.u_a.length; ++i) {
			value tmp = value_call(variables, func, 1, op.core.u_a.a + i);
			if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_BREAK) {
				break;
			} else if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_YIELD) {
				if (res.type == VALUE_NIL) res = value_init(VALUE_ARY);
				value_append_now(&res, *tmp.core.u_stop.core);
			} else if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				res = tmp;
				break;
			}
			value_clear(&tmp);
		}
		
	} else if (op.type == VALUE_LST) {	
		value optr = op;
		while (optr.type == VALUE_LST) {
			value tmp = value_call(variables, func, 1, optr.core.u_l + 0);
			if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_BREAK) {
				break;
			} else if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_YIELD) {
				if (res.type == VALUE_NIL) res = value_init(VALUE_ARY);
				value_append_now(&res, *tmp.core.u_stop.core);
			} else if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				res = tmp;
				break;
			}
			value_clear(&tmp);
			
			optr = optr.core.u_l[1];
		}
	
	} else if (op.type == VALUE_HSH) {
		size_t i, j;
		for (i = 0; i < op.core.u_h.length; ++i) {
			if (op.core.u_h.a[i].type == VALUE_ARY) {
				value bucket = op.core.u_h.a[i];
				for (j = 0; j < bucket.core.u_a.length; ++j) {
					if (bucket.core.u_a.a[j].type != VALUE_ARY || bucket.core.u_a.a[j].core.u_a.length != 2)
						continue;
					value tmp = value_call(variables, func, 2, bucket.core.u_a.a[j].core.u_a.a + 0);
					if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_BREAK) {
						break;
					} else if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_YIELD) {
						if (res.type == VALUE_NIL) res = value_init(VALUE_ARY);
						value_append_now(&res, *tmp.core.u_stop.core);
					} else if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
						value_clear(&res);
						res = tmp;
						break;
					}
					value_clear(&tmp);
				}
			}
		}
		
	} else if (op.type == VALUE_RNG) {
		if (value_eq(op.core.u_r->min, op.core.u_r->max))
			return res;
		value min = value_set(op.core.u_r->min);
		value max = value_set(op.core.u_r->max);
		int reversed_p = value_gt(min, max);
		if (op.core.u_r->inclusive_p)
			if (reversed_p)
				value_dec_now(&max);
			else value_inc_now(&max);
		
		for (; reversed_p ? value_gt(min, max) : value_lt(min, max); reversed_p ? value_dec_now(&min) : value_inc_now(&min)) {
			value tmp = value_call(variables, func, 1, &min);
			
			if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_BREAK) {
				break;
			} else if (tmp.type == VALUE_STOP && tmp.core.u_stop.type == STOP_YIELD) {
				if (res.type == VALUE_NIL) res = value_init(VALUE_ARY);
				value_append_now(&res, *tmp.core.u_stop.core);
			} else if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				res = tmp;
				break;
			}
			value_clear(&tmp);
		}
		
		value_clear(&min);
		value_clear(&max);
		
	} else {
		value_error(1, "Type Error: each() is undefined where op is %ts (iterable expected).", op);
		res = value_init_error();
	}
		
	return res;
}

value value_each_index(value *variables, value op, value func)
{
	value res = value_init_nil();
	if (op.type == VALUE_ARY || op.type == VALUE_LST) {
		value ei_length = value_length_std(op);
		if (ei_length.type == VALUE_ERROR) return ei_length;
		value range = value_range_until(value_zero, ei_length);
		value_clear(&ei_length);
		if (range.type == VALUE_ERROR) return range;
		
		res = value_each(variables, range, func);
		value_clear(&range);
		
	} else {
		value_error(1, "Type Error: each_index() is undefined where op is %ts (linear container expected).", op);
		res = value_init_error();
	}
	
	return res;
}

int value_empty_p(value op)
{
	if (op.type == VALUE_NIL) {
		return TRUE;
	} else if (op.type == VALUE_STR) {
		return *op.core.u_s == '\0';
	} else if (op.type == VALUE_ARY) {
		return value_length(op) == 0;
	} else if (op.type == VALUE_LST) {
		return FALSE;
	} else if (op.type == VALUE_HSH) {
		return value_hash_size(op) == 0;
	} else {
		value_error(1, "Type Error: empty?() is undefined where op is %ts (string, array, list or hash expected).", op);
		return VALUE_ERROR;
	}
}

value value_empty_p_std(value op)
{
	int b = value_empty_p(op);
	if (b == VALUE_ERROR)
		return value_init_error();
	return value_set_bool(b);
}

value value_filter(value *variables, value op, value func)
{
	value res = value_init_nil();
	if (op.type == VALUE_ARY) {
		res = value_init(VALUE_ARY);
		size_t i;
		for (i = 0; i < op.core.u_a.length; ++i) {
			value cond = value_call(variables, func, 1, op.core.u_a.a + i);
			if (cond.type == VALUE_ERROR || cond.type == VALUE_STOP && (cond.core.u_stop.type == STOP_RETURN || cond.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				return cond;
			}
			if (value_true_p(cond))
				value_append_now(&res, op.core.u_a.a[i]);
			value_clear(&cond);
		}
	
	} else if (op.type == VALUE_LST) {
		res = value_init(VALUE_LST);
		value rptr = res;
		value optr = op;
		
		while (TRUE) {
			value cond = value_call(variables, func, 1, optr.core.u_l + 0);
			if (cond.type == VALUE_ERROR || cond.type == VALUE_STOP && (cond.core.u_stop.type == STOP_RETURN || cond.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				return cond;
			}
			int make_new_p = FALSE;
			if (value_true_p(cond)) {
				rptr.core.u_l[0] = value_set(optr.core.u_l[0]);
				make_new_p = TRUE;
			}
			
			optr = optr.core.u_l[1];
			if (optr.type != VALUE_LST) break;
			if (make_new_p) rptr = rptr.core.u_l[1] = value_init(VALUE_LST);
		}
		
	} else if (op.type == VALUE_RNG) {
		value temp = value_cast(op, VALUE_ARY);
		res = value_filter(variables, temp, func);
		value_clear(&temp);
		
	} else {
		value_error(1, "Type Error: filter() is undefined where op is %ts (linear container expected).", op);
		res = value_init_error();
	}
	
	return res;
}

value value_filter_f(value op, int (*f)(value))
{
	if (op.type == VALUE_ARY) {
		size_t oplen = value_length(op);
		value res[oplen];
		size_t i, length = 0;
		for (i = 0; i < oplen; ++i)
			if ((*f)(op.core.u_a.a[i]))
				// These will both point to the same value, but that's okay 
				// because all the values are copied at the end of filter().
				res[length++] = op.core.u_a.a[i];
		
		return value_set_ary(res, length);
	} else if (op.type == VALUE_LST) {
		value res = value_init(VALUE_LST);
		value ptr = res;
		value temp;
		while (op.type != VALUE_NIL) {
			temp = value_set(op.core.u_l[0]);
			if ((*f)(temp)) {
				ptr = value_init(VALUE_LST);
				ptr.core.u_l[0] = value_set(op.core.u_l[0]);
				ptr = ptr.core.u_l[1] = value_init_nil();
			}
			value_clear(&temp);
			op = op.core.u_l[1];
		}
		
		return res;
		
	}
	
	value_error(1, "Type Error: filter() is undefined where op is %ts (array or list expected).", op);
	return value_init_error();
}

value value_find(value *variables, value op, value func)
{
	value res = value_init_nil();
	if (op.type == VALUE_ARY) {
		size_t i;
		for (i = 0; i < op.core.u_a.length; ++i) {
			value tmp = value_call(variables, func, 1, op.core.u_a.a + i);
			if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				res = tmp;
				break;
			} else if (value_true_p(tmp)) {
				value_clear(&tmp);
				return value_set(op.core.u_a.a[i]);
			}
			value_clear(&tmp);
		}
		
	} else if (op.type == VALUE_LST) {	
		value optr = op;
		while (optr.type == VALUE_LST) {
			value tmp = value_call(variables, func, 1, optr.core.u_l + 0);
			if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				res = tmp;
				break;
			} else if (value_true_p(tmp)) {
				value_clear(&tmp);
				return value_set(optr.core.u_l[0]);
			}
			value_clear(&tmp);
			
			optr = optr.core.u_l[1];
		}
	
	} else if (op.type == VALUE_HSH) {
		size_t i, j;
		for (i = 0; i < op.core.u_h.length; ++i) {
			if (op.core.u_h.a[i].type == VALUE_ARY) {
				value bucket = op.core.u_h.a[i];
				for (j = 0; j < bucket.core.u_a.length; ++j) {
					if (bucket.core.u_a.a[j].type != VALUE_ARY || bucket.core.u_a.a[j].core.u_a.length != 2)
						continue;
					value tmp = value_call(variables, func, 2, bucket.core.u_a.a[j].core.u_a.a + 0);
					if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
						value_clear(&res);
						res = tmp;
						break;
					} else if (value_true_p(tmp)) {
						value_clear(&tmp);
						return value_set(bucket.core.u_a.a[j]);
					}
					value_clear(&tmp);
				}
			}
		}
		
	} else if (op.type == VALUE_RNG) {
		if (value_eq(op.core.u_r->min, op.core.u_r->max))
			return res;
		value min = value_set(op.core.u_r->min);
		value max = value_set(op.core.u_r->max);
		int reversed_p = value_gt(min, max);
		if (op.core.u_r->inclusive_p)
			if (reversed_p)
				value_dec_now(&max);
			else value_inc_now(&max);
		
		for (; reversed_p ? value_gt(min, max) : value_lt(min, max); reversed_p ? value_dec_now(&min) : value_inc_now(&min)) {
			value tmp = value_call(variables, func, 1, &min);
			if (tmp.type == VALUE_ERROR || tmp.type == VALUE_STOP && (tmp.core.u_stop.type == STOP_RETURN || tmp.core.u_stop.type == STOP_EXIT)) {
				value_clear(&res);
				res = tmp;
				break;
			} else if (value_true_p(tmp)) {
				value_clear(&tmp);
				value_clear(&max);
				return min;
			}
			value_clear(&tmp);
		}
		
		value_clear(&min);
		value_clear(&max);
		
	} else {
		value_error(1, "Type Error: filter() is undefined where op is %ts (iterable expected).", op);
		res = value_init_error();
	}
		
	return res;
}

value value_flatten(value op)
{
	if (op.type == VALUE_ARY) {
		value res = value_set(op);
		size_t total_length = value_private_total_length(res);
		value array[total_length];
		value_private_flatten_recursive(array, res, 0);
		value_clear(&res);
		res = value_set_ary(array, total_length);
		size_t i;
		for (i = 0; i < total_length; ++i)
			value_clear(&array[i]);
		return res;
	} else if (op.type == VALUE_LST) {
		if (value_empty_p(op))
			return op;
		if (op.core.u_l[0].type == VALUE_LST) {
			value res = value_flatten(op.core.u_l[0]);
			value tmp = value_flatten(op.core.u_l[1]);
			value_concat_now(&res, tmp);
			value_clear(&tmp);
			return res;
		} else {
			value res = value_flatten(op.core.u_l[1]);
			value_cons_now(op.core.u_l[0], &res);
			return res;
		}
	}
	
	// This is used for a list's recursive definition. 
	return value_set(op);
}

value value_flatten_now(value *op)
{
	value res = value_flatten(*op);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op);
	*op = res;
	return value_init_nil();
}

/* WARNING: This function will MODIFY the contents of (op). If you do not want 
 * its contents to be modified, be sure to make a copy.
 */
size_t value_private_flatten_recursive(value array[], value op, size_t index)
{
	if (op.type == VALUE_ARY) {
		size_t length = value_length(op);
		size_t i;
		for (i = 0; i < length; ++i)
			index = value_private_flatten_recursive(array, op.core.u_a.a[i], index);
		
	} else {
		array[index++] = value_set(op);
	}
	
	return index;
}

size_t value_private_total_length(value op)
{
	if (op.type == VALUE_ARY) {
		size_t tlen = value_length(op);
		size_t i, length = 0;
		for (i = 0; i < tlen; ++i) {
			if (op.core.u_a.a[i].type == VALUE_ARY)
				length += value_private_total_length(op.core.u_a.a[i]);
			else ++length;
		}
		return length;
	} else return 1;
}

/* 
 * There should be a way to have no initial value and just use the first 
 * element of op as the initial value.
 */
value value_fold(value *variables, value op, value initial, value func)
{
	value ary[] = { initial, value_init_nil() };
	
	if (op.type == VALUE_ARY) {
		size_t i;
		for (i = 0; i < op.core.u_a.length; ++i) {
			ary[1] = op.core.u_a.a[i];
			ary[0] = value_call(variables, func, 2, ary);
			if (ary[0].type == VALUE_ERROR || ary[0].type == VALUE_STOP && (ary[0].core.u_stop.type == STOP_RETURN || ary[0].core.u_stop.type == STOP_EXIT))
				break;
		}
		
	} else if (op.type == VALUE_LST) {	
		value optr = op;
		while (optr.type == VALUE_LST) {
			ary[1] = optr.core.u_l[0];
			ary[0] = value_call(variables, func, 2, ary);			
			if (ary[0].type == VALUE_ERROR || ary[0].type == VALUE_STOP && (ary[0].core.u_stop.type == STOP_RETURN || ary[0].core.u_stop.type == STOP_EXIT))
				break;
			optr = optr.core.u_l[1];
		}
	
	} else if (op.type == VALUE_HSH) {
		size_t i, j;
		for (i = 0; i < op.core.u_h.length; ++i) {
			if (op.core.u_h.a[i].type == VALUE_ARY) {
				value bucket = op.core.u_h.a[i];
				for (j = 0; j < bucket.core.u_a.length; ++j) {
					if (bucket.core.u_a.a[j].type != VALUE_ARY || bucket.core.u_a.a[j].core.u_a.length != 2)
						continue;
					ary[1] = bucket.core.u_a.a[j].core.u_a.a[0];
					ary[0] = value_call(variables, func, 2, ary);
					if (ary[0].type == VALUE_ERROR || ary[0].type == VALUE_STOP && (ary[0].core.u_stop.type == STOP_RETURN || ary[0].core.u_stop.type == STOP_EXIT))
						break;
				}
			}
			if (ary[0].type == VALUE_ERROR || ary[0].type == VALUE_STOP && (ary[0].core.u_stop.type == STOP_RETURN || ary[0].core.u_stop.type == STOP_EXIT))
				break;
		}
				
	} else if (op.type == VALUE_RNG) {
		if (value_eq(op.core.u_r->min, op.core.u_r->max))
			return ary[0];
		ary[1] = value_set(op.core.u_r->min);
		value max = value_set(op.core.u_r->max);
		int reversed_p = value_gt(ary[1], max);
		if (op.core.u_r->inclusive_p)
			if (reversed_p)
				value_dec_now(&max);
			else value_inc_now(&max);
		
		for (; reversed_p ? value_gt(ary[1], max) : value_lt(ary[1], max); reversed_p ? value_dec_now(&ary[1]) : value_inc_now(&ary[1])) {
			ary[0] = value_call(variables, func, 2, ary);		
			if (ary[0].type == VALUE_ERROR || ary[0].type == VALUE_STOP && (ary[0].core.u_stop.type == STOP_RETURN || ary[0].core.u_stop.type == STOP_EXIT))
				break;
		}
		
		value_clear(&ary[1]);
		value_clear(&max);
		
	} else {
		value_error(1, "Type Error: fold() is undefined where op is %ts (iterable expected).", op);
		ary[0] = value_init_error();
	}
		
	return ary[0];
}

value value_join(value op1, value op2)
{
	if (op1.type == VALUE_ARY) {
		if (op2.type != VALUE_STR) {
			value_error(1, "Type Error: join() is undefined where op2 is %ts (string expected).", op2);
			return value_init_error();
		}
		
		size_t length = value_length(op1);
		if (length == 0)
			return value_set_str("");
		
		value res = value_cast(op1.core.u_a.a[0], VALUE_STR);
		size_t i;
		for (i = 1; i < length; ++i) {
			value_add_now(&res, op2);
			value cast = value_cast(op1.core.u_a.a[i], VALUE_STR);
			value_add_now(&res, cast);
			value_clear(&cast);
		}
		
		return res;
	} else if (op1.type == VALUE_LST) {
		if (op2.type != VALUE_STR) {
			value_error(1, "Type Error: join() is undefined where op2 is %ts (string expected).", op2);
			return value_init_error();
		}
		
		if (value_empty_p(op1))
			return value_set_str("");
		
		value res = value_cast(op1.core.u_l[0], VALUE_STR);
		op1 = op1.core.u_l[1];
		while (!value_empty_p(op1)) {
			value_add_now(&res, op2);
			value cast = value_cast(op1.core.u_l[0], VALUE_STR);
			value_add_now(&res, cast);
			value_clear(&cast);
			op1 = op1.core.u_l[1];
		}
		
		return res;
	} else if (op1.type == VALUE_RNG) {
		value ary = value_cast(op1, VALUE_ARY);
		value res = value_join(ary, op2);
		value_clear(&ary);
		return res;
	}
	
	value_error(1, "Type Error: join() is undefined where op1 is %ts (iterable expected).", op1);
	
	if (op2.type != VALUE_STR)
		value_error(1, "Type Error: join() is undefined where op2 is %ts (string expected).", op2);
	
	return value_init_error();
}

value value_last(value op)
{
	if (op.type == VALUE_NIL) {
		return value_init_nil();
	} else if (op.type == VALUE_ARY) {
		size_t length = value_length(op);
		
		if (length > 0)
			return value_set(op.core.u_a.a[length-1]);
		else return value_init_nil();
	} else if (op.type == VALUE_LST) {
		value ptr = op;
		while (ptr.core.u_l[1].type == VALUE_LST)
			ptr = ptr.core.u_l[1];
		return value_set(ptr.core.u_l[0]);
	} else {
		value_error(1, "Type Error: last() is undefined where op is %ts (array or list expected).", op);
		return value_init_error();
	}

}

value value_map(value *variables, value op, value func)
{
	value res = value_init_nil();
	if (op.type == VALUE_ARY) {
		res.type = VALUE_ARY;
		value_malloc(&res, op.core.u_a.length);
		return_if_error(res);
		res.core.u_a.length = op.core.u_a.length;
		
		size_t i;
		for (i = 0; i < op.core.u_a.length; ++i) {
			res.core.u_a.a[i] = value_call(variables, func, 1, op.core.u_a.a + i);
			if (res.core.u_a.a[i].type == VALUE_STOP && res.core.u_a.a[0].core.u_stop.type == STOP_BREAK) {
				value_clear(&res.core.u_a.a[i]);
				res.core.u_a.length = i;
				value_realloc(&res, next_size(res.core.u_a.length));
				return_if_error(res);
				break;
			} else if (res.core.u_a.a[i].type == VALUE_ERROR || res.core.u_a.a[i].type == VALUE_STOP && 
					(res.core.u_a.a[i].core.u_stop.type == STOP_RETURN || res.core.u_a.a[i].core.u_stop.type == STOP_EXIT))
				return res.core.u_a.a[i];
		}
				
	} else if (op.type == VALUE_LST) {
		res = value_init(VALUE_LST);
		value optr = op;
		value rptr = res;
		while (TRUE) {
			rptr.core.u_l[0] = value_call(variables, func, 1, optr.core.u_l + 0);
			if (rptr.core.u_l[0].type == VALUE_STOP && rptr.core.u_l[0].core.u_stop.type == STOP_BREAK) {
				value_clear(&rptr);
				break;
			} else if (rptr.core.u_l[0].type == VALUE_ERROR || rptr.core.u_l[0].type == VALUE_STOP && 
				(rptr.core.u_l[0].core.u_stop.type == STOP_RETURN || rptr.core.u_l[0].core.u_stop.type == STOP_EXIT))
				return rptr.core.u_l[0];
			optr = optr.core.u_l[1];
			if (optr.type != VALUE_LST) break;
			rptr = rptr.core.u_l[1] = value_init(VALUE_LST);
		}
	
	} else if (op.type == VALUE_HSH) {
		res = value_init(VALUE_HSH);
		
		size_t i, j;
		for (i = 0; i < op.core.u_h.length; ++i) {
			if (op.core.u_h.a[i].type == VALUE_ARY) {
				value bucket = op.core.u_h.a[i];
				for (j = 0; j < bucket.core.u_a.length; ++j) {
					if (bucket.core.u_a.a[j].type != VALUE_ARY || bucket.core.u_a.a[j].core.u_a.length != 2)
						continue;
					value pair = value_call(variables, func, 2, bucket.core.u_a.a[j].core.u_a.a + 0);
					if (pair.type == VALUE_STOP && pair.core.u_stop.type == STOP_BREAK) {
						value_clear(&pair);
						break;
					} else if (pair.type == VALUE_ERROR) {
						value_clear(&res);
						return pair;
					}
					if (pair.type != VALUE_ARY || pair.core.u_a.length != 2) {
						value_error(1, "Error: For hashes, each iteration of map() must return a two-element array (%ts returned instead).", pair);
						value_clear(&res);
						return value_init_error();
					}
					
					value_hash_put(&res, pair.core.u_a.a[0], pair.core.u_a.a[1]);
					value_clear(&pair);
				}
			}
		}
		
	} else if (op.type == VALUE_RNG) {
		value ary = value_cast(op, VALUE_ARY);
		if (ary.type == VALUE_ERROR) return ary;
		res = value_map(variables, ary, func);
		value_clear(&ary);
		
	} else {
		value_error(1, "Type Error: map() is undefined where op is %ts (iterable expected).", op);
		res = value_init_error();
	}
	
	return res;
}

/* 
 * This is a slow implementation, because it requires copying the container. It 
 * would be better to perform a map in place.
 */
value value_map_now(value *variables, value *op, value func)
{
	value res = value_map(variables, *op, func);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op);
	*op = res;
	return value_init_nil();
}

value value_map_drop(value *variables, value op, value block, value drop)
{	
	return value_init_error();
}

value value_pop(value op)
{
	value res = value_init_nil();
	
	if (op.type == VALUE_NIL) {
		return value_init_nil();
		
	} else if (op.type == VALUE_ARY) {
	
		size_t length = op.core.u_a.length;
		
		if (length == 0)
			return value_init_nil();
		
		--length;
		
		value array[length];
		size_t i;
		for (i = 0; i < length; ++i)
			array[i] = op.core.u_a.a[i];
		
		res = value_set_ary(array, length);
		
	} else if (op.type == VALUE_LST) {
		if (op.core.u_l[1].type != VALUE_LST) {
			res = value_init_nil();
		} else {
			res = value_pop(op.core.u_l[1]);
			value_cons_now(op.core.u_l[0], &res);
		}

		
	} else {
		value_error(1, "Type Error: pop() is undefined where op is %ts (array expected).", op);
		res = value_init_error();
	}

	return res;
}

value value_pop_now(value *op)
{
	if (op->type != VALUE_ARY) {
		value_error(1, "Type Error: pop!() is undefined where op is %ts (array expected).", *op);
		value_clear(op);
		*op = value_init_error();
		return value_init_error();
	}
	
	size_t length = op->core.u_a.length;
	
	if (length == 0)
		return value_init_nil();
		
	value res = value_set(op->core.u_a.a[length-1]);
	
	value_clear(&(op->core.u_a.a[length-1]));
	--op->core.u_a.length;
	if (resize_p(length))
		value_realloc(op, next_size(length));
	
	return res;
}

value value_shuffle(value op)
{
	value res = value_set(op);
	value error = value_shuffle_now(&res);
	if (error.type == VALUE_ERROR)
		return error;
	return res;
}

value value_shuffle_now(value *op)
{
	if (op->type == VALUE_ARY) {
		// Use the Fischer-Yates shuffling algorithm
		value temp;
		size_t i, k;
		for (i = value_length(*op) - 1; i > 0; --i) {
			k = (size_t) (genrand_real2() * i);
			temp = op->core.u_a.a[k];
			op->core.u_a.a[k] = op->core.u_a.a[i];
			op->core.u_a.a[i] = temp;
		}
	} else if (op->type == VALUE_LST) {
		if (value_empty_p(*op) || value_empty_p(op->core.u_l[1])) {
			// do nothing, it cannot be shuffled
		} else {
			// Use something like a reverse quicksort
			value left = value_init_nil();
			value right = value_init_nil();
			value ptr = *op;
			while (!value_empty_p(ptr)) {
				if (genrand_real2() < 0.5)
					value_cons_now2(&ptr.core.u_l[0], &left);
				else
					value_cons_now2(&ptr.core.u_l[0], &right);
				ptr = ptr.core.u_l[1];
			}
			
			value_shuffle_now(&left);
			value_shuffle_now(&right);
			value_concat_now2(&left, &right);
			*op = left;
		}

	} else {
		value_error(1, "Type Error: shuffle() is undefined where op is %ts (array or list expected).", *op);
		return value_init_error();
	}
	
	return value_init_nil();
}

size_t value_size(value op)
{
	if (op.type == VALUE_ARY || op.type == VALUE_LST || op.type == VALUE_BLK)
		return value_length(op);
	else if (op.type == VALUE_HSH)
		return value_hash_size(op);
	else return 1;
}

value value_size_std(value op)
{
	return value_set_long((long) value_size(op));
}

value value_sort(value op)
{
	if (op.type == VALUE_ARY) {
		value res = value_set(op);
	
		if (value_private_sort_recursive(res.core.u_a.a, 0, value_length(res) - 1) == VALUE_ERROR) {
			value_error(1, "Error: sort() is undefined where the types of the elements of op do not match.");
			return value_init_error();
		}
		return res;
	} else if (op.type == VALUE_LST) {
		value res = value_set(op);
		value err = value_sort_now(&res);
		if (err.type == VALUE_ERROR) {
			value_clear(&res);
			return err;
		}
		return res;
	}
	
	value_error(1, "Type Error: sort() is undefined where op is %ts (array or list expected).", op);
	return value_init_error();	
}

value value_sort_now(value *op)
{
	if (op->type == VALUE_NIL) {
		;
	} else if (op->type == VALUE_ARY) {
		if (value_private_sort_recursive(op->core.u_a.a, 0, value_length(*op) - 1) == VALUE_ERROR) {
			value_error(1, "Error: sort() is undefined where the types of the elements of op do not match.");
			return value_init_error();
		}
	} else if (op->type == VALUE_LST) {
		// Uses merge sort to quickly sort a linked list. Better than quicksort because the 
		// only easy way to find a pivot is to take the head of the list, in which case a 
		// sorted or reverse-sorted list will take O(n^2).
		
		if (value_private_sort_list(op))
			return value_init_error();
		
	} else {
		value res = value_sort(*op);
		if (res.type == VALUE_ERROR)
			return res;
		value_clear(op);
		*op = res;
	}
	
	return value_init_nil();
}

int value_private_sort_lt(value op)
{
	return value_lt(op, value_private_sort_pivot);
}

int value_private_sort_eq(value op)
{
	return value_eq(op, value_private_sort_pivot);
}

int value_private_sort_gt(value op)
{
	return value_gt(op, value_private_sort_pivot);
}

int value_private_sort_recursive(value array[], int left, int right)
{
	if (right - left > 12) {
		int i = left;
		int j = right;
		int cmp;
		
		// The pivot's index is selected randomly between the two spots 
		// closest to the center, to decrease the probability of O(n^2) 
		// runtime.
		value pivot = array[(left+right)/2 + (genrand_int31() & 1)];
		
		do {
			while ((cmp = value_cmp_any(array[i], pivot)) == -1) ++i;
			if (cmp == -2) return VALUE_ERROR;
			while ((cmp = value_cmp_any(array[j], pivot)) > 0) --j;
			if (cmp == -2) return VALUE_ERROR;
			if (i <= j) {
				value temp = array[i];
				array[i] = array[j];
				array[j] = temp;
				++i; --j;
			}
			
		} while (i <= j);
		
		if (left < j) 
			if (value_private_sort_recursive(array, left, j) == VALUE_ERROR)
				return VALUE_ERROR;
		if (i < right) 
			if (value_private_sort_recursive(array, i, right) == VALUE_ERROR)
				return VALUE_ERROR;

	} else {
		int i, j, cmp;
		for (i = left; i <= right; ++i) {
			value temp = array[i];
			j = i - 1;
			while (j >= 0 && (cmp = value_cmp_any(array[j], temp)) > 0) {
				array[j+1] = array[j--];
			}
			if (cmp == -2) return VALUE_ERROR;
			
			array[j+1] = temp;
		}
	}
	
	return 0;
}

/* 
 * Implements merge sort for linked lists. Because it is intended to be able to 
 * deal with very long lists, the code is highly optimized. Function calls are 
 * minimized, and it is not checked whether (op) is a list. If (op) is not nil 
 * or a list, this will crash the program.
 * 
 * Crashes if the list has more than about 2000 elements.
 */
int value_private_sort_list(value *op)
{
	if (op->type == VALUE_NIL || op->core.u_l[1].type == VALUE_NIL) {
		return 0;
	}
	
	value left = value_nil;
	value right = value_nil;
	value ptr = *op;
	value temp;
	size_t i;
	
	// Partition (op) into left and right halves.
	for (i = 0; ptr.type == VALUE_LST; ++i) {
		if (i & 1) {
			temp = left;
			left.type = VALUE_LST;
			left.core.u_l = value_malloc(NULL, sizeof(value) * 2);
			if (left.core.u_l == NULL) return 1;
			left.core.u_l[0] = ptr.core.u_l[0];
			left.core.u_l[1] = temp;
		} else {
			temp = right;
			right.type = VALUE_LST;
			right.core.u_l = value_malloc(NULL, sizeof(value) * 2);
			if (right.core.u_l == NULL) return 1;
			right.core.u_l[0] = ptr.core.u_l[0];
			right.core.u_l[1] = temp;			
		}
		
		ptr = ptr.core.u_l[1];
	}
	
	int code = value_private_sort_list(&left);

#define clear_allocated_bits(list) ptr = (list); \
		while (ptr.type == VALUE_LST) { \
			value temp = ptr.core.u_l[1]; \
			value_free(ptr.core.u_l); \
			ptr = temp; \
		}
	
	// If (res) has an error code, clear the allocated bits in (left) and (right) 
	// but don't clear the contents, because those were copied directly from (op).
	if (code) {
		clear_allocated_bits(left);
		clear_allocated_bits(right);
		return code;
	}
	
	code = value_private_sort_list(&right);
	if (code) {
		clear_allocated_bits(left);
		clear_allocated_bits(right);
		return code;
	}
	
	// Merge the left and right halves.
	value lptr = left;
	value rptr = right;
	ptr = *op;
	
	// The contents of (left) and (right) are put directly back into (op) instead of 
	// creating a new list.
	while (TRUE) {
		if (value_cmp_any(lptr.core.u_l[0], rptr.core.u_l[0]) <= 0) {
			ptr.core.u_l[0] = lptr.core.u_l[0];
			temp = lptr;
			lptr = lptr.core.u_l[1];
			value_free(temp.core.u_l);
			if (lptr.type != VALUE_LST) {
				// (lptr) is empty. Stick (rptr) onto the end of the result list.
				ptr.core.u_l[1] = rptr;
				break;
			}
		} else {
			ptr.core.u_l[0] = rptr.core.u_l[0];
			temp = rptr;
			rptr = rptr.core.u_l[1];
			value_free(temp.core.u_l);
			if (rptr.type != VALUE_LST) {
				ptr.core.u_l[1] = lptr;
				break;
			}
		}
		
		ptr = ptr.core.u_l[1];
	}
	
	return 0;
}

/* 
 * Uses one of two different algorithms, depending on how long the array is. The 
 * first algorithm is better for shorter arrays, but becomes very slow for longer 
 * ones.
 */
value value_uniq(value op)
{
	if (op.type == VALUE_ARY) {
		value res = value_init(VALUE_ARY);
		size_t i, length = value_length(op);
		if (length < 150) {
			for (i = 0; i < length; ++i)
				if (!value_contains_p(res, op.core.u_a.a[i]))
					value_append_now(&res, op.core.u_a.a[i]);
		} else {
			value hash = value_hash_init_capacity(length * 2);
			for (i = 0; i < length; ++i)
				if (FALSE == value_hash_exists(hash, op.core.u_a.a[i])) {
					value_hash_put(&hash, op.core.u_a.a[i], value_init_nil());
					value_append_now(&res, op.core.u_a.a[i]);
				}
			value_clear(&hash);
		}

		
		return res;
	} else {
		value_error(1, "Type Error: uniq() is undefined where op is %ts (array expected).", op);
		return value_init_error();
	}
	
}

value value_uniq_now(value *op)
{
	value res = value_uniq(*op);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op);
	*op = res;
	return value_init_nil();
}

value value_uniq_sort(value op)
{
	value res = value_init_nil();
	if (op.type == VALUE_ARY) {
		value copy = value_set(op);
		size_t old_length = value_length(copy);
		value_private_sort_recursive(copy.core.u_a.a, 0, old_length - 1);
		
		value array[old_length];
		size_t i, new_length;
		for (i = 0, new_length = 0; i < old_length; ++i) {
			if (i == 0 || value_cmp_any(copy.core.u_a.a[i-1], copy.core.u_a.a[i]) != 0)
				array[new_length++] = copy.core.u_a.a[i];
		}
				
		res = value_set_ary(array, new_length);
		value_clear(&copy);
	
	} else if (op.type == VALUE_LST) {
		value copy = value_sort(op);
		res = value_init_nil();
		value *rptr = &res;
		
		value ptr = copy;
		value *prev = NULL;
		while (ptr.type == VALUE_LST) {
			if (prev == NULL || value_cmp_any(*prev, ptr.core.u_l[0]) != 0) {
				*rptr = value_init(VALUE_LST);
				rptr->core.u_l[0] = value_set(ptr.core.u_l[0]);
				rptr = &rptr->core.u_l[1];
			}
			
			prev = &ptr.core.u_l[0];
			ptr = ptr.core.u_l[1];
		}
		
		value_clear(&copy);
		
	} else {
		value_error(1, "Type Error: uniq_sort() is undefined where op is %ts (array expected).", op);
		res = value_init_error();
	}
	
	return res;
}

value value_uniq_sort_now(value *op)
{
	value res = value_uniq_sort(*op);
	if (res.type == VALUE_ERROR)
		return res;
	value_clear(op);
	*op = res;
	return value_init_nil();
}

value value_append_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "append()") ? value_init_error() : value_append(argv[0], argv[1]);
}

value value_append_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "append!()") ? value_init_error() : value_append_now(&argv[0], argv[1]);
}

value value_array_with_length_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "array_with_length()") ? value_init_error() : value_array_with_length(argv[0]);
}

value value_at_arg(int argc, value argv[])
{
	if (argc == 1)
		return missing_arguments(argc, argv, "at()") ? value_init_error() : value_at(argv[0], value_init_nil(), argv+1, argc-1);
	else
		return missing_arguments(argc, argv, "at()") ? value_init_error() : value_at(argv[0], argv[1], argv+2, argc-2);
}

value value_at_assign_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "at=()") ? value_init_error() : value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, value_init_nil(), argv[argc-1]);
}

value value_at_assign_add_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at+=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_add_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_at_assign_sub_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at-=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_sub_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_at_assign_mul_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at*=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_mul_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_at_assign_div_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at/=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_div_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_at_assign_mod_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at%=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_mod_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_at_assign_and_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at&=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_and_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_at_assign_xor_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at^=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_xor_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_at_assign_or_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at|=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_or_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+4, argc-4, func, argv[3]);
	value_clear(&func);
	return res;
}

value value_at_assign_shl_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at<<=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_shl_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_at_assign_shr_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	if (missing_arguments(argc-1, argv+1, "at>>=()"))
		return value_init_error();
	
	value func = value_set_fun(&value_shr_arg);
	return_if_error(func);
	value res = value_at_assign_do(tmp, &argv[1], argv[2], argv+3, argc-4, func, argv[argc-1]);
	value_clear(&func);
	return res;
}

value value_concat_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "concat()") ? value_init_error() : value_concat(argv[0], argv[1]);
}

value value_delete_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "delete()") ? value_init_error() : value_delete(argv[0], argv[1]);
}

value value_delete_all_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "delete_all()") ? value_init_error() : value_delete_all(argv[0], argv[1]);
}

value value_delete_at_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "delete_at()") ? value_init_error() : value_delete_at(argv[0], argv[1]);
}

value value_delete_at_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "delete_at!()") ? value_init_error() : value_delete_at_now(&argv[0], argv[1]);
}

value value_each_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "each()") ? value_init_error() : value_each(tmp, argv[1], argv[2]);
}

value value_each_index_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "each_index()") ? value_init_error() : value_each_index(tmp, argv[1], argv[2]);
}

value value_empty_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "empty?()") ? value_init_error() : value_empty_p_std(argv[0]);
}

value value_filter_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "filter()") ? value_init_error() : value_filter(tmp, argv[1], argv[2]);
}

value value_find_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "find()") ? value_init_error() : value_find(tmp, argv[1], argv[2]);
}

value value_flatten_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "flatten()") ? value_init_error() : value_flatten(argv[0]);
}

value value_flatten_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "flatten!()") ? value_init_error() : value_flatten_now(&argv[0]);
}

value value_fold_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "fold()") ? value_init_error() : value_fold(tmp, argv[1], argv[2], argv[3]);
}

value value_join_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "join()") ? value_init_error() : value_join(argv[0], argv[1]);
}

value value_last_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "last()") ? value_init_error() : value_last(argv[0]);
}

value value_map_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "map()") ? value_init_error() : value_map(tmp, argv[1], argv[2]);
}

value value_map_now_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "map!()") ? value_init_error() : value_map_now(tmp, &argv[1], argv[2]);
}

value value_map_drop_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "map_drop()") ? value_init_error() : value_map_drop(tmp, argv[1], argv[2], argv[3]);
}

value value_pop_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "pop()") ? value_init_error() : value_pop(argv[0]);
}

value value_pop_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "pop!()") ? value_init_error() : value_pop_now(&argv[0]);
}

value value_shuffle_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "shuffle()") ? value_init_error() : value_shuffle(argv[0]);
}

value value_shuffle_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "shuffle!()") ? value_init_error() : value_shuffle_now(&argv[0]);
}

value value_size_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "size()") ? value_init_error() : value_size_std(argv[0]);
}

value value_sort_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "sort()") ? value_init_error() : value_sort(argv[0]);
}

value value_sort_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "sort!()") ? value_init_error() : value_sort_now(&argv[0]);
}

value value_uniq_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "uniq()") ? value_init_error() : value_uniq(argv[0]);
}

value value_uniq_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "uniq!()") ? value_init_error() : value_uniq_now(&argv[0]);
}

value value_uniq_sort_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "uniq_sort()") ? value_init_error() : value_uniq_sort(argv[0]);
}

value value_uniq_sort_now_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "uniq_sort!()") ? value_init_error() : value_uniq_sort_now(&argv[0]);
}

