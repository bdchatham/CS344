#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#define MAXNUMBEROFARGUMENTS 512
#define MAXCOMMANDLENGTH 2048

char* shReadInput()
{
	//Position of reading input
	int readingIndex = 0;
	//Allocate memory for string in heap to pass back
	char* shTempInput = malloc(sizeof(char) * MAXCOMMANDLENGTH);
	//Variable for holding the character value in integer form because EOF is an int in the case that our argument is a file
	int c;

	if(!shTempInput)
	{
		fprintf(stderr, "Failed to allocate memory for the input string.\n");
		exit(EXIT_FAILURE);
	}
	
	//Loop until end of the input which will call a break
	while(readingIndex < MAXCOMMANDLENGTH)
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
	}

	fprintf(stderr, "Command input too long.\n");
	exit(EXIT_FAILURE);
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
	
	while(currentToken != NULL && tokenCount < MAXNUMBEROFARGUMENTS)
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

int shExecuteArguments(char** arguments)
{
	pid_t childPID, waitPID;
	childPID = fork();
	int status;
	
	if(childPID == 0)//Child Process. Run the arguments.
	{
		if(execvp(arguments[0], arguments) == -1)//Child did not properly execute program.
		{
			fprintf(stderr, "Error executing argument.\n");
			exit(EXIT_FAILURE);
		}
	}
	else if(childPID > 0)//Parent Process.
	{
		do
		{
			waitPID = waitpid(childPID, &status, WUNTRACED);	
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));//Waits for the child process until it exits or is killed by a signal.
	}
	else//Parent Process but child could not be created.
	{
		fprintf(stderr, "Error creating child process.\n.");
		exit(EXIT_FAILURE);
	}

	exit(0);
}

void sh_loop()
{
	//Input text
	char* input;
	//Array of arguments
	char** arguments;
	//Status of command
	int status = 1;

	do
	{
		printf(": ");
		input = shReadInput();
		arguments = shParseInput(input);
		status = shExecuteArguments(arguments);

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




