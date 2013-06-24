/*
 *  value_hash.c
 *  Simfpl
 *
 *  Created by Michael Dickens on 3/16/10.
 *
 */

/* 
 * Hash Table Implementation
 * 
 * A hash table is implemented as an array of buckets. Each bucet
 * contains a two-element value array with a key and a value. This is 
 * an implementation of the separate chaining method. An array is 
 * used to represent the bucket rather than a linked list because 
 * lists take at least five times longer to iterate over.
 * 
 * (name).core.u_h is used to store the array. On the surface, a hash 
 * is identical to an array except that its type is VALUE_HSH instead 
 * of VALUE_ARY.
 * 
 */

#include "value.h"

value value_hash_init()
{
	return value_hash_init_capacity(HASH_DEFAULT_CAPACITY);
}

value value_hash_init_capacity(size_t capacity)
{
	if (capacity == 0)
		capacity = HASH_DEFAULT_CAPACITY;
	else if (capacity < 0) {
		value_error(1, "Argument Error: Cannot initialize a hash with capacity %d.", capacity);
		return value_init_error();
	}
	
	value hash;
	
	hash.type = VALUE_HSH;
	hash.core.u_h.a = value_malloc(NULL, sizeof(value) * next_size(capacity));
	return_if_null(hash.core.u_h.a);
	hash.core.u_h.length = capacity;
	hash.core.u_h.occupied = 0;
	hash.core.u_h.size = 0;
	
	size_t i;
	for (i = 0; i < capacity; ++i)
		hash.core.u_h.a[i] = value_init_nil();
				
	return hash;
}

void value_hash_clear(value *hash)
{
	if (hash->type != VALUE_HSH) {
		value_error(1, "Type Error: hash_clear() is undefined where hash is %ts (hash expected).", *hash);
		return;
	}
	size_t i, length = value_hash_length(*hash);
	
	for (i = 0; i < length; ++i)
		value_clear(&hash->core.u_h.a[i]);
	
value_free(hash->core.u_h.a);
	hash->type = VALUE_NIL;
}

/* The length of the array used to represent the hash.
 */
size_t value_hash_length(value hash)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_length() is undefined where hash is %ts (hash expected).", hash);
		return -1;
	}
	return hash.core.u_h.length;
}

/* A count of the number of currently occupied buckets.
 */
size_t value_hash_occupied(value hash)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_occupied() is undefined where hash is %ts (hash expected).", hash);
		return -1;
	}
	return hash.core.u_h.occupied;
}

/* The number of elements in the hash.
 */
size_t value_hash_size(value hash)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_size() is undefined where hash is %ts (hash expected).", hash);
		return -1;
	}
	return hash.core.u_h.size;
}

value value_hash_resize(value *hash)
{
	if (hash->type != VALUE_HSH) {
		value_error(1, "Type Error: hash_resize() is undefined where hash is %ts (hash expected).", *hash);
		return value_init_error();
	}
	size_t inner_len, length = value_hash_length(*hash);
	
	value new = value_hash_init_capacity(next_size(length * 4));
	size_t i, j;
	for (i = 0; i < length; ++i) {
		if (hash->core.u_h.a[i].type == VALUE_NIL)
			continue;
		inner_len = value_length(hash->core.u_h.a[i]);
		for (j = 0; j < inner_len; ++j)
			value_hash_put_refs(&new, &hash->core.u_h.a[i].core.u_a.a[j].core.u_p->head, &hash->core.u_h.a[i].core.u_a.a[j].core.u_p->head);
	}
	
	// Don't clear the keys and the values, because they are now references in the new hash. But clear everything else.
//	value_hash_clear(hash);
	for (i = 0; i < length; ++i) {
		if (hash->core.u_h.a[i].type == VALUE_NIL)
			continue;
		inner_len = value_length(hash->core.u_h.a[i]);
		for (j = 0; j < inner_len; ++j)
			value_free(hash->core.u_h.a[i].core.u_a.a[j].core.u_p);
		value_free(hash->core.u_h.a[i].core.u_a.a);
	}
	
	value_free(hash->core.u_h.a);
	
	*hash = new;
	
	return value_init_nil();
}

