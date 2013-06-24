/*
 *  value_list.c
 *  Calculator
 *
 *  Created by Michael Dickens on 3/15/10.
 *
 */

#include "value.h"

/* 
 * Functions for working with lists. Notice that all of these functions 
 * work with arrays as well.
 */

/* 
 * Linked List Implementation
 * 
 * Linked lists are implemented as a two-element array in core.u_l, 
 * which is pretty much identical to core.u_a; the difference in name 
 * is only for clarification purposes. The first element is the actual 
 * value, and the second element is a pointer to the next list. If it 
 * is the end of the list, this should be VALUE_NIL.
 * 
 * The implementation of empty lists is a little unconventional. An 
 * empty list is not stored as NULL because this decentralizes memory 
 * allocation (it would be nice to always allocate memory in value_init(), 
 * but if in value_init() the list is set to NULL then this doesn't 
 * work so memory allocation has to be done elsewhere). So instead, 
 * a list is considered empty when its tail (cdr) is VALUE_NIL and its  
 * head (car) is VALUE_EMPTY. If the head is ever VALUE_EMPTY when the 
 * tail is not VALUE_NIL, or if an array element is VALUE_EMPTY or 
 * something like that, then it is wrong. The only time when VALUE_EMPTY 
 * is to be used is to denote a list as empty.
 * 
 * If you want to find the tail of a list you can use tail(), but in many 
 * cases this is not what you want to do. This will create a copy of the 
 * tail and return it. To directly access the tail of (list), use 
 * list.core.u_l[1]. Similarly, the head can be accessed by using
 * list.core.u_l[0].
 */

value value_cons(value op1, value op2)
{
	value res = value_init_nil();
	if (op2.type == VALUE_NIL) {
		res = value_init(VALUE_PAR);
		res.core.u_p->head = value_set(op1);
	} else if (op2.type == VALUE_ARY) {
		size_t length = value_length(op2) + 1;
		value array[length];
		array[0] = op1;
		size_t i;
		for (i = 1; i < length; ++i)
			array[i] = op2.core.u_a.a[i-1];
		res = value_set_ary(array, length);
	} else if (op2.type == VALUE_LST) {
		res = value_init(VALUE_LST);
		res.core.u_l[0] = value_set(op1);
		res.core.u_l[1] = value_set(op2);
	} else if (op2.type == VALUE_PAR) {
		res = value_init(VALUE_PAR);
		res.core.u_l[0] = value_set(op1);
		res.core.u_l[1] = value_set(op2);
	} else {
		value_error(1, "Type Error: cons() is undefined where op2 is %ts (nil, array or list expected).", op2);
		res = value_init_error();
	}
	
	return res;
}

value value_cons_now(value op1, value *op2)
{
	value tmp = value_set(op1);
	return value_cons_now2(&tmp, op2);
}

value value_cons_now2(value *op1, value *op2)
{
	if (op2->type == VALUE_NIL) {
		op2->type = VALUE_PAR;
		value_malloc(op2, 2);
		if (op2->type == VALUE_ERROR) return;
		op2->core.u_p->head = *op1;
		op2->core.u_p->tail = value_init_nil();
		
	} else if (op2->type == VALUE_ARY) {
		value res;
		res.type = VALUE_ARY;
		size_t i, length = value_length(*op2);
		if (resize_p(length+1)) {
			value_realloc(op1, next_size(length+1) + 1);
			if (op1->type == VALUE_ERROR)
				return value_init_error();
		}
		for (i = 1; i < length; ++i)
			res.core.u_a.a[i+1] = op2->core.u_a.a[i];
		res.core.u_a.a[0] = *op1;
		res.core.u_a.length = op2->core.u_a.length + 1;
		*op2 = res;
	} else if (op2->type == VALUE_LST) {
		value res;
		res.type = VALUE_LST;
		value_malloc(&res, 2);
		if (res.type == VALUE_ERROR) {
			return value_init_error();
		}
		res.core.u_l[0] = *op1;
		res.core.u_l[1] = *op2;
		*op2 = res;		
	} else if (op2->type == VALUE_PAR) {
		value res;
		res.type = VALUE_PAR;
		value_malloc(&res, 2);
		return_if_error(res);
		res.core.u_l[0] = *op1;
		res.core.u_l[1] = *op2;
		*op2 = res;		
	} else {
		value_error(1, "Type Error: cons() is undefined where op2 is %ts (nil, array or list expected).", *op2);
		return value_init_error();
	}
	
	return value_init_nil();
}

