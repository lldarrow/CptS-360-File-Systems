/*
Luke Darrow 11349190
CptS 360 Lab 2
*/
#include "lab2.h"

//return int corresponding with command
int getCommand()
{
    if(!strcmp("menu", command))
        return 0;
    else if(!strcmp("mkdir", command))
        return 1;
    else if(!strcmp("rmdir", command))
        return 2;
    else if(!strcmp("cd", command))
        return 3;
    else if(!strcmp("ls", command))
        return 4;
    else if(!strcmp("pwd", command))
        return 5;
    else if(!strcmp("creat", command))
        return 6;
    else if(!strcmp("rm", command))
        return 7;
    else if(!strcmp("save", command))
        return 8;
    else if(!strcmp("reload", command))
        return 9;
    else if(!strcmp("quit", command))
        return 10;
    else
        return 11;
}

//initialize the directories
void initialize()
{
    root = malloc(sizeof(Node));
    cwd = malloc(sizeof(Node));
    strcpy(root->name, "/");
    root->type = 'D';
    //set parent of root to root, in case someone tries to go up
    root->parentPtr = NULL;
    root->childPtr = NULL;
    root->siblingPtr = NULL;
    //set current working directory to root
    cwd = root;
}

//shows valid commands
void menu()
{
    printf("valid commands: mkdir, rmdir, cd, ls, pwd, creat, rm, save, reload, quit.\n");
}

Node* findNode(char pathname[64])
{
    //iterators
    int i = 0, j = 0;
    //name of directory or file node
    char myname[64] = "";
    Node *temp = malloc(sizeof(Node));
    temp = root;
    //if the pathname is empty or '/' then return root
    if(strcmp(pathname, "/") == 0 || pathname[0] == '\0')
        return root;
    //if starts with '/', then it starts from root, else start from cwd
    if(pathname[0] == '/')
        i++;
    else
        temp = cwd;

    //else traverse the tree through the pathname
    for(;pathname[i] != '\0'; i++)
    {
        if(pathname[i] != '/')
        {
            myname[j] = pathname[i];
            j++;
        }
        else
        {
            //printf("looking for %s\n", myname);
            //if no childPtr, nothing else to look for, return NULL
            if(temp->childPtr == NULL)
            {
                temp = NULL;
                error = 1;

                return temp;
            }
            //returns null if two slashes after each other
            if(!strcmp(myname, ""))
                return temp = NULL;
            
            //if the child is next part of directory then set temp to it
            if(!strcmp(temp->childPtr->name, myname))
            {
                temp = temp->childPtr;
                //printf("found %s\n", temp->name);
            }            
            else
            {
                //set temp to childPtr to look through linked list
                temp = temp->childPtr;
                while(temp->siblingPtr != NULL)
                {
                    if(!strcmp(temp->siblingPtr->name, myname))
                    {
                        //sibling is the next part of directory
                        temp = temp->siblingPtr;
                        //printf("found %s\n", temp->name);
                        break;
                    }
                    else
                        temp = temp->siblingPtr;
                }
                //if you reach the end and none of them match return null
                if(temp->siblingPtr == NULL && !strcmp(temp->name, myname))
                {
                    error = 1;
                    return temp = NULL;
                }
            }
            //reset name for next directory/file if not end of pathname
            if(pathname[i+1] != '\0')
            {
                for(j = 0; j < 64; j++)
                    myname[j] = '\0';
                j = 0;
            }
        }
    }
    //if no childPtr, nothing else to look for, return NULL
    if(temp->childPtr == NULL)
        return temp = NULL;
    //returns null if two slashes after each other
    if(!strcmp(myname, ""))
        return temp = NULL;
    
    //if the child is next part of directory then set temp to it
    if(!strcmp(myname, temp->childPtr->name))
        return temp = temp->childPtr;
    else
    {
        //set temp to childPtr to look through linked list
        temp = temp->childPtr;
        while(temp->siblingPtr != NULL)
        {
            if(!strcmp(temp->siblingPtr->name, myname))
            {
                //sibling is the next part of directory
                return temp = temp->siblingPtr;
            }
            else
                temp = temp->siblingPtr;
        }
    }
    //check if destination node matches end of pathname, else return NULL ptr
    if(!strcmp(temp->name, myname))
        return temp;
    return temp = NULL;
}

