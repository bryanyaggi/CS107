#
# Makefile for the malloc lab driver
#

# Compiler
CC = gcc

# Compile Flags
#	-g					compile with debug information
#	-Wall				giver verbose compiler warnings
#	-Wno-unused_result	suppress warnings for unused result
#	-Winline			warn if function cannot be inlined
#	-O0					do not optimize generated code
#	-O3					-O2 with inlining
#	-std=gnu99			use the GNU99 standard language definition
#	-DDRIVER			define macro 'DRIVER'
#	-DDEBUG				define macro 'DEBUG'
CFLAGS = -std=gnu99 -Wall -Wno-unused-result -Winline -g -O3 -DDRIVER

# Object Files
OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o 

all: mdriver

mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS)

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

clean:
	rm -f *~ *.o mdriver

