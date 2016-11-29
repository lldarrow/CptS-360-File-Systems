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

unsigned long search(const MINODE *mip, const char *name)
{
    char buf[BLOCK_SIZE];
	int i;
    for(i = 0; i < 12; ++i)
    {
        if(mip->INODE.i_block[i] != 0)
        {
            get_block(mip->dev, mip->INODE.i_block[i], buf);
            DIR *dir = (DIR *)&buf;
            int pos = 0;
            while(pos < BLOCK_SIZE)
            {
                char dirname[dir->name_len + 1];
                strncpy(dirname, dir->name, dir->name_len);
                dirname[dir->name_len] = '\0';

				if(!strcmp(dirname, name))
                    return dir->inode;

                // move rec_len bytes
                char *loc = (char *)dir;
                loc += dir->rec_len;
				pos += dir->rec_len;
                dir = (DIR *)loc;
            }
        }
    }
	return -1;
}

//tokenize path
char **token_path(char *pathname)
{
    char    **patharray = NULL;
    char    *tmppath = NULL;    
    int     npaths = 0;
    
    tmppath = strtok(pathname, "/");
    
    while(tmppath)
    {
		patharray = realloc (patharray, sizeof(char*) * ++npaths);
        
        if(tmppath == NULL)
        {
            break;
        }
        patharray[npaths - 1] = tmppath;
        tmppath = strtok(NULL, "/");
    }

    // realloc one extra element for the last NULL
    patharray = realloc (patharray, sizeof(char*) * (npaths + 1));
    patharray[npaths] = 0;
	
    return patharray;         // free(patharray);
}

//returns inode address
int getino(int *dev, char pathname[64])
{
	MINODE *minode = proc[0].cwd; 	// Change if doing mount

	if(pathname == NULL)
	{
		*dev = minode->dev;
		return minode->ino;
	}
	if(pathname[0] == '/')
		minode = &minode[0];

	minode->refCount++;
	char **paths = token_path(pathname);

	int i = 0;
	while(paths[i] != 0)
	{
		if((minode->INODE.i_mode & 0x4000) != 0x4000 && paths[i + 1] != 0) // In case one of the paths is not a dir
		{
			iput(minode);
			return -1;
		}

		int ino = search(minode, paths[i]);
		
		if(ino == -1) 	// In case search fails to find entry
		{
			iput(minode);
			return -1;
		}

		iput(minode);
		minode = iget(0, ino); 	// Fix this Should use div
		++i;
	}

	int ino = minode->ino;
	iput(minode);
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
	char buf[1024];
	int i;
	
	for(i = 0; i < 12; i++)
	{
		if(ino.i_block[i])
		{
			get_block(dev, ino.i_block[i], buf);
			dp = (DIR *)buf;
			cp = buf;

			while(cp < buf + BLKSIZE)
			{
				dp->name[dp->name_len] = '\0';
				printf("%s\n", dp->name);
				cp += dp->rec_len;
				dp = (DIR *)cp;
			}
		}
	}

	printf("\n");
}

//prints everything located at the pathname
void ls(char *pathname)
{
	int ino;
	int dev = running->cwd;
	MINODE *mip = running->cwd;
	char name[64][64], temp[64];

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
			dev = root->dev;
		}

		//search for path to print
		ino = getino(&dev, pathname);
		mip = iget(dev, ino);

		print_dir(mip->INODE);
	}
	else
	{
		//print root dir
		print_dir(root->INODE);
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
	char pathname[64] = "/Y", buf[1024];
	char *cp;
	DIR *dp;

	init();
	mount_root();

	printf("root mode: %4x\n", root->INODE.i_mode);
	printf("root size: %d\n", root->INODE.i_size);
	
	printf("getting ino for %s\n", pathname);
	printf("%d\n", getino(root, pathname));
	//ask for a command string, e.g. ls pathname
	//printf("input pathname for ls pathname: ");
	//scanf("%s", &pathname);

	ls(pathname);

	//ask for a command string, e.g. cd pathname
	//printf("input pathname for cd pathname: ");
	//scanf("%s", &pathname);

	//cd(pathname);

	//ask for a command string, e.g. stat pathname
	//printf("input pathname for stat pathname: ");
	//scanf("%s", &pathname);

	//stat(pathname, &mystat);
}
