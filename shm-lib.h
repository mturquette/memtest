/* -*- C++ -*-
 *
 * shm-lib.h
 *
 * Simple class for SysV shared memory segments
 */

#include <sys/fcntl.h>
#include <sys/types.h>

class ShmSeg;
class ShmSemaphore;
class ShmSemArray;


class ShmSeg
{
public:
	key_t	key;
	int	shmid;
	int	size;
	char *	address;
	
	ShmSeg (const char *name, int size, int flags, int mode);
	virtual ~ShmSeg ();
};


class ShmSemaphore 
{
public:
	const ShmSemArray & array;
	int bit;
	unsigned long *location;

	ShmSemaphore (ShmSemArray &, int);
	
	void down();
	void up();
};

class ShmSemArray : public ShmSeg
{
public:
	ShmSemArray (const char *name, int size, int flags, int mode);

	ShmSemaphore operator [] (int);
};
