/*
Luke Darrow 11349190
CptS 360 Lab 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct node
{
    char name[64];//name string of the node
    char type;//node type: 'D' or 'F'
    struct node *childPtr, *siblingPtr, *parentPtr;//node pointers
}Node;

Node *root, *cwd;//root and CWD pointers
char link[128];//user input line
char command[16], pathname[64];//user inputs
char dirname[64], basename[64];//string holders
int error;//error flag

//returns int corresponding with command
int getCommand();
//initializes the root node
void initialize();
//print commands
void menu();
//create a node
Node* createNode(char *pathname, char nodeType);
//find a node and return it
Node* findNode();

//rpwd
void rpwd(Node *myNode, char *myName);

//commands
int mkdir(char *pathname);
int rmdir(char *pathname);
void cd(char *pathname);
void ls(char *pathname);
void pwd();
int creat(char *pathname);
int rm(char *pathname);
int save(char *filename);
int reload(char *filename);
void quit(char *filename);
