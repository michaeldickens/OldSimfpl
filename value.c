/*
 *  value.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 1/23/10.
 *
 */

#include "value.h"


int init_values()
{
	mpfr_set_default_rounding_mode(value_mpfr_round = GMP_RNDN);
	value_mpfr_round_cast = GMP_RNDZ; // How to round when casting from MPFR to an integer.
	mpfr_set_default_prec(value_mpfr_default_prec = 53);
	
	value_int_min = value_set_long(sizeof(size_t) == sizeof(int) ? INT_MIN : LONG_MIN);
	value_zero = value_set_long(0);
	value_one = value_set_long(1);
	value_int_max = value_set_long(sizeof(size_t) == sizeof(int) ? INT_MAX : LONG_MAX);
	value_nil = value_init_nil();
	
	value_symbol_in = value_set_symbol("in");
	value_symbol_dotimes = value_set_symbol("dotimes");
	value_symbol_if = value_set_symbol("if");

	value_nil_function_spec.needs_variables_p = FALSE;
	value_nil_function_spec.keep_arg_p = FALSE;
	value_nil_function_spec.delay_eval_p = FALSE;
	value_nil_function_spec.argc = 0;
	value_nil_function_spec.optional = INT_MAX;
	value_nil_function_spec.rest_p = FALSE;
	value_nil_function_spec.associativity = '\0';
	value_nil_function_spec.precedence = 0;

	generic_error = exception_init(NULL, "GenericError");
	runtime_error = exception_init(&generic_error, "RuntimeError");
	argument_error = exception_init(&runtime_error, "ArgumentError");
	memory_error;
	syntax_error;
	type_error;
	
	return 0;
}

/* 
 * Called in each _arg function to see if there are missing arguments.
 */
int missing_arguments(int argc, value argv[], char *name)
{	
	int i;
	int missing = 0;
	for (i = 0; i < argc; ++i)
		if (argv[i].type == VALUE_MISSING_ARG)
			++missing;
		// If one of the arguments is an error, then there's no sense in trying to 
		// evaluate the function.
		else if (argv[i].type == VALUE_ERROR)
			return TRUE;
	
	if (missing > 0) {
		if (missing == 1)
			value_error(1, "Argument Error: in %c, %d missing argument (%d expected, %d found).", name, missing, argc, argc - missing);
		else
			value_error(1, "Argument Error: in %c, %d missing arguments (%d expected, %d found).", name, missing, argc, argc - missing);
		return TRUE;
	}
	
	return FALSE;
}

const char * type_to_string(int type)
{
	switch (type) {
		case VALUE_ERROR:
			return "Error";
		case VALUE_NIL:
			return "Nil";
		case VALUE_NAN:
			return "NaN";
		case VALUE_INF:
			return "Infinity";
		case VALUE_BOO:
			return "Boolean";
		case VALUE_MPZ:
			return "Integer";
		case VALUE_MPF:
			return "Float";
		case VALUE_STR:
			return "String";
		case VALUE_RGX:
			return "Regex";
		case VALUE_SYM:
			return "Symbol";
		case VALUE_ARY:
			return "Array";
		case VALUE_LST:
			return "List";
		case VALUE_PAR:
			return "Pair";
		case VALUE_HSH:
			return "Hash";
		case VALUE_TRE: 
			return "Tree";
		case VALUE_PTR:
			return "Pointer";
		case VALUE_RNG:
			return "Range";
		case VALUE_BLK:
			return "Block";
		case VALUE_ID:
			return "Id";
		case VALUE_VAR:
			return "Variable";
		case VALUE_STOP:
			return "Stop";
		case VALUE_SPEC:
			return "Spec";
		case VALUE_BIF:
			return "Primitive";
		case VALUE_UDF:
			return "Function";
		case VALUE_UDF_SHELL:
			return "FunctionShell";
		case VALUE_TYP:
			return "Type";
		case VALUE_MISSING_ARG:
			return "MissingArg";
		default:
			return "UnknownType";
	}
}

int value_array_id_index(value vals[], int length, char *val)
{
	int i;
	for (i = 0; i < length; ++i) {
		if (vals[i].type == VALUE_ID && streq(vals[i].core.u_id, val))
			return i;
	}
	
	return -1;
}

int value_array_copy(value target[], value source[], int size)
{
	int i;
	for (i = 0; i < size; ++i)
		target[i] = value_set(source[i]);
	return 0;
}

value value_init_nil()
{
	return value_nil;
}

value value_init_nil_type(int type)
{
	value res;
	
	res.type = VALUE_NIL;
	res.core.u_nil = type;
	return res;	
}

value value_init_error()
{
	value res;
	
	res.type = VALUE_ERROR;
	return res;
}

value value_init(int type)
{
	value res;
	
	res.type = type;
	switch (type) {
		case VALUE_NIL:
		case VALUE_NAN:
		case VALUE_INF:
		case VALUE_MISSING_ARG:
		case VALUE_ERROR:
			break;
		case VALUE_BOO:
			res.core.u_b = FALSE;
			break;
		case VALUE_MPZ:
			mpz_init(res.core.u_mz);
			break;
		case VALUE_MPF:
			mpfr_init(res.core.u_mf);
			break;
		case VALUE_STR:
		case VALUE_RGX:
		case VALUE_SYM:
		case VALUE_ID:
		case VALUE_VAR:
			value_malloc(&res, 1);
			return_if_error(res);
			res.core.u_s[0] = '\0';
			break;
		case VALUE_ARY:
			res.core.u_a.a = NULL;
			res.core.u_a.length = 0;
			break;
		case VALUE_LST:
			value_malloc(&res, 2);
			return_if_error(res);
			res.core.u_l[0].type = VALUE_NIL;
			res.core.u_l[1].type = VALUE_NIL;
			break;
		case VALUE_PAR:
			value_malloc(&res, 1);
			return_if_error(res);
			res.core.u_p->head.type = VALUE_NIL;
			res.core.u_p->tail.type = VALUE_NIL;
			break;
		case VALUE_HSH:
			res = value_hash_init();
			break;
		case VALUE_RNG:
			value_error(1, "Error: Cannot initialize a range.");
			res.type = VALUE_ERROR;
			break;
		case VALUE_BLK:
			res.core.u_blk.a = NULL;
			res.core.u_blk.length = 0;
			res.core.u_blk.s = NULL;
			break;
		case VALUE_PTR:
			res.core.u_ptr = NULL;
			break;
		case VALUE_BIF:
			res.core.u_bif = NULL;
			break;
		case VALUE_UDF:
		case VALUE_UDF_SHELL:
			res.core.u_udf = value_malloc(NULL, sizeof(struct value_function));
			return_if_null(res.core.u_udf);
			res.core.u_udf->name = NULL;
			res.core.u_udf->vars = value_init_nil();
			res.core.u_udf->body = value_init_nil();
			res.core.u_udf->spec = compile_spec("0l15");
			break;
		default:
			value_error(1, "Type Error: init() is undefined for type %d.", type);
			res.type = VALUE_ERROR;
			break;
	}
	return res;
}

int value_clear(value *op)
{
	size_t i, length;
	
	switch (op->type) {
	case VALUE_NIL:
	case VALUE_NAN:
	case VALUE_INF:
	case VALUE_TYP:
	case VALUE_ERROR:
	case VALUE_SPEC:
		// Do nothing.
		break;
	case VALUE_BOO:
		op->core.u_b = FALSE;
		break;
	case VALUE_MPZ:
		mpz_clear(op->core.u_mz);
		break;
	case VALUE_MPF:
		mpfr_clear(op->core.u_mf);
		break;
	case VALUE_STR:
	case VALUE_RGX:
	case VALUE_SYM:
	case VALUE_ID:
	case VALUE_VAR:
		value_free(op->core.u_s);
		break;
	case VALUE_ARY:
		if (op->core.u_a.a) {
			for (i = 0; i < op->core.u_a.length; ++i)
				value_clear(&(op->core.u_a.a[i]));
			value_free(op->core.u_a.a);
		}
		break;
	case VALUE_LST:
		;
		value ptr = *op;
		value ptr2 = ptr;
		while (ptr.type == VALUE_LST) {
			value_clear(&ptr.core.u_l[0]);
			ptr2 = ptr;
			ptr = ptr.core.u_l[1];
			value_free(ptr2.core.u_l);
			ptr2.type = VALUE_NIL;
		}
		
		break;
	case VALUE_PAR:
		if (op->core.u_p) {
			value_clear(&op->core.u_p->head);
			value_clear(&op->core.u_p->tail);
			value_free(op->core.u_p);
		}
		break;
	case VALUE_HSH:
		value_hash_clear(op);
		break;
	case VALUE_PTR:
		value_clear(op->core.u_ptr);
		op->core.u_ptr = NULL;
		break;
	case VALUE_RNG:
		value_clear(&op->core.u_r->min);
		value_clear(&op->core.u_r->max);
		value_free(op->core.u_r);
		break;
	case VALUE_BLK:
		length = value_length(*op);
		for (i = 0; i < length; ++i)
			value_clear(&(op->core.u_blk.a[i]));
		if (op->core.u_blk.a)
			value_free(op->core.u_blk.a);
		if (op->core.u_blk.s)
			value_free(op->core.u_blk.s);
		break;
	case VALUE_STOP:
		if (op->core.u_stop.core) {
			value_clear(op->core.u_stop.core);
			value_free(op->core.u_stop.core);
		}
		break;
	case VALUE_BIF:
		if (op->core.u_bif)
			value_free(op->core.u_bif);
		break;
	case VALUE_UDF:
	case VALUE_UDF_SHELL:
		if (op->core.u_udf->name)
			value_free(op->core.u_udf->name);
		value_clear(&op->core.u_udf->vars);
		value_clear(&op->core.u_udf->body);
		value_free(op->core.u_udf);
		break;
	case VALUE_EXC:
		if (op->core.u_exc.name)
			value_free(op->core.u_exc.name);
		if (op->core.u_exc.description)
			value_free(op->core.u_exc.description);
		if (op->core.u_exc.stack_trace) {
			value_clear(op->core.u_exc.stack_trace);
			value_free(op->core.u_exc.stack_trace);
		}
		op->core.u_exc.parent = NULL;
		op->core.u_exc.name = NULL;
		op->core.u_exc.description = NULL;
		op->core.u_exc.stack_trace = NULL;
		break;
	default:
		value_error(1, "Error: In value_clear(), undefined op type %d.", op->type);
		return VALUE_ERROR;
	}
	
	op->type = VALUE_NIL;	
	return 0;
}

