#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

/*
 * Pre-defined shell interface limitations.
 */

#define MAXNUMBEROFARGUMENTS 512
#define MAXCOMMANDLENGTH 2048

/*
 * Forward Declarations.
 */ 

void shLoop();
char* shReadInput();
char** shParseInput(char* userInput);
int shChangeDirectory(char** arguments);
int shHelp(char** arguments);
int shStatus(char** arguments);
int shExit(char** arguments);
void shLoop();
int shBuiltInFunctionListSize();
int shExecuteArguments(char** arguments);
int (*builtInFunctions[])(char** arguments);


/*
 * Strings of built-in command names.
 */

char *builtInCommands[] =
{
	"cd",
	"help",
	"status",
	"exit"
};

/*
 * Array of addresses to built-in functions. Must match the ordering of builtInCommand string list.
 */

int (*builtInFunctions[])(char**) =
{
	&shChangeDirectory,
	&shHelp,
	&shStatus,
	&shExit
};

/*
 * Reads input from user character by character and adds it to a character array to return.
 */ 

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

/*
 * Prints out information on this shell.
 */

int shHelp(char** arguments)
{	
	int i;
	printf("Brandon Chatham's shell. The following commands are supported by my implementations:\n");
	for(i = 0; i < shBuiltInFunctionListSize(); i++)
		printf("%s\n", builtInCommands[i]);
	printf("Have fun!\n");	
	
	return 1;
}

/*
 * Tells the user general information of the shell.
 */

int shStatus(char** arguments)
{

	return 1;
}

/*
 * Parses the user input into individual arguments using strtok.
 */

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

/*
 * Changes to the directory with the name in the spot of the second argument if it exists. 
 */ 

int shChangeDirectory(char** arguments)
{
	if(arguments[1] == NULL )
	{
		fprintf(stderr, "Expected additional argument for \"cd\" operation.\n");//Tells user they did not input enough arguments for cd command.
	}
	else 
	{
		if(chdir(arguments[1]) != 0)//Changes directories to the second argument. If fails error is reported.
			perror("cd");//Send error info.
	}

	return 1;//Successfully completed cd command.
}

/*
 * Exit the shLoop by returning 0 to the status value. Exits the shell.
 */

int shExit(char** arguments)
{
	return 0;
}

/*
 * Returns the number of pre-defined commands the shell supports.
 */

int shBuiltInFunctionListSize()
{
	return (sizeof(builtInCommands) / sizeof(char));//Returns the number of built-in functions the shell supports. 
}

/*
 * Checks if commands given are supported. If they are, command is executed. If not, the shell is forked and a child process attempts to run the command. 
 */

int shExecuteArguments(char** arguments)
{
	int i;

	if(arguments[0] == NULL)//No arguments found. Exit.
		return(1);
	
	for(i = 0; i < shBuiltInFunctionListSize(); i++) 
	{
		if(strcmp(arguments[0], builtInCommands[i]) == 0)
			return (*builtInFunctions[i])(arguments); 
	}

	return shLaunch(arguments);//Command is not supported by this shell. 
	
}

/*
 * Forks the process to run the given arguments if a child process is correctly created.
 */

int shLaunch(char** arguments)
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

/*
 * Main shell loop to call functions for user to interface with.
 */

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
}




