/*
Luke Darrow
11349190
CptS 360 Lab 6
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

typedef unsigned int   u32;

//define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;//need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 

#define BLKSIZE 1024

char buf[BLKSIZE];
//buffers for indirections
unsigned int buf1[256];
unsigned int buf2[256];
int fd;
int imap, bmap;
int ninodes, nblocks, nfreeInodes, nfreeBlocks;
//beginning of block
int inodeBeginBlock;
int test_number;
char *device;
char *cp;

//step 0
void get_block(int fd, int blk, char buf[BLKSIZE])
{
	lseek(fd, (long)(blk*BLKSIZE), 0);
	read(fd, buf, BLKSIZE);
}

//taken from prelab
super()
{
  
  // read SUPER block
  get_block(fd, 1, buf);  
  sp = (SUPER *)buf;

  // check for EXT2 magic number:
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }

  printf("************  super block info: *************\n");
  printf("inode_count\t\t\t%d\n", sp->s_inodes_count);
  printf("blocks_count\t\t\t%d\n", sp->s_blocks_count);
  printf("r_blocks_count\t\t\t%d\n", sp->s_r_blocks_count);

  printf("free_blocks_count\t\t%d\n", sp->s_free_blocks_count);
  printf("free_inodes_count\t\t%d\n", sp->s_free_inodes_count);
  printf("log_block_size\t\t\t%d\n", sp->s_log_block_size);
  
  printf("first_data_block\t\t%d\n", sp->s_first_data_block);
  printf("magic number = %x\n", sp->s_magic);

  printf("rev_level\t\t\t%d\n", sp->s_rev_level);
  printf("inode_size\t\t\t%d\n", sp->s_inode_size);
  //this super only prints the description of the first block, 0
  printf("block_group_nr\t\t\t0\n");
  printf("blksize\t\t\t\t%d\n", BLKSIZE);
  printf("inode_per_group\t\t\t%d\n", sp->s_inodes_per_group);

  //printf("---------------------------------------------\n");
  //inode size/4
  //printf("desc_per_block\t\t\t%d\n", sp->s_inode_size/4);
  // blksize/inode_size
  //printf("inodes_per_block\t\t%d\n", BLKSIZE/sp->s_inode_size);
  //printf("inode_size_ratio\t\t1\n");
}


//step 7
int search(INODE *inodePtr, char *name)
{
	//search for name in the data blocks of this INODE
	//if found, return name's ino;
	//return 0
	//task 5
	
	//read a block into buffer
	get_block(fd, inodePtr->i_block[0], buf);

	dp = (DIR *)buf;//access buf[] as DIR entries
	cp = buf;//char pointer pointing at buf[ ]

	printf("\n    i_number rec_len name_len\tname\n");
	while(cp < buf + BLKSIZE)
	{
		dp->name[dp->name_len] = '\0';
		printf("%8d %8d %7d      %-s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
		if(strcmp(name, dp->name) == 0)//if dir name matches search name return the inode number
		{
			printf("found %s : inode = %d\n", name, dp->inode);
			return dp->inode;
		}
		cp += dp->rec_len;// advance cp by rlen in bytes
		dp = (DIR *)cp;// pull dp to the next DIR entry
	}
	printf("name %s does not exist\n", name);
	return 0;
}

//main function, execution of the program starts here
int main(int argc, char *argv[])
{
	int i, j, n, ino;
	int inumber, offset;
	int blocks;

	//the unbroken path
	char path[2048];
	char *temp;	
	//two dimensional character array holds broken up path
	char name[1024][1024];
	
	//check if enough arguments
	if(argc > 2)
	{
		device = argv[1];
		strcpy(path, argv[2]);
	}
	else
	{
		printf("not enough arguments\n");
		exit(0);
	}
	
	//task 4
	//parse the path
	i = 0;
	n = 0;
	strcat(path, "/");
	temp = strtok(path, "/");
	while(temp != NULL)
	{
		strcpy(name[i], temp);
		temp = strtok(NULL, "/");
		i++;
		n++;
	}

	//print the parsed path along with n
	printf("n = %d", n);

	for(i = 0; i < n; i++)
		printf("  %s  ", name[i]);

	printf("\n");

	//open the device and set file descriptor
	fd = open(device, O_RDONLY);
	if(fd < 0)
	{
		//device didn't open properly
		printf("open %s failed\n", device);
		exit(1);
	}

	//task 1
	//read in Superblock and print some of its important fields
	super();

	//task 2
	//read in the group descriptor block (block #2)
	get_block(fd, 2, buf);
	gp = (GD *)buf;
	//determine where INODEs begin on the disk
	inodeBeginBlock = gp->bg_inode_table;

	printf("************  group 0 info ******************\n");
	printf("Blocks bitmap block\t\t%d\n", gp->bg_block_bitmap);
	printf("Inodes bitmap block\t\t%d\n", gp->bg_inode_bitmap);
	printf("Inodes table block\t\t%d\n", gp->bg_inode_table);
	printf("Free blocks count\t\t%d\n", gp->bg_free_blocks_count);
	printf("Free inodes count\t\t%d\n", gp->bg_free_inodes_count);
	printf("Directories count\t\t%d\n", gp->bg_used_dirs_count);
	printf("inodes_start\t\t\t%d\n", inodeBeginBlock);

	printf("***********  root inode info ***************\n");
	get_block(fd, inodeBeginBlock, buf);
	ip = (INODE *)buf + 1;
	printf("File mode\t\t\t%4x\n", ip->i_mode);
	printf("Size in bytes\t\t\t%d\n", ip->i_size);
	printf("Blocks count\t\t\t%d\n", ip->i_blocks);
	printf("hit a key to continue: ");
	getchar();
	printf("block[0] = %d\n", ip->i_block[0]);

	printf("********* root dir entries ***********\n");
	printf("block = %d\n", ip->i_block[0]);
	

	//print information
	get_block(fd, ip->i_block[0], buf);

	dp = (DIR *)buf;//access buf[] as DIR entries
	cp = buf;//char pointer pointing at buf[ ]

	printf("    i_number rec_len name_len\tname\n");
	while(cp < buf + BLKSIZE)
	{
		dp->name[dp->name_len] = '\0';
		printf("%8d %8d %7d      %-s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);
		cp += dp->rec_len;// advance cp by rlen in bytes
		dp = (DIR *)cp;// pull dp to the next DIR entry
	}

	get_block(fd, inodeBeginBlock, buf);
	ip = (INODE *)buf + 1;
	
	printf("hit a key to continue: ");
	getchar();

	//start searching for each part of the pathname
	printf("mode=%4x uid=%d gid=%d\n", ip->i_mode, ip->i_uid, ip->i_uid);
	for(i = 0; i < n; i++)
	{
		printf("===========================================\n");
		printf("i=%d name[%d]=%s\n", i, i, name[i]);
		printf("search for %s\n", name[i]);
		printf("i=%d i_block[0]=%d\n", i, ip->i_block[i]);
		inumber = search(ip, name[i]);
		if(inumber == 0)
		{
			//can't find name[i], bomb out!
			return 0;
		}
		else
		{
			offset = (inumber - 1) % 8;
			printf("group=%d inumber=%d\n", ip->i_gid, inumber - 1);
			printf("blk = 1 disp = %d\n", offset);
			//printf("firstD=1 gblk=0 gdisp=0\n");
			printf("group %d inodes_start = %d\n", ip->i_gid, inodeBeginBlock);
			printf("blk=%d disp=%d\n", ((inumber - 1)/8 + inodeBeginBlock), offset);
			
			if(S_ISREG(ip->i_mode) && i < n-1)
			{
				printf("ERROR: Not Valid Directory\n");
				return 1;
			}
			
			get_block(fd, ((inumber - 1) / 8 + inodeBeginBlock), buf);
			ip = (INODE *)buf + offset;
			getchar();
		}
	}

	//blocks is the ceiling of the inode size divided by the block size
	blocks = ceil(ip->i_size/1024.0);
	printf("\n\nsize=%d  blocks=%d\n", ip->i_size, blocks);
	printf("****************  DISK BLOCKS  *******************\n");

	for(i = 0; i < 15; i++)
	{
		if(ip->i_block[i] != 0)
		{
			printf("block[%d] = %d\n", i, ip->i_block[i]);
		}
	}

	//remaining blocks print them out!
	printf("\n================ Direct Blocks ===================\n");

	//set up test check for /Z/hugefile
	//test_number = 65;

	for(i = 0; i < 12; i++)
	{
		if(ip->i_block[i] != 0)
		{
			printf("%d ", ip->i_block[i]);
			//if(ip->i_block[i] != test_number)
			//{
				//test_flag = 1;
			//}
			blocks--;
		}
		if(i == 9)
			putchar('\n');
	}

	if(ip->i_block[12])
	{
		printf("\nblocks remain = %d\n", blocks);
		printf("===============  Indirect blocks   ===============\n");
		
		//danger zone
		//(char*buf1)
		get_block(fd, ip->i_block[12], (char*)buf1);
		
		for(i = 0; i < 256; i++)
		{
			if(buf1[i])
			{
				if(i % 10 == 0 && i != 0)
					putchar('\n');
				printf("%d ", buf1[i]);
				blocks--;
			}
		}
		
		printf("\n");

		//if blocks remaining, do double indirect
		if(ip->i_block[13])
		{
			printf("\nblocks remain = %d\n", blocks);
			printf("===========  Double Indirect blocks   ============\n");
			get_block(fd, ip->i_block[13], (char*)buf1);
	        for(i = 0; i < 256; i++)
    	    {
    	        if(buf1[i])
    	        {
    	            get_block(fd, buf1[i], (char*)buf2);
    	            for(j = 0; j < 256; j++)
    	            {
    	                if(buf2[j])
    	                {
    	                    if((j != 0) && (j%10 == 0))
    	                    {
    	                        printf("\n");
    	                    }
    	                    printf("%d ", buf2[j]);
							blocks--;
    	                }
    	            }
    	        }
    	    }
			printf("\nblocks remain = %d\n", blocks);
		}
	}
	printf("\n");

	return 0;
}