//create node, takes pathname and node type. Basically the same code for findNode but it stops at the parent since it's to be created
Node* createNode(char *pathname, char nodeType)
{
    //iterators
    int i = 0, j = 0;
    //name of directory or file node
    char myname[64] = "";
    Node *temp = malloc(sizeof(Node)), *new = malloc(sizeof(Node));
    temp = root;
    
    //check if trying to make root, because you can't
    if(!strcmp(pathname, "/"))
    {
        return NULL;
    }

    //if starts with '/', then it starts from root, else start from cwd
    if(pathname[0] == '/')
        i++;
    else
        temp = cwd;

    //else traverse the tree through the pathname
    for(;pathname[i] != '\0'; i++)
    {
        if(pathname[i] != '/')
        {
            myname[j] = pathname[i];
            j++;
        }
        else
        {
            //returns null if two slashes after each other
            if(!strcmp(myname, ""))
                return temp = NULL;
            //if the child is next part of directory then set temp to it
            if(!strcmp(myname, temp->childPtr->name))
                temp = temp->childPtr;
            else
            {
                temp = temp->childPtr;
                while(temp->siblingPtr != NULL)
                {
                    if(!strcmp(temp->siblingPtr->name, myname))
                    {
                        //sibling is the next part of directory
                        temp = temp->siblingPtr;
                        break;
                    }
                    else
                        temp = temp->siblingPtr;
                }
            }
            //reset name for next directory/file if not end of pathname
            if(pathname[i+1] != '\0')
            {
                for(j = 0; j < 64; j++)
                    myname[j] = '\0';
                j = 0;
            }
        }
    }
    //set up the new node
    //printf("%s\n", myname);
    if(temp->type == 'F')
    {
        printf("invalid parent type\n");
        return NULL;
    }
    strcpy(new->name, myname);
    new->parentPtr = temp;
    new->type = nodeType;
    new->childPtr = NULL;
    //if no childptr for parent, point to new node
    if(temp->childPtr == NULL)
    {
        temp->childPtr = new;
    }
    else
    {
        //move until no sibling, then insert the new node as the new sibling
        temp = temp->childPtr;
        //move until no more siblings
        while(temp->siblingPtr != NULL)
            temp = temp->siblingPtr;
        //set newest sibling to new
        temp->siblingPtr = new;
    }
    //print that node was created
    //printf("created: %s\n", new->name);
    return new;
}

//creates a new directory, doesn't check if possible to reach pathname though
//possible way to check if possible to reach is the make findNode check if
//there's more text left after it determines theres no destination
//if it's in the main body of the findNode, then that means there's a '/'
//meaning there's more text. otherwise, that's the last element. Therefore
//the return null there should be special.
//created int error in header, would have to implement this for all pathname errors of this type however
int mkdir(char *pathname)
{
    Node *temp = malloc(sizeof(Node));
    temp = findNode(pathname);
    if(temp != NULL)
    {
        printf("error, directory already exists\n");
        return 1;
    }
    if(error == 1)
    {
        printf("error, can't reach destination\n");
        error = 0;
        return 1;
    }
    //use create node function
    createNode(pathname, 'D');
    printf("created %s\n", pathname);
    return 0;
}

