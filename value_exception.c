/*
 *  value_exception.c
 *  Calculator
 *
 *  Created by Michael Dickens on 7/13/10.
 *
 */

#include "value.h"

exception exception_init(exception *parent, char *name)
{
	exception res;
	res.parent = parent;
	res.name = value_malloc(NULL, strlen(name) + 1);
	if (res.name) strcpy(res.name, name);
	res.description = NULL;
	res.stack_trace = NULL;
	return res;
}

value value_throw(exception op, char *description)
{
	value exc;
	exc.core.u_exc = op;
	value res = value_set(exc);
	if (res.core.u_exc.description)value_free(res.core.u_exc.description);
	res.core.u_exc.description = value_malloc(NULL, strlen(description) + 1);
	return_if_null(res.core.u_exc.description);
	strcpy(res.core.u_exc.description, description);
	return res;
}