/*
Luke Darrow 11349190
CptS 360
Lab 7 mount root
*/

#include "type.h"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MOUNT  mounttab[5];

char names[64][128],*name[64]; 
int fd, dev, n;
int nblocks, ninodes, bmap, imap, inode_start, iblock;
int inodeBeginBlock;
char pathname[256], parameter[256];
int DEBUG;

//2. Initialize data structures of LEVEL-1
init()
{
	//2 PROCs, P0 with uid=0, P1 with uid=1, all PROC.cwd = 0
	//MINODE minode[100]; all with refCount=0
	//MINODE *root = 0;
	int i;
	
	//allocate memory for running process
	running = malloc(sizeof(PROC));
	
	proc[0].pid = 1;
	proc[0].uid = 0;
	proc[0].cwd = 0;

	proc[1].pid = 2;
	proc[1].uid = 1;
	proc[1].cwd = 0;

	running = &proc[0];
	
	//MINODE minode[100] all with refCount=0
	for(i = 0; i < 100; i++)
	{
		minode[i].refCount = 0;
		minode[i].ino = 0;
	}
	//MINODE *root = 0;
	root = 0;
}

//get block from lab 6
void get_block(int fd, int blk, char buf[BLKSIZE])
{
	lseek(fd, (long)(blk*BLKSIZE), 0);
	read(fd, buf, BLKSIZE);
}

//get block from lab 6
void put_block(int fd, int blk, char buf[BLKSIZE])
{
	lseek(fd, (long)(blk*BLKSIZE), 0);
	write(fd, buf, BLKSIZE);
}

//Search function from lab 6
int search(MINODE *mip, char *name)
{
	//search for name in the data blocks of this INODE
	//if found, return name's ino;
	//return 0
	
	int i;
	char buf[BLKSIZE], *cp;

	//search through the direct blocks
	for(i = 0; i < 12; i++)
	{
		//if the data block has stuff in it
		if(mip->INODE.i_block[i])
		{
			//get the block
			get_block(dev, mip->INODE.i_block[i], buf);
			
			dp = (DIR *)buf;
			cp = buf;

			while(cp < buf + BLKSIZE)
			{
				//null terminate dp->name for strcmp
				dp->name[dp->name_len] = '\0';
				//check if it's the name we're looking for
				if(strcmp(name, dp->name) == 0)
					return dp->inode;//name matches, return the inode
				cp += dp->rec_len;
				dp = (DIR *)cp;
			}
		}
	}
	printf("name %s does not exist\n", name);
	return 0;
}

//returns inode address
int getino(MINODE *mip, char pathname[64])
{
	int ino = 0, i = 0, n = 0;
	int inumber, offset;
	char path[64];
	char name[64][64];
	char *temp;
	char buf[1024];

	//check if starts at root
	if(pathname[0] == '/')
	{
		mip = root;
	}

	//if there's a pathname, then parse it
	if(pathname)
	{
		//parse the string and put it into name
		strcat(pathname, "/");
		temp = strtok(pathname, "/");

		while(temp != NULL)
		{
			strcpy(name[i], temp);
			temp = strtok(NULL, "/");
			i++;
			n++;
		}
		//parsing complete
	}

	//time to do the searching
	for(i = 0; i < n; i++)
	{
		printf("now searching for %s\n", name[i]);
		ino = search(mip, name[i]);

		if(ino == 0)
		{
			//can't find name[i]
			return 0;
		}

		printf("found %s at inode %d\n", name[i], ino);

		offset = (ino - 1) % 8;
		get_block(dev, ((ino - 1) / 8) + inodeBeginBlock, buf);
		mip = (MINODE*)buf + offset;
	}
	
	return ino;
}

//loads inode into slot in minode[] array
//currently this returns an MINODE from block
MINODE *iget(int dev, int ino)
{
	
	int ipos = 0;
	int offset = 0;
	char buf[1024];
	MINODE *mip = malloc(sizeof(MINODE));

	//mailman's
	ipos = (ino - 1)/8 + INODE_START_POS;
	offset = (ino - 1) % 8;

	//get the block
	get_block(dev, ipos, buf);
	//load inode
	ip = (INODE *)buf + offset;

	//if it's a directory, set the minode->INODE to the inode
	if(S_ISDIR(ip->i_mode))
	{
		mip->INODE = *ip;
		return mip;
	}
	else
	{
		printf("a file is not a directory\n");
		return NULL;
	}
}

