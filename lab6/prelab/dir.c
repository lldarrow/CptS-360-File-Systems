#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

#define BLKSIZE 1024

int dir(int fd)
{
	unsigned int size = 0;
	unsigned char buf[1024];

	lseek(fd, 1024*first_block, SEEK_SET);
	read(fd, &buf, 1024);
	
	dp = (DIR*)buf;
	
	printf("\n***directories***\n");
	while(size < inode_size)
	{
		printf("%d\t%d\t%d\t%s\n", dp->inode, dp->file_type, dp->name_len, dp->name);
		dp = (void *)dp + dp->rec_len;
		size += dp->rec_len;
	}
}

main(int argc, char *argv[])
{
	if(argc == 2)
	{
		dir(argv[1]);
	}
	else
		exit(1);
}