//remove directory
int rmdir(char *pathname)
{
    Node *directory = malloc(sizeof(Node));
    Node *parent = malloc(sizeof(Node));
    char myName[64];
    
    //set file to the directory node to be deleted
    directory = findNode(pathname);

    if(directory == cwd)
    {
        printf("can't delete cwd\n");
        return 1;
    }
    
    //if findNode returns NULL, pathname does not exist or if node is not directory type, return 1
    if(directory == NULL || directory->type != 'D')
    {
        printf("could not find directory\n");
        error = 0;
        return 1;
    }
    //save the name, going to figure out where it is exactly
    strcpy(myName, directory->name);
    //set parent to parent of file
    parent = directory->parentPtr;
    //set file to the child of the parent
    directory = parent->childPtr;
    //easiest case, the parent's child is the one to be deleted
    if(!strcmp(directory->name, myName))
    {
        printf("removing a %s\n", myName);
        parent->childPtr = directory->siblingPtr;
        return 0;
    }
    //find which one of children siblings is to be deleted, if it is child, then set child to next sibling if any
    while(directory->siblingPtr != NULL)
    {
        if(!strcmp(directory->siblingPtr->name, myName))
        {
            printf("removing b %s\n", myName);
            //close the gap, leaving sibling out
            directory->siblingPtr = directory->siblingPtr->siblingPtr;
            return 0;
        }
        directory = directory->siblingPtr;
    }
    return 0;
}

//change directory
//cd poop/wow did not work when poop existed and wow didn't
void cd(char *pathname)
{
    //tmp node
    Node *temp = malloc(sizeof(Node));
    //see if you can even find the directory specified, else print error
    temp = findNode(pathname);
    //if node != null and temp is a directory, then set cwd to it
    if(temp == NULL)
    {
        printf("Directory does not exist\n");
    }
    else if(temp != NULL && temp->type == 'D')
    {
        cwd = temp;
    }
    else if(temp->type == 'F')
    {
        printf("File type not directory\n");
    }
    else
    {
        printf("Unknown error\n");
    }
    //reset error flag in case it was triggered
    error = 0;
}

//show contents of directory, if pathname empty, show cwd contents
void ls(char *pathname)
{
    Node *show = malloc(sizeof(Node));
    show = cwd;
    
    if(pathname[0] == '\0')
    {
        printf("%s contains: ", show->name);
        if(show->childPtr != NULL)
        {
            show = show->childPtr;
            printf("%s ", show->name);
            while(show->siblingPtr != NULL)
            {
                show = show->siblingPtr;
                printf("%s ", show->name);
            }
        }
    }
    else
    {
        show = findNode(pathname);
        if(show == NULL || show->type == 'F')
        {
            printf("Invalid directory\n");
            error = 0;
        }
        else
        {
            printf("%s contains: ", show->name);
            if(show->childPtr != NULL)
            {
                show = show->childPtr;
                printf("%s ", show->name);
                while(show->siblingPtr != NULL)
                {
                    show = show->siblingPtr;
                    printf("%s ", show->name);
                }
            }
        }
    }
    printf("\n");
}

//printf absolute pathname of CWD
void rpwd(Node *myNode, char* myName)
{
    char absolute[64] = "", temporary[64] = "";
    Node *temp = malloc(sizeof(Node));
    
    //append cwd name to pathnam
    strcpy(absolute, "/");
    strcat(absolute, myNode->name);
    
    if(myNode != root)
        temp = myNode->parentPtr;
    else
    {
        temp = root;
        strcpy(absolute, "/");
    }
        
    while(temp != root)
    {
        //put absolute in temporary as we add to absolute
        strcpy(temporary, absolute);
        //set absolute to / before adding directory/file name
        strcpy(absolute, "/");
        //add the name
        strcat(absolute, temp->name);
        //put the absolute back on the end
        strcat(absolute, temporary);
        temp = temp->parentPtr;
    }
    strcpy(myName, absolute);
}


//printf absolute pathname of CWD
void pwd()
{
    char myName[64] = "";
    printf("pathname of CWD: ");  
    rpwd(cwd, myName);
    printf("%s\n", myName);
}

