#
# CS 107 Assignment 4
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

# Linker Flags
# 	-lm		link in libm (the math library)
LDFLAGS = -lm

# Executables
EXES = bits_test

bits_test: bits_test.o bits.o
	$(CC) $(CFLAGS) -o $@ bits_test.c bits.c $(LDFLAGS)

bits_test.o: bits_test.c bits.h
	$(CC) $(CFLAGS) -c bits_test.c

bits.o: bits.c bits.h
	$(CC) $(CFLAGS) -c bits.c

clean:
	rm $(EXES) *.o

.PHONY: clean

