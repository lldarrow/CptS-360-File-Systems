/*****************************************
CptS 360 Final Project

touch_file

Programmers:
David Hoekman 11423672
Luke Darrow 11349190

Last Modified: 11/23/15
*****************************************/


//update time if it exists, if it doesn't exist, it creates it
void touch_file(char path[124])
{
  int ino;
  int newmode = 0;
  MINODE *mip = running->cwd;
  MINODE *touchmip = NULL;
  INODE *ip = NULL;
  char fullpath[128];
  strcpy(fullpath, path);

  if (!strcmp(path, ""))
  {
    printf("No pathname given!\n");
    return;
  }

  printf("path = %s\n", path);
  ino = getino(running->cwd, path);
  if (ino != 0) //The target exists, touch it
  {
    printf("The target exists, touching...\n");
    touchmip = iget(dev, ino);
    ip = &touchmip->INODE;

    ip->i_mtime = time(0L);//update time
    touchmip->dirty = 1;

    iput(touchmip);
	return;
  }
  else //The target doesnt exist and we must make a file
  {
    printf("The target does not exists, creating file...\n");
    creat_file(fullpath);
  }

  return;
}