value value_drop(value op, value n)
{
	if (op.type == VALUE_NIL) {
		return value_init_nil();
	} else if (op.type == VALUE_ARY) {
		if (n.type == VALUE_MPZ) {
			value length = value_set_long(op.core.u_a.length);
			value res = value_range(op, n, length);
			value_clear(&length);
			return res;
		}
	} else if (op.type == VALUE_LST) {
		if (n.type == VALUE_MPZ) {
			if (value_lt(n, value_zero)) {
				value_error(1, "Domain Error: drop() is undefined where n is %s (>= 0 expected).", n);
				return value_init_error();
			} else if (value_gt(n, value_int_max)) {
				value_error(1, "Domain Error: drop() is undefined where n is %s (<= %s expected).", n, value_int_max);
				return value_init_error();				
			}
			size_t i, max = value_get_long(n);
			value ptr = op;
			for (i = 0; i < max && ptr.type == VALUE_LST; ++i) {
				ptr = ptr.core.u_l[1];
			}
			
			return value_set(ptr);
		}
	} else if (op.type == VALUE_PAR) {
		if (n.type == VALUE_MPZ) {
			if (value_lt(n, value_zero)) {
				value_error(1, "Domain Error: drop() is undefined where n is %s (>= 0 expected).", n);
				return value_init_error();
			} else if (value_gt(n, value_int_max)) {
				value_error(1, "Domain Error: drop() is undefined where n is %s (<= %s expected).", n, value_int_max);
				return value_init_error();				
			}
			size_t i, max = value_get_long(n);
			value ptr = op;
			for (i = 0; i < max && ptr.type == VALUE_PAR; ++i) {
				ptr = ptr.core.u_p->tail;
			}
			
			return value_set(ptr);
		}
	} else {
		value_error(1, "Type Error: drop() is undefined where op1 is %ts (array or list expected).", op);
		if (n.type == VALUE_MPZ)
			return value_init_error();
	}
	
	value_error(1, "Type Error: drop() is undefined where op2 is %ts (integer expected).", n);
	return value_init_error();
}

value value_head(value op)
{
	if (op.type == VALUE_ARY) {
		if (value_length(op) == 0) {
			value_error(1, "Error: cannot find head() of an empty array.");
			return value_init_error();
		}

		return value_set(op.core.u_a.a[0]);
	} else if (op.type == VALUE_LST) {
		return value_set(op.core.u_l[0]);
	} else if (op.type == VALUE_PAR) {
		return value_set(op.core.u_p->head);
	} else {
		value_error(1, "Type Error: head() is undefined where op is %ts (array or list expected).", op);
		return value_init_error();
	}
}

value * value_head_ref(value op)
{
	if (op.type == VALUE_ARY) {
		if (value_length(op) == 0) {
			value_error(1, "Error: cannot find head() of an empty array.");
			return NULL;
		}

		return &op.core.u_a.a[0];
	} else if (op.type == VALUE_LST) {
		return &op.core.u_l[0];
	} else if (op.type == VALUE_PAR) {
		return &op.core.u_p->head;
	} else {
		value_error(1, "Type Error: head() is undefined where op is %ts (array or list expected).", op);
	}

	return NULL;
}

value value_tail(value op)
{
	if (op.type == VALUE_ARY) {
		size_t length = value_length(op);
		if (length == 0) {
			value_error(1, "Error: cannot find tail() of an empty array.");
			return value_init_error();
		}
		
		value array[length - 1];
		size_t i;
		for (i = 0; i < length-1; ++i)
			array[i] = op.core.u_a.a[i+1];
		
		value res = value_set_ary(array, length-1);
		
		return res;
		
	} else if (op.type == VALUE_LST) {		
		return value_set(op.core.u_l[1]);
		
	} else if (op.type == VALUE_PAR) {
		return value_set(op.core.u_p->tail);
	} else {
		value_error(1, "Type Error: tail() is undefined where op is %ts (array or list expected).", op);
		return value_init_error();
	}

}

