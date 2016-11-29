/*
Luke Darrow 11349190
CptS 360 Lab 3
*/

#include "functions.c"

int main(int argc, char **argv, char* envp[])
{
    //input from user, array of parsed commands, temp array for strtok. command and argument for use in the simpleCommand function
    char input[1024], *cmd_array[1024], temp_array[1024], *cmd, *arg;
    //token for breaking apart the bin paths, temp used to hold result from getenv() for parsing
    char *token, *temp = getenv("PATH");
    //iterators i and j. process id and status.
    int i = 0, j = 0, pid, status;
    int pd[2];
    
    //set up path
    token = strtok(temp, ":");
    while(token != NULL)
    {
        path[i++] = token;
        token = strtok(NULL, ":");
    }
    //set last value to null for termination later
    path[i] = NULL;
    i = 1;

    //set home based on the environment using getenv()
    //getenv() returns a pointer to a string containing the value
    //for the specified name. else a null pointer is returned.
    home = getenv("HOME");

    //print before input based off the style of kc's example
    printf("mysh % :");

    //cmd arg1 arg2 arg3 ... argn   remember fgets takes in the entire line fgets(location, buffer, format)
    while(fgets(input, 1024, stdin) != NULL)
    {
        //get rid of new line char at end
        strtok(input, "\n");
        printf("input = %s\n", input);
        //make sure input is valid and not empy
        if(strcmp(input, "") != 0 && strcmp(input, "\n") != 0)
        {
            //tokenize the input into seperate pipes
            cmd_array[0] = strtok(input, "|");
            //strtok picks up where earlier call left off
            //keep going until no more |'s
            while(cmd_array[i] = strtok(NULL, "|"))
            {
                i++;
            }
            
            //process commands
            //if i == 1, there's only one command to do
            if(i == 1)
            {
                printf("processing single command\n");
                //setting up variables for simpleCommand function
                strcpy(temp_array, cmd_array[0]);

                cmd = strtok(temp_array, " ");
                arg = strtok(NULL, " ");
                //printf("cmd = %s, arg = %s\n", cmd, arg);
                if(simpleCommand(cmd, arg) == 0)
                {
                    //fork
                    pid = fork();
                    //printf("pid = %d\n", pid);
                    if(pid)
                    {
                        //parent
                        pid = wait(&status);
                    }
                    else
                    {
                        //child
                        doCommand(cmd_array[0], envp);
                    }
                    printf("child PROC %d died : exit status = %d\n", pid, status);
                }
            }
            else
            {
                //pipe present
                //can only do simple head and tail pipe
                printf("processing pipe\n");
                //pass head and tail into doPipe
                pid = fork();
                if(pid)
                {
                    pid = wait(&status);
                }
                else
                {
                    doPipe(cmd_array[j], cmd_array[j+1], envp);
                }
            }
        }
        else
            printf("invalid input");

        //reset everything for the next input
        memset(input, 0, sizeof(input));
        i = 1;
        printf("mysh % :");
    }

    return 0;
}

