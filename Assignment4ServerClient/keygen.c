#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv)
{
	time_t t;
	int i, length;
	char randomLetter;

	srand((unsigned)time(&t));

	if(argc != 2)
	{
		fprintf(stderr, "Usage: keyGen\n");
		exit(1);
	}

	sscanf(argv[1], "%d", &length);
	
	if(length == 0)
	{
		fprintf(stderr, "Key length cannot be 0");
		exit(1);
	}
	
	for(i = 0; i < length; i++)
	{
		randomLetter = (char)(rand() % (90 - 64 + 1) + 64);
		if(randomLetter == '@')
			randomLetter = ' ';
		
		printf("%c", randomLetter);
	}
	
	printf("\n");

	return 0;	
}
