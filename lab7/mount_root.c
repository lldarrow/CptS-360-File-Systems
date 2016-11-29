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
MINODE *iget(int dev, int ino)
{
	
	int ipos = 0;
	int offset = 0;
	char buf[1024];
	MINODE *mip = malloc(sizeof(MINODE));

	ipos = (ino - 1)/8 + INODE_START_POS;
	offset = (ino - 1) % 8;

	get_block(dev, ipos, buf);
	ip = (INODE *)buf + offset;

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

	//if refCount > 0 or dirty return
	if (mip->refCount || !mip->dirty)
		return;

	//mail man's to determine disk block and which inode in that block
	ipos = (ino - 1) / 8 + INODE_START_POS;
	offset = (ino -1) % 8;

	//read that block in
	get_block(dev, ipos, buf);

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
	dev = open("diskimage", O_RDWR);

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

	//ls root dir
	if(!strcmp(pathname, "/") || !strcmp(pathname, ""))
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
		printf("%d\n", ino);
		getchar();
		mip = iget(dev, ino);

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

	if(pathname)
	{
		//cd to pathname
		printf("cd to %s\n", pathname);

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
			ino = getino(mip, name[i]);
			printf("inode is: %d\n", ino);
			//getino returns 0 if not found
			if(ino == 0)
			{
				//bad path, exit
				exit(1);
			}

			mip = iget(dev, ino);
		}
		
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
	printf("ERROR: no more free inodes\n");
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
	char pathname[64], buf[1024];
	char command[64];
	char input[1024];
	char *cp;
	DIR *dp;
	MINODE *mip = malloc(sizeof(MINODE));

	init();
	mount_root();

	mip = root;

	printf("root mode: %4x\n", root->INODE.i_mode);
	printf("root links: %d\n", root->INODE.i_links_count);
	
	while(1)
	{
		printf("cwd links: %d\n", running->cwd->INODE.i_links_count);
		printf("input command: ");
		scanf("%s", command);

		if(!strcmp(command, "quit"))
			quit();

		printf("input path: ");
		scanf("%s", pathname);
		if(!strcmp(command, "ls"))
			ls(pathname);
		else if(!strcmp(command, "cd"))
			cd(pathname);
		else
			printf("invalid command\n");
	}

	//printf("getting ino for %s\n", pathname);
	//printf("%d\n", getino(mip, pathname));
	//ask for a command string, e.g. ls pathname
	//printf("input pathname for ls pathname: ");
	//scanf("%s", &pathname);

	//ls(pathname);

	//ask for a command string, e.g. cd pathname
	//printf("input pathname for cd pathname: ");
	//scanf("%s", &pathname);

	//cd(pathname);

	//ask for a command string, e.g. stat pathname
	//printf("input pathname for stat pathname: ");
	//scanf("%s", &pathname);

	//stat(pathname, &mystat);
}
