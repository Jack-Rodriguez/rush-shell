#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//sets max # of arguments to 128, 256 is the total characters of input so this number of args is not likely to be reached
#define MAXARGS 128 
//sets max # of paths to 128
#define MAXPATHS 128 

//the array of paths
char *paths[MAXPATHS];
//aamount of paths in the above array
int pathc = 0;

//this is our parsing function. It goes through the given input and seperates tokens by spaces, tabs, and newlines. It also returns the argc to be used in other functions.
int make_token(char *line, char **args, int maxargs)
{
    //initializes an argument counter
    int argc = 0;
    //sets a pointer to the line
    char *p = line;
    //sets a pointer to the token
    char *token;

    //while the token isn't null, seperate the tokens by " \t\n" which is spaces, tabs, and newlines.
    while ((token = strsep(&p, " \t\n")) != NULL)
    {
        //if the token is empty, skip it
        if(*token == '\0')
        {
            continue;
        }
        //if the argument counter reaches one less than max arguments, we print an error. This shouldn't ever happen but better safe than sorry.
        if(argc >= maxargs - 1)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return -1;
        }

        //token now iterates to the next token in args, using the argument counter to keep track of the index. this also increments the argument counter for us.
        args[argc++] = token;
    }

    //we have to set the larst argument as null so that it can be used later. functions will need to know when it is done.
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
        //if there are 2 arguments, we change the working directory to the second argument
    else
    {
        chdir(args[1]);
    }

    return 0;
}

//this function handles whenever the path command is used in the console. It can create new paths or delete all current ones
int path(char **args, int argc)
{
    //this frees all current paths
    for(int i = 0; i < pathc; i++)
    {
        free(paths[i]);
        paths[i] = NULL;
    }

    //we dont really need this as pathc would work well on its own but I just prefer it so that I can return the value
    int counter = 0;
    //also sets path counter to 0
    pathc = 0;

    //this shouldnt even be possible but if there are no arguments, we print an error
    if (argc == 0)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    //if there are no arguments after path, then we can just return as is because we already cleared the paths
    else if (argc == 1)
    {
        return 0;
    }
    //adds the new paths to the paths array
    else
    {
        //while the counter is less than the argument count (minus one because we dont want to include the path command)
        while(counter < argc - 1) 
        {
            //sets the current path index to be the next argument
            paths[counter] = strdup(args[counter + 1]);
            //increments the counter
            counter++;
        }
    }

    //returns the amount of paths. At the time of coding I don't know if this will be needed but better safe than sorry.
    return counter;
}

//this is my main
int main(int argc, char *argv[]) 
{

    if(argc > 1) 
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    //this initializes the paths array with /bin, and iterates the path counter to 1
    paths[0] = strdup("/bin");
    pathc = 1;

    while(1) 
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

        // Built-in "path"
        if(strcmp(args[0], "path") == 0)
        {
            pathc = path(args, argc);
        }

        
        free(line);
    }
}