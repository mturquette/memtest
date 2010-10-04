/*
 * mmap001.c
 * 
 * Tests mmapping a big file and writing it once
 * 
 * (C) Juan Quintela <quintela@fi.udc.es>, 2000
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "misc_lib.h"

#define FILENAME "testing_file"

int main(int argc, char * argv[])
{
        char *array;
	unsigned int i;
	int fd;
	int ramsize = MEGAS(RAMSIZE);

	if(argc == 2) 
		ramsize = MEGAS(atoi(argv[1]));

        unlink(FILENAME);
        fd = open(FILENAME, O_RDWR | O_CREAT, 0666);
        if ((fd == -1))
                error_exit(errno, "Problems opening files");
        
        if (lseek(fd, ramsize, SEEK_SET) != ramsize)
                error_exit(errno, "Problems doing the lseek");

        if (write(fd,"\0",1) !=1)
                error_exit(errno, "Problems writing");

	array = mmap(NULL, ramsize, PROT_WRITE | PROT_READ | PROT_EXEC,
		     MAP_SHARED, fd, 0);;

        if (array == MAP_FAILED)
                error_exit(errno, "The mmap has failed");

        for(i = 0; i < ramsize; i++) {
                array[i] = i;
        } 

        msync(array, ramsize, MS_SYNC);
        close(fd);
        unlink(FILENAME);
        exit(EXIT_SUCCESS);
}
