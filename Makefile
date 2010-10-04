
MAKEFLAGS = --no-builtin-rules
# We don't use any suffix rules
.SUFFIXES :

PROGS    = mmap001 mmap002 misc001 mtest fillmem ipc001 shm-stress
LIB_OBJS = misc_lib.o
LIBS     = -L. -lmisc

all : libmisc.a $(PROGS)

CFLAGS    	= -Wall
CFLAGS   	+= -W 
CFLAGS   	+= -Wstrict-prototypes
CFLAGS   	+= -Wmissing-prototypes  
CFLAGS   	+= -Wmissing-declarations
#CFLAGS   	+= -Wredundant-decls
CFLAGS   	+= -Wnested-externs
CFLAGS  	+= -Wshadow
CFLAGS   	+= -Winline
#CFLAGS   	+= -Waggregate-return 
#CFLAGS   	+= -Wpointer-arith
CFLAGS   	+= -Wbad-function-cast
CFLAGS   	+= -Wno-unused 
CFLAGS   	+= -Wcast-align
CFLAGS   	+= -Wcast-qual 
#CFLAGS   	+= -Wconversion 
CFLAGS   	+= -ggdb3
CFLAGS   	+= -O2
CC        = gcc

CC_OPTS = $(CFLAGS) $($*_CC_OPTS)
CXX_OPTS = $(CFLAGS) -frepo $($*_CC_OPTS)

shm-stress: shm-stress.o shm-lib.o
	$(CXX) $(CXX_OPTS) -o $@ $^

% : %.o libmisc.a
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

%.o : %.c
	$(CC) $(CC_OPTS) -c $< -o $@

%.a : $(LIB_OBJS) 
	ar -rc $@ $^

%.o: %.cc
	$(CXX) $(CXX_OPTS) -c $<

clean::
	rm -f *.a *.o *~ $(PROGS)