value value_tail_now(value *op)
{
	if (op->type == VALUE_NIL) {
		value_error(1, "Error: cannot find tail!() of an empty list.");
		return value_init_error();
	} else if (op->type == VALUE_ARY) {
		size_t i, length = value_length(*op);
		if (length == 0) {
			value_error(1, "Error: cannot find tail!() of an empty array.");
			return value_init_error();
		}
		
		value_clear(&op->core.u_a.a[0]);
		for (i = 0; i < length-1; ++i)
			op->core.u_a.a[i] = op->core.u_a.a[i+1];
		
		--op->core.u_a.length;
	} else if (op->type == VALUE_LST) {
		if (value_empty_p(*op)) {
			value_error(1, "Error: cannot find tail!() of an empty list.");
			return value_init_error();
		} else {
			value_clear(&op->core.u_l[0]);
			value_free(op->core.u_l);
			*op = op->core.u_l[1];
		}
	} else if (op->type == VALUE_PAR) {
		value tmp = op->core.u_p->tail;
		value_clear(&op->core.u_p->head);
		value_free(op->core.u_p);
		*op = tmp;
	} else {
		value_error(1, "Type Error: tail!() is undefined where op is %ts (array or list expected).", *op);
		return value_init_error();
	}

	
	return value_init_nil();
}

value value_take(value op, value n)
{
	if (op.type == VALUE_NIL) {
		return value_init_nil();
	} else if (op.type == VALUE_ARY) {
		if (n.type == VALUE_MPZ) {
			value start = value_set_long(0);
			value res = value_range(op, start, n);
			value_clear(&start);
			return res;
		}
	} else if (op.type == VALUE_LST) {
		if (n.type == VALUE_MPZ) {
			if (value_lt(n, value_zero)) {
				value_error(1, "Domain Error: drop() is undefined where n is %s (>= 0 expected).", n);
				return value_init_error();
			} else if (value_gt(n, value_int_max)) {
				value_error(1, "Domain Error: drop() is undefined where n is %s (<= %s expected).", n, value_int_max);
				return value_init_error();				
			}
			
			value res = value_init_nil();
			size_t i, max = value_get_long(n);
			value ptr = op;
			for (i = 0; i < max && ptr.type == VALUE_LST; ++i) {
				value_cons_now(ptr.core.u_l[0], &res);
				ptr = ptr.core.u_l[1];
			}
			
			value_reverse_now(&res);
			
			return res;
		}
	} else if (op.type == VALUE_PAR) {
		if (n.type == VALUE_MPZ) {
			if (value_lt(n, value_zero)) {
				value_error(1, "Domain Error: drop() is undefined where n is %s (>= 0 expected).", n);
				return value_init_error();
			} else if (value_gt(n, value_int_max)) {
				value_error(1, "Domain Error: drop() is undefined where n is %s (<= %s expected).", n, value_int_max);
				return value_init_error();				
			}
			
			value res = value_init_nil();
			size_t i, max = value_get_long(n);
			value ptr = op;
			for (i = 0; i < max && ptr.type == VALUE_PAR; ++i) {
				value_cons_now(ptr.core.u_p->tail, &res);
				ptr = ptr.core.u_p->tail;
			}
			
			value_reverse_now(&res);
			
			return res;
		}
	} else {
		value_error(1, "Type Error: take() is undefined where op is %ts (array or list expected).", op);
		if (n.type == VALUE_MPZ)
			return value_init_error();
	}
	
	value_error(1, "Type Error: drop() is undefined where n is %ts (integer expected).", n);
	return value_init_error();

}

value value_cons_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "cons()") ? value_init_error() : value_cons(argv[0], argv[1]);
}

value value_cons_now_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "cons!()"))
		return value_init_error();
	value_cons_now(argv[0], &argv[1]);
	return value_init_nil();
}

value value_drop_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "drop()") ? value_init_error() : value_drop(argv[0], argv[1]);
}

value value_head_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "head()") ? value_init_error() : value_head(argv[0]);
}

value value_tail_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "tail()") ? value_init_error() : value_tail(argv[0]);
}

value value_take_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "take()") ? value_init_error() : value_take(argv[0], argv[1]);
}

