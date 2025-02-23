#ifndef LAB_H
#define LAB_H

#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define UNUSED(x) (void)(x); // Fixed macro

#ifdef __cplusplus
extern "C"
{
#endif

struct shell {
    int shell_is_interactive;
    pid_t shell_pgid;
    struct termios shell_tmodes;
    int shell_terminal;
    char *prompt;
    pid_t last_stopped_pid; // Added to track last stopped process
};

/**
 * @brief Set the shell prompt. This function will attempt to load a prompt
 * from the requested environment variable. If the environment variable is
 * not set, a default prompt of "shell> " (with space) is returned. 
 * This function calls malloc internally and the caller must free the resulting string.
 *
 * @param env The environment variable
 * @return const char* The prompt
 */
char *get_prompt(const char *env);

/**
 * Changes the current working directory of the shell. Uses the Linux system
 * call chdir. With no arguments, the user’s home directory is used.
 *
 * @param dir The directory to change to
 * @return On success, zero is returned. On error, -1 is returned, and
 * errno is set to indicate the error.
 */
int change_dir(char **dir);

/**
 * @brief Convert a line read from the user into a format that will work with
 * execvp. We limit the number of arguments to ARG_MAX loaded from sysconf.
 * This function allocates memory that must be reclaimed with the cmd_free function.
 *
 * @param line The line to process
 * @return The parsed command in a format suitable for exec
 */
char **cmd_parse(char const *line);

/**
 * @brief Free the line that was constructed with parse_cmd
 *
 * @param line The line to free
 */
void cmd_free(char ** line);

/**
 * @brief Trim whitespace from the start and end of a string.
 * For example, " ls -a " becomes "ls -a".
 * This function modifies the input string.
 *
 * @param line The line to trim
 * @return The trimmed line with no leading/trailing whitespace
 */
char *trim_white(char *line);

/**
 * @brief Checks if the first argument is a built-in command like exit, cd, or jobs.
 * If the command is built-in, it will be executed inside the shell.
 *
 * @param sh The shell instance
 * @param argv The command to check
 * @return True if the command was a built-in command
 */
bool do_builtin(struct shell *sh, char **argv);

/**
 * @brief Initializes the shell for use. Allocates necessary data structures,
 * grabs control of the terminal, and puts the shell in its own process group.
 *
 * @param sh The shell instance
 */
void sh_init(struct shell *sh);

/**
 * @brief Destroys the shell. Frees any allocated memory and resources and exits
 * normally.
 *
 * @param sh The shell instance
 */
void sh_destroy(struct shell *sh);

/**
 * @brief Parse command line args from the user when the shell was launched
 *
 * @param argc Number of args
 * @param argv The argument array
 */
void parse_args(int argc, char **argv);

/**
 * @brief Resumes the last stopped process using the 'fg' command.
 */
void resume_last_stopped();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
