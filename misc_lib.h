
/*
 * misc_lib.c
 * 
 * Misc routines needed 
 * 
 * (C) Juan Quintela, 2000
 */

#ifndef __MISC_LIB_H
#define __MISC_LIB_H

#define MEGA		(1024 * 1024)
#define MEGAS(num)	((num) * MEGA)

/* in megabytes */
#define RAMSIZE	        128

#define PAGESIZE	sysconf(_SC_PAGESIZE);

void error_exit(int err_num, char *msg);
void *safe_malloc(size_t size, char *error_message);

#endif /* __MISC_LIB_H */

