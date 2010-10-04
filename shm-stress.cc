/*
 * test.c
 * 
 * Tests kernel handling of shared private memory.
 * 
 * (C) Stephen C. Tweedie <sct@redhat.com>, 2000
 */

#include "shm-lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/fcntl.h>

int pagesize;
int heapsize = 16 * 1024 * 1024;
int nr_pages;
int nr_children = 5;

bool writer = true;
bool do_fork = true;

ShmSeg * heap_seg, * pattern_seg;
ShmSemArray * semaphores;

char * heap;
int * patterns;

void fork_new_child(void);		/* Perform a fork/die in this child */
void fork_child(int n);			/* Spawn a new child process */
void null_handler(int);			/* Just a null signal handler */
void test_memory(void);			/* Run the test sweep over the heap */
void page_error(int);			/* Report a heap pattern mismatch */

static inline int & page_val(int page)
{
	char * page_ptr = heap + page*pagesize;
	int * int_ptr = (int *) page_ptr;
	return * int_ptr;
}

int main(void)
{
	int i;

	/* Use a pagesize of hardware page size / 256.  This allows us
           to cause colliding accesses to different words in a single
           page of shared memory. */

	pagesize = getpagesize() / 4;

	nr_pages = heapsize / pagesize;
	heap_seg = new ShmSeg (0, heapsize, O_CREAT, 0700);
	pattern_seg = new ShmSeg (0, nr_pages*sizeof(int), O_CREAT, 0700);
	semaphores = new ShmSemArray (0, nr_pages, O_CREAT, 0700);

	heap = (char *) heap_seg->address;
	patterns = (int *) pattern_seg->address;

	printf ("Initialising %dMB heap...\n", heapsize/1024/1024);
	for (i=0; i<nr_pages; i++)
		patterns[i] = page_val(i) = random();
	printf ("Done.\n");

	setpgrp();
	for (i=0; i<nr_children; i++)
		fork_child(i);

	for (;;) {
		pid_t pid;
		int status;
		
		/* Catch child error statuses and report them. */
		pid = wait3(&status, 0, 0);
		if (pid < 0)	/* No more children? */
			break;
		if (WIFEXITED (status)) {
			if (WEXITSTATUS (status))
				fprintf (stderr,
					 "Child %d exited with status %d\n",
					 pid, WEXITSTATUS(status));
			else {
#if 0
				fprintf (stderr,
					 "Child %d exited normally\n",
					 pid);
#endif
				fork_child(i++);
			}
		} else {
			fprintf (stderr,
				 "Child %d exited with signal %d\n",
				 pid, WTERMSIG(status));
		}
	}
}


void fork_child(int n)
{
	pid_t pid, parent;
	
	parent = getpid();
	signal (SIGUSR1, null_handler);

	pid = fork();
	if (pid == -1) {
		perror ("fork");
		kill (-getpgrp(), SIGTERM);
		exit(errno);
	}
	
	if (pid) {
		/* Are we the parent?  Wait for the child to print the
		   startup banner. */
		/* 		pause(); */
		return;
	} else {
		/* Are we the child?  Print a banner, then signal the parent
		   to continue. */
#if 0
		fprintf (stderr, "Child %02d started with pid %05d\n", 
			 n, getpid());
		kill (parent, SIGUSR1);
#endif
		test_memory();
	}
}

void null_handler(int n)
{
}

void test_memory(void)
{
	int count = 0;
	int time_to_live = 0;
	int page;
	
	/* Give each child a different random seed. */
	srandom(getpid() * time(0));
	
	time_to_live = 50 + random() % 50;

	for (;;) {
		/* Track the time until the next fork/die round */
		if (do_fork) {
			if (!--time_to_live)
				exit(0);
		}

		/* Pick a page and check its contents. */
		page = ((unsigned) random()) % nr_pages;

		ShmSemaphore sem = (*semaphores)[page];
		sem.down();
		
		/* Writer tasks should modify pages occasionally, too. */

		/* There is a tradeoff here.  By modifying the page
                   _before_ we check the old contents, we get to test
                   write faults which touch swap but at the cost of
                   missing some fault detection power. */

		if (writer && count++ > 10) {
			count = 0;
			patterns[page] = page_val(page) = random();
		}   

		if (page_val(page) != patterns[page])
			page_error(page);


		sem.up();
	}
}


void page_error(int page)
{
	fprintf (stderr, 
		 "\nChild %05d failed at page %d, address %p: "
		 "expected %08x, found %08x\n",
		 getpid(), page, &page_val(page),
		 patterns[page], page_val(page));
	exit(3);
}


void fork_new_child(void)
{
	int old_pid = getpid();
	int pid;
	
	pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(errno);
	}
	
	if (pid) {
		/* Are we the parent?  Wait for the child to print the
		   fork banner. */
		/* pause(); */
		exit(0);
	} else {
		/* Are we the child?  Print a banner, then signal the parent
		   to continue. */
		fprintf (stderr, "Child %05d forked into pid %05d\n", 
			 old_pid, getpid());
		kill (old_pid, SIGUSR1);
	}
}