value value_set(value op)
{
	value res;
	
	res.type = op.type;
	size_t i;
	
	value ptr;
	size_t length;
	
	switch (op.type) {
	case VALUE_NIL:
		res.core.u_nil = op.core.u_nil;
		break;
	case VALUE_MISSING_ARG: case VALUE_NAN: case VALUE_INF: case VALUE_ERROR:
		// Do nothing.
		break;
	case VALUE_BOO:
		res.core.u_b = op.core.u_b;
		break;
	case VALUE_MPZ:
		mpz_init_set(res.core.u_mz, op.core.u_mz);
		break;
	case VALUE_MPF:
		mpfr_init_set(res.core.u_mf, op.core.u_mf, value_mpfr_round);
		break;
	case VALUE_STR: case VALUE_RGX: case VALUE_SYM: case VALUE_ID: case VALUE_VAR:
		value_malloc(&res, strlen(op.core.u_s)+1);
		return_if_error(res);
		strcpy(res.core.u_s, op.core.u_s);
		break;
	case VALUE_ARY:
		if (op.core.u_a.a) {
			value_malloc(&res, next_size(op.core.u_a.length));
			return_if_error(res);
			for (i = 0; i < op.core.u_a.length; ++i)
				res.core.u_a.a[i] = value_set(op.core.u_a.a[i]);
			res.core.u_a.length = op.core.u_a.length;
		} else res.core.u_a.a = NULL;
		break;
	case VALUE_LST:
		// This is more complicated than the recursive version, but it's nearly twice 
		// as fast. Speed here is crucial.
		res.type = VALUE_LST;
		value_malloc(&res, 2);
		if (res.type == VALUE_ERROR)
			break;
		ptr = res;
		while (TRUE) {
			ptr.core.u_l[0] = value_set(op.core.u_l[0]);
			if (op.core.u_l[1].type == VALUE_LST) {
				ptr.core.u_l[1].type = VALUE_LST;
				value_malloc(&ptr.core.u_l[1], 2);
				if (ptr.core.u_l[1].type == VALUE_ERROR) {
					ptr.core.u_l[1].type = VALUE_NIL;
					value_clear(&res);
					res = value_init_error();
					break;
				}
				ptr = ptr.core.u_l[1];
				op = op.core.u_l[1];
			} else {
				ptr.core.u_l[1] = value_init_nil();
				break;
			}
		}
		break;

	case VALUE_PAR:
		value_malloc(&res, 1);
		return_if_error(res);
		res.core.u_p->head = value_set(op.core.u_p->head);
		res.core.u_p->tail = value_set(op.core.u_p->tail);
		break;

	case VALUE_HSH:
		// Notice that (op.core.u_h.length) will already be a legal size, so a call to next_size() 
		// is not necessary.
		value_malloc(&res, op.core.u_h.length);
		return_if_error(res);
		for (i = 0; i < op.core.u_h.length; ++i)
			res.core.u_h.a[i] = value_set(op.core.u_h.a[i]);
		res.core.u_h.length = op.core.u_h.length;
		res.core.u_h.size = op.core.u_h.size;
		res.core.u_h.occupied = op.core.u_h.occupied;
		break;
	case VALUE_RNG:
		value_malloc(&res, sizeof(struct value_range));
		return_if_error(res);
		res.core.u_r->inclusive_p = op.core.u_r->inclusive_p;
		res.core.u_r->min = value_set(op.core.u_r->min);
		res.core.u_r->max = value_set(op.core.u_r->max);
		break;
	case VALUE_PTR:
		res.core.u_ptr = op.core.u_ptr;
		break;
	case VALUE_BLK:
		length = res.core.u_blk.length = op.core.u_blk.length;
		value_malloc(&res, next_size(length));
		return_if_error(res);
		for (i = 0; i < length; ++i)
			res.core.u_blk.a[i] = value_set(op.core.u_blk.a[i]);
		if (op.core.u_blk.s == NULL)
			res.core.u_blk.s = NULL;
		else {
			value_malloc(&res, strlen(op.core.u_blk.s) + 1);
			return_if_error(res);
			strcpy(res.core.u_blk.s, op.core.u_blk.s);
		}
		break;
	case VALUE_TYP:
		res.core.u_type = op.core.u_type;
		break;
	case VALUE_STOP:
		res.core.u_stop.type = op.core.u_stop.type;
		if (op.core.u_stop.core) {
			res.core.u_stop.core = value_malloc(NULL, sizeof(value));
			return_if_null(res.core.u_stop.core);
			*res.core.u_stop.core = *op.core.u_stop.core;
		}
		break;
	case VALUE_SPEC:
		res.core.u_spec = op.core.u_spec;
		break;
	case VALUE_BIF:
		res.core.u_bif = value_malloc(NULL, sizeof(struct value_bif));
		return_if_null(res.core.u_bif);
		res.core.u_bif->f = op.core.u_bif->f;
		res.core.u_bif->spec = op.core.u_bif->spec;
		break;
	case VALUE_UDF: case VALUE_UDF_SHELL:
		res.core.u_udf = value_malloc(NULL, sizeof(struct value_function));
		return_if_null(res.core.u_udf);
		if (op.core.u_udf->name) {
			res.core.u_udf->name = value_malloc(NULL, strlen(op.core.u_udf->name) + 1);
			return_if_null(res.core.u_udf->name);
			strcpy(res.core.u_udf->name, op.core.u_udf->name);
		} else {
			res.core.u_udf->name = NULL;
		}

		res.core.u_udf->vars = value_set(op.core.u_udf->vars);
		return_if_error(res.core.u_udf->vars);
		res.core.u_udf->body = value_set(op.core.u_udf->body);
		return_if_error(res.core.u_udf->body);
		res.core.u_udf->spec = op.core.u_udf->spec;
		break;
	case VALUE_EXC:
		res.core.u_exc.parent = op.core.u_exc.parent;
		if (op.core.u_exc.name) {
			res.core.u_exc.name = value_malloc(NULL, strlen(op.core.u_exc.name) + 1);
			return_if_null(res.core.u_exc.name);
			strcpy(res.core.u_exc.name, op.core.u_exc.name);
		} else res.core.u_exc.name = NULL;
		
		if (op.core.u_exc.description) {
			res.core.u_exc.description = value_malloc(NULL, strlen(op.core.u_exc.description) + 1);
			return_if_null(res.core.u_exc.description);
			strcpy(res.core.u_exc.description, op.core.u_exc.description);
		} else res.core.u_exc.description = NULL;
		
		if (op.core.u_exc.stack_trace) {
			res.core.u_exc.stack_trace = value_malloc(NULL, sizeof(value));
			return_if_null(res.core.u_exc.stack_trace);
			*res.core.u_exc.stack_trace = value_set(*op.core.u_exc.stack_trace);
		} else res.core.u_exc.stack_trace = NULL;
		break;
	default:
		value_error(1, "Error: In value_set(), undefined op type %d.", op.type);
		res.type = VALUE_ERROR;
		break;
	}
	
	return res;

}

value value_set_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "set()") ? value_init_error() : value_set(argv[0]);
}

value value_set_bool(int x)
{
	value res;
	res.type = VALUE_BOO;
	if (x) res.core.u_b = TRUE;
	else res.core.u_b = FALSE;
	return res;
}

