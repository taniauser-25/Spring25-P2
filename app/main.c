#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  // For getopt()
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../src/lab.h"  // Ensure this file contains version macros

// Stores the last stopped process ID
static pid_t last_stopped_pid = -1;

static void explain_waitpid(int status)
{
    if (!WIFEXITED(status))
    {
        fprintf(stderr, "Child exited with status %d\n", WEXITSTATUS(status));
    }

    if (WIFSIGNALED(status))
    {
        fprintf(stderr, "Child exited via signal %d\n", WTERMSIG(status));
    }

    if (WIFSTOPPED(status))
    {
        fprintf(stderr, "Child stopped by %d\n", WSTOPSIG(status));
    }

    if (WIFCONTINUED(status))
    {
        fprintf(stderr, "Child was resumed by delivery of SIGCONT\n");
    }
}

int main(int argc, char *argv[])
{
    int opt;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "v")) != -1)
    {
        switch (opt)
        {
            case 'v':
                printf("Shell version: %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    parse_args(argc, argv);
    struct shell sh;
    sh.prompt = get_prompt("MY_PROMPT");  // Dynamically fetch the prompt
    sh_init(&sh);

    char *line;
    while (1)
    {
        // Update prompt dynamically in case MY_PROMPT changes
        free(sh.prompt);
        sh.prompt = get_prompt("MY_PROMPT");

        // Display shell prompt and read input using readline()
        line = readline(sh.prompt);

        // If user presses Ctrl+D (EOF), exit the shell
        if (!line)
        {
            printf("\nExiting shell...\n");
            free(sh.prompt);
            break;
        }

        // Trim whitespace
        line = trim_white(line);
        if (!*line)  // Ignore empty input
        {
            free(line);
            continue;
        }

        // Add command to history
        add_history(line);

        // Parse command
        char **cmd = cmd_parse(line);

        // Execute built-in command first (no forking needed)
        if (do_builtin(&sh, cmd))
        {
            cmd_free(cmd);  // Free memory if it's a built-in command
        } 
        else 
        {
            // Fork and execute external command
            pid_t pid = fork();
            if (pid == 0)
            {
                // Child process
                pid_t child = getpid();
                setpgid(child, child);
                tcsetpgrp(sh.shell_terminal, child);

                // Restore default signal handling in child
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);

                // Execute the command
                execvp(cmd[0], cmd);
                perror("execvp failed");  // Print error if execvp fails
                exit(EXIT_FAILURE);
            }
            else if (pid < 0)
            {
                perror("fork return < 0 Process creation failed!");
                abort();
            }

            // Parent process: set child as foreground process and wait for it
            setpgid(pid, pid);
            tcsetpgrp(sh.shell_terminal, pid);
            
            int status;
            int rval = waitpid(pid, &status, WUNTRACED);
            if (rval == -1)
            {
                fprintf(stderr, "Wait pid failed with -1\n");
                explain_waitpid(status);
            }

            // Restore control to the shell after child process ends
            tcsetpgrp(sh.shell_terminal, sh.shell_pgid);

            // Handle stopped process (Ctrl+Z)
            if (WIFSTOPPED(status)) {
                fprintf(stderr, "Process %d stopped\n", pid);
                last_stopped_pid = pid;  // Store stopped job
            }

            // Free allocated command memory
            cmd_free(cmd);
        }

        // Free input line
        free(line);
    }

    return 0;
}