//releases a minode[]
void iput(MINODE *mip)
{
	int ino = getino(mip, ".");
	int offset, ipos;
	char buf[1024];
	
	//decrement refCount by 1
	mip->refCount--;

	//check refcount to see if anyone using it
	//check dirty to see if it's been changed, dirty == 1 if changed
	//if refCount > 0 or dirty return
	if (mip->refCount || !mip->dirty)
		return;

	//mail man's to determine disk block and which inode in that block
	ipos = (ino - 1) / 8 + INODE_START_POS;
	offset = (ino -1) % 8;

	//read that block in
	get_block(dev, ipos, buf);

	//copy minode's inode into the inode area in that block
	ip = (INODE*)buf + offset;
	*ip = mip->INODE;

	//write block back to disk
	put_block(dev, ipos, buf);
}

//mount root file system, establish / and CWDs
mount_root()
{
	char buf[1024];
	//open device for RW
	char name[64] = "";

	printf("Input device name:");
	scanf("%s", name);
	dev = open(name, O_RDWR);

	//check if open() worked
	if(dev < 0)
	{
		printf("ERROR: failed ot open mydisk\n");
		exit(0);
	}

	//read super block to verify it's an EXT2 FS
	get_block(dev, SUPERBLOCK, buf);
	sp = (SUPER *)buf;
	//verify if it's an EXT2 FS
	if(sp->s_magic != 0xEF53)
	{
		printf("NOT AN EXT2 FS\n");
		exit(1);
	}

	//set some vars
	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;
	
	//read group block for info
	get_block(dev, GDBLOCK, buf);
	gp = (GD *)buf;

	imap = gp->bg_inode_bitmap;
	bmap = gp->bg_block_bitmap;

	inodeBeginBlock = gp->bg_inode_table;
	
	//get root inode
	root = iget(dev, 2);

	//let cwd of both p0 and p1 point at the root minode (refCount=3)
	proc[0].cwd = iget(dev, 2);
	proc[1].cwd = iget(dev, 2);

	root->refCount = 3;
}

//print directory
void print_dir(INODE ino)
{
	DIR *dp;
	char *cp;
	char buf[1024], temp[1024];
	
	get_block(dev, ino.i_block[0], buf);
	dp = (DIR*)buf;
	cp = buf;

	while(cp < buf + BLKSIZE)
	{
		strncpy(temp, dp->name, dp->name_len);
		temp[dp->name_len] = 0;

		printf("%s\n", temp);
		memset(temp, 0, 1024);
		cp += dp->rec_len;
		dp = (DIR*)cp;
	}

	printf("\n");
}

//prints everything located at the pathname
void ls(char *pathname)
{
	int ino, offset;
	MINODE *mip = running->cwd;
	char name[64][64], temp[64];
	char buf[1024];

	//ls cwd
	if(!strcmp(pathname, ""))
	{
		print_dir(mip->INODE);
		return;
	}

	//ls root dir
	if(!strcmp(pathname, "/"))
	{
		print_dir(root->INODE);
		return;
	}

	//if there's a pathname, ls pathname
	if(pathname)
	{
		//check if path starts at root
		if(pathname[0] == '/')
		{
			mip = root;
		}

		//search for path to print
		ino = getino(mip, pathname);
		if(ino == 0)
		{
			return;
		}
		printf("%d\n", ino);
		if(iget(dev, ino) != NULL)
			mip = iget(dev, ino);
		else
		{
			printf("ERROR: cannot ls a file\n");
			return;
		}
		print_dir(mip->INODE);
	}
	else
	{
		//print root dir
		print_dir(root->INODE);
	}
}

