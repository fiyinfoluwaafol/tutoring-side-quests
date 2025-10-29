#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        exit(1);
    }

    int pipefd1[2]; // Pipe between cat and grep
    int pipefd2[2]; // Pipe between grep and sort
    int pid1, pid2;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL}; // Use argument from command line
    char *sort_args[] = {"sort", NULL};

    // Create first pipe for cat -> grep
    if (pipe(pipefd1) == -1)
    {
        perror("pipe");
        exit(1);
    }

    pid1 = fork();

    if (pid1 == 0)
    {
        // Child process (P2 - handles "grep <search_term>")

        // Create second pipe for grep -> sort
        if (pipe(pipefd2) == -1)
        {
            perror("pipe");
            exit(1);
        }

        pid2 = fork();

        if (pid2 == 0)
        {
            // Grandchild process (P3 - handles "sort")

            // Replace stdin with input part of second pipe (from grep)
            dup2(pipefd2[0], 0);

            // Close all pipes
            close(pipefd1[0]);
            close(pipefd1[1]);
            close(pipefd2[0]);
            close(pipefd2[1]);

            // Execute sort
            execvp("sort", sort_args);
            perror("execvp failed");
            exit(1);
        }
        else
        {
            // Child process (P2 - handles "grep <search_term>")

            // Replace stdin with input part of first pipe (from cat)
            dup2(pipefd1[0], 0);

            // Replace stdout with output part of second pipe (to sort)
            dup2(pipefd2[1], 1);

            // Close all pipes
            close(pipefd1[1]);
            close(pipefd2[0]);
            close(pipefd2[1]);

            // Execute grep
            execvp("grep", grep_args);
            perror("execvp failed");
            exit(1);
        }
    }
    else
    {
        // Parent process (P1 - handles "cat scores")

        // Replace stdout with output part of first pipe (to grep)
        dup2(pipefd1[1], 1);

        // Close all pipes
        close(pipefd1[0]);
        close(pipefd1[1]);

        // Execute cat
        execvp("cat", cat_args);
        perror("execvp failed");
        exit(1);
    }

    // Wait for both child and grandchild processes to finish
    wait(NULL);
    wait(NULL);

    return 0;
}
