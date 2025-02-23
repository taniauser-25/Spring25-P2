
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
#include "lab.h"

// Displays the prompt
char *get_prompt(const char *env) {
  char *prompt = getenv(env);
    return prompt ? strdup(prompt) : strdup("shell>");
}

// Changes dir
int change_dir(char **dir){
  const char *target = dir[1];

  if(target == NULL){
    target = getenv("HOME");
    if(!target){
      struct passwd *pw = getpwuid(getuid());
      if(pw){
        target = pw->pw_dir;
      }else{
        fprintf(stderr, "cd: Cannot determine home directory\n");
        return -1;
      }
    }
  }

  if(chdir(target) != 0){
    perror("cd");
    return -1;
  }

  return 0;
}

//Parses a command string
char **cmd_parse(char const *line){
  if (!line || *line == '\0') return NULL;

    long arg_max = sysconf(_SC_ARG_MAX);
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

    args[i] = NULL;
    free(copy); 
    return args;
}

// Frees the memory allocated for an array of strings and the array itself
void cmd_free(char ** line){
  if (!line){
    return;
  } 
  for (size_t i = 0; line[i] != NULL; i++) {
      if (line[i] != NULL) {
          // printf("Freeing line[%zu]: %p\n", i, (void*)line[i]); //Debug statement
          free(line[i]);
      }
  }

  // printf("Freeing line array itself: %p\n", (void*)line); // Debug statement
  free(line);
}

//Trims out any unnecessary white spaces
char *trim_white(char *line){
  if (!line) return NULL;

    while (isspace(*line)) line++;
    char *end = line + strlen(line) - 1;
    while (end > line && isspace(*end)) *end-- = '\0';

    return line;
}

// Executes shell commands
bool do_builtin(struct shell *sh, char **argv){
  if (!argv || !argv[0]) return false;

    if (strcmp(argv[0], "exit") == 0) {
        sh_destroy(sh);
        exit(0);
    } else if (strcmp(argv[0], "cd") == 0) {
        return change_dir(argv) == 0;
    } else if (strcmp(argv[0], "history") == 0) {
        HIST_ENTRY **history = history_list();
        if (history) {
            for (int i = 0; history[i]; i++) {
                printf("%d: %s\n", i + history_base, history[i]->line);
            }
        }
        return true;
    }
    return false;
}

// Initializes the shell
void sh_init(struct shell *sh){
  sh->prompt = get_prompt("MY_PROMPT");
  sh->shell_terminal = STDIN_FILENO;
  sh->shell_pgid = getpid();

  // Puts the shell in its own process group
  setpgid(sh->shell_pgid, sh->shell_pgid);
  tcsetpgrp(sh->shell_terminal, sh->shell_pgid);

  // Ignores the signals
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
}

// Destroys the shell and frees accordingly
void sh_destroy(struct shell *sh){
  if (sh->prompt) {
    free(sh->prompt);
    sh->prompt = NULL;
  }
}

//Parses Command line args
void parse_args(int argc, char **argv){
  int c;
  while ((c = getopt(argc, argv, "v:")) != -1) {
      switch (c) {
          case 'v':
              printf("Shell Version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
              exit(0);
          default:
              fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
              exit(EXIT_FAILURE);
      }
  }
}



