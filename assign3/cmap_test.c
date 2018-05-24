/**
 * cmap_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <error.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "cmap.h"

static void verify_int(int expected, int found, char * msg)
{
    printf("%s expect: %d found %d. %s\n", msg, expected, found, (expected == found) ? "Seems ok." : "##### PROBLEM HERE #####");
}

static void verify_ptr(void * expected, void * found, char * msg)
{
    printf("%s expect: %p found: %p. %s\n", msg, expected, found, (expected == found) ? "Seems ok." : "##### PROBLEM HERE #####");
}

static void verify_int_ptr(int expected, int * found, char * msg)
{
    if (found == NULL)
    {
        printf("%s found: %p. %s\n", msg, found, "##### PROBLEM HERE #####");
    }
    else
    {
        verify_int(expected, * found, msg);
    }
}

static void simple_cmap()
{
    printf("-------------- Testing simple ops -------------- \n");

    char * words[] = {"apple", "pear", "banana", "cherry", "kiwi", "melon", "grape", "plum"};
    char * extra = "strawberry";
    int len;
    int nwords = sizeof(words)/sizeof(words[0]);

    cmap * cmap_ptr = cmap_create(sizeof(int), nwords, NULL);
    printf("Created empty cmap.\n");
    verify_int(0, cmap_count(cmap_ptr), "cmap_count");
    verify_ptr(NULL, cmap_get(cmap_ptr, "nonexistent"), "cmap_get(\"nonexistent\")");
    
    printf("\nAdding %d keys to cmap.\n", nwords);
    for (int i = 0; i < nwords; i++)
    {
        len = strlen(words[i]);
        cmap_put(cmap_ptr, words[i], &len); // associate word with its strlen
    }
    verify_int(nwords, cmap_count(cmap_ptr), "cmap_count");
    verify_int_ptr(strlen(words[0]), cmap_get(cmap_ptr, words[0]), "cmap_get(\"apple\")");

    printf("\nAdd one more key to cmap.\n");
    len = strlen(extra);
    cmap_put(cmap_ptr, extra, &len);
    verify_int(nwords + 1, cmap_count(cmap_ptr), "cmap_count");
    verify_int_ptr(len, cmap_get(cmap_ptr, extra), "cmap_get(\"strawberry\")");

    printf("\nReplace existing key in cmap.\n");
    len = 2 * strlen(extra);
    cmap_put(cmap_ptr, extra, &len);
    verify_int(nwords + 1, cmap_count(cmap_ptr), "cmap_count");
    verify_int_ptr(len, cmap_get(cmap_ptr, extra), "cmap_get(\"strawberry\")");

    printf("\nRemove key from cmap.\n");
    cmap_remove(cmap_ptr, words[0]);
    verify_int(nwords, cmap_count(cmap_ptr), "cmap_count");
    verify_ptr(NULL, cmap_get(cmap_ptr, words[0]), "cmap_get(\"apple\")");

    printf("\nUse iterator to count keys.\n");
    printf("First key: %s\n", cmap_first(cmap_ptr));
    int nkeys = 0;
    for (const char * key = cmap_first(cmap_ptr); key != NULL; key = cmap_next(cmap_ptr, key))
    {
        nkeys++;
    }
    verify_int(cmap_count(cmap_ptr), nkeys, "Number of keys");

    cmap_dispose(cmap_ptr);
}

static void frequency_test()
{
    printf("\n--------------- Testing frequency --------------- \n");
    
    cmap * counts = cmap_create(sizeof(int), 26, NULL);

    int val;
    int zero = 0;
    char buf[2];
    buf[1] = '\0'; // null terminator for string of one char

    // Initialize map to have entries for all lowercase letters, count = 0
    for (char ch = 'a'; ch <= 'z'; ch++)
    {
        buf[0] = ch;
        cmap_put(counts, buf, &zero);
    }

    FILE * fp = fopen("gettysburg_frags", "r");
    assert(fp != NULL);

    while ((val = getc(fp)) != EOF)
    {
        if (isalpha(val))
        {
            buf[0] = tolower(val);
            (* (int *) cmap_get(counts, buf))++;
        }
    }
    fclose(fp);

    int total = 0;
    for (const char * key = cmap_first(counts); key != NULL; key = cmap_next(counts, key))
    {
        total += * (int *) cmap_get(counts, key);
    }

    printf("Total of all frequencies = %d\n", total);
    // correct count should agree with shell command
    // tr -c -d "[:alpha:]" < gettysburg_frags | wc -c
    
    cmap_dispose(counts);
}

int main(int argc, char * argv[])
{
    bool single = false;
    int which = 1;

    if (argc > 1)
    {
        which = atoi(argv[1]);

        if (which < 1 || which > 2)
        {
            error(1, 0, "argument must be from 1 to 2 to select test");
        }

        single = true;
    }

    switch (which)
    {
        case 1:
            simple_cmap();
            if (single)
            {
                break;
            }
        case 2:
            frequency_test();
            if (single)
            {
                break;
            }
    }

    return EXIT_SUCCESS;
}

