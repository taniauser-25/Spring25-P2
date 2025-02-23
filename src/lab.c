#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <limits.h>  // For ARG_MAX
#include "lab.h"

// Stores the last stopped process
static pid_t last_stopped_pid = -1;

// Displays the prompt
char *get_prompt(const char *env) {
    char *prompt = getenv(env);
    
    if (prompt && strlen(prompt) > 0) {
       // printf("[DEBUG] Using custom MY_PROMPT: '%s'\n", prompt);
        return strdup(prompt);
    }

    // Ensure the default prompt has a space at the end
    char *default_prompt = "shell> ";
   // printf("[DEBUG] Using default prompt: '%s' (length: %lu)\n", default_prompt, strlen(default_prompt));

    return strdup(default_prompt);
}


// Changes directory
int change_dir(char **dir) {
    const char *target = dir[1];

    // If no argument is given, go to HOME directory
    if (target == NULL) {
        target = getenv("HOME");
        if (!target) {
            struct passwd *pw = getpwuid(getuid());
            if (pw) {
                target = pw->pw_dir;
            } else {
                fprintf(stderr, "cd: Cannot determine home directory\n");
                return -1;
            }
        }
    }

    // Change directory and handle errors
    if (chdir(target) != 0) {
        perror("cd");
        return -1;
    }

    return 0;
}

// Parses a command string
char **cmd_parse(char const *line) {
    if (!line || *line == '\0') return NULL; // Ignore empty input

    long arg_max = sysconf(_SC_ARG_MAX);
    if (arg_max < 0) arg_max = _POSIX_ARG_MAX;  // Fallback

    char **args = malloc(sizeof(char *) * (arg_max + 1));
    if (!args) {
        perror("malloc failed");
        return NULL;
    }

    char *copy = strdup(line);
    if (!copy) {
        perror("strdup failed");
        free(args);
        return NULL;
    }

    int i = 0;
    char *token = strtok(copy, " ");
    while (token && i < arg_max) {
        args[i] = strdup(token);
        if (!args[i]) {
            perror("strdup failed");
            for (int j = 0; j < i; j++) free(args[j]);
            free(copy);
            free(args);
            return NULL;
        }
        token = strtok(NULL, " ");
        i++;
    }

    args[i] = NULL;  // NULL-terminate the array for execvp()
    free(copy);
    return args;
}

// Frees the memory allocated for an array of strings and the array itself
void cmd_free(char **line) {
    if (!line) return;

    for (size_t i = 0; line[i] != NULL; i++) {
        free(line[i]);
    }
    free(line);
}

// Trims out any unnecessary whitespace
char *trim_white(char *line) {
    if (!line) return NULL;

    while (isspace(*line)) line++;
    char *end = line + strlen(line) - 1;
    while (end > line && isspace(*end)) *end-- = '\0';

    return line;
}

// Executes built-in commands
bool do_builtin(struct shell *sh, char **argv) {
    if (!argv || !argv[0]) return false;

    // Built-in 'exit' command
    if (strcmp(argv[0], "exit") == 0) {
        sh_destroy(sh);  // Clean up allocated memory
        printf("Exiting shell...\n");
        exit(0);
    }

    // Built-in 'cd' command
    else if (strcmp(argv[0], "cd") == 0) {
        return change_dir(argv) == 0;
    }

    // Built-in 'history' command
    else if (strcmp(argv[0], "history") == 0) {
        HIST_ENTRY **hist_list = history_list();
        if (hist_list) {
            for (int i = 0; hist_list[i]; i++) {
                printf("%d: %s\n", i + history_base, hist_list[i]->line);
            }
        }
        return true;
    }

    // Built-in 'fg' command: resumes the last stopped process
    else if (strcmp(argv[0], "fg") == 0) {
        if (last_stopped_pid > 0) {
            printf("Resuming process %d\n", last_stopped_pid);
            kill(last_stopped_pid, SIGCONT);  // Resume the process
            tcsetpgrp(STDIN_FILENO, last_stopped_pid); // Give it terminal control
            waitpid(last_stopped_pid, NULL, WUNTRACED);  // Wait for it
            tcsetpgrp(STDIN_FILENO, getpid());  // Restore shell control
            last_stopped_pid = -1;  // Reset
        } else {
            printf("fg: No stopped jobs\n");
        }
        return true;
    }

    return false; // Not a built-in command, will be handled by execvp()
}

// Initializes the shell and ignores certain signals
void sh_init(struct shell *sh) {
    sh->prompt = get_prompt("MY_PROMPT");
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_pgid = getpid();

    // Put the shell in its own process group
    setpgid(sh->shell_pgid, sh->shell_pgid);
    tcsetpgrp(sh->shell_terminal, sh->shell_pgid);

    // Ignore signals in the parent shell
    signal(SIGINT, SIG_IGN);   // Ignore Ctrl+C
    signal(SIGQUIT, SIG_IGN);  // Ignore Ctrl+\
    signal(SIGTSTP, SIG_IGN);  // Ignore Ctrl+Z
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
}

// Destroys the shell and frees allocated memory
void sh_destroy(struct shell *sh) {
    if (sh->prompt) {
        free(sh->prompt);
        sh->prompt = NULL;
    }
}

// Parses command line arguments from user input
void parse_args(int argc, char **argv) {
    int c;
    while ((c = getopt(argc, argv, "v:")) != -1) {
        switch (c) {
            case 'v':
                printf("Shell Version: %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
                exit(0);
            default:
                fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}
