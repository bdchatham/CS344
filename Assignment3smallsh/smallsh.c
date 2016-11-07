#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

char* shReadInput()
{
	//Arbitrary buffer size that will be expanded if needed
	int readBufferSize = 300;
	//Position of reading input
	int readingIndex = 0;
	//Allocate memory for string in heap to pass back
	char* shTempInput = malloc(sizeof(char) * readBufferSize);
	//Variable for holding the character value in integer form because EOF is an int in the case that our argument is a file
	int c;

	if(!shTempInput)
	{
		fprintf(stderr, "Failed to allocate memory for the input string.\n");
		exit(EXIT_FAILURE);
	}
	
	//Loop until end of the input which will call a break
	while(1)
	{
		//Read the current character
		c = getchar();
		
		//If eof or end of line
		if(c == EOF || c == '\n')
		{
			shTempInput[readingIndex] = '\0';
			return shTempInput;	
		}
		else
		{
			shTempInput[readingIndex] = c; 
			readingIndex++;
		}
		
		if(readingIndex == readBufferSize)
		{
			readBufferSize += readBufferSize;
			shTempInput = realloc(shTempInput, readBufferSize);
			if(!shTempInput)
			{
				fprintf(stderr, "Failed ro reallocate memory.\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char** shParseInput(char* userInput)
{
	char* tokenDelimiters = " \t\r";
	int tokenBufferSize = 50;
	int tokenCount = 0;
	
	char** inputTokens = malloc(sizeof(char*) * tokenBufferSize);
	
	char* currentToken;

	if(!inputTokens)
	{
		fprintf(stderr, "Failed to allocate memory for inputTokens.\n");
		exit(EXIT_FAILURE);
	}
	
	currentToken = strtok(userInput, tokenDelimiters);
	
	while(currentToken != NULL)
	{
		inputTokens[tokenCount] = currentToken;
		tokenCount++;

		if(tokenCount > tokenBufferSize)
		{
			tokenBufferSize += tokenBufferSize;
			inputTokens = realloc(inputTokens, tokenBufferSize);

			if(!inputTokens)
			{
				fprintf(stderr, "Failled to reallocate memort for inputTokens.\n");
				exit(EXIT_FAILURE);
			}
		}
	
		currentToken = strtok(NULL, tokenDelimiters);
	}

	return inputTokens;
}

void sh_loop()
{
	//Input text
	char* input;
	//Array of arguments
	char** arguments;
	//Status of command
	int status;

	do
	{
		printf("> ");
		input = shReadInput();
		arguments = shParseInput(input);
		//status = shExecuteArguments(arguments);

		free(input);
		free(arguments);
	}while(status);
}

int main(int argc, char** argv)
{
	//Shell loop
	sh_loop();

	exit(0);
}




