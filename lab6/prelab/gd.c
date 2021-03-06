#include  <fcnt1.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ext2fs/ext2_fs.h>

int gd(int fd)
{
	char buf[1024];
	
	get_block(fd, 2, buf);
	gp = (GD *)buf;
	
	printf("\n**************** group descriptor ****************\n");
	printf("bg_block_bitmap \t=\t%d\n", gp->bg_block_bitmap);
	printf("bg_inode_bitmap \t=\t%d\n", gp->bg_inode_bitmap);
	printf("bg_inode_table \t\t=\t%d\n", gp->bg_inode_table);
	printf("bg_free_blocks_count \t=\t%d\n", gp->bg_free_blocks_count);
	printf("bg_free_inodes_count \t=\t%d\n", gp->bg_free_inodes_count);
	printf("bg_used_dirs_count \t=\t%d\n", gp->bg_used_dirs_count);
	block_bitmap = gp->bg_block_bitmap;
	inode_bitmap = gp->bg_inode_bitmap;
	inode_table = gp->bg_inode_table;
	printf("\n**************************************\n");
}

int main(int argc, char *argv[])
{
	gd();
}