//change cwd to pathname. If no pathname, set cwd to root.
void cd(char *pathname)
{
	int i = 0, n = 0, ino;
	char name[64][64];
	char *temp;

	MINODE *mip = running->cwd;

	if(strcmp(pathname, ""))
	{
		//cd to pathname
		printf("changing directory to %s\n", pathname);

		if(pathname[0] == '/')
		{
			mip = root;
		}
		
		//parse the pathname and put it into name
		strcat(pathname, "/");
		temp = strtok(pathname, "/");
		
		while(temp != NULL)
		{
			strcpy(name[i], temp);
			temp = strtok(NULL, "/");
			i++;
			n++;
		}
		
		//null terminate the name array
		strcpy(name[i], "\0");

		//go through the pathname until end
		for(i = 0; i < n; i++)
		{
			printf("getting ino for: %s\n", name[i]);
			ino = getino(mip, name[i]);
			//printf("inode is: %d\n", ino);
			//getino returns 0 if not found
			if(ino == 0)
			{
				//bad path, exit
				printf("ERROR: bad path\n");
				return;
			}

			//if iget returns null, break!
			if(iget(dev, ino) != NULL)
				mip = iget(dev, ino);
			else
			{
				//set mip back to cwd in case we're in the middle of a pathname and already jumped into another directory
				mip = running->cwd;
				printf("ERROR: can't cd into file\n");
				break;
			}
		}

		//sets cwd to new directory or just cwd again if above case fails
		running->cwd = mip;
	}
	else
	{
		printf("cd to root\n");
		//cd to root
		running->cwd = root;
	}
}

//helper functions for ialloc from lab 6 prelab

void set_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8;
	j = bit % 8;
	buf[i] = (1 << j);
}

void decFreeInodes(int dev)
{
	char buf[BLKSIZE];

	//dec free inodes count in SUPER and gd
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_inodes_count--;
	put_block(dev, 1, buf);

	get_block(dev, 2, buf);
	gp = (GD *)buf;
	gp->bg_free_inodes_count--;
	put_block(dev, 2, buf);
}

int tst_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8;
	j = bit % 8;

	if(buf[i] & (1 << j))
		return 1;

	return 0;
}

//end of helper functions

//allocates a free inode number
//taken from lab 6 pre lab
int ialloc(int dev)
{
	int i;
	char buf[BLKSIZE];

	//read inode_bitmap block
	get_block(dev, imap, buf);

	for(i = 0; i < ninodes; i++)
	{
		if(tst_bit(buf, i) == 0)
		{
			set_bit(buf, i);
			decFreeInodes(dev);

			put_block(dev, imap, buf);

			return i + 1;
		}
	}
	printf("ERROR: no more free inodes\n");
	return 0;
}

//allocates a free block number
int balloc(int dev)
{
	int i;
	char buf[BLKSIZE];

	//read block_map block
	get_block(dev, bmap, buf);

	for(i = 0; i < nblocks; i++)
	{
		if(tst_bit(buf, i) == 0)
		{
			set_bit(buf, i);
			decFreeInodes(dev);
			
			put_block(dev, bmap, buf);

			return i + 1;
		}
	}
	printf("ERROR: no more free blocks\n");
	return 0;
}

//iput all DIRTY minodes before shutdown
int quit()
{
	int i;
	
	//go through all the minodes
	for(i = 0; i < NMINODE; i++)
	{
		//check it used and dirty
		if(minode[i].refCount > 0 && minode[i].dirty == 1)
		{
			minode[i].refCount = 1;
			iput(&minode[i]);
		}
	}
	exit(0);
}

int main()
{
	struct stat mystat;
	char command[64];
	char pathname[64] = "", buf[1024];
	char input[128] = "";
	char *cp;
	DIR *dp;
	MINODE *mip = malloc(sizeof(MINODE));
	int (*functions[3])(char path[]);
	char *commands[3] = {"ls", "cd", "quit"};
	int i = 0;

	functions[0] = ls;
	functions[1] = cd;
	functions[2] = quit;

	init();
	mount_root();

	mip = root;
	
	while(1)
	{
		printf(">");
		
		fgets(input, 128, stdin);
		//remove \r at end of input
		input[strlen(input) - 1] = 0;
		if(input[0] == 0)
			continue;
		
		//split up the input into the variables
		sscanf(input, "%s %s", command, pathname);
		
		//look for the command
		for(i = 0; i < 3; i++)
		{
			if(!strcmp(commands[i], command))
			{
				(*functions[i])(pathname);
				break;
			}
		}
		if(i == 3)
			printf("invalid command\n");

		//reset variables
		strcpy(pathname, "");
		strcpy(command, "");
		strcpy(input, "");
	}
}
