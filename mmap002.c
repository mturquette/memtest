/*
 * mmap002.c
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
        char *array2;
        int i;
	int ramsize = MEGAS(RAMSIZE);
        int fd = open(FILENAME, O_RDWR | O_CREAT, 0666);
        int fd2 = open("/dev/zero", O_RDWR);

	if(argc == 2) 
		ramsize = MEGAS(atoi(argv[1]));
        
        if ((fd == -1) || (fd2 == -1))
                error_exit(errno, "Problems opening files");
        
	unlink(FILENAME);
        if (lseek(fd, ramsize*2, SEEK_SET) != ramsize*2)
                error_exit(errno, "Problems doing the lseek");

        if (write(fd,"\0",1) !=1)
                error_exit(errno, "Problems writing");
 
        array = mmap(NULL, ramsize*2, PROT_WRITE | PROT_READ | PROT_EXEC,
		     MAP_SHARED, fd, 0);
        if (array == MAP_FAILED)
                error_exit(errno, "The mmap has failed");

        array2 = mmap(NULL, ramsize, PROT_WRITE | PROT_READ | PROT_EXEC,
		      MAP_PRIVATE | MAP_ANON, fd2, 0);
        if (array2 == MAP_FAILED)
                error_exit(errno, "The mmap has failed");

        for(i = 0; i < ramsize; i++) {
                array[i] = i;
        } 
        msync(array, ramsize, MS_SYNC);
        for(i = 0; i < ramsize; i++) {
                array2[i] = array[i];
        } 
        for(i = 0; i < ramsize; i++) {
                array[i+ramsize] = array2[i];
        } 

        msync(array, ramsize*2, MS_SYNC);
        close(fd);
        close(fd2);
        unlink(FILENAME);
        exit(EXIT_SUCCESS);
}
 