value value_set_long(long x)
{
	value res;
	res.type = VALUE_MPZ;
	mpz_init_set_si(res.core.u_mz, x);
	return res;
}

value value_set_ulong(unsigned long x)
{
	value res;
	
	res.type = VALUE_MPZ;
	mpz_init_set_ui(res.core.u_mz, x);
	return res;
}

value value_set_double(double x)
{
	value res;
	
	res.type = VALUE_MPF;
	mpfr_init_set_d(res.core.u_mf, x, value_mpfr_round);
	return res;
}

value value_set_str_smart(char *str, int base)
{
	value res = value_init_error();
	int isnum;
	if (same_type_determiner == SAME_TYPE_VALUE)
		isnum = isnumeric(str);
	else isnum = minus_isnumeric(str);
	size_t length;
	
	switch (isnum) {
		case VALUE_ERROR:
			res = value_init_error();
			break;
		case 1:
			res.type = VALUE_MPZ;
			mpz_init(res.core.u_mz);
			mpz_set_str(res.core.u_mz, str, base);
			break;
		case 2:
			res.type = VALUE_MPF;
			// 4 is the first integer over log2(10) ≈ 3.32.
			// If the given number is bigger than the default 
			// precision, make it big enough to hold the given 
			// number.
			if ((length = strlen(str)) * 4 > value_mpfr_default_prec)
				mpfr_init2(res.core.u_mf, length * 4);
			else mpfr_init(res.core.u_mf);
			mpfr_set_str(res.core.u_mf, str, base, value_mpfr_round);
			break;
		default:
			if (is_string_literal(str)) {
				size_t len = strlen(str);
				str[len] = '\0';
				char holder[len];
				strcpy(holder, str+1);
				holder[len-2] = '\0';
				res.type = VALUE_STR;
				res.core.u_s = convert_to_literal(holder);
			} else if (is_regex_literal(str)) {
				size_t len = strlen(str);
				str[len] = '\0';
				char holder[len];
				strcpy(holder, str+1);
				holder[len-2] = '\0';
				res.type = VALUE_RGX;
				res.core.u_x = convert_regex_to_literal(holder);
			} else if (str[0] == ':') {
				size_t len = strlen(str);
				res.type = VALUE_SYM;
				res.core.u_s = value_malloc(NULL, len);
				return_if_null(res.core.u_s);
				strcpy(res.core.u_s, str+1);
				
			} else if (streq(str, "true"))
				res = value_set_bool(TRUE);
			else if (streq(str, "false"))
				res = value_set_bool(FALSE);
			else if (streq(str, "nil"))
				res = value_init_nil();
			else
				return value_init_error();
	}
		
	return res;
}

value value_set_id(char *id)
{
	value res;
	
	res.type = VALUE_ID;
	res.core.u_id = value_malloc(NULL, strlen(id) + 1);
	return_if_null(res.core.u_id);
	strcpy(res.core.u_id, id);
	return res;
}

value value_copy(value op)
{
	value res;
	
	res.type = op.type;
	if (op.type == VALUE_ARY) {
		res.core.u_a.length = op.core.u_a.length;
		value_malloc(&res, next_size(res.core.u_a.length));
		return_if_error(res);
		size_t i;
		for (i = 0; i < res.core.u_a.length; ++i) {
			res.core.u_a.a[i] = op.core.u_a.a[i];
		}
	} else if (op.type == VALUE_LST) {
		value ptr = res;
		// I'm not sure if this will work. It hasn't been tested.
		while (op.type != VALUE_NIL) {
			value_malloc(&ptr, 2);
			return_if_error(res);
			ptr.core.u_l[0] = op.core.u_l[0];
			ptr.core.u_l[1].type = VALUE_LST;
			ptr = ptr.core.u_l[1];
			op = op.core.u_l[1];
		}
	
	} else if (op.type == VALUE_HSH) {
		size_t inner_len, length = value_hash_length(op);
		
		res = value_hash_init_capacity(next_size(length));
		size_t i, j;
		for (i = 0; i < length; ++i) {
			if (op.core.u_h.a[i].type == VALUE_NIL)
				continue;
			inner_len = value_length(op.core.u_h.a[i]);
			for (j = 0; j < inner_len; ++j)
				value_hash_put_refs(&res, &op.core.u_h.a[i].core.u_a.a[j].core.u_a.a[0], &op.core.u_h.a[i].core.u_a.a[j].core.u_a.a[1]);
		}
		
	} else {
		res = value_set(op);
	}
	
	return res;
}

void * value_malloc(value *op, size_t size)
{
	if (op == NULL) {
		
	} else if (op->type == VALUE_STR || op->type == VALUE_RGX || op->type == VALUE_SYM || op->type == VALUE_ID || op->type == VALUE_VAR) {
		op->core.u_s = NULL;
	} else if (op->type == VALUE_ARY) {
		op->core.u_a.a = NULL;
	} else if (op->type == VALUE_LST) {
		op->core.u_l = NULL;
	} else if (op->type == VALUE_PAR) {
		op->core.u_p = NULL;
	} else if (op->type == VALUE_HSH) {
		op->core.u_h.a = NULL;
	} else if (op->type == VALUE_RNG) {
		op->core.u_r = NULL;
	} else if (op->type == VALUE_BLK) {
		op->core.u_blk.a = NULL;
	} else {
		value_error(1, "Type Error: malloc() is undefined where op is %ts (linear container expected).", *op);
		value_clear(op);
		*op = value_init_error();
		return NULL;
	}
	
	return value_realloc(op, size);
}

void * value_realloc(value *op, size_t size)
{
	void *res;
	if (op == NULL) {
		res = malloc(size);
		if (res == NULL) {
			value_error(1, "Memory Error: Allocation failed.");
		}
				
	} else if (op->type == VALUE_STR || op->type == VALUE_RGX || op->type == VALUE_SYM || op->type == VALUE_ID || op->type == VALUE_VAR) {
		res = op->core.u_s = realloc(op->core.u_s, sizeof(char) * size);
		if (op->core.u_s == NULL) {
			value_error(1, "Memory Error: String allocation failed.");
			*op = value_init_error();
		}
	} else if (op->type == VALUE_ARY) {
		res = op->core.u_a.a = realloc(op->core.u_a.a, sizeof(value) * size);
		if (op->core.u_a.a == NULL) {
			value_error(1, "Memory Error: Array allocation failed.");
			*op = value_init_error();
		}
	} else if (op->type == VALUE_LST) {
		res = op->core.u_l = realloc(op->core.u_l, sizeof(value) * size);
		if (op->core.u_l == NULL) {
			value_error(1, "Memory Error: List allocation failed.");
			*op = value_init_error();
		}
	} else if (op->type == VALUE_PAR) {
		res = op->core.u_p = realloc(op->core.u_p, sizeof(struct value_pair) * size);
		if (op->core.u_l == NULL) {
			value_error(1, "Memory Error: Pair allocation failed.");
			*op = value_init_error();
		}
	} else if (op->type == VALUE_HSH) {
		res = op->core.u_h.a = realloc(op->core.u_h.a, sizeof(value) * size);
		if (op->core.u_h.a == NULL) {
			value_error(1, "Memory Error: Hash allocation failed.");
			*op = value_init_error();
		}
	} else if (op->type == VALUE_RNG) {
		res = op->core.u_r = realloc(op->core.u_r, sizeof(struct value_range) * size);
		if (op->core.u_r == NULL) {
			value_error(1, "Memory Error: Range allocation failed.");
			*op = value_init_error();
		}
	} else if (op->type == VALUE_BLK) {
		res = op->core.u_blk.a = realloc(op->core.u_blk.a, sizeof(value) * size);
		if (op->core.u_blk.a == NULL) {
			value_error(1, "Memory Error: Block allocation failed.");
			*op = value_init_error();
		}
	} else {
		value_error(1, "Type Error: malloc() is undefined where op is %ts (linear container expected).", *op);
		value_clear(op);
		*op = value_init_error();
		return res;
	}
	
	if (res == NULL) {
		int x = 0;
		x = 10 / x;
		return &x;
	}
	return res;
}

// This is defined using a macro.
//int value_free(void *ptr)
//{
//	free(ptr);
//	return 0;
//}

value value_import(value op)
{
	if (op.type != VALUE_STR) {
		value_error(1, "Type Error: import is undefined where op is %ts (string expected).", op);
		return value_init_error();
	}
	
	
	FILE *fp = fopen(op.core.u_s, "r");
	if (fp == NULL) {
		value_error(1, "IO Error: Cannot access file %s.", op);
		return value_init_error();
	}
	
	int old_linenum = linenum;
	linenum = 0;
	FILE *old_stream = input_stream;
	input_stream = fp;
	
	run_interpreter();
		
	fclose(fp);
	input_stream = old_stream;
	linenum = old_linenum;
	
	is_eof = FALSE;
	return value_init_nil();
}

value value_import_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "import") ? value_init_error() : value_import(argv[0]);
}

