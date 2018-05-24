/**
 * bits_test.c
 */

#include <assert.h>
#include <error.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>

#include "bits.h"

static char * boolstr(bool b)
{
    return b ? "true" : "false";
}

/*
static char * sign(int num)
{
    return num == 0 ? "zero" : (num < 0 ? "negative" : "positive");
}
*/

static void test_cmp_bits()
{
    printf("\ncmp_bits(%#x,%#x) = %d\n", 0xa, 0x5, (cmp_bits(0xa, 0x5)));
    printf("cmp_bits(%#x,%#x) = %d\n", 0xf, 0x1, (cmp_bits(0xf, 0x1))); 
    printf("cmp_bits(%#x,%#x) = %d\n", 0xffff, 0x0, (cmp_bits(0xffff, 0x0000)));
    printf("cmp_bits(%#x,%#x) = %d\n", 0x8001, 0xffff, (cmp_bits(0x8001, 0xffff)));
}

static void test_sudoku()
{
    int rows[] = {1, 2, 5};
    int cols[] = {2, 6, 7};
    int block[] = {1, 3, 4, 9};

    printf("\nrows bit vector set = 0x%x\n", make_set(rows, 3));
    printf("cols bit vector set = 0x%x\n", make_set(cols, 3));
    printf("block bit vector set = 0x%x\n", make_set(block, 4));

    printf("\nis_single returned %s\n", boolstr(is_single(make_set(rows, 3), make_set(cols, 3), make_set(block, 4))));
    printf("is_single returned %s\n", boolstr(is_single(make_set(rows, 2), make_set(cols, 2), make_set(block, 3))));
    printf("is_single returned %s\n", boolstr(is_single(0, 0, 0)));
}

static void test_saturate()
{
    stype s1 = -9;
    stype s2 = 116;
    stype s3 = 127;
    stype s4 = -128;
    stype s5 = -10;
    utype u1 = 11;
    utype u2 = 96;
    utype u3 = 255;

    printf("\nSaturate stype is %s\n", SAT_NAME);

    printf(SFMT" + "SFMT" = "SFMT" (signed)\n", s1, s2, sat_add_signed(s1, s2));
    printf(SFMT" + "SFMT" = "SFMT" (signed)\n", s3, s3, sat_add_signed(s3, s3));
    printf(SFMT" + "SFMT" = "SFMT" (signed)\n", s4, s5, sat_add_signed(s4, s5)); 
    printf(UFMT" + "UFMT" = "UFMT" (unsigned)\n", u1, u2, sat_add_unsigned(u1, u2));
    printf(UFMT" + "UFMT" = "UFMT" (unsigned)\n", u3, u3, sat_add_unsigned(u3, u3));
}

/**
 * binade: set of numbers in binary floating-point that all have the same exponent
 * epsilon: the distance from the floating point number to the next larger magnitude representable float (i.e. its neighbor on the floating point number line moving away from zero)
 * Note: All the float values within one binade have the same epsilon.
 *
 * For each number,
 *      - compute float bits
 *      - compute epsilon
 *      - express as a sum/difference of powers-of-2
 *      - print value in decimal
 *
 * a) -100.0
 * b) smallest positive normalized float value (FLT_MIN)
 * c) median float value from the same binade as FLT_MAX
 * d) largest odd integer that is exactly representable as a float
 * e) smallest float value that can be added to FLT_MAX to sum to infinity
 */  
