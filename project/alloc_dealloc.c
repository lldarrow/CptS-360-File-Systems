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

//WRITE C CODE FOR

//deallocate an ino
int idealloc(int *dev, int ino)
{

	return 0;
}

//deallocate a bno
int bdealloc(int *dev, int bno)
{
	return 0;
}
