/*
 * misc001.c
 * 
 * Several test running together
 * 
 * (C) Arjan van de Ven <arjan@fenrus.demon.nl> , 2000
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "misc_lib.h"

/*
	Writes a random-access stream to a file  
*/
#define FILENAME "./bigfile"

int pagesize;
int ramsize = MEGAS(RAMSIZE);

static void dirty_buffer(void)
{
	FILE *file;
	char *buffer;
	int  i;

	unlink(FILENAME);
	file = fopen(FILENAME, "w+");
	assert(file != NULL);
	
	buffer = safe_malloc(ramsize/2, "dirty_buffer");
	
	fwrite(buffer, 1, ramsize/2, file);
	
	free(buffer);
		
	buffer = safe_malloc(pagesize, "dirty_buffer");
	
	for (i=0; i < pagesize; i++) {
	 	buffer[i] = i&255;
	}

	printf("Starting dirty buffer\n");	
	while (1) {
		int pos,result;
		
		pos = rand() % (ramsize/pagesize/2);
		
		result = fseek(file, pagesize * pos, SEEK_SET);
		assert(result==0);
		
		result = fwrite(buffer, 1, pagesize, file);
		assert(result == pagesize);
		
	}
	fclose(file);	
	free(buffer);
}

/* 
   	malloc()'s small pieces of memory, dirties them and then
	frees that memory
*/

static void malloc_test(void)
{
	printf("Starting malloc test\n");	
	
	while (1) {
		int blocksize;
		char *buffer;
		
		blocksize = rand() % (pagesize*1024);
		buffer = safe_malloc(blocksize, "malloc_test");
		memset(buffer,rand()%255, blocksize);
		free(buffer);
	}
}

/*
  	mmaps() /dev/null for 60% of the available physical ram, mremaps
	and dirties it.
*/

static void mmap_test(void)
{
	printf("Starting mmap test\n");	
	while (1) {
		char *buffer;
		int fd, result;
		int i;
		
                fd = open("/dev/zero", O_RDWR);

		buffer = mmap(NULL, ramsize/2, PROT_READ|PROT_WRITE| PROT_EXEC,
		              MAP_PRIVATE,fd,0);
		if (buffer == MAP_FAILED) {
			printf("%s\n", strerror(errno));
			continue;
		}
		
		for (i=0;i<ramsize/pagesize/2;i++) {
			buffer[i*pagesize] = 4;
		}
		
		result = munmap(buffer, ramsize/2);
		assert(result != -1);
		
		close(fd);
	}
}

int main(int argc, char *argv[])
{

	if(argc == 2) 
		ramsize = MEGAS(atoi(argv[1]));

	pagesize = PAGESIZE;

	if (fork()==0) {
		dirty_buffer();
		return 0;
	};

	if (fork()==0) {
		malloc_test();
		return 0;
	};


	if (fork()==0) {
		mmap_test();
		return 0;
	};
		
	sleep(10000);	
	exit(EXIT_SUCCESS);
}