static void test_float_point()
{
    printf("\n---------- A ---------- \n");

    printf("Calculate float bits:\n");
    printf("Step 1. Determine sign bit.\n");
    printf("\tsign bit = 1\n");
    
    printf("Step 2. Write number in base-2 scientific notation. (1 <= n < 2)\n");
    printf("\t100 / 2^6 = (1 + fraction)\n");
    printf("\tfraction = 0.5625\n");

    printf("Step 3. Determine exponent bits.\n");
    printf("\texponent = 6\n");
    printf("\texponent + bias = 133; bias = 127\n");
    printf("\texponent bits: 10000101\n");

    printf("Step 4. Determine mantissa bits.\n");
    printf("\tfraction in binary = .1001\n");
    printf("\tmantissa bits: 1001...\n");

    printf("Step 5. Combine sign, exponent, and mantissa bits.\n");
    printf("\tfloat bits: 1 10000101 10010000000000000000000 = 0xc2c80000\n");

    unsigned int float_bits = 0xc2c80000;
    float float_val = * (float *) &float_bits;

    printf("\nCompute epsilon:\n");
    unsigned int next_float_bits = float_bits + 1;
    float epsilon = (* (float *) &next_float_bits) - (* (float *) &float_bits);
    printf("\tepsilon = %.9g\n", epsilon);

    printf("\nExpress as powers-of-2:\n");
    printf("\tsum: -(2^6 + 2^5 + 2^2)\n");

    printf("\nExpress in decimal:\n");
    printf("\tdecimal: %.9g\n", float_val);

    printf("\n---------- B ---------- \n");

    printf("Calculate float bits:\n");
    printf("\tfloat bits: 0 00000001 00000000000000000000000 = 0x00800000\n");

    float_bits = 0x00800000;
    float_val = * (float *) &float_bits;

    printf("\nCompute epsilon:\n");
    next_float_bits = float_bits + 1;
    epsilon = (* (float *) &next_float_bits) - (* (float *) &float_bits);
    printf("\tepsilon = %.9g\n", epsilon);

    printf("\nExpress as powers-of-2:\n");
    printf("\tsum: 2^-126\n");

    printf("\nExpress in decimal:\n");
    printf("\tdecimal: %.9g\n", float_val);
    printf("\texpected: %.9g\n", FLT_MIN);

    printf("\n---------- C ---------- \n");

    printf("Calculate float bits:\n");
    printf("\tfloat bits: 0 11111110 01111111111111111111111 = 0x7f3fffff\n");

    float_bits = 0x7f3fffff;
    float_val = * (float *) &float_bits;

    printf("\nCompute epsilon:\n");
    next_float_bits = float_bits + 1;
    epsilon = (* (float *) &next_float_bits) - (* (float *) &float_bits);
    printf("\tepsilon = %.9g\n", epsilon);

    printf("\nExpress as powers-of-2:\n");
    printf("\tsum: 2^127 + 2^126\n");

    printf("\nExpress in decimal:\n");
    printf("\tdecimal: %.9g\n", float_val);

    printf("\n---------- D ---------- \n");

    printf("Calculate float bits:\n");
    printf("\tfloat bits: 0 10010110 11111111111111111111111 = 0x4b7ffff\n");

    float_bits = 0x4b7fffff;
    float_val = * (float *) &float_bits;

    printf("\nCompute epsilon:\n");
    next_float_bits = float_bits + 1;
    epsilon = (* (float *) &next_float_bits) - (* (float *) &float_bits);
    printf("\tepsilon = %.9g\n", epsilon);

    printf("\nExpress as powers-of-2:\n");
    printf("\tsum: 2^24 - 2^0\n");

    printf("\nExpress in decimal:\n");
    printf("\tdecimal: %.9g\n", float_val);

    printf("\n---------- E ---------- \n");

    printf("Calculate float bits:\n");
    
    float flt_max = FLT_MAX;
    unsigned int prev_flt_bits = (* (int *) &flt_max) - 1;
    float flt_max_eps = flt_max - (* (float *) &prev_flt_bits);
    printf("\tFLT_MAX epsilon = %.9g\n", flt_max_eps);
    printf("\thalf of FLT_MAX epsilon = %.9g\n", flt_max_eps / 2);

    printf("\tfloat bits: 0 11100110 0000000000000000000000 = 0x73000000\n");

    float_bits = 0x73000000;
    float_val = * (float *) &float_bits;

    printf("\nCompute epsilon:\n");
    next_float_bits = float_bits + 1;
    epsilon = (* (float *) &next_float_bits) - (* (float *) &float_bits);
    printf("\tepsilon = %.9g\n", epsilon);

    printf("\nExpress as powers-of-2:\n");
    printf("\tsum: 2^103\n");

    printf("\nExpress in decimal:\n");
    printf("\tdecimal: %.9g\n", float_val);

    printf("\nexpect inf: FLT_MAX + float = %.9g\n", FLT_MAX + float_val);

    float_bits = 0x72ffffff;
    float_val = * (float *) &float_bits;
    printf("\nnext smallest float bits: 0 11100101 fffffffffffffffffffffff = 0x72ffffff\n");
    printf("expect non-inf: FLT_MAX + next_smallest_float = %.9g\n", FLT_MAX + float_val);
}

static void test_disassemble()
{
    unsigned char imm[] = {0x68, 0x10, 0x3f, 0x00, 0x00}; //push immediate constant
    unsigned char reg[] = {0x53}; // push register
    unsigned char ind[] = {0xff, 0x32}; // push register indirect
    unsigned char displ[] = {0xff, 0x70, 0x08}; // push register indirect with displacement
    unsigned char scaled[] = {0xff, 0x74, 0x8d, 0xff}; // push register indirect with displacement and scaled index

    printf("\nDissassembling raw instructions:\n");

    disassemble(imm);
    disassemble(reg);
    disassemble(ind);
    disassemble(displ);
    disassemble(scaled);
}

int main(int argc, char * argv[])
{
    enum {cmpbits = 1 << 1, sudoku = 1 << 2, saturate = 1 << 3, float_point = 1 << 4, disassemble = 1 << 5, all = (1 << 6) - 1} which = all;

    if (argc > 1)
    {
        which = 1 << atoi(argv[1]);

        if (which < cmpbits || which > disassemble)
        {
            error(1, 0, "argument must be 1 to 5 to select test");
        }
    }

    if (which & cmpbits)
    {
        test_cmp_bits();
    }

    if (which & sudoku)
    {
        test_sudoku();
    }

    if (which & saturate)
    {
        test_saturate();
    }

    if (which & float_point)
    {
        test_float_point();
    }

    if (which & disassemble)
    {
        test_disassemble();
    }

    return 0;
}
