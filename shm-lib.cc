/* -*- C++ -*-
 *
 * shm-lib.cc
 *
 * Simple class for SysV shared memory segments
 *
 * (C) Stephen C. Tweedie <sct@redhat.com>, 2000
 */

#include "shm-lib.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sched.h>

#include <asm/bitops.h>

static void sys_assert(const char *where, int why)
{
	if (why)
		return;
	
	fprintf (stderr, "Error: %s: %s\n", where, strerror(errno));
	exit (1);
}

ShmSeg::ShmSeg (const char *name, int size_, int flags, int mode)
	: size(size_)
{
	int err;
	
	if (name) {
		key = ftok (name, 'S');
		sys_assert ("ftok", key != -1);
	} else
		key = IPC_PRIVATE;
	
	if (flags & O_CREAT) 
		shmid = shmget(key, size, IPC_CREAT | (mode & 0777));
	else 
		shmid = shmget(key, size, 0);
	sys_assert ("shmget", shmid != -1);

	address = (char *) shmat (shmid, 0, 0);
	sys_assert ("shmat", address != (char *) -1);

	// Auto-delete the attached shared memory segment after the last
	// user exits.

	err = shmctl (shmid, IPC_RMID, 0);
	sys_assert ("shmctl", err != -1);
}

ShmSeg::~ShmSeg ()
{
	int err;
	
	err = shmdt (address);
	sys_assert ("shmdt", err != -1);
}


//
// ShmSemArray stuff:
//
// Create a class to manage an array of semaphore bits, and to do atomic
// test-and-set / clear operations for locking in shared memory.
//

ShmSemaphore::ShmSemaphore (ShmSemArray &array_, int offset) 
	: array(array_)
{
	location = (unsigned long *) (array.address + (offset / sizeof(unsigned long)));
	bit = offset % sizeof(unsigned long);
}

void ShmSemaphore::up()
{
	clear_bit (bit, location);
}

void ShmSemaphore::down()
{
	while (test_and_set_bit (bit, location))
		sched_yield();
}

ShmSemArray::ShmSemArray (const char *name, int bits, int flags, int mode)
	: ShmSeg (name, (bits + sizeof(char)) / sizeof(char), flags, mode)
{
	memset (address, 0, size);
}

ShmSemaphore ShmSemArray::operator [] (int bit) 
{
	return ShmSemaphore (*this, bit);
}

