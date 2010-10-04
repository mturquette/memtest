
/*
 * fillmem.c
 * 
 * Empty all the memory
 * 
 * (C) Jeff Garzik <jgarzik@mandrakesoft.com>, 2000
 */


#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include "misc_lib.h"

int main (int argc, char *argv[])
{
	void **data;
	int i, r;
	size_t megs = RAMSIZE;

	if ((argc >= 2) && (atoi(argv[1]) > 0))
		megs = atoi(argv[1]);

	data = safe_malloc (megs * sizeof (void*), "fill_mem 1");

	memset (data, 0, megs * sizeof (void*));

	srand(time(NULL));

	for (i = 0; (unsigned int)i < megs; i++) {
		data[i] = safe_malloc(MEGA, "fill_mem 2");
		memset (data[i], i, MEGA);
		printf("malloc/memset %03d/%03u\n", i+1, megs);
	}
	for (i = megs - 1; i >= 0; i--) {
		r = rand() % 200;
		memset (data[i], r, MEGA);
		printf("memset #2 %03d/%03u = %u\n", i+1, megs, r);
	}
	printf("done\n");
	return 0;
}
