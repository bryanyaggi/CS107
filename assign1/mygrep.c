/**
 * mygrep.c
 *
 * Prints lines of text that match a given regular expression (regex)
 *
 * Supports . ^ * metacharaters.
 *
 * Usage: ./mygrep <regex> [<filename>]
 *  regex is the regular expression
 *  filname is optional and is the name of a file
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LINE_CHARS 120 // maximum characters read per line
#define INV_ATT "\x1b[7m" // invert display attributes
#define RST_ATT "\x1b[0m" // reset display attributes

/**
 * str_bounds contains pointers to string bounds
 */
struct str_bounds {
    char * beg;
    char * end;
};

char * regex_match(char * line, char * regex);

struct str_bounds regex_locate(char * line, char * regex);

/**
 * print_usage prints the correct usage to stdout
 * @param arg0 character pointer to argv[0]
 */
void print_usage(char * arg0)
{
    fprintf(stdout, "Usage: %s <regrex> [<filename>]\n", arg0);
}

/**
 * regex_match determines if there is a regex match starting at the given string location
 * @param line character pointer to string location
 * @param regex character pointer to regex
 * @return character pointer to end of matched regex in line and NULL if no match
 */
char * regex_match(char * line, char * regex)
{
    if (* regex == '\0')
    {
        return line; // returns pointer to end of match
    }

    if (regex[1] == '*') // next char is '*' metacharacter
    {
        if (* regex == '.')
        {
            regex = regex + 2;
            struct str_bounds submatch_bnds;
            submatch_bnds.beg = NULL;
            submatch_bnds.end = NULL;
            char * match_end = NULL;

            // Look for remaining regex in rest of line; greedy search
            while ((submatch_bnds = regex_locate(line, regex)).beg != NULL)
            {
                match_end = submatch_bnds.end;
                line = submatch_bnds.beg + 1;
            }

            return match_end;
        }

        char repeat_char[2]; // c string to hold repeating char
        sprintf(repeat_char, "%c", * regex);
        regex = regex + 2; // increment pointer past '*'
        char * end_repeat = line; // pointer to line at end of repeating char

        // Greedy match character until failure
        while ((end_repeat = regex_match(line, repeat_char)) != NULL)
        {
            line = end_repeat;
        }

        return regex_match(line, regex); // continue matching
    }
    
    if (* line == '\0')
    {
        return NULL;
    }

    if (* line == * regex || * regex == '.')
    {
        line++;
        regex++;
        return regex_match(line, regex);
    }
    else
    {
        return NULL;
    }
}

/**
 * regex_locate locates a regex match in a string
 * @param line character pointer to string
 * @param regex character pointer to regex
 * @return structure str_bounds containing regex match string bounds
 */
struct str_bounds regex_locate(char * line, char * regex)
{
    struct str_bounds match_bounds; 
    match_bounds.beg = line;
    match_bounds.end = NULL;

    if (* line == '\0')
    {
        match_bounds.beg = NULL;
        return match_bounds;
    }

    if (* regex == '^')
    {
        regex++;

        if ((match_bounds.end = regex_match(line, regex)) != NULL) // search from beginning
        {
            return match_bounds;
        }
        else // no match found
        {
            match_bounds.beg = NULL;
            return match_bounds;
        }
    }

    if (* regex == * line)
    {
        line++;
        regex++;
       
        if ((match_bounds.end = regex_match(line, regex)) != NULL)
        {
            return match_bounds;
        }
        else // proceed searching through string
        {
            regex--;
            return regex_locate(line, regex);
        }
    }
    else // proceed searching through string
    {
        line++;
        return regex_locate(line, regex);
    }
}

int main(int argc, char * argv[])
{
    // Check for valid input arguments
    if (argc < 2 || argc > 3)
    {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    char * regex = argv[1];
    FILE * stream = NULL;
    char line[MAX_LINE_CHARS + 2];

    if (argc == 2) // input stream is stdin
    {
        stream = stdin;
    }
    else // argc == 3
    {
        if ((stream = fopen(argv[2], "r")) == NULL) // input stream is file
        {
            fprintf(stdout, "Unable to open %s\n", argv[2]);
            return EXIT_SUCCESS;
        }
    }

    // Process input one line at a time
    while (fgets(line, MAX_LINE_CHARS + 2, stream) != NULL)
    {
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0'; // remove '\n' from line
        }

        struct str_bounds match_bounds;
        match_bounds.beg = NULL;
        match_bounds.end = NULL;

        if ((match_bounds = regex_locate(line, regex)).beg != NULL) // match found
        {
            int i = 0;

            // Print line with matched regex highlighted
            while (line[i] != '\0')
            {
                if (&line[i] == match_bounds.beg)
                {
                    fprintf(stdout, INV_ATT "%c", line[i]);
                }
                else if (&line[i] == match_bounds.end)
                {
                    fprintf(stdout, RST_ATT "%c", line[i]);
                }
                else
                {
                    fprintf(stdout, "%c", line[i]);
                }
                
                i++;
            }

            fprintf(stdout, RST_ATT "\n");
        }
    }

    if (argc != 2)
    {
        fclose(stream);
    }

    return EXIT_SUCCESS;
}

