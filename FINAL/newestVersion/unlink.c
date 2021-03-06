/*****************************************
CptS 360 Final Project

unlink

Programmers:
David Hoekman 11423672
Luke Darrow 11349190

Last Modified: 12/8/15
*****************************************/

//unlinks a file
void my_unlink(char *path)
{
	int ino, i;
	int parent_ino;
	MINODE *mip;
	MINODE *parent_mip;
	INODE *ip;
	INODE *parent_ip;

	char temp[64];
	char my_dirname[64];
	char my_basename[64];

	if(!path)//check for path
	{
		printf("ERROR: no path given\n");
		return;
	}

	ino = getino(running->cwd, path);

	if(ino == 0)
	{
		printf("ERROR: bad path\n");
		return;
	}

	mip = iget(dev, ino);

	if(!mip)//make sure file exists
	{
		printf("ERROR: missing minode\n");
		return;
	}

	if(S_ISDIR(mip->INODE.i_mode))//make sure it's a file
	{
		printf("ERROR: can't unlink a directory\n");
		return;
	}
	
	printf("doing the unlinking\n");
	ip = &mip->INODE;

	//decrement the links count
	ip->i_links_count--;
	//printf("links: %d\n", ip->i_links_count);

	//deallocate its blocks
	for(i = 0; i < 12 && ip->i_block[i] != 0; i++)
	{
		bdealloc(dev, ip->i_block[i]);
	}

	//deallocate its inode
	idealloc(dev, ino);

	//get basename and dirname
	strcpy(temp, path);
	strcpy(my_dirname, dirname(temp));

	strcpy(temp, path);
	strcpy(my_basename, basename(temp));

	printf("dirname is %s basename is %s\n", my_dirname, my_basename);

	//get its parent
	parent_ino = getino(running->cwd, my_dirname);
	parent_mip = iget(dev, parent_ino);
	parent_ip = &parent_mip->INODE;

	//remove child from parent
	printf("removing %s from %s\n", my_basename, my_dirname);
	rm_child(parent_mip, my_basename);
	
	parent_ip->i_atime = time(0L);//update times and mark dirty
	parent_ip->i_mtime = time(0L);
	parent_mip->dirty = 1;
	
	//write back
	iput(parent_mip);
	mip->dirty = 1;
	iput(mip);

	return;
}
