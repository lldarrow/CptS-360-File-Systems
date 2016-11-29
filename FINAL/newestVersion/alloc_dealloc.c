/*****************************************
CptS 360 Final Project

alloc dealloc functions

Programmers:
David Hoekman 11423672
Luke Darrow 11349190

Last Modified: 12/8/15
*****************************************/

//sets bit 
void set_bit(char *buf, int bit)
{
	int i, j;
	i = bit / 8;
	j = bit % 8;
	buf[i] |= (1 << j);
}

//decrements the amount of free inodes on the device
//this is used to ensure that we don't use more inodes than we
//have room for
void decFreeInodes(int dev)
{
	char buf[BLKSIZE];

	//dec free inodes count in SUPER and Group Descriptor
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_inodes_count--;
	put_block(dev, 1, buf);

	get_block(dev, 2, buf);
	gp = (GD *)buf;
	gp->bg_free_inodes_count--;
	put_block(dev, 2, buf);
}

//tests bit
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

//allocates a free inode for writing and stuff
//taken from lab 6 pre lab
int ialloc(int dev)
{
	int i;
	char buf[BLKSIZE];
	printf("imap is %d\n", imap);
	//read inode_bitmap block
	get_block(dev, imap, buf);

	printf("inode count %d\n", ninodes);
	//set the bits
	for(i = 0; i < ninodes; i++)
	{
		if(tst_bit(buf, i) == 0)
		{
			//set the bit
			set_bit(buf, i);
			
			//decrement free inodes on the device
			decFreeInodes(dev);

			//write back to block
			put_block(dev, imap, buf);

			return i + 1;
		}
	}
	printf("ERROR: no more free inodes\n");
	return 0;
}

//allocates a free block so we can put stuff in it
int balloc(int dev)
{
	int i;
	char buf[BLKSIZE];
	printf("bmap is %d\n", bmap);
	//read block_map block
	get_block(dev, bmap, buf);

	//set bits
	for(i = 0; i < nblocks; i++)
	{
		if(tst_bit(buf, i) == 0)
		{
			set_bit(buf, i);

			decFreeInodes(dev);

			put_block(dev, bmap, buf);

			return i;
		}
	}
	printf("ERROR: no more free blocks\n");
	return 0;
}

//WRITE C CODE FOR

//deallocate an inode for a given ino on the dev
int idealloc(int dev, int ino)
{
	char buf[1024];
	int byte;
	int bit;

	//clear bit(bmap, bno)
	get_block(dev, imap, buf);

	//mailmans to where it is
	byte = ino / 8;
	bit = ino % 8;

	//negate it
	buf[byte] &= ~(1 << bit);

	//write back to block
	put_block(dev, imap, buf);

	//increment free blocks
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_blocks_count++;
	put_block(dev, 1, buf);

	get_block(dev, 2, buf);
	gp = (GD *)buf;
	gp->bg_free_blocks_count++;
	put_block(dev, 2, buf);
}

//deallocate a block with bno on the dev
int bdealloc(int dev, int bno)
{
	char buf[1024];
	int byte;
	int bit;

	//clear bit(bmap, bno)
	get_block(dev, bmap, buf);

	//mailman to location
	byte = bno / 8;
	bit = bno % 8;

	//negate it
	buf[byte] &= ~(1 << bit);

	//write back
	put_block(dev, bmap, buf);

	//increment free blocks
	get_block(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_blocks_count++;
	put_block(dev, 1, buf);

	get_block(dev, 2, buf);
	gp = (GD *)buf;
	gp->bg_free_blocks_count++;
	put_block(dev, 2, buf);

	return 0;
}
