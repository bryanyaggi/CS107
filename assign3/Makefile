#
# CS 107 Assignment 2
#
# Stanford CS 107 Makefile Template
#

# Compiler
CC = gcc

# Compile Flags
# 	-g			compile with debug information
# 	-Wall		give verbose compiler warnings
# 	-O0			do not optimize generated code
# 	-std=c99	use the C99 standard language definition
CFLAGS = -g -Wall -O0 -std=c99
# Compile Flags for use of strdup()
CFLAGS_STRDUP = -g -Wall -O0 -std=gnu99

# Linker Flags
# 	-lm		link in libm (the math library)
LDFLAGS = -lm

# Executables
EXES = cvec_test cmap_test cmapcvec_test

cvec_test: cvec_test.o cvector.o
	$(CC) $(CFLAGS) -o $@ cvec_test.c cvector.c $(LDFLAGS)

cmap_test: cmap_test.o cmap.o
	$(CC) $(CFLAGS) -o $@ cmap_test.c cmap.c $(LDFLAGS)

cmapcvec_test: cmapcvec_test.o cvector.o cmap.o
	$(CC) $(CFLAGS_STRDUP) -o $@ cmapcvec_test.c cvector.c cmap.c $(LDFLAGS)

cvec_test.o: cvec_test.c cvector.h
	$(CC) $(CFLAGS) -c cvec_test.c

cmap_test.o: cmap_test.c cmap.h
	$(CC) $(CFLAGS) -c cmap_test.c

cmapcvec_test.o: cmapcvec_test.c cvector.h cmap.h
	$(CC) $(CFLAGS_STRDUP) -c cmapcvec_test.c

cvector.o: cvector.c cvector.h
	$(CC) $(CFLAGS) -c cvector.c

cmap.o: cmap.c cmap.h
	$(CC) $(CFLAGS) -c cmap.c

clean:
	rm $(EXES) *.o

.PHONY: clean

