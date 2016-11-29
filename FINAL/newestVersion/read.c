/*****************************************
CptS 360 Final Project

read

Programmers:
David Hoekman 11423672
Luke Darrow 11349190

Last Modified: 12/9/15
*****************************************/

//read's a file
//reads in a number of bytes from a file
int read_file(char *path)
{
	int nbytes = atoi(third), actual= 0;
	int fd = 0;
	char buf[nbytes + 1];

	MINODE *mip;
	INODE* ip;

	strcpy(buf, "");

	printf("Reading file!\n");
	//make sure there's a fd and nbytes
	if (!strcmp(path, ""))
	{
		printf("No fd specified!\n");
		return;
	}

	fd = atoi(path);

	if (!strcmp(third, ""))
	{
		printf("No byte amount specified!\n");
		return;
	}

	//call my_read to do the reading
	//actual is how many bytes were actually read
	//this is since nbytes could be larger than what's actually there
	actual = my_read(fd, buf, nbytes);

	if (actual == -1)
	{
		printf("ERROR: Couldn't read file!\n");
		strcpy(third, "");
		return;
	}

	//null terminate buffer and print its contents
	buf[actual] = '\0';
	printf("%s\n", buf);

	return actual;
}

//does the actual reading
//outputs to buf as output parameter and returns number of bytes read
int my_read(int fd, char *buf, int nbytes)
{
	MINODE *mip;
	OFT *oftp;

	int count = 0;
	int lbk, blk, startByte, remain, ino;
	int avil;
	int *ip;

	int indirect_blk;
	int indirect_off;

	char *cq, *cp;
	char readbuf[1024];
	char temp[1024];

	//get the OFT based on the fd
	oftp = running->fd[fd];
	mip = oftp->inodeptr;

	//avil is the available number of bytes to read
	avil = mip->INODE.i_size - oftp->offset;
	cq = buf;

	//while there are more bytes available and more bytes we want to read
	while(nbytes && avil)
	{
		//mail man's to logical block number and startbyte in that block from offset
		lbk = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;

		if(lbk < 12)//direct blocks
		{
			blk = mip->INODE.i_block[lbk];//map logical lbk to physical block
		}
		else if(lbk >= 12 && lbk < 256 + 12)
		{
			printf("DOING INDIRECT\n");
			//indirect blocks
			get_block(mip->dev, mip->INODE.i_block[12], readbuf);
			
			ip = (int *)readbuf + lbk - 12;
			blk = *ip;
		}
		else
		{
			printf("DOING DOUBLE INDIRECT\n");
			//double indirect blocks
			get_block(mip->dev, mip->INODE.i_block[13], readbuf);
			
			indirect_blk = (lbk - 256 - 12) / 256;
			indirect_off = (lbk - 256 - 12) % 256;

			ip = (int *)readbuf + indirect_blk;

			get_block(mip->dev, *ip, readbuf);
			
			ip = (int *)readbuf + indirect_off;
			blk = *ip;
		}
		
		//get the data block into readbuf[BLKSIZE]
		get_block(mip->dev, blk, readbuf);

		//copy from startbyte to buf[] at most remain bytes in this block
		cp = readbuf + startByte;
		
		remain = BLKSIZE - startByte;//number of bytes available
		
		/*
		if(nbytes < avil)
		{
			strncpy(temp, cp, nbytes);
			strcat(cq, temp);
			oftp->offset += nbytes;
			count += nbytes;
			avil -= nbytes;
			nbytes -= nbytes;
			remain -= nbytes;
		}
		else
		{
			strncpy(temp, cp, avil);
			strcat(cq, temp);
			oftp->offset += avil;
			count += avil;
			avil -= avil;
			nbytes -= avil;
			remain -= avil;
		}
		
		*/
		
		//old way of doing this (one byte at a time)
		while(remain > 0)
		{
			*cq++ = *cp++;
			oftp->offset++;
			count++;
			avil--;
			nbytes--;
			remain--;
			if(nbytes <= 0 || avil <= 0)
				break;
		}
		
		//if one data block is not enough, loop back to OUTER while for more...
	}
	printf("\nmyread: read %d char from file  descriptor %d\n", count, fd);
	printf("remain is: %d\n", remain);
	printf("nbytes is: %d\n", nbytes);
	return count;//return actual number of bytes read
}
