#
# CS 107 Assignment 1
#
# Stanford CS 107 Makefile Template
#

# Compiler
CC = gcc

# Compile Flags
#	-g			compile with debug information
# 	-Wall		give verbose compiler warnings
# 	-O0			do not optimize generated code
# 	-std=c99	use the C99 standard language definition
CFLAGS = -g -Wall -O0 -std=c99

# Linker Flags
# 	-lm		link in libm (the math library)
LDFLAGS = -lm

mywhich: mywhich.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

mygrep: mygrep.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm mywhich mygrep

.PHONY: clean