value value_array_arg(int argc, value argv[])
{
	return value_set_ary(argv, argc);
}

value value_list_arg(int argc, value argv[])
{
	int i;
	value res = value_init_nil();
	for (i = argc - 1; 0 <= i; --i)
		if (value_cons_now(argv[i], &res).type == VALUE_ERROR)
			return value_init_error();
	
	return res;
}

value value_hash_arg(int argc, value argv[])
{
	// If only one argument is given and it's an array, construct a hash using 
	// that array.
	if (argc == 1 && argv[0].type == VALUE_ARY) {
		return value_hash_arg(argv[0].core.u_a.length, argv[0].core.u_a.a);
	}
	
	size_t i;
	value res = value_hash_init_capacity(argc * 2);
	int error_p = FALSE;
	for (i = 0; i < argc; ++i)
		if (argv[i].type == VALUE_PAR) {
			if (error_p == FALSE)
				value_hash_put(&res, argv[i].core.u_p->head, argv[i].core.u_p->tail);
		} else {
			value_error(1, "Type Error: hash() constructor is undefined where argv[%d] is %ts (pair expected).", i, argv[i]);
			error_p = TRUE;
		}

	if (error_p) {
		value_clear(&res);
		res = value_init_error();
	}
	return res;
}

// This is ugly. It should be refactored.
value value_cast(value op, int type)
{
	value res = value_init_nil();
	int error_p = FALSE;
	
	switch (type) {
		case VALUE_NIL:
			value_clear(&res);
			res.type = VALUE_NIL;
			break;
			
		case VALUE_BOO:
			res.type = VALUE_BOO;
			res.core.u_b = value_true_p(op);
			break;
		
		case VALUE_MPZ:
			if (op.type == VALUE_MPZ)
				res = value_set(op);
			else if (op.type == VALUE_MPF) {
				mpz_init(res.core.u_mz);
				mpfr_get_z(res.core.u_mz, op.core.u_mf, value_mpfr_round_cast);
			} else if (op.type == VALUE_STR)
				mpz_init_set_str(res.core.u_mz, op.core.u_s, 0);
			else error_p = TRUE;
			
			res.type = VALUE_MPZ;
			break;
			
		case VALUE_MPF:			
			if (op.type == VALUE_MPF)
				res = value_set(op);
			else if (op.type == VALUE_MPZ) {
				mpfr_init_set_z(res.core.u_mf, op.core.u_mz, value_mpfr_round);
			} else if (op.type == VALUE_STR)
				mpfr_init_set_str(res.core.u_mf, op.core.u_s, 0, value_mpfr_round);
			else error_p = TRUE;
			res.type = VALUE_MPF;
			break;
			
		case VALUE_STR:
			if (op.type == VALUE_STR)
				res = value_set(op);
			else if (op.type == VALUE_RGX || op.type == VALUE_SYM || op.type == VALUE_ID || op.type == VALUE_VAR) {
				res = value_set(op);
				res.type = VALUE_STR;
			} else {
				res.type = VALUE_STR;
				size_t buf_len = BUFSIZE;
				while (TRUE) {
					char buffer[buf_len];
					int error_p = value_put(buffer, buf_len, op, NULL);
					if (error_p == 0) {
						res.core.u_s = value_malloc(NULL, strlen(buffer) + 1);
						return_if_null(res.core.u_s);
						strcpy(res.core.u_s, buffer);
						break;
					}
					
					buf_len <<= 4;
				}
			}
			break;
		
		case VALUE_RGX:
			if (op.type == VALUE_STR) {
				res = value_set(op);
				res.type = VALUE_RGX;
			} else error_p = TRUE;
			break;

		case VALUE_ARY:
			if (op.type == VALUE_ARY)
				res = value_set(op);
			else if (op.type == VALUE_STR) {
				size_t length = strlen(op.core.u_s);
				char str[2];
				value array[length];
				size_t i;
				str[1] = '\0';
				for (i = 0; i < length; ++i) {
					str[0] = op.core.u_s[i];
					array[i] = value_set_str(str);
				}
				
				res = value_set_ary(array, length);
				for (i = 0; i < length; ++i)
					value_clear(&array[i]);
				
			} else if (op.type == VALUE_LST) {
				size_t i, length = value_length(op);
				value array[length];
				value ptr = op;
				for (i = 0; i < length; ++i) {
					array[i] = ptr.core.u_l[0];
					ptr = ptr.core.u_l[1];
				}
				
				res = value_set_ary(array, length);
				
			} else if (op.type == VALUE_PAR) {
				size_t i, length = value_length(op);
				value array[length];
				value ptr = op;
				for (i = 0; i < length; ++i) {
					array[i] = ptr.core.u_p->head;
					ptr = ptr.core.u_p->tail;
				}
				
				res = value_set_ary(array, length);
				
			} else if (op.type == VALUE_HSH) {
				size_t size = op.core.u_h.size;
				// size is too large.
				value bucket, array[size];
				size_t i, j, k;
				for (i = 0, j = 0; i < op.core.u_h.length; ++i) {
					bucket = op.core.u_h.a[i];
					if (bucket.type != VALUE_ARY) continue;
					for (k = 0; k < bucket.core.u_a.length; ++k) {
						if (bucket.core.u_a.a[k].type != VALUE_ARY || bucket.core.u_a.a[k].core.u_a.length != 2)
							continue;
						// Put this pair into the new array.
						array[j++] = bucket.core.u_a.a[k];
					}
				}
				
				res = value_set_ary(array, size);
				
			} else if (op.type == VALUE_RNG) {
				value tmp = value_sub(op.core.u_r->max, op.core.u_r->min);
				value diff = value_abs(tmp);
				value_clear(&tmp);
				size_t length = (size_t) value_get_long(diff);
				if (op.core.u_r->inclusive_p)
					++length;
				value array[length];
				if (value_le(op.core.u_r->min, op.core.u_r->max)) {
					// Ascending values.
					value inc = value_set(op.core.u_r->min); size_t i = 0;
					while (TRUE) {
						array[i++] = inc;
						if (i >= length)
							break;
						inc = value_inc(inc);
					}
					
					res = value_set_ary_ref(array, length);
				} else {
					// Descending values.
					value dec = value_set(op.core.u_r->min); size_t i = 0;
					while (TRUE) {
						array[i++] = dec;
						if (i >= length)
							break;
						dec = value_dec(dec);
					}
					
					res = value_set_ary_ref(array, length);
				}
			
			} else if (op.type == VALUE_BLK) {
				res.type = VALUE_ARY;
				res.core.u_a.length = op.core.u_blk.length;
				value_malloc(&res, next_size(res.core.u_a.length));
				return_if_error(res);
				size_t i;
				for (i = 0; i < res.core.u_a.length; ++i) {
					res.core.u_a.a[i] = value_set(op.core.u_blk.a[i]);
				}

			} else {
				res = value_set_ary(&op, 1);
			}
			
			break;
		
		case VALUE_LST:
			if (op.type == VALUE_LST)
				res = value_set(op);
			else if (op.type == VALUE_ARY) {
				size_t length = value_length(op);
				res = value_init(VALUE_LST);
				if (length == 0)
					break;
				value ptr = res;
				size_t i;
				for (i = 0; i < length-1; ++i) {
					ptr.core.u_l[0] = value_set(op.core.u_a.a[i]);
					ptr = ptr.core.u_l[1] = value_init(VALUE_LST);
				}
				
				ptr.core.u_l[0] = value_set(op.core.u_a.a[length-1]);
				ptr.core.u_l[1] = value_init_nil();
			} else if (op.type == VALUE_RNG) {
				res = value_init_nil();
				value min = value_set(op.core.u_r->min);
				if (value_le(op.core.u_r->min, op.core.u_r->max)) {
					// Ascending values.
					value dec = value_set(op.core.u_r->max);
					if (op.core.u_r->inclusive_p == FALSE)
						value_dec_now(&dec);
					while (value_ge(dec, min)) {
						value_cons_now2(&dec, &res);
						dec = value_dec(dec);
					}
				} else {
					// Descending values.
					value_inc_now(&min);
					value inc = value_set(op.core.u_r->max);
					if (op.core.u_r->inclusive_p == FALSE)
						value_inc_now(&inc);
					while (value_lt(inc, min)) {
						value_cons_now2(&inc, &res);
						inc = value_inc(inc);
					}
				}
				value_clear(&min);
				
			} else if (op.type == VALUE_BLK) {
				size_t length = value_length(op);
				res = value_init(VALUE_LST);
				if (length == 0)
					break;
				value ptr = res;
				size_t i;
				for (i = 0; i < length-1; ++i) {
					ptr.core.u_l[0] = value_set(op.core.u_blk.a[i]);
					ptr = ptr.core.u_l[1] = value_init(VALUE_LST);
				}
				
				ptr.core.u_l[0] = value_set(op.core.u_blk.a[length-1]);
				ptr.core.u_l[1] = value_init_nil();
			} else {
				res = value_init(VALUE_LST);
				res.core.u_l[0] = value_set(op);
				res.core.u_l[1] = value_init_nil();
			}
			break;
		
		case VALUE_HSH:
			if (op.type == VALUE_HSH)
				res = value_set(op);
			else if (op.type == VALUE_ARY) {
				size_t length = value_length(op);
				res = value_hash_init_capacity(length * 2);
				size_t i;
				for (i = 0; i < length; ++i)
					value_hash_put(&res, op.core.u_a.a[i], value_init_nil());
			} else if (op.type == VALUE_LST) {
				res = value_hash_init_capacity(value_length(op) * 2);
				value ptr = op;
				while (!value_empty_p(ptr)) {
					value_hash_put(&res, ptr.core.u_l[0], value_init_nil());
					ptr = ptr.core.u_l[1];
				}
			} else {
				res = value_hash_init();
				value_hash_put(&res, op, value_init_nil());
			}

			break;
			
		default:
			error_p = TRUE;
			break;
	}
	
	if (error_p) {
		value_error(1, "Argument Error: Unsupported cast from %ts to %ts.", op, type);
		value_clear(&res);
		return value_init_error();
	} else return res;
}

value value_to_a_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_a()") ? value_init_error() : value_cast(argv[0], VALUE_ARY);
}

value value_to_f_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_f()") ? value_init_error() : value_cast(argv[0], VALUE_MPF);
}

value value_to_h_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_h()") ? value_init_error() : value_cast(argv[0], VALUE_HSH);
}

value value_to_i_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_i()") ? value_init_error() : value_cast(argv[0], VALUE_MPZ);
}

value value_to_l_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_l()") ? value_init_error() : value_cast(argv[0], VALUE_LST);
}

value value_to_r_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_r()") ? value_init_error() : value_cast(argv[0], VALUE_RGX);
}

value value_to_s_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "to_s()") ? value_init_error() : value_cast(argv[0], VALUE_STR);
}

value value_to_s_base_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "to_s_base()")) {
		return value_init_error();
	} else {
		char *res = value_to_string_base(argv[0], argv[1]);
		if (res)
			return value_set_str(res);
		return value_init_error();
	}
}

value value_type(value op)
{
	value res;
	
	res.type = VALUE_TYP;
	res.core.u_type = op.type;
	return res;
}

value value_type_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "type()") ? value_init_error() : value_type(argv[0]);
}


int value_true_p(value op)
{
	switch (op.type) {
		case VALUE_NIL:
		case VALUE_ERROR:
			return FALSE;
		case VALUE_BOO:
			return op.core.u_b;
		case VALUE_MPZ:
		case VALUE_MPF:
			return value_ne(op, value_zero);
		case VALUE_STR:
		case VALUE_ARY:
		case VALUE_LST:
		case VALUE_HSH:
		case VALUE_TRE:
		case VALUE_RNG:
			return !value_empty_p(op);
		default:
			return TRUE;
	}
}

value value_true_p_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "true?()") ? value_init_error() : value_set_bool(value_true_p(argv[0]));
}

value value_assign(value *variables, value op1, value op2)
{
	if (op1.type == VALUE_VAR) {
		if (op2.type != VALUE_ERROR) {
			if (op1.core.u_var[0] == '$')
				value_hash_put(&global_variables, op1, op2);
			else value_hash_put(variables, op1, op2);
			return value_set(op2);
		} else
			return value_init_error();
	} else {
		value_error(1, "Argument Error: assignment is undefined for %ts (variable expected).", op1);
		return value_init_error();
	}
}

value value_assign_arg(int argc, value argv[])
{
	value *tmp = value_deref(argv[0]);
	return missing_arguments(argc-1, argv+1, "assignment") ? value_init_error() : value_assign(tmp, argv[1], argv[2]);
}

value value_assign_add_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "+="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: += is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_add(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_add(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In +=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}

}

value value_assign_sub_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "-="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: -= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_sub(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_sub(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In -=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}
}

value value_assign_mul_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "*="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: *= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_mul(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_mul(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In *=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}
}

value value_assign_div_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "/="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: /= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_div(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_div(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In /=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}
}

value value_assign_mod_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "%="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: %= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_mod(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_mod(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In %=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}

}

value value_assign_and_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "&="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: &= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_and(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_and(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In &=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}
}

value value_assign_xor_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "^="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: ^= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_xor(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_xor(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In ^=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}
}

value value_assign_or_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "|="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: |= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_or(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_or(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In |=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}
}

value value_assign_shl_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, "<<="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: <<= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_shl_std(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_shl_std(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In <<=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}
}

value value_assign_shr_arg(int argc, value argv[])
{
	if (missing_arguments(argc-1, argv+1, ">>="))
		return value_init_error();
	if (argv[1].type != VALUE_VAR) {
		value_error(1, "Type Error: >>= is undefined where op1 is %ts (variable expected).", argv[1]);
		return value_init_error();
	}
	
	value *variables = value_deref(argv[0]);
	if (value_hash_exists(*variables, argv[1])) {
		value *op1 = value_hash_get_ref(*variables, argv[1]);
		value res = value_shr_std(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(variables, argv[1], res);
		return res;
	} else if (value_hash_exists(global_variables, argv[1])) {
		value *op1 = value_hash_get_ref(global_variables, argv[1]);
		value res = value_shr_std(*op1, argv[2]);
		return_if_error(res);
		value_hash_put(&global_variables, argv[1], res);
		return res;
	} else {
		value_error(1, "Error: In >>=, unrecognized variable %s.", argv[1]);
		return value_init_error();
	}
}



value value_make_pair(value op1, value op2)
{
	value res;
	res.type = VALUE_PAR;
	value_malloc(&res, 1);
	return_if_error(res);
	
	res.core.u_p->head = value_set(op1);
	res.core.u_p->tail = value_set(op2);
	return res;
}

value value_make_pair_refs(value *op1, value *op2)
{
	value res;
	res.type = VALUE_PAR;
	value_malloc(&res, 1);
	return_if_error(res);
	
	res.core.u_p->head = *op1;
	res.core.u_p->tail = *op2;
	return res;
}

value value_make_pair_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "make_pair") ? value_init_error() : value_make_pair(argv[0], argv[1]);
}

value value_refer(value *op)
{
	value res;
	res.type = VALUE_PTR;
	res.core.u_ptr = op;
	return res;
}

value * value_deref(value op)
{
	if (op.type == VALUE_PTR) {
		if (op.core.u_ptr == NULL) {
			value_error(1, "Error: Cannot dereference a null pointer.");
			return NULL;
		}
		return op.core.u_ptr;
	} else {
		value_error(1, "Argument Error: Cannot dereference %ts (pointer expected).", op);
		return NULL;
	}
}

value value_refer_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "refer()") ? value_init_error() : value_refer(&argv[0]);	
}

value value_gets()
{
	char str[BUFSIZE];
	char *ptr = fgets(str, BUFSIZE, stdin);
	if (ptr == NULL)
		return value_init_nil();
	value res = value_set_str(str);
	return res;
}

value value_gets_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "gets()") ? value_init_error() : value_gets();
}

value value_set_default_prec(value prec)
{
	if (prec.type == VALUE_MPZ) {
		mpfr_set_default_prec(value_get_ulong(prec));
		return value_init_nil();
	} else {
		value_error(1, "Type Error: set_default_prec() is undefined where prec is %ts (integer expected).", prec);
		return value_init_error();
	}
}

value value_set_default_prec_arg(int argc, value argv[])
{
	return missing_arguments(argc, argv, "set_default_prec()") ? value_init_error() : value_set_default_prec(argv[0]);
}




/* 
 * IO, setter and getter functions.
 */

double value_get_double(value op)
{
	if (op.type == VALUE_MPZ)
		return mpz_get_d(op.core.u_mz);
	else if (op.type == VALUE_MPF)
		return mpfr_get_d(op.core.u_mf, value_mpfr_round);
	else {
		value_error(1, "Argument Error: Cannot get a double from %ts (number expected).", op);
		return 0;
	}
}

long value_get_long(value op)
{
	if (op.type == VALUE_MPZ)
		return mpz_get_si(op.core.u_mz);
	else if (op.type == VALUE_MPF)
		return mpfr_get_si(op.core.u_mf, value_mpfr_round);
	else {
		value_error(1, "Argument Error: Cannot get a long from %ts (number expected).", op);
		return 0;
	}
}

char * value_get_string(value op)
{
	if (op.type == VALUE_STR || op.type == VALUE_ID || op.type == VALUE_VAR) {
		char *str = value_malloc(NULL, value_length(op));
		if (str == NULL) return NULL;
		strcpy(str, op.core.u_s);
		return str;
	}
	
	value_error(1, "Argument Error: Cannot get a string from %ts (string expected).", op);
	return NULL;
}

