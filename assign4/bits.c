/**
 * bits.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "bits.h"

/*
int cmp_bits(int a, int b)
{
    int a_on = 0;
    int b_on = 0;

    int msb_on = 0x1 << (3 + 4 * (sizeof(int) - 1));

    for (int i = 0x1; i <= msb_on; i <<= 1)
    {
        if ((a & i) != 0x0)
        {
            a_on++;
        }

        if ((b & i) != 0x0)
        {
            b_on++;
        }
    }

    return a_on - b_on;
}
*/

/**
 * BE WARY OF C RIGHT SHIFT
 * Right shift is defined by the compiler to be logical or arithmetic.
 */
int cmp_bits(int a, int b)
{
    int a_on = 0;
    int b_on = 0;

    while (a != 0x0)
    {
        if (a & 0x1)
        {
            a_on++;
        }

        a >>= 1;
    }

    while (b != 0x0)
    {
        if (b & 0x1)
        {
            b_on++;
        }

        b >>= 1;
    }

    return a_on - b_on;
}

unsigned short make_set(int values[], int nvalues)
{
    unsigned short set = 0x0;

    for (int i = 0; i < nvalues; i++)
    {
        assert(values[i] > 0 && values[i] <= 9);
        set |= (0x1 << values[i]);
    }

    return set;
}

bool is_single(unsigned short used_in_row, unsigned short used_in_col, unsigned short used_in_block)
{
    int mask_arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int mask_nvals = 9;
    unsigned short mask = make_set(mask_arr, mask_nvals);

    used_in_row &= mask;
    used_in_col &= mask;
    used_in_block &= mask;

    // Check number of unused digits
    unsigned short digits_possible = (~ (used_in_row | used_in_col | used_in_block)) & mask;

    if ((digits_possible & (digits_possible - 1)) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

stype sat_add_signed(stype a, stype b)
{
    utype max_u = 0x0 - 1;
    max_u /= 2;

    stype max = max_u;
    stype min = max + 1;

    if (b > (max - a))
    {
        return max;
    }
    else if (b < (min - a))
    {
        return min;
    }

    return a + b;
}

utype sat_add_unsigned(utype a, utype b)
{
    utype result = a + b;

    if (result < a)
    {
        return 0x0 - 1;
    }

    return result;
}

/**
 * print_hex_bytes
 *
 * Prints hexidecimal representation for a given char array. No carriage return, space after last hex.
 * @param bytes_arr char array of bytes
 * @param num_bytes number of bytes
 */
static void print_hex_bytes(const unsigned char * bytes_arr, int num_bytes)
{
    for (int i = 0; i < num_bytes; i++)
    {
        printf("%x ", bytes_arr[i]);
    }
}

void disassemble(const unsigned char * raw_instr)
{
    char * registers[] = {"%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi"};

    unsigned char immediate = 0x68;
    unsigned char ms5_mask = 0xf8;
    unsigned char reg_var = 0x50;
    unsigned char indirect = 0xff;

    if (raw_instr[0] == immediate)
    {
        print_hex_bytes(raw_instr, 5);

        unsigned int const_bits = 0x0;
        for (int i = 1; i < 5; i++)
        {
            const_bits |= (raw_instr[i] << (8 * (i - 1)));
        }

        printf("\tpushq $0x%x\n", const_bits);

        return;
    }
    else if ((raw_instr[0] & ms5_mask) == reg_var)
    {
        print_hex_bytes(raw_instr, 1);

        printf("\t\tpushq %s\n", registers[raw_instr[0] & (~ ms5_mask)]);
    }
    else if (raw_instr[0] == indirect)
    {
        unsigned char no_opt = 0x30;
        unsigned char disp = 0x70;
        unsigned char disp_scaled = 0x74;

        if ((raw_instr[1] & ms5_mask) == no_opt)
        {
            print_hex_bytes(raw_instr, 2);
            printf("\t\tpushq (%s)\n", registers[raw_instr[1] & (~ ms5_mask)]);
        }
        else if (raw_instr[1] == disp_scaled)
        {
            print_hex_bytes(raw_instr, 4);

            unsigned char scales[] = {1, 2, 4, 8};
            unsigned char scale_factor = scales[(raw_instr[2] & 0xc0) / 64];
            unsigned char reg1_index = ((raw_instr[2] & 0x38) >> 3);
            unsigned char reg0_index = (raw_instr[2] & 0x07);

            printf("\tpushq 0x%x(%s,%s,%d)\n", raw_instr[3], registers[reg0_index], registers[reg1_index], scale_factor);
        }
        else if ((raw_instr[1] & ms5_mask) == disp)
        {
            print_hex_bytes(raw_instr, 3);
            printf("\tpushq 0x%x(%s)\n", raw_instr[2], registers[raw_instr[1] & (~ ms5_mask)]);
        }
    }
}

