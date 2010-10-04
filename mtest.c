/*
 * test.c
 * 
 * Tests kernel handling of shared private data pages.
 * 
 * (C) Stephen C. Tweedie <sct@redhat.com>, 1998
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "misc_lib.h"

int memory	= 8;	/* Default --- use 8 MB of heap */
int rprocesses	= 8;	/* Number of reader processes */
int wprocesses	= 1;	/* Number of writer processes */
int do_fork	= 0;	/* Enable random fork/die if true */

int pagesize;

char *progname;

char *heap;		/* Allocated space for a large working set */
int *patterns;		/* Record the patterns we expect in each heap page */
int nr_pages;		/* How many pages are in the heap */

static void setup_memory(void);		/* Setup the heap and pattern arrays*/
static void fork_child(int n, int writer);	/* Spawn a new child process */
static void fork_new_child(void);		/* Perform a fork/die
                                                   in this child */
static void run_test(int writer);		/* Run the test sweep
                                                   within a child */
static void null_handler(int);			/* Just a null signal handler */
static void page_error(int);			/* Report a heap
                                                   pattern mismatch */

#define page_val(page) (* (int *) (heap + pagesize * (page)))

static void usage(char *error) 
{
	if (error)
		fprintf (stderr, "%s: %s\n", progname, error);
	fprintf (stderr,
		 "Usage: %s [-h] [-m memsize] "
		 "[-r readers] [-w writers]\n", progname);
	exit (error ? 1 : 0);
}

int main(int argc, char *argv[])
{
	int c, i;

	progname = argv[0];
	
	while (c = getopt(argc, argv, "fhm:r:w:"), c != EOF) {
		switch (c) {
		case ':':
			usage("missing argument");
		case '?':
			usage("unrecognised argument");
		case 'f':
			do_fork = 1;
			break;
		case 'h':
			usage(0);
		case 'm':
			memory = strtoul(optarg, 0, 0);
			break;
		case 'r':
			rprocesses = strtoul(optarg, 0, 0);
			break;
		case 'w':
			wprocesses = strtoul(optarg, 0, 0);
			break;
		default:
			usage("unknown error");
		}
	}
	
	fprintf (stderr, "Starting test run with %d megabyte heap.\n", memory);
	
	setup_memory();
	
	for (i=0; i<rprocesses; i++)
		fork_child(i, 0);
	for (; i<rprocesses+wprocesses; i++)
		fork_child(i, 1);
	
	fprintf (stderr, "%d child processes started.\n", i);
	
	for (;;) {
		pid_t pid;
		int status;
		
		/* Catch child error statuses and report them. */
		pid = wait3(&status, 0, 0);
		if (pid < 0)	/* No more children? */
			exit(0);
		if (WIFEXITED (status)) {
			if (WEXITSTATUS (status))
				fprintf (stderr,
					 "Child %d exited with status %d\n",
					 pid, WEXITSTATUS(status));
			else
				fprintf (stderr,
					 "Child %d exited with normally\n",
					 pid);
		} else {
			fprintf (stderr,
				 "Child %d exited with signal %d\n",
				 pid, WTERMSIG(status));
		}
	}
}


static void setup_memory(void)
{
	int i;
	
	pagesize = getpagesize();
	nr_pages = memory * 1024 * 1024 / pagesize;

	fprintf (stderr, "Setting up %d %dkB pages for test...", 
		 nr_pages, pagesize);
	
	patterns = safe_malloc(nr_pages * sizeof(*patterns),"setup_memory");
	heap	 = safe_malloc(nr_pages * pagesize, "setup_memory");
	
	for (i=0; i<nr_pages; i++) {
		page_val(i) = i;
		patterns[i] = i;
	}

	fprintf (stderr, " done.\n");
	
}

static void fork_child(int n, int writer)
{
	pid_t pid, parent;
	
	parent = getpid();
	signal (SIGUSR1, null_handler);

	pid = fork();
	if (pid) {
		/* Are we the parent?  Wait for the child to print the
		   startup banner. */
		pause();
	} else {
		/* Are we the child?  Print a banner, then signal the parent
		   to continue. */
		fprintf (stderr, "Child %02d started with pid %05d, %s\n", 
			 n, getpid(), writer ? "writer" : "readonly");
		kill (parent, SIGUSR1);
		run_test(writer);
		/* The test should never terminate.  Exit with an error if
		   it does. */
		exit(2);
	}
}

static void run_test(int writer)
{
	int count = 0;
	int time_to_live = 0;
	int page;
	
	/* Give each child a different random seed. */
	srandom(getpid() * time(0));
	
	for (;;) {
		/* Track the time until the next fork/die round */
		if (do_fork) {
			if (time_to_live) {
				if (!--time_to_live)
					fork_new_child();
			}
			else
				time_to_live = random() % 50;
		}
					
		/* Pick a page and check its contents. */
		page = ((unsigned) random()) % nr_pages;
		if (page_val(page) != patterns[page])
			page_error(page);
		/* Writer tasks should modify pages occasionally, too. */
		if (writer && count++ > 10) {
			count = 0;
			patterns[page] = page_val(page) = random();
		}
	}
}

static void fork_new_child(void)
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
		pause();
		exit(0);
	} else {
		/* Are we the child?  Print a banner, then signal the parent
		   to continue. */
		fprintf (stderr, "Child %05d forked into pid %05d\n", 
			 old_pid, getpid());
		kill (old_pid, SIGUSR1);
	}
}


static void null_handler(int n)
{
}

static void page_error(int page)
{
	fprintf (stderr, 
		 "Child %05d failed at page %d, address %p: "
		 "expected %08x, found %08x\n",
		 getpid(), page, &page_val(page),
		 patterns[page], page_val(page));
	exit(3);
}