unsigned long value_get_ulong(value op)
{
	switch (op.type) {
		case VALUE_MPZ:
			return mpz_get_ui(op.core.u_mz);
		case VALUE_MPF:
			return mpfr_get_ui(op.core.u_mf, value_mpfr_round);
		default:
			value_error(1, "Argument Error: Cannot get an unsigned long from non-number %ts.", op);
			return 0;
	}
}

/* 
 * Only integers and floats have been tested where buffer is too small to hold 
 * the string.
 * 
 * Return Codes
 * 0: Success.
 * -1: (buffer) not long enough to hold (op).
 * -2: Undefined specifier (format).
 * -3: No memory available for malloc().
 * -4: mpfr_get_str() returned NULL.
 */
int value_put(char buffer[], size_t length, value op, char *format)
{
	if (length < 1) return VALUE_ERROR;
	buffer[0] = '\0';
	
	char *fptr = format;
	int op_base = 10;
	char specifier = 's';
	size_t width = length-1;
	size_t precision = 10;
	int is_precision_default = TRUE;
	int is_width_default = TRUE;
		
	// GOTO might strike fear into the hearts of men, but it's still better than putting 
	// a big if statement over a bunch of stuff, which would be ugly and unclear.
	if (format == NULL)
		goto done_with_format;
	
	if (isdigit(*fptr)) {
		is_width_default = FALSE;
		width = *fptr - '0';
		while (isdigit(*(++fptr)))
			width = 10*width + *fptr - '0';
	}
	
	if (*fptr == '.') {
		is_precision_default = FALSE;
		precision = 0;
		while (isdigit(*(++fptr)))
			precision = 10*precision + *fptr - '0';
	}
	
	char print_type = 0;
	if (*fptr == 'r' || *fptr == 't') {
		print_type = *fptr;
		if (is_width_default)
			width = 72;
		++fptr;
	}

	switch (*fptr) {
	case 'b': 
	case 'e': 
	case 'o': 
	case 's': 
	case 'v': 
	case 'x': 
		specifier = *fptr;
		break;
	default:
		;
		char buf[2];
		buf[0] = *fptr;
		buf[1] = '\0';
		value_error(1, "Error: In format string, undefined format specifier %c.", buf);
		return -2;
	}
		
	if (specifier == 'b')
		op_base = 2;
	else if (specifier == 'o')
		op_base = 8;
	else if (specifier == 'x')
		op_base = 16;
	
	// Only print the type of the outermost value. This prevents recursive type prints.
	if (print_type == 't')
		*(--fptr) = 's';
	else if (print_type)
		--fptr;

done_with_format: 
	
	
	if (op.type == VALUE_NIL) {
		if (specifier == 's') {
			if (length < 4) return VALUE_ERROR;
			sprintf(buffer, "nil");
		}
	} else if (op.type == VALUE_BOO) {
		if (op.core.u_b)
			if (length < 5) return VALUE_ERROR;
			else sprintf(buffer, "true");
		else
			if (length < 6) return VALUE_ERROR;
			else sprintf(buffer, "false");
		
	} else if (op.type == VALUE_MPZ) {
		if (length < mpz_sizeinbase(op.core.u_mz, op_base)) return VALUE_ERROR;
		mpz_get_str(buffer, op_base, op.core.u_mz);
		
	} else if (op.type == VALUE_MPF) {		
		if (length < 8) return VALUE_ERROR;
		
		size_t int_size = 0;
		
		if (specifier != 'e') {
			// Convert the number to MPZ so we'll know how many digits the integer part 
			// is, and can make sure the string is big enough.
			mpz_t int_op;
			mpz_init(int_op);
			mpfr_get_z(int_op, op.core.u_mf, value_mpfr_round_cast);
			size_t int_size = mpz_sizeinbase(int_op, op_base);
			mpz_clear(int_op);
			if (length < int_size) return VALUE_ERROR;
		}
		
		// n is the number of characters to be output.
		// (precision) decimals plus (int_size) digits plus 1 decimal point.
		size_t n = precision + int_size + 1;
		if (mpfr_sgn(op.core.u_mf) < 0)
			++n;
		
		if (length < int_size + 3) return VALUE_ERROR;
		
		// + 2 because 1 for null terminator, 1 for minus sign
		if (length < n + 2) n = length - 3;
		
		mp_exp_t *expptr = value_malloc(NULL, sizeof(mp_exp_t));
		if (expptr == NULL) return -3;
		if (mpfr_get_str(buffer, expptr, op_base, n, op.core.u_mf, value_mpfr_round) == NULL) {
			value_free(expptr);
			return -4;
		}
		
		mp_exp_t i, buflen = strlen(buffer);
		
		if (specifier == 'e') {
			char old2, old = '.';
			for (i = 1; i < buflen; ++i) {
				old2 = buffer[i];
				buffer[i] = old;
				old = old2;
			}
			
			char expstr[100];
			--*expptr;
			sprintf(expstr, "%ld", *expptr);
			if (length - buflen < strlen(expstr) + 2) return VALUE_ERROR;
			
			// Don't print out any trailing zeros.
			char *bufptr = buffer + buflen - 1;
			while (*bufptr == '0')
				--bufptr;
			sprintf(bufptr + 1, "e%s", expstr);
			
		} else {
			mpfr_sprintf(buffer, "%Rf", op.core.u_mf);
//			printf("buffer before: %s\n", buffer);
//			
//			// Insert a decimal point into the output buffer.
//			char ctmp, old = '.';
//			i = *expptr;
//			if (mpfr_sgn(op.core.u_mf) < 0)
//				++i;
//			printf("i = %ld\n", i);
//			if (i <= 0) {
//				--i;
//				// If the exponent is negative, insert zeros at the beginning.
//				if (length <= buflen + -i) return VALUE_ERROR;
//				memmove(buffer + -i, buffer, sizeof(char) * -i);
//				buflen += -i;
//				size_t j;
//				for (j = 0; j < -i; ++j)
//					buffer[j] = '0';
//				printf("buffer in between: %s\n", buffer);
//				i = (i + 1) + -i;
//			}
//			
//			// Make room for the decimal point.
//			memmove(buffer + i + 1, buffer + i, sizeof(char) * (buflen - i));
//			buffer[i] = '.';
//			++buflen;
//						
//			if (is_precision_default) {
//				printf("buflen = %ld\n", buflen);
//				// Unless a precision is explicitly given, remove trailing zeros.
//				while (buflen > 2 && buffer[buflen-1] == '0' && buffer[buflen-2] != '.')
//					buffer[--buflen] = '\0';
//			}
		}
		
		value_free(expptr);
		
	} else if (op.type == VALUE_STR) {
		if (specifier == 's') {
			// + 3 because one for each quotation mark and one for the null terminator.
			size_t i = 0;
			char *ptr = op.core.u_s - 1;
			if (i >= length) return VALUE_ERROR;
			buffer[i++] = '"';
			
			while (*(++ptr)) {
				if (i >= length) return VALUE_ERROR;
				if (isprint(*ptr)) buffer[i++] = *ptr;
				else {
					switch (*ptr) {
					case '\n': 
						buffer[i++] = '\\';
						if (i >= length) return VALUE_ERROR;
						buffer[i++] = 'n';
						break;
					case '\r': 
						buffer[i++] = '\\';
						if (i >= length) return VALUE_ERROR;
						buffer[i++] = 'r';
						break;
					case '\t':
						buffer[i++] = '\\';
						if (i >= length) return VALUE_ERROR;
						buffer[i++] = 't';
						break;
					}
				}

			}
			
			if (i >= length) return VALUE_ERROR;
			buffer[i++] = '"';
			if (i >= length) return VALUE_ERROR;
			buffer[i] = '\0';
			
//			if (strlen(op.core.u_s) + 3 > length) return VALUE_ERROR;
//			sprintf(buffer, "\"%s\"", op.core.u_s);
		} else {
			if (strlen(op.core.u_s) + 1 > length) return VALUE_ERROR;
			sprintf(buffer, "%s", op.core.u_s);
		}

	} else if (op.type == VALUE_RGX) {
		if (specifier == 's') {
			if (strlen(op.core.u_x) + 3 > length) return VALUE_ERROR;
			sprintf(buffer, "'%s'", op.core.u_x);
		} else {
			if (strlen(op.core.u_x) + 1 > length) return VALUE_ERROR;
			sprintf(buffer, "%s", op.core.u_x);
		}
	
	} else if (op.type == VALUE_SYM) {
		if (strlen(op.core.u_s) + 2 > length) return VALUE_ERROR;
		sprintf(buffer, ":%s", op.core.u_s);

	} else if (op.type == VALUE_ARY) {
		if (strlen("(array)") + 1 > length) return VALUE_ERROR;
		sprintf(buffer, "(array");
		size_t added_len = strlen(buffer);
		char *ptr = buffer + added_len;
		size_t i, ptrlen = length - added_len;
		
		for (i = 0; i < op.core.u_a.length; ++i) {
			if (ptrlen < 3) return VALUE_ERROR;
			*(ptr++) = ' '; --ptrlen;
			
			int error_p = value_put(ptr, ptrlen, op.core.u_a.a[i], format);
			if (error_p) return error_p;
			added_len = strlen(ptr);
			ptr += added_len;
			ptrlen -= added_len;
		}
		
		if (ptrlen < 2) return VALUE_ERROR;
		sprintf(ptr, ")");
		
	} else if (op.type == VALUE_LST) {
		if (strlen("(list)") + 1 > length) return VALUE_ERROR;
		sprintf(buffer, "(list");
		size_t added_len = strlen(buffer);
		char *ptr = buffer + added_len;
		size_t ptrlen = length - added_len;
		
		value opptr;
		for (opptr = op; opptr.type != VALUE_NIL; opptr = opptr.core.u_l[1]) {
			if (ptrlen < 3) return VALUE_ERROR;
			*(ptr++) = ' '; --ptrlen;
			
			int error_p = value_put(ptr, ptrlen, opptr.core.u_l[0], format);
			if (error_p) return error_p;
			added_len = strlen(ptr);
			ptr += added_len;
			ptrlen -= added_len;
		}
		
		if (ptrlen < 2) return VALUE_ERROR;
		sprintf(ptr, ")");
		
	} else if (op.type == VALUE_PAR) {
		int error_p = value_put(buffer, length, op.core.u_p->head, format);
		if (error_p) return VALUE_ERROR;
		size_t added_len = strlen(buffer);
		char *ptr = buffer + added_len;
		size_t ptrlen = length - added_len;
		
		if (ptrlen < 6) return VALUE_ERROR;
		sprintf(ptr, " -> ");
		
		added_len = strlen(ptr);
		ptr += added_len;
		ptrlen -= added_len;
		
		error_p = value_put(ptr, ptrlen, op.core.u_p->tail, format);
		if (error_p) return error_p;	
		
	} else if (op.type == VALUE_HSH) {
		if (4 > length) return VALUE_ERROR;
		sprintf(buffer, "(");
		size_t added_len = strlen(buffer);
		char *ptr = buffer + added_len;
		size_t i, j, ptrlen = length - added_len;
		int first_p = TRUE;
		
		for (i = 0; i < op.core.u_h.length; ++i) 
		if (op.core.u_h.a[i].type != VALUE_NIL) {
			for (j = 0; j < op.core.u_h.a[i].core.u_a.length; ++j)
			if (op.core.u_h.a[i].core.u_a.a[j].type != VALUE_NIL) {
				if (first_p) {
					if (ptrlen < 3) return VALUE_ERROR;
					first_p = FALSE;
				} else {
					if (ptrlen < 4) return VALUE_ERROR;
					*(ptr++) = ','; --ptrlen;		
					*(ptr++) = ' '; --ptrlen;		
				}

				
				int error_p = value_put(ptr, ptrlen, op.core.u_h.a[i].core.u_a.a[j].core.u_p->head, format);
				if (error_p) return error_p;
				added_len = strlen(ptr);
				ptr += added_len;
				ptrlen -= added_len;

				if (ptrlen < 5) return VALUE_ERROR;
				*(ptr++) = ' '; --ptrlen;
				*(ptr++) = '-'; --ptrlen;
				*(ptr++) = '>'; --ptrlen;
				*(ptr++) = ' '; --ptrlen;
				
				error_p = value_put(ptr, ptrlen, op.core.u_h.a[i].core.u_a.a[j].core.u_p->tail, format);
				if (error_p) return error_p;
				added_len = strlen(ptr);
				ptr += added_len;
				ptrlen -= added_len;
			}
		}
		
		if (ptrlen < 2) return VALUE_ERROR;
		sprintf(ptr, ")");
		
	} else if (op.type == VALUE_PTR) {
		
		// WARNING: This has not been tested.
		
		if (strlen("pointer ()") + 1 > length) return VALUE_ERROR;
		sprintf(buffer, "pointer (");
		size_t added_len = strlen(buffer);
		
		int error_p = value_put(buffer+added_len, length-added_len, *op.core.u_ptr, format);
		if (error_p) return error_p;
		
		added_len = strlen(buffer);
		if (added_len + 2 > length) return VALUE_ERROR;
		sprintf(buffer+added_len, ")");
		
	} else if (op.type == VALUE_RNG) {
		int error_p = value_put(buffer, length, op.core.u_r->min, format);
		if (error_p) return VALUE_ERROR;
		size_t added_len = strlen(buffer);
		char *ptr = buffer + added_len;
		size_t ptrlen = length - added_len;
		
		if (ptrlen < 4) return VALUE_ERROR;
		if (op.core.u_r->inclusive_p)
			sprintf(ptr, "..");
		else sprintf(ptr, "...");
		
		added_len = strlen(ptr);
		ptr += added_len;
		ptrlen -= added_len;
		
		error_p = value_put(ptr, ptrlen, op.core.u_r->max, format);
		if (error_p) return error_p;
			
	} else if (op.type == VALUE_BLK) {
		if (strlen("()") + 1 > length) return VALUE_ERROR;
		sprintf(buffer, "(");
		size_t added_len = strlen(buffer);
		char *ptr = buffer + added_len;
		size_t i, ptrlen = length - added_len;
		
		if (op.core.u_blk.s) {
			if (strlen(op.core.u_blk.s) + 2 > ptrlen) return VALUE_ERROR;
			sprintf(ptr, "%s)", op.core.u_blk.s);
			
		} else {
			for (i = 0; i < op.core.u_blk.length; ++i) {
				if (ptrlen < 3) return VALUE_ERROR;
				if (i) { *(ptr++) = ' '; --ptrlen; }
				
				int error_p = value_put(ptr, ptrlen, op.core.u_blk.a[i], format);
				if (error_p) return error_p;
				added_len = strlen(ptr);
				ptr += added_len;
				ptrlen -= added_len;
			}
			
			if (ptrlen < 2) return VALUE_ERROR;
			sprintf(ptr, ")");
			added_len = strlen(ptr);
			ptr += added_len;
			ptrlen -= added_len;
		}
		
		*ptr = '\0';
		
	} else if (op.type == VALUE_NAN) {
		if (strlen("NaN") > length + 1) return VALUE_ERROR;
		sprintf(buffer, "NaN");
		
	} else if (op.type == VALUE_INF) {
		if (strlen("infinity") > length + 1) return VALUE_ERROR;
		sprintf(buffer, "infinity");
		
	} else if (op.type == VALUE_TYP) {
		const char *tstr = type_to_string(op.core.u_type);
		if (strlen(tstr) > length + 1) return VALUE_ERROR;
		strcpy(buffer, tstr);
	
	} else if (op.type == VALUE_ID) {
		if (strlen(op.core.u_id) + 1 > length) return VALUE_ERROR;
		sprintf(buffer, "%s", op.core.u_id);		
	
	} else if (op.type == VALUE_VAR) {
		if (strlen(op.core.u_var) + 1 > length) return VALUE_ERROR;
		sprintf(buffer, "%s", op.core.u_var);
	
	} else if (op.type == VALUE_SPEC) {
		if (length < 100) return VALUE_ERROR;
		sprintf(buffer, "Spec [ changes cope? %d, needs variables? %d, keep arg? %d, delay eval? %d, argc %d, optional %d, rest? %d, associativity %c, precedence %d ]", 
				op.core.u_spec.change_scope_p, op.core.u_spec.needs_variables_p, op.core.u_spec.keep_arg_p, op.core.u_spec.delay_eval_p, 
				op.core.u_spec.argc, op.core.u_spec.optional, op.core.u_spec.rest_p, op.core.u_spec.associativity, op.core.u_spec.precedence);
		
		
	} else if (op.type == VALUE_BIF) {
		value *ref = value_hash_get_ref(primitive_names, op);
		if (ref == NULL)
			sprintf(buffer, "unknown");
		else {
			int error_p = value_put(buffer, length, *ref, format);
			if (error_p) return error_p;
		}
		
	} else if (op.type == VALUE_UDF || op.type == VALUE_UDF_SHELL) {
		if (op.core.u_udf->name == NULL) {
			if (strlen("(unnamed)") + 1 >= length) return VALUE_ERROR;
			else sprintf(buffer, "(unnamed)");
		} else if (strlen(op.core.u_udf->name) + 1 >= length) return VALUE_ERROR;
		else sprintf(buffer, "%s", op.core.u_udf->name);
		
		if (specifier == 's') {
			size_t added_len = strlen(buffer);
			int error_p = value_put(buffer + added_len, length - added_len, op.core.u_udf->vars, format);
			if (error_p) return VALUE_ERROR;
		}
		
		// Print out the body of the function.
//		added_len += strlen(buffer + added_len);
//		error_p = value_put(buffer + added_len, length - added_len, op.core.u_udf->body, format);
//		if (error_p) return VALUE_ERROR;	
		
	} else if (op.type == VALUE_ERROR) {
		if (strlen("error") > length + 1) return VALUE_ERROR;
		sprintf(buffer, "error");
	} else {
		if (strlen("unknown") > length + 1) return VALUE_ERROR;
		sprintf(buffer, "unknown");		
	}
	
	size_t buflen = strlen(buffer);
	
	if (print_type) {
		char *ptr = buffer + buflen;
		if (length - (ptr - buffer) <= strlen(type_to_string(op.type))) return VALUE_ERROR;
		sprintf(ptr, ":%s", type_to_string(op.type));
		buflen += strlen(ptr);
	}
	
	if (buflen > width) {
		size_t middle = width / 2 - 2;
		if (width - middle < 5) middle = width - 5;
		if (middle < 0) middle = 0;
		
		if (length - middle <= 5) return VALUE_ERROR;
		if (width < 5) width = 5;
		sprintf(buffer + middle, "(...)%s", buffer + buflen - (width - middle - 5));
	}
	
	buffer[width] = '\0';
	
	return 0;
}