value value_hash_put(value *hash, value key, value val)
{
	value k = value_set(key), v = value_set(val);
	return value_hash_put_refs(hash, &k, &v);
}

value value_hash_put_refs(value *hash, value *key, value *val)
{
	return value_hash_put_refs_generic(hash, key, val, TRUE);
}

value value_hash_put_refs_override(value *hash, value *key, value *val)
{
	return value_hash_put_refs_generic(hash, key, val, FALSE);
}

value value_hash_put_refs_generic(value *hash, value *key, value *val, int clear_p)
{
	if (hash->type != VALUE_HSH) {
		value_error(1, "Type Error: hash_put_refs() is undefined where hash is %ts (hash expected).", *hash);
		return value_init_error();
	}
	
	size_t length = value_hash_length(*hash);
	if (length == 0) {
		value_hash_clear(hash);
		*hash = value_hash_init();
		length = value_hash_length(*hash);
	}
	size_t index = value_private_hash_function(*key) % length;
	
	int count;
	if (hash->core.u_h.a[index].type == VALUE_NIL) {
		// This bucket hasn't been initialized. Initialize it, and add the first element.
		value ary[] = { *key, *val };
		
		hash->core.u_h.a[index].type = VALUE_ARY;
		hash->core.u_h.a[index].core.u_a.a = value_malloc(NULL, sizeof(value) * next_size(1));
		if (hash->core.u_h.a[index].core.u_h.a == NULL) {
			return value_init_nil();
		}
		hash->core.u_h.a[index].core.u_a.length = 1;
		hash->core.u_h.a[index].core.u_a.a[0] = value_set_ary_ref(ary, 2);
		++hash->core.u_h.occupied;
		count = 1;
	} else {
		if (hash->core.u_h.a[index].core.u_a.length == 0)
			++hash->core.u_h.occupied;
		
		count = value_private_put_pair_in_bucket(&hash->core.u_h.a[index], key, val, clear_p);
		if (count == VALUE_ERROR) {
			return value_init_error();
		}
	}

	hash->core.u_h.size += count;

	/* If more than 75% of the indices are occupied, it's to make a new, bigger hash. */
	if ((hash->core.u_h.occupied * 100) / length > 75) {
		value_hash_resize(hash);
	}
	
	return value_init_nil();
}

value value_hash_put_str(value *hash, const char *key, value val)
{
	if (hash->type != VALUE_HSH) {
		value_error(1, "Type Error: hash_put_str() is undefined where hash is %ts (hash expected).", *hash);
		return value_init_error();
	}

	value vkey;
	
	vkey.type = VALUE_STR;
	value_malloc(&vkey, strlen(key) + 1);
	if (vkey.type == VALUE_ERROR) return vkey;
	strcpy(vkey.core.u_s, key);
	value res = value_hash_put(hash, vkey, val);
	value_free(vkey.core.u_s);
	
	return res;
}

value value_hash_put_var(value *hash, const char *key, value val)
{
	if (hash->type != VALUE_HSH) {
		value_error(1, "Type Error: hash_put_str() is undefined where hash is %ts (hash expected).", *hash);
		return value_init_error();
	}

	value vkey;
	
	vkey.type = VALUE_VAR;
	value_malloc(&vkey, strlen(key) + 1);
	if (vkey.type == VALUE_ERROR) return vkey;
	strcpy(vkey.core.u_s, key);
	value res = value_hash_put(hash, vkey, val);
	value_free(vkey.core.u_s);
	
	return res;
}

value value_hash_put_str_str(value *hash, const char *key, const char *val)
{
	if (hash->type != VALUE_HSH) {
		value_error(1, "Type Error: hash_put_str_str() is undefined where hash is %ts (hash expected).", *hash);
		return value_init_error();
	}
	value vkey;
	
	vkey.type = VALUE_STR;
	value_malloc(&vkey, strlen(key) + 1);
	if (vkey.type == VALUE_ERROR) return vkey;
	strcpy(vkey.core.u_s, key);
	
	value vval;
	
	vval.type = VALUE_STR;
	value_malloc(&vval, strlen(val) + 1);
	if (vval.type == VALUE_ERROR) return vval;
	strcpy(vval.core.u_s, val);
	
	return value_hash_put_refs(hash, &vkey, &vval);
}

