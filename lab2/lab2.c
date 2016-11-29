/*
Luke Darrow 11349190
CptS 360 Lab 2
*/
#include "functions.c"

int main()
{
    char input[128];
    int ID = 0;
    *input = '\0';
    *command = '\0';
    *pathname = '\0';
    error = 0;
    //initialize root and cwd and set cwd to root
    initialize();

    while(1)
    {
        strcpy(pathname, "");
        strcpy(command, "");
        printf("input a command: ");
        //var, buffersize, type of in
        fgets(input, 128, stdin);
        //sscanf splits the command from the pathname
        sscanf(input, "%s %s", &command, &pathname);
        //sets ID to int based on given command
        ID = getCommand();
        //executes function for given command
        switch(ID)
        {
            case 0 : menu();     break;
            case 1 : mkdir(pathname);    break;
            case 2 : rmdir(pathname);    break;
            case 3 : cd(pathname);       break;
            case 4 : ls(pathname);       break;
            case 5 : pwd();      break;
            case 6: creat(pathname);     break;
            case 7: rm(pathname);        break;
            case 8: save(pathname);      break;
            case 9: reload(pathname);    break;
            case 10: quit(pathname);     break;
            default: printf("Invalid command, try again\n"); break;
        }
        //quit
        if(ID == 10)
            break;
    }
    return 0;
}