/* 
 * This returns the address of a local variable, which is kind of a bad idea. But it's also 
 * currently the best way to get a string.
 */
char * value_to_string(value op)
{
	size_t buf_len = BUFSIZE;
	
	while (buf_len <= BIGBUFSIZE) {
		char buffer[buf_len];
		int error_p = value_put(buffer, buf_len, op, NULL);
		if (error_p == 0) return buffer;
		
		buf_len <<= 4;
	}
	
	value_error(1, "IO Error: In to_string(), op is too big to output.");
	return value_to_string(value_init_error());
}

/* 
 * A NULL string indicates that there was an error.
 */
char * value_to_string_base(value op, value base)
{
	char str[BIGBUFSIZE];
	
	int error_p = FALSE;
	if (base.type != VALUE_MPZ) {
		value_error(1, "Type Error: to_string_base() is undefined where base is %ts (integer expected).", base);
		error_p = TRUE;
	}
	
	value zero = value_set_long(0), 
		one = value_set_long(1), 
		thirty_six = value_set_long(36);
	if (error_p == FALSE && (value_lt(base, zero) || value_eq(base, one) || value_gt(base, thirty_six))) {
		value_error(1, "Type Error: to_string_base() is undefined where base equals %s (0 or 2-36 expected).", base);
		error_p = TRUE;
	}
	value_clear(&zero); value_clear(&one); value_clear(&thirty_six);
	
	int b = (int) value_get_ulong(base);
	
	if (op.type == VALUE_MPZ) {
		if (error_p == FALSE)
			mpz_get_str(str, b, op.core.u_mz);
	} else {
		value_error(1, "Type Error: to_string_base() is undefined where op is %ts (integer expected).", op);
		error_p = TRUE;
	}
	
	if (error_p)
		return NULL;
	return str;
}

