/*
Luke Darrow 11349190
CptS 360
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
//included this for ls
#include <dirent.h>

#define MAX 10000
typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];


main(int argc, char *argv[]) 
{
  int i, m, r;
  char cwd[128];
  //my variables
  FILE *fp, *gp;
  int c;
  char buf[4096];
  //ls variables
  DIR *dir;
  //directory entry
  struct dirent *dir_entry;

  m = getinputs();    // get user inputs name=value into entry[ ]
  getcwd(cwd, 128);   // get CWD pathname

  printf("Content-type: text/html\n\n");
  printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);

  printf("<H1>Echo Your Inputs</H1>");
  printf("You submitted the following name/value pairs:<p>");
 
  for(i=0; i <= m; i++)
     printf("%s = %s<p>", entry[i].name, entry[i].value);
  printf("<p>");


  /*****************************************************************
   Write YOUR C code here to processs the command
         mkdir dirname
         rmdir dirname
         rm    filename
         cat   filename
         cp    file1 file2
         ls    [dirname] <== ls CWD if no dirname
  *****************************************************************/
  
  if(!strcmp(entry[0].value, "mkdir"))
  {
	  //do mkdir
	  if(!strcmp(entry[1].value, ""))
		  printf("missing directory name\r");
	  else
		  mkdir(entry[1].value, 0775);
  }
  else if(!strcmp(entry[0].value, "rmdir"))
  {  
	  //do rmdir
	  if(!strcmp(entry[1].value, ""))
		  printf("missing directory name\r");
	  else
		  rmdir(entry[1].value);
  }
  else if(!strcmp(entry[0].value, "rm"))
  {
	  //do rmi
	  if(!strcmp(entry[1].value, ""))
		  printf("missing file name\r");
	  else
		  unlink(entry[1].value);
  }
  else if(!strcmp(entry[0].value, "cat"))
  {
	  //do cat
	  //if there is no filename, don't do anything
	  if(entry[1].value)
	  {
		  fp = fopen(entry[1].value, "r");
		  if(fp != NULL)
		  {
			  while((c = fgetc(fp)) != EOF)
			  {
				  //for each \n spit out \r
				  if(c == 10)
                  {
                      printf("<p>");
					  putchar('\r');
                   }
				  else
					  putchar(c);
			  }
		  }
		  fclose(fp);
	  }
  }
  else if(!strcmp(entry[0].value, "cp"))
  {
	  //do cp
	  //if there aren't two filenames, don't do anything
	  if(entry[1].value && entry[2].value)
	  {
		  fp = fopen(entry[1].value, "r");
		  if(fp != NULL)
		  {
			  gp = fopen(entry[2].value, "w");
			  if(gp != NULL)
			  {
				  while(c = fread(buf, 1, 4096, fp))
			  	  {
					  fwrite(buf, 1, c, gp);
			  	  }
			  }
			  fclose(gp);
		  }
		  fclose(fp);
	  }
  }
  else if(!strcmp(entry[0].value, "ls"))
  {
	  //do ls
	  //if argument is empty, then ls cwd
	  if(strcmp(entry[1].value, ""))
	  {
		  //not empty, ls argument
		  dir = opendir(entry[1].value);
		  while((dir_entry = readdir(dir)) != NULL)
			  printf("%s ", dir_entry->d_name);
		  closedir(dir);
		  printf("\n");
	  }
	  else
	  {
		  dir = opendir(".");
		  while((dir_entry = readdir(dir)) != NULL)
			  printf("%s ", dir_entry->d_name);
		  closedir(dir);
		  printf("\n");
	  }
  }
  //reset c to 0
  c = 0;

  // create a FORM webpage for user to submit again 
  printf("</title>");
  printf("</head>");
  printf("<body bgcolor=\"#FF0000\" link=\"#330033\" leftmargin=8 topmargin=8");
  printf("<p>------------------ DO IT AGAIN ----------------\n");
  
  printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~darrow/cgi-bin/mycgi\">");

  //------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
  //printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~YOURNAME/cgi-bin/mycgi\">");
  
  printf("Enter command : <INPUT NAME=\"command\"> <P>");
  printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
  printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
  printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
  printf("</form>");
  printf("------------------------------------------------<p>");

  printf("</body>");
  printf("</html>");
}

