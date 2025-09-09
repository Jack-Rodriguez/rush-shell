#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_LINE 80 /* Maximum length of command line */

/* Function to parse command line input */
void parse_command(char *line, char **args, int *background) {
    int i = 0;
    char *token = strtok(line, " \t\n");
    
    while (token != NULL && i < MAX_LINE/2) {
        args[i] = token;
        i++;
        token = strtok(NULL, " \t\n");
    }
    
    /* Check for background execution */
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        *background = 1;
        args[i-1] = NULL;
    } else {
        *background = 0;
        args[i] = NULL;
    }
}

/* Built-in command: exit */
int builtin_exit(char **args) {
    (void)args; /* Suppress unused parameter warning */
    printf("Goodbye!\n");
    exit(0);
}

/* Built-in command: cd */
int builtin_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "rush: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("rush");
        }
    }
    return 1;
}

/* Built-in command: pwd */
int builtin_pwd(char **args) {
    (void)args; /* Suppress unused parameter warning */
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("rush");
    }
    return 1;
}

/* Array of built-in commands */
char *builtin_str[] = {
    "exit",
    "cd",
    "pwd"
};

int (*builtin_func[]) (char **) = {
    &builtin_exit,
    &builtin_cd,
    &builtin_pwd
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/* Execute built-in command */
int execute_builtin(char **args) {
    int i;
    
    if (args[0] == NULL) {
        return 1;
    }
    
    for (i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    
    return 0; /* Not a built-in command */
}

/* Execute external command */
void execute_command(char **args, int background) {
    pid_t pid;
    int status;
    
    pid = fork();
    
    if (pid == 0) {
        /* Child process */
        if (execvp(args[0], args) == -1) {
            perror("rush");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        /* Error forking */
        perror("rush");
    } else {
        /* Parent process */
        if (!background) {
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        } else {
            printf("[Process %d running in background]\n", pid);
        }
    }
}

/* Main shell loop */
int main(void) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *args[MAX_LINE/2 + 1];
    int background;
    
    printf("Welcome to rush shell!\n");
    printf("Type 'exit' to quit.\n\n");
    
    while (1) {
        printf("rush> ");
        
        /* Read command line */
        read = getline(&line, &len, stdin);
        
        if (read == -1) {
            printf("\nGoodbye!\n");
            break;
        }
        
        /* Skip empty lines */
        if (strlen(line) <= 1) {
            continue;
        }
        
        /* Parse command */
        parse_command(line, args, &background);
        
        /* Skip if no command */
        if (args[0] == NULL) {
            continue;
        }
        
        /* Try to execute as built-in command */
        if (!execute_builtin(args)) {
            /* Execute as external command */
            execute_command(args, background);
        }
    }
    
    free(line);
    return 0;
}