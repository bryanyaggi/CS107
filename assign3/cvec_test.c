/**
 * cvec_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <error.h>
#include <string.h>

#include "cvector.h"

static void verify_int(int expected, int found, char * msg)
{
    printf("%s expect: %d found %d. %s\n", msg, expected, found, (expected == found) ? "Seems ok." : "##### PROBLEM HERE #####");
}

/**
 * Exercises the cvector storing integers. Exercises the operations to add elements (append/insert), remove/change (remove/replace), and access elements (nth).
 */
static void simple_cvec()
{
    printf("--------------- Testing simple ops --------------- \n");

    cvector * cvec_ptr = cvec_create(sizeof(int), 20, NULL);
    printf("Created empty cvector.\n");
    verify_int(0, cvec_count(cvec_ptr), "cvec_count");

    printf("\nAppending 10 ints to cvector.\n");
    for (int i = 0; i < 10; i++)
    {
        cvec_append(cvec_ptr, &i);
    }
    verify_int(10, cvec_count(cvec_ptr), "cvec_count");
    verify_int(5, * (int *) cvec_nth(cvec_ptr, 5), "* value for cvec_nth(5)");

    printf("Contents are: ");
    for (int * cur = cvec_first(cvec_ptr); cur != NULL; cur = cvec_next(cvec_ptr, cur))
    {
        printf("%d ", * cur);
    }
    printf("\n");

    printf("\nNegate every other elem using pointer access.\n");
    for (int i = 0; i < cvec_count(cvec_ptr); i += 2)
    {
        (* (int *) cvec_nth(cvec_ptr, i)) *= -1;
    }
    verify_int(1, * (int *) cvec_nth(cvec_ptr, 1), "* value for cvec_nth(1)");
    verify_int(-2, * (int *) cvec_nth(cvec_ptr, 2), "* value for cvec_nth(2)");

    printf("\nUn-negate using replace function.\n");
    for (int i = 0; i < cvec_count(cvec_ptr); i += 2)
    { 
        cvec_replace(cvec_ptr, &i, i);
    }
    verify_int(3, * (int *) cvec_nth(cvec_ptr, 3), "* value for cvec_nth(3)");
    verify_int(4, * (int *) cvec_nth(cvec_ptr, 4), "* value for cvec_nth(4)");

    printf("\nInsert new elem at indexes 3 and 6.\n");
    int val = 99;
    cvec_insert(cvec_ptr, &val, 3);
    cvec_insert(cvec_ptr, &val, 6);
    verify_int(12, cvec_count(cvec_ptr), "cvec_count");
    verify_int(val, * (int *) cvec_nth(cvec_ptr, 3), "* value for cvec_nth(3)");
    verify_int(6, * (int *) cvec_nth(cvec_ptr, 8), "* value for cvec_nth(8)");

    cvec_dispose(cvec_ptr);
}

static int cmp_char(const void * p1, const void * p2)
{
    return (* (char *) p1 - * (char *) p2);
}

static void sortsearch_test()
{
    char * jumbled = "xatmpdvyhglzjrknicoqsbuewf";
    char * alphabet = "abcdefghijklmnopqrstuvwxyz";

    printf("\n-------------- Testing sort & search ------------- \n");
    cvector * cvec_ptr = cvec_create(sizeof(char), 4, NULL);
    for (int i = 0; i < strlen(jumbled); i++)
    {
        cvec_append(cvec_ptr, &jumbled[i]);
    }

    printf("\nDoing linear searches on unsorted cvector.\n");
    char ch = '*';
    verify_int(0, cvec_search(cvec_ptr, &jumbled[0], cmp_char, 0, false), "linear search");
    verify_int(9, cvec_search(cvec_ptr, &jumbled[9], cmp_char, 0, false), "linear search");
    verify_int(-1, cvec_search(cvec_ptr, &ch, cmp_char, 10, false), "linear search");

    printf("\nSorting cvector.\n");
    cvec_sort(cvec_ptr, cmp_char);
    verify_int(alphabet[0], * (char *) cvec_nth(cvec_ptr, 0), "* value for cvec_nth(0)");
    verify_int(alphabet[10], * (char *) cvec_nth(cvec_ptr, 10), "* value for cvec_nth(10)");

    printf("\nDoing binary searches on sorted cvector.\n");
    verify_int(0, cvec_search(cvec_ptr, &alphabet[0], cmp_char, 0, true), "binary search");
    verify_int(20, cvec_search(cvec_ptr, &alphabet[20], cmp_char, 10, true), "binary search");
    verify_int(20, cvec_search(cvec_ptr, &alphabet[20], cmp_char, 10, false), "linear search");
    verify_int(-1, cvec_search(cvec_ptr, &ch, cmp_char, 10, true), "linear search");

    cvec_dispose(cvec_ptr);
}

static int cmp_int(const void * p1, const void * p2)
{
    return (* (int *) p1) - (* (int *) p2);
}

static void large_test(int size)
{
    printf("\n------------- Testing large cvector -------------- \n");
    printf("(These operations can be slow. Have patience...)\n");

    printf("\nFilling cvector with ints from 1 to %d in random order.\n", size);
    cvector * cvec_ptr = cvec_create(sizeof(int), 4, NULL);
    for (int i = 0; i < size; i++)
    {
        cvec_insert(cvec_ptr, &i, rand() % (cvec_count(cvec_ptr) + 1));
    }

    printf("Sorting cvector.\n");
    cvec_sort(cvec_ptr, cmp_int);
    printf("Verifying cvector is in sorted order.\n");
    for (int i = 0; i < cvec_count(cvec_ptr); i++)
    {
        if (i != * (int *) cvec_nth(cvec_ptr, i))
        {
            verify_int(i, * (int *) cvec_nth(cvec_ptr, i), "cvec_nth()");
            break;
        }
    }

    cvec_dispose(cvec_ptr);
}

int main(int argc, char * argv[])
{
    bool single = false;
    int which = 1;

    if (argc > 1)
    {
        which = atoi(argv[1]);

        if (which < 1 || which > 3)
        {
            error(1, 0, "argument must be from 1 to 3 to select test");
        }

        single = true;
    }

    switch (which)
    {
        case 1:
            simple_cvec();
            if (single)
            {
                break;
            }
        case 2:
            sortsearch_test();
            if (single)
            {
                break;
            }
        case 3:
            large_test(25000);
            if (single)
            {
                break;
            }
    }

    return EXIT_SUCCESS;
}

