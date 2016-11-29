/*****************************************
CptS 360 Final Project

pfd

Programmers:
David Hoekman 11423672
Luke Darrow 11349190

Last Modified: 11/23/15
*****************************************/

//prints open files
void my_pfd(char *path)
{
	int i;
	OFT *ofp;

	printf("\n  fd   mode   offset   device   inode \n");
	printf("  --   ----   ------   ------   ----- \n");

	//iterate through the open file table, checking for open files
	for(i = 0; i < NOFT; i++)
	{
		ofp = &OpenFileTable[i];

		if(ofp->refCount == 0)
		{
			return;
		}

		printf("  %02d    ", i);

		//print the correct mode
		switch(ofp->mode)
		{
			case 0:
				printf("RD");
				break;
			case 1:
				printf("WR");
				break;
			case 2:
				printf("RW");
				break;
			case 3:
				printf("AP");
				break;
			default:
				printf("--");
				break;
		}
		//print the rest of the values
		printf("    %6d     %2d     %5d\n", ofp->offset, ofp->inodeptr->dev, ofp->inodeptr->ino);
	}
	return 0;
}
