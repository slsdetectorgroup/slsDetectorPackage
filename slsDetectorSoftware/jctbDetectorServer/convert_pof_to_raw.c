// Converts POF files into RAW files for flashing

#include <stdio.h>
#include <stdlib.h>

// Warning: This program is for testing only.
// It makes some assumptions regarding the pof file and the flash size that might be wrong.
// It also overwrites the destination file without any hesitation.
// Handle with care.

int main(int argc, char* argv[])
{
	FILE* src;
	FILE* dst;
	int x;
	int y;
	int i;
	int filepos;

	if (argc < 3)
	{
		printf("%s Sourcefile Destinationfile\n",argv[0]);
		return -1;
	}

	src = fopen(argv[1],"rb");
	dst = fopen(argv[2],"wb");

	// Remove header (0...11C)
	for (filepos=0; filepos < 0x11C; filepos++)
		fgetc(src); 

	// Write 0x80 times 0xFF (0...7F)
	for (filepos=0; filepos < 0x80; filepos++)
		fputc(0xFF,dst);

	// Swap bits and write to file
	for (filepos=0x80; filepos < 0x1000000; filepos++)
	{
		x = fgetc(src);
		if (x < 0) break;

		y=0;
		for (i=0; i < 8; i++)
			y=y| (   (( x & (1<<i) ) >> i)    << (7-i)     );	// This swaps the bits

		fputc(y,dst);
	}
	if (filepos < 0x1000000)
		printf("ERROR: EOF before end of flash\n");

	printf("To flash the file in Linux do:\n");
	printf("    cat /proc/mtd (to findout the right mtd)\n");
	printf("    flash_eraseall /dev/mtdX\n");
	printf("    cat file > /dev/mtdX\n");

	return 0;
}
