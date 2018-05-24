/**
 * mywhich.c
 *
 * Prints path of executable
 *
 * Usage: ./mywhich [-p <searchdirs>] <executables>
 *  <searchdirs> is a colon-delimited sequence of directories to search
 *  <executables> is a space-separated list of executables
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <unistd.h>

/**
 * print_usage prints the correct usage to stdout
 * @param arg0 character pointer to argv[0]
 */
void print_usage(char * arg0)
{
    fprintf(stdout, "Usage: %s [-p <searchdirs>] <executables>\n", arg0);
}

/**
 * check_path checks if a file is present and executable
 * @param full_path c string representing full path of file
 * @return boolean indicating whether file is present and executable
 */
bool check_path(const char full_path[])
{
    // Check if file is present and executable
    if (access(full_path, X_OK) == 0)
    {
        fprintf(stdout, "%s\n", full_path);
        
        return true;
    }

    return false;
}

int main(int argc, char * argv[], char * envp[])
{
    if (argc == 1)
    {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    char * dirs; // pointer to address of first directory
    int exec_arg; // argv index of first executable argument

    // Check if optional argument provided
    if (strcmp(argv[1], "-p") == 0)
    {
        if (argc == 2)
        {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }

        dirs = argv[2];
        exec_arg = 3;
    }
    else
    {
        // Find index of PATH environment variable
        int path_index = 0;
        while (strncmp(envp[path_index], "PATH", 4) != 0)
        {
            path_index++;
        }
        
        dirs = envp[path_index] + 5; // pointer to beginning of directory
        exec_arg = 1;
    }

    // Search directories for executable
    for (int i = exec_arg; i < argc; i++)
    {
        char * dir_beg = dirs;
        char * dir_end = NULL; // pointer to end of directory
        char full_path[PATH_MAX] = "";

        bool search_done = false;

        while (!search_done)
        {
            // Check if last directory in PATH
            if ((dir_end = strchr(dir_beg, ':')) == NULL)
            {
                search_done = true;

                dir_end = dirs + strlen(dirs);
            }

            int j = 0;

            // append directory chars to full_path
            while (dir_beg != dir_end)
            {
                full_path[j] = * dir_beg;
                dir_beg++;
                j++;
            }

            dir_beg++; // increment pointer address of next char

            full_path[j] = '\0'; // append null terminating char

            // append executable to full_path
            strcat(full_path, "/");
            strcat(full_path, argv[i]);

            if (check_path(full_path))
            {
                search_done = true;
            }
        }
    }

    return EXIT_SUCCESS;
}

