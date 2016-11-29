/*****************************************
CptS 360 Final Project

Main

Programmers:
David Hoekman 11423672
Luke Darrow 11349190

Last Modified: 12/3/15
*****************************************/



//link oldfilename newfilename
void my_link(char *path)
{
	char old[64], new[64], temp[64];
	char link_parent[64], link_child[64];
	int ino;
	int p_ino;
	MINODE *mip;
	MINODE *p_mip;
	INODE *ip;
	INODE *p_ip;
	
	if(!strcmp(path, ""))
	{
		printf("ERROR: no old file\n");
		return;
	}
	
	if(!strcmp(third, ""))
	{
		printf("ERROR: no new file\n");
		return;
	}

	strcpy(old, path);
	strcpy(new, third);
	
	//get oldfilename's inode
	ino = getino(running->cwd, old);
	mip = iget(dev, ino);

	//verify old file exists
	if(!mip)
	{
		printf("ERROR: %s does not exist\n", old);
		return;
	}

	if(S_ISDIR(mip->INODE.i_mode))
	{
		printf("ERROR: can't link directory\n");
		return;
	}
	printf("got it!\n");
	
	//get new's dirname
	if(!strcmp(new, "/"))
	{
		strcpy(link_parent, "/");
	}
	else
	{
		strcpy(temp, new);
		strcpy(link_parent, dirname(temp));
	}

	//get new's basename
	strcpy(temp, new);
	strcpy(link_child, basename(temp));

	printf("after stuff new is %s\n", new);

	//get new's parent
	printf("parent %s\n", link_parent);
	//get new's parent ino and minode pointer
	p_ino = getino(running->cwd, link_parent);
	p_mip = iget(dev, p_ino);

	//verify that link parent exists
	if(!p_mip)
	{
		printf("ERROR: no parent\n");
		return;
	}

	//verify link parent is a directory
	if(!S_ISDIR(p_mip->INODE.i_mode))
	{
		printf("ERROR: not a directory\n");
		return;
	}

	//verify that link child does not exist yet
	if(getino(running->cwd, new))
	{
		printf("ERROR: %s already exists\n", new);
		return;
	}

	printf("entering name for %s\n", link_child);
	//enter new file in parent directory's data blocks
	//this ino is the ino of the old file
	//this is how it's linked
	enter_name(p_mip, ino, link_child);
	
	//grab the inode of mip
	ip = &mip->INODE;

	//increment links count
	ip->i_links_count++;
	mip->dirty = 1;//mark dirty

	//
	p_ip = &p_mip->INODE;

	p_ip->i_atime = time(0L);
	p_mip->dirty = 1;
	

	iput(p_mip);
	
	iput(mip);

	printf("end of link\n");
	return;
}
