
/*
 * misc_lib.c
 * 
 * Misc routines needed 
 * 
 * (C) Juan Quintela, 2000
 *     quintela@fi.udc.es
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "misc_lib.h"

void error_exit(int err_num, char *message)
{
        fprintf(stderr, "%s:%s\n", message, strerror(err_num));
        exit(EXIT_FAILURE);
}

void *safe_malloc(size_t size, char *error_message)
{
	void * result = malloc(size);
	if (!result)
                error_exit(errno, error_message);
	return result;
}




