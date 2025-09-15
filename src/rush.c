#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXARGS 128 //sets max # of arguments to 128, 256 is the total characters of input so this number of args is not likely to be reached

int make_token(char *line, char **args, int maxargs)
{
    
    int argc = 0;
    char *p = line;
    char *token;

    while ((token = strsep(&p, " \t\n")) != NULL)
    {
        if(*token == '\0')
        {
            continue;
        }
        if(argc >= maxargs - 1)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return -1;
        }

        args[argc++] = token;
    }

    args[argc] = NULL;
    return argc;
}

int change_dir(char **args, int argc)
{
    //cd can only take one argument, and cd itself is one so we need argc to be 2
    if (argc != 2)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        return -1;
    }
    else
    {
        chdir(args[1]);
    }
}

int main(int argc, char *argv[]) {
    
    if(argc > 1) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    while (1) 
    {
        printf("rush> ");
        fflush(stdout);

        char *line = NULL;
        size_t len = 0;
        ssize_t nread = getline(&line, &len, stdin);

        if (nread == -1) 
        { 
            free(line);
            exit(0);
        }

        // Remove trailing newline
        if(line[nread - 1] == '\n') 
        {
            line[nread - 1] = '\0';
        }

        //this creates an array of character pointers (strings)
        char *args[MAXARGS];
        //calls the make_tokens function to tokenize the given/stripped line.
        int argc = make_token(line, args, MAXARGS);

        //if the line is empty or less than one, this iteration is skipped
        if(argc < 0 || argc == 0)
        {
            free(line);
            continue;
        }

        printf("argc: %d\n", argc);
       

        // Built-in "exit"
        if(strcmp(line, "exit") == 0) 
        {
            free(line);
            exit(0);
        }

        // Built-in "cd"
        if(strcmp(args[0], "cd") == 0)
        {
            change_dir(args, argc);
        }

        // For now, do nothing with other input
        free(line);
    }
}