int value_hash_exists(value hash, value key)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_exists() is undefined where hash is %ts (hash expected).", hash);
		return FALSE;
	}
	size_t i, length = value_hash_length(hash);
	if (length == 0)
		return FALSE;
	size_t index = value_private_hash_function(key) % length;
	if (hash.core.u_h.a[index].type == VALUE_NIL)
		return FALSE;
	size_t inner_len = value_length(hash.core.u_h.a[index]);
	for (i = 0; i < inner_len; ++i)
		if (value_eq(hash.core.u_h.a[index].core.u_a.a[i].core.u_p->head, key))
			return TRUE;
	return FALSE;
}

int value_hash_exists_str(value hash, char *key)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_exists_str() is undefined where hash is %ts (hash expected).", hash);
		return FALSE;
	}
	value vkey;
	
	vkey.type = VALUE_STR;
	value_malloc(&vkey, strlen(key) + 1);
	if (vkey.type == VALUE_ERROR) return FALSE;
	strcpy(vkey.core.u_s, key);
	int res = value_hash_exists(hash, vkey);
value_free(vkey.core.u_s);
	return res;
}

int value_contains_value(value op, value val)
{
	if (op.type != VALUE_HSH) {
		value_error(1, "Type Error: contains_value() is undefined where op is %ts (hash expected.", op);
		return FALSE;
	}
	
	size_t i, length = value_hash_length(op);
	size_t index = value_private_hash_function(val) % length;
	if (op.core.u_h.a[index].type == VALUE_NIL)
		return FALSE;
	size_t inner_len = value_length(op.core.u_h.a[index]);
	for (i = 0; i < inner_len; ++i)
		if (value_eq(op.core.u_h.a[index].core.u_a.a[i].core.u_p->tail, val))
			return TRUE;
	
	return FALSE;
}

value value_contains_value_std(value op, value val)
{
	if (op.type != VALUE_HSH) {
		value_error(1, "Type Error: contains_value() is undefined where hash is %ts (hash expected).", op);
		return value_init_error();
	}
	return value_set_bool(value_contains_value(op, val));
}

value value_contains_value_arg(int argc, value argv[])
{
	return value_contains_value_std(argv[0], argv[1]);
}

value value_hash_delete_at(value *hash, value key)
{
	if (hash->type != VALUE_HSH) {
		value_error(1, "Type Error: hash_delete_at() is undefined where hash is %ts (hash expected).", *hash);
		return value_init_error();
	}
	
	size_t length = hash->core.u_h.length;
	if (length == 0)
		return value_init_nil();
	size_t index = value_private_hash_function(key) % length;
	if (hash->core.u_h.a[index].type == VALUE_NIL)
		return value_init_nil();
	
	value res = value_init_nil();
	value ptr = hash->core.u_h.a[index];
	
	size_t i;
	for (i = 0; i < ptr.core.u_a.length; ++i) {
		if (value_eq(ptr.core.u_a.a[i].core.u_p->head, key)) {
			--hash->core.u_h.size;
			--hash->core.u_h.a[index].core.u_a.length;
			res = ptr.core.u_a.a[i];
//			memmove(ptr.core.u_a.a + i, ptr.core.u_a.a + i + 1, ptr.core.u_a.length - i);
			for (; i < ptr.core.u_a.length; ++i)
				ptr.core.u_a.a[i] = ptr.core.u_a.a[i+1];
			
			if (hash->core.u_h.a[index].core.u_a.length == 0)
				--hash->core.u_h.occupied;
		}
	}
	
	return res;
}

void value_hash_delete_at_void(value *hash, value key)
{
	value clr = value_hash_delete_at(hash, key);
	value_clear(&clr);
}

/* 
 * Returns a reference to the object.
 * 
 * Warning: If the value is not found, returns NULL. Also returns NULL if there is 
 * an error. But the only case in which an error will return NULL is when (hash) is  
 * not a VALUE_HSH, so that should be easily avoidable.
 */
value * value_hash_get_ref(value hash, value key)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_get_ref() is undefined where hash is %ts (hash expected).", hash);
		return NULL;
	}
	size_t length = value_hash_length(hash);
	if (length == 0)
		return NULL;
	size_t index = value_private_hash_function(key) % length;
	if (hash.core.u_h.a[index].type == VALUE_NIL)
		return NULL;
	value pair = value_private_find_pair_in_bucket(hash.core.u_h.a[index], key);
	if (pair.type == VALUE_NIL)
		return NULL;
	return &pair.core.u_p->tail;
}

/* Returns nil if the key is not found.
 */
value value_hash_get(value hash, value key)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_get() is undefined where hash is %ts (hash expected).", hash);
		return value_init_error();
	}
	value *ref = value_hash_get_ref(hash, key);
	if (ref == NULL) return value_init_nil();
	return value_set(*ref);
}

value * value_hash_get_ref_str(value hash, char *key)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_get_ref_str() is undefined where hash is %ts (hash expected).", hash);
		return NULL;
	}
	value vkey;
	
	vkey.type = VALUE_STR;
	value_malloc(&vkey, strlen(key) + 1);
	if (vkey.type == VALUE_ERROR) return NULL;
	strcpy(vkey.core.u_s, key);
	value *res = value_hash_get_ref(hash, vkey);
	value_free(vkey.core.u_s);
	return res;
}

value value_hash_get_str(value hash, char *key)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_get_str() is undefined where hash is %ts (hash expected).", hash);
		return value_init_error();
	}
	value vkey;
	
	vkey.type = VALUE_STR;
	value_malloc(&vkey, strlen(key) + 1);
	return_if_error(vkey);
	strcpy(vkey.core.u_s, key);
	value res = value_hash_get(hash, vkey);
	value_free(vkey.core.u_s);
	return res;
}

value * value_hash_get_pair_ref(value hash, value key)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_get_pair_ref() is undefined where hash is %ts (hash expected).", hash);
		return NULL;
	}
	size_t length = value_hash_length(hash);
	if (length == 0)
		return NULL;
	size_t index = value_private_hash_function(key) % length;
	if (hash.core.u_h.a[index].type == VALUE_NIL)
		return NULL;
	value *pair = value_malloc(NULL, sizeof(value));
	if (pair == NULL) return pair;
	*pair = value_private_find_pair_in_bucket(hash.core.u_h.a[index], key);
	if (pair->type == VALUE_NIL)
		return NULL;
	
	return pair;
}

value value_hash_get_pair(value hash, value key)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: get_pair() is undefined where hash is %ts (hash expected).", hash);
		return value_init_error();
	}
	value *ref = value_hash_get_pair_ref(hash, key);
	if (ref == NULL) return value_init_nil();
	return value_set(*ref);
}

/* Assumes that bucket is of the correct type.
 * 
 * A return value of 0 indicates that a previous value was overwritten. 1 indicates 
 * that a new pair was created.
 */
int value_private_put_pair_in_bucket(value *bucket, value *key, value *val, int clear_p)
{	
	size_t i, length = value_length(*bucket);
	for (i = 0; i < length; ++i) {
		if (bucket->core.u_a.a[i].type == VALUE_ARY && value_eq(bucket->core.u_a.a[i].core.u_p->head, *key)) {
			if (clear_p) {
				value_clear(&bucket->core.u_a.a[i].core.u_p->head);
				value_clear(&bucket->core.u_a.a[i].core.u_p->tail);
			}
			bucket->core.u_a.a[i].core.u_p->head = *key;
			bucket->core.u_a.a[i].core.u_p->tail = *val;
			return 0;
		}
	}
	
	// Add the new pair to the end of the array.
	value pair;
	
	pair.type = VALUE_PAR;
	// The pair will never be reallocated so it's (* 2) and not (* next_size(2)).
	pair.core.u_p = value_malloc(NULL, sizeof(struct value_pair));
	if (pair.core.u_a.a == NULL) return VALUE_ERROR;
	pair.core.u_p->head = *key;
	pair.core.u_p->tail = *val;
		
	value_append_now2(bucket, &pair);
	return 1;
}

/* Assumes that (bucket) is already of type VALUE_ARY and that each element 
 * of (bucket) is a two-element array.
 */
value value_private_find_pair_in_bucket(value bucket, value key)
{
	size_t i, length = value_length(bucket);
	for (i = 0; i < length; ++i)
		if (value_eq(bucket.core.u_a.a[i].core.u_p->head, key))
			return bucket.core.u_a.a[i];
	return value_init_nil();
}

int value_private_hash_function_old(value op)
{
	int hash = 5381;

	char *tmp = value_to_string(op);
	--tmp;
	
	while (*(++tmp))
		hash = ((hash << 5) + hash) + *tmp;
		
	// Just in case it overflowed, return the absolute value.
	return (size_t) abs(hash);
}

size_t value_private_hash_function(value op)
{
	size_t hash = 5381 - VALUE_STR + op.type;
	char *tmp;
	
	switch (op.type) {
		case VALUE_NIL:
		case VALUE_INF:
		case VALUE_ERROR:
			break;
		case VALUE_BOO:
			hash += op.core.u_b;
			break;
		case VALUE_MPZ:
			// The number is randomized by a single iteration of a Linear Congruential 
			// Generator. It's likely that the most common numbers will be close 
			// together (e.g. 0, 1, 2, 3) so this will separate them.
			hash += mpz_get_si(op.core.u_mz) * 1103515245 + 12345;
			break;
		case VALUE_MPF:
			// A similar idea to MPZ. Adds INT_MAX/2 so that 1, 2, 3 won't hash to the 
			// same things as 1.0, 2.0, 3.0.
			hash += (size_t) ((mpfr_get_d(op.core.u_mf, value_mpfr_round) + INT_MAX/2) * 1103515245 + 12345);
			break;
		case VALUE_STR:
		case VALUE_ID:
		case VALUE_VAR:
			tmp = op.core.u_s - 1;
			while (*(++tmp))
				hash = ((hash << 5) + hash) + *tmp;
			break;
		case VALUE_ARY:
		case VALUE_HSH:
		case VALUE_BLK:
			;
			size_t i;
			// What (length / 10) + 1 does is that it ensures that no more than 
			// ten elements out of the array will be hashed. Doing more than that 
			// would be slow and not provide much benefit.
			for (i = 0; i < op.core.u_h.length; i += (op.core.u_h.length / 10) + 1)
				hash += value_private_hash_function(op.core.u_a.a[i]);
			hash += op.core.u_h.length;
			break;
		case VALUE_LST:
			hash += value_private_hash_function(op.core.u_l[0]);
			hash += value_private_hash_function(op.core.u_l[1]);
			break;
		case VALUE_PAR:
			hash += value_private_hash_function(op.core.u_p->head);
			hash += value_private_hash_function(op.core.u_p->tail); 
			break;
		case VALUE_PTR:
			hash += value_private_hash_function(*op.core.u_ptr);
			break;
		default:
			break;
	}
	
	// Just in case it overflowed, return the absolute value.
	return (size_t) abs(hash);	
}

int value_hash_print(value hash)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_print() is undefined where hash is %ts (hash expected).", hash);
		return -1;
	}
	size_t i, length = value_hash_length(hash);
	printf("{ ");
	for (i = 0; i < length; ++i) {
		if (hash.core.u_h.a[i].type == VALUE_NIL)
			continue;
		size_t j, inner_len = value_length(hash.core.u_h.a[i]);
		for (j = 0; j < inner_len; ++j) {
			value_print(hash.core.u_h.a[i].core.u_a.a[j]);
			printf(", ");
		}
	}
	
	printf("}");
	return 0;
}

int value_hash_println(value hash)
{
	if (hash.type != VALUE_HSH) {
		value_error(1, "Type Error: hash_clear() is undefined where hash is %ts (hash expected).", hash);
		return -1;
	}
	value_hash_print(hash);
	printf("\n");
	return 0;
}

