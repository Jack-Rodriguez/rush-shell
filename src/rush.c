#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
//had a ton of errors so i needed the above definitions to be added
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


//sets max # of arguments to 128, 256 is the total characters of input so this number of args is not likely to be reached
#define MAXARGS 128 
//sets max # of paths to 128
#define MAXPATHS 128 

//the array of paths
char *paths[MAXPATHS];
//aamount of paths in the above array
int pathc = 0;

//this will store the executable path if there is one
char full_path[1024];

//stores the file that the output will be redirected to
char output_file[1024];


//this is our parsing function. It goes through the given input and seperates tokens by spaces, tabs, and newlines. It also returns the argc to be used in other functions.
int make_token(char *line, char **args, int maxargs)
{
    //initializes an argument counter
    int argc = 0;
    //sets a pointer to the line
    char *p = line;
    //sets a pointer to the token
    char *token;

    //sets output file as empty
    output_file[0] = '\0';

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
            fflush(stderr);
            return -1;
        }

        //token now iterates to the next token in args, using the argument counter to keep track of the index. this also increments the argument counter for us.
        args[argc++] = token;
    }

    //we have to set the larst argument as null so that it can be used later. functions will need to know when it is done.
    args[argc] = NULL;

    //this will search for the > symbol so that any errors can be handeled
    for(int i = 0; i < argc; i++)
    {
        if(strcmp(args[i], ">") == 0)
        {
            //if there are multiple > symbols or if there is more than one arguments to the right of >
            if(i != argc - 2 || strcmp(args[i+1], ">") == 0)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                fflush(stderr);
                return -1;
            }
             //no errors
            else
            {
                //makes the output file whatever the last argument is
                strncpy(output_file, args[i+1], sizeof output_file);
                output_file[sizeof output_file - 1] = '\0';

                //ensures that execv doesnt use the > symbol or the file name
                args[i] = NULL;
                break; 
            }
        }
    }
    //returns argument count
    return argc;
}

//change directory function
int change_dir(char **args, int argc)
{
    //cd can only take one argument, and cd itself is one so we need argc to be 2
    if (argc != 2)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);
        return -1;
    }
        //if there are 2 arguments, we change the working directory to the second argument
    else
    {
        //changes directory to the second argument
        chdir(args[1]);
    }

    return 0;
}

//this function handles whenever the path command is used in the console. It can create new paths or delete all current ones
int path(char **args, int argc)
{
    //this will ensure that all passed arguments are executable
    for(int i = 1; i < argc; i++)
        {
            if(access(args[i], X_OK) != 0)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                fflush(stderr);
                return pathc;
            }
        }
    
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
        fflush(stderr);
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

int is_executable(char *cmd, char **paths, int pathc)
{
    for(int i = 0; i < pathc; i++)
    {
        //before hand I didnt know how to combine the strings, so I got the snprint function from chatgpt, and learned how it works. I still wrote this function myself though.
        //so we set a string to be equal to the current path
        char *dir = paths[i];
        //next we create a new string to be the full path of the command
        char exPath[1024];
        //we set the cap to the size of the string, we should never exceed this but its important to be safe
        size_t cap = sizeof exPath;
        //combines the path, a slash, and the command into the new string
        int n = snprintf(exPath, cap, "%s/%s", dir, cmd);
        //checks the size
        if (n >= 0 && (size_t)n < sizeof exPath)
        {
            //this checks if the path is executable. if so we return 0
            if(access(exPath, X_OK) == 0)
            {
                //sets the full path to be the executable path
                strncpy(full_path, exPath, sizeof full_path);
                full_path[sizeof full_path - 1] = '\0';
                return 0;
            }
        }
    }
    //this will be returned if the all paths are not executable, and if the size was too big
    return -1;
}

//this is my main
int main(int argc, char *argv[]) 
{
    //we end up not using argv but I left it in just in case. It was giving me a warning so i added this line
    (void)argv; 

    //if there are more than one arguments, we print an error and exit
    if(argc > 1) 
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);
        exit(1);
    }

    //this initializes the paths array with /bin, and iterates the path counter to 1
    paths[0] = strdup("/bin");
    pathc = 1;

    //main loop of shell
    while(1) 
    {
        //prints the prompt
        printf("rush> ");
        fflush(stdout);

        //collects user input
        char *line = NULL;
        size_t len = 0;
        ssize_t nread = getline(&line, &len, stdin);

        //if there was no input, we go to the next iteration
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
        //___________________Built Ins_______________________
        // Built-in "exit"
        if(strcmp(args[0], "exit") == 0) 
        {
            //if there is more/less than one argument, we print an error
            if(argc != 1)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                fflush(stderr); 
                free(line);
                continue;
            }
            //if there is only one argument (which is correct), we exit
            free(line);
            exit(0);
        }
        // Built-in "cd"
        else if(strcmp(args[0], "cd") == 0)
        {
            change_dir(args, argc);
        }
        // Built-in "path"
        else if(strcmp(args[0], "path") == 0)
        {
            pathc = path(args, argc);
        }
        //-___________________Executables_______________________
        else
        {
            if(is_executable(args[0], paths, pathc) == 0)
            {
                //I got most of this section from the chapter 5 lecture slides
                //creates a child process
                int rc = fork();
                if (rc < 0) 
                { 
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    fflush(stderr);
                    exit(1);
                }
                //CHILD PROCESS
                else if (rc == 0)
                {
                    //not sure why this was needed but it was the only way I could get "non built in" error handling to match that of the examples
                    args[0] = full_path;  

                    //if there is an output file, we redirect the output to that file
                    if (output_file[0] != '\0') 
                    {
                        if (!freopen(output_file, "w", stdout)) 
                        { 
                            char error_message[30] = "An error has occurred\n";
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            fflush(stderr);
                            exit(1);
                        }
                    }
                    //executes the command inputted by the user
                    execv(full_path, args);
                    //if execv fails, the program will return here and print the error
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    fflush(stderr);
                    exit(1);
                }
                //PARENT PROCESS
                else 
                { 
                    //ensures that the parent process waits for the child process to finish
                    wait(NULL);
                }
            }
            else
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                fflush(stderr);
            }
        }

        
        free(line);
    }
}