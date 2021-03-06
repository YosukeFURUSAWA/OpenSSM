#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void putbin(unsigned char buf)
{
	printf("%c", '0' + ((buf >> 7) & 0x01));
	printf("%c", '0' + ((buf >> 6) & 0x01));
	printf("%c", '0' + ((buf >> 5) & 0x01));
	printf("%c", '0' + ((buf >> 4) & 0x01));
	printf("%c", '0' + ((buf >> 3) & 0x01));
	printf("%c", '0' + ((buf >> 2) & 0x01));
	printf("%c", '0' + ((buf >> 1) & 0x01));
	printf("%c", '0' + ((buf     ) & 0x01));

	return;
}


int main(int argc, char *argv[])
{
	int fd;
	unsigned char buf;
	unsigned int col;

	if(argc == 1){
		printf( "FONTX2 to Text Converter   (C) 2010 Yosuke FURUSAWA.\n"
				"usage : %s [FONTX2 File]\n", argv[0]);
		return(0);
	}

	if( (fd = open(argv[1], O_RDWR)) < 0){
		printf("Can't open %s\n", argv[1]);
		return(0);
	}

	col = 0;
	while(read(fd, &buf, 1) > 0){
		printf("0x%02X, ", buf);
		col++;
		if(col > 100/6){
			col = 0;
			printf("\n");
		}
	}

	close(fd);
	return(0);
}