char * value_to_string_safe(value op)
{
	size_t buf_len = BUFSIZE;
	
	while (buf_len <= BIGBUFSIZE) {
		char *buffer = value_malloc(NULL, buf_len);
		if (buffer == NULL) return NULL;
		int error_p = value_put(buffer, buf_len, op, NULL);
		if (error_p == 0) return buffer;
		
		buf_len <<= 4;
		value_free(buffer);
	}
	
	value_error(1, "IO Error: In to_string(), op is too big to output.");
	return value_to_string_safe(value_init_error());
}

int value_print(value op)
{
	return value_printf("%v", op);
}

int value_println(value op)
{
	value_print(op);
	printf("\n");
	return 0;
}

int value_printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return value_vfprintf(stdout, format, ap);
}

int value_fprintf(FILE *fp, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);	
	return value_vfprintf(fp, format, ap);
}

int value_vfprintf(FILE *fp, const char *format, va_list ap)
{
	return value_vprintf_generic(fp, NULL, format, 0, NULL, ap);
}

int value_sprintf(char *str, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return value_vprintf_generic(NULL, str, format, 0, NULL, ap);
}

int value_vprintf_generic(FILE *fp, char *str, const char *format, int argc, value *argv, va_list ap)
{
	value op;
	const char *ptr = format - 1;
	int argi = 0;
	
	while (*(++ptr)) {
		if (*ptr != '%') {
			if (fp) fputc(*ptr, fp);
			else *(str++) = *ptr;
			continue;
		}
		
		if (argv) {
			if (argi >= argc) {
				value_error(1, "Error: In formatted output, too many format tags.");
				return VALUE_ERROR;
			}
			op = argv[argi++];
		} else op = va_arg(ap, value);
		
		// Put the format into the one_fmt string, to be passed into value_put().
		char one_fmt[BUFSIZE];
		size_t i = 0;
		while (*(++ptr)) {
			one_fmt[i++] = *ptr;
			if (!(*ptr == '.' || isdigit(*ptr) || *ptr == 'r' || *ptr == 't')) {
				++ptr;
				break;
			}
		}
		one_fmt[i] = '\0';
		
		--ptr;
		
		if (one_fmt[0] == '%') {
			--argi; // %% doesn't count as a format tag.
			if (fp) fprintf(fp, "%%");
			else *(str++) = '%';
			continue;
		}
		
		size_t buf_len = BUFSIZE;
		
		while (buf_len <= BIGBUFSIZE) {
			char buffer[buf_len];
			int error_p = value_put(buffer, buf_len, op, one_fmt);
			if (error_p == 0) {
				if (fp) {
					fprintf(fp, "%s", buffer);
				} else {
					sprintf(str, "%s", buffer);
					str += strlen(str);
				}
				break;
			} else if (error_p != VALUE_ERROR) {
				// There was an error not related to buffer size.
				char error_buf[100];
				
				if (error_p == -2) sprintf(error_buf, "(error in output: undefined format specifier %s)", one_fmt);
				else if (error_p == -3) sprintf(error_buf, "(error in output: ran out of memory)");
				else if (error_p == -4) sprintf(error_buf, "(error in output: error in mpfr_get_str())");
				
				if (fp) {
					fprintf(fp, error_buf);
				} else {
					sprintf(str, error_buf);
				}
				if (argv == NULL) va_end(ap);
				return VALUE_ERROR;
			}
			
			buf_len <<= 4;
		}
		
		if (buf_len > BIGBUFSIZE) {
			value_error(1, "IO Error: In print, op is too big to output.");
			va_end(ap);
			return VALUE_ERROR;
		}
	}
	
	va_end(ap);
	
	return 0;
}

int value_error(int severity, char *format, ...)
{
	if (print_errors_p == FALSE)
		return 1;
	
	va_list ap;
	va_start(ap, format);

	error_count += severity;
	
	if (linenum != 0)
		fprintf(stderr, "From line %d, ", linenum);
	char *ptr = format - 1;
	while (*(++ptr)) {
		if (*ptr != '%') {
			fputc(*ptr, stderr);
			continue;
		}
		
		++ptr;
		if (*ptr == 'd') {
			int val = va_arg(ap, int);
			fprintf(stderr, "%d", val);
		} else if (*ptr == 'c') {
			char *val = va_arg(ap, char *);
			fprintf(stderr, "%s", val);
		} else if (*ptr == 'l') {
			++ptr;
			if (*ptr == 'd') {
				long val = va_arg(ap, long);
				fprintf(stderr, "%ld", val);
			}
		} else {
			char one_fmt[50];
			int i = 0;
			one_fmt[i++] = '%';
			while (*ptr == '.' || isdigit(*ptr) || *ptr == 'r' || *ptr == 't')
				one_fmt[i++] = *ptr++;
			one_fmt[i++] = *ptr;
			one_fmt[i] = '\0';
			value_vfprintf(stderr, one_fmt, ap);
		}

	}
	
	fprintf(stderr, "\n");
	va_end(ap);
	
	return 0;
}

value value_print_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "print"))
		return value_init_error();
	value_print(argv[0]);
	return value_set(argv[0]);
}

value value_println_arg(int argc, value argv[])
{
	int new_p = FALSE;
	if (argv[0].type == VALUE_MISSING_ARG) {
		new_p = TRUE;
		argv[0] = value_set_str("");
	}
	if (missing_arguments(argc, argv, "println"))
		return value_init_error();
	value_println(argv[0]);
	if (new_p)
		return argv[0];
	else return value_set(argv[0]);
}

value value_printf_arg(int argc, value argv[])
{
	if (missing_arguments(argc, argv, "println"))
		return value_init_error();
	if (argv[0].type != VALUE_STR) {
		value_error(1, "Type Error: printf() is undefined where op1 is %ts (string expected.", argv[0]);
	}
	int error_p = value_vprintf_generic(stdout, NULL, argv[0].core.u_s, argc-1, argv+1, 0);
	if (error_p) return value_init_error();
	return value_init_nil();
}

int print_values(value words[], size_t length)
{
	size_t i;
	for (i = 0; i < length; ++i)
		value_printf("%s ", words[i]);
	printf("\n");
	return 0;
}