//create file node
int creat(char *pathname)
{
    //check if already exists
    if(findNode(pathname) != NULL || findNode(pathname) == root)
    {
        printf("error: pathname taken\n");
        error = 0;
        return 1;
    }
    //doesn't exist, create it based on the pathname
    createNode(pathname, 'F');
    printf("created %s\n", pathname);
    return 0;
}

//if this file is the child of the parent then the child needs to move over to sibling if any
int rm(char *pathname)
{
    Node *file = malloc(sizeof(Node));
    Node *parent = malloc(sizeof(Node));
    char myName[64] = "";
    
    //set file to the file node to be deleted
    file = findNode(pathname);
    
    if(file == cwd)
    {
        printf("can't delete cwd\n");
        return 1;
    }

    //if findNode returns NULL, pathname does not exist or if node is not file type, return 1
    if(file == NULL || file->type != 'F')
    {
        printf("file not found\n");
        error = 0;
        return 1;
    }
    //save the name, going to figure out where it is exactly
    strcpy(myName,file->name);
    //set parent to parent of file
    parent = file->parentPtr;
    //set file to the child of the parent
    file = parent->childPtr;
    //easiest case, the parent's child is the one to be deleted
    if(!strcmp(file->name, myName))
    {
        //set parent's child to next one over
        printf("removing %s\n", file->name);
        parent->childPtr = file->siblingPtr;
        return 0;
    }
    //find which one of children siblings is to be deleted, if it is child, then set child to next sibling if any
    while(file->siblingPtr != NULL)
    {
        if(!strcmp(file->siblingPtr->name, myName))
        {
            printf("removing %s\n", file->siblingPtr->name);
            //close the gap, leaving sibling out
            file->siblingPtr = file->siblingPtr->siblingPtr;
            return 0;
        }
        file = file->siblingPtr;
    }
    return 0;
}

//save to file
//writes type, then pathname to file
//fprintf(fp, "%c %s", 'D', "namestring\n"); write line to file
//since root is always initialized, it might not need to save or open root
int save(char *filename)
{
    Node *temp = malloc(sizeof(Node));
    char myName[64] = "";
    //open FILE stream for WRITE
    FILE *fp = fopen(filename, "w+");

    //let's just start by printing the traversal in order
    fprintf(fp, "D %s\n",root->name);
    temp = root;
    while(temp != NULL)
    {
        if(temp->childPtr != NULL)
        {
            temp = temp->childPtr;
            rpwd(temp, myName);
            fprintf(fp, "%c %s\n",temp->type, myName);
        }
        else
        {
            //we've reached the bottom of something, print out every sibling
            while(temp->siblingPtr != NULL)
            {
                temp = temp->siblingPtr;
                rpwd(temp, myName);
                fprintf(fp, "%c %s\n",temp->type, myName);
            }
            temp = temp->parentPtr;
            while(temp->siblingPtr == NULL && temp->parentPtr != NULL)
            {
                temp = temp->parentPtr;
            }
            temp = temp->siblingPtr;
            if(temp != NULL)
            {
                rpwd(temp, myName);
                fprintf(fp, "%c %s\n", temp->type, myName);
            }
        }
    }    
    printf("%s saved\n", filename);
    fclose(fp);
    return 0;
}

//load from file
int reload(char *filename)
{
    //temp line holder set to 66 as pathname can only be 64 long
    //and type + space makes it 66 at the most
    char line[66], myPath[64], myType;
    //create temp node and open file
    Node *temp = malloc(sizeof(Node));
    FILE *fp = fopen(filename, "r");
    //reset file system
    root->childPtr = NULL;

    //check if file opened correctly
    if(fp == NULL)
    {
        printf("error, file not found\n");
        return 1;
    }

    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%c %s", &myType, &myPath);
        createNode(myPath, myType);
    }

    printf("%s opened\n", filename);
    fclose(fp);
    return 0;
}

//quit, calls save then quits
void quit(char *filename)
{
    //no file specified don't get to save
    if(!strcmp(filename, ""))
        printf("no save file specified, closing without saving\n");
    else
        save(filename);
}
