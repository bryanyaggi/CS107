/**
 * bits.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Compares the count of "on" bits between two signed integers. Returns a negative result if the bitwise representation of a has fewer "on" bits than b, a positive result if a has more "on" than b, and zero if both have the same number of "on" bits.
 * @param a first int to compare
 * @param b second int to compare
 * @return difference in number of "on" bits
 */
int cmp_bits(int a, int b);

/**
 * make_set
 *
 * Creates a bit vestor set from an array. In a bit vector set, each individual bit is designated to represent a particular value and a bit is turned on to indicate its corresponding value is contained in the set. This bit vector set will use the range [1-9]. The bit at the nth position is designated to represent the value n.
 * @param values array of integer values [1-9]
 * @param nvalues number of elements in array
 * @return bit vector set
 */
unsigned short make_set(int values[], int nvalues);

/**
 * is_single
 *
 * Returns true if the already used digits force only one option for this cell(number of possibilities is exactly one) and false otherwise (number of possibilities is zero or two or more).
 * @param used_in_row bit vector set of digits used in cell row
 * @param used_in_col bit vector set of digits used in cell column
 * @param used_in_block bit vector set of digits used in cell block
 * @return true if one solution, false otherwise
 */
bool is_single(unsigned short used_in_row, unsigned short used_in_col, unsigned short used_in_block);

#define SAT_NAME "char"
#define SFMT "%hhd"
#define UFMT "%hhu"

typedef signed char stype;
typedef unsigned char utype;

/**
 * sat_add_signed
 *
 * Returns the sum of two values if it does not overflow/underflow the representable range or otherwise returns the appropriate "sticky" value (maximum or minimum number that can be represented). This function works on signed types.
 * @param a
 * @param b
 * @return sum or "sticky" value
 */
stype sat_add_signed(stype a, stype b);

/**
 * sat_add_unsigned
 *
 * Returns the sum of two values if it does not overflow/underflow the representable range or otherwise returns the appropriate "sticky" value (maximum or minimum number that can be represented). This function works on unsigned types.
 * @param a
 * @param b
 * @return sum or "sticky" value
 */
utype sat_add_unsigned(utype a, utype b);

/**
 * disassemble
 *
 * Takes a pointer to a sequence of raw bytes that represent a single machine instruction and prints the assembly language instruction.
 * @param raw_instr array of raw bytes
 */
void disassemble(const unsigned char * raw_instr);

