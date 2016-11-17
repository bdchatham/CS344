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

#define MAXCHILDREN 20
#define MAXNUMBEROFARGUMENTS 512
#define MAXCOMMANDLENGTH 2048


struct backgroundChildren
{
	pid_t* ids;
	int mazSize;
	int count;
};

/*
 * Command info structure.
 */

struct CommandInfo
{
	int inStream;
	int outStream;
	char* outFileName;
	char* inFileName;
	bool isBackground;
	int* backgroundChildren;
	int childCount;
};


/*
 * Command Info constructor.
 */

struct CommandInfo* CommandInfoInit()
{
	struct CommandInfo* ci;
	ci = malloc(sizeof(struct CommandInfo));
	ci->inStream = STDIN_FILENO;;
	ci->outStream = STDOUT_FILENO;
	ci->outFileName = NULL;
	ci->inFileName = NULL;
	ci->isBackground = false;
	ci->backgroundChildren =  malloc(sizeof(int) * MAXCHILDREN);
	ci->childCount = 0;

	return ci;
}

/*
 * Forward Declarations.
 */ 

int shLaunch(char** arguments, struct CommandInfo* ci);
void shLoop();
char* shReadInput();
char** shParseInput(char* userInput, struct CommandInfo* ci);
int shChangeDirectory(char** arguments);
int shHelp(char** arguments);
int shStatus(char** arguments);
int shExit(char** arguments);
int shBuiltInFunctionListSize();
int shExecuteArguments(char** arguments, struct CommandInfo* ci);
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
		fflush(stdout);
		
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
	fflush(stdout);
	for(i = 0; i < shBuiltInFunctionListSize(); i++)
	{
		printf("%s\n", builtInCommands[i]);
		fflush(stdout);
	}
	printf("Have fun!\n");
	fflush(stdout);	
	
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

char** shParseInput(char* userInput, struct CommandInfo* ci)
{
	char* tokenDelimiters = " \n";
	int tokenCount = 0;
	
	char** inputTokens = malloc(sizeof(char*) * MAXNUMBEROFARGUMENTS);
	
	char* currentToken;

	if(!inputTokens)
	{
		fprintf(stderr, "Failed to allocate memory for inputTokens.\n");
		exit(EXIT_FAILURE);
	}
	
	currentToken = strtok(userInput, tokenDelimiters);
	
	if(strncmp(currentToken, "#", 1) == 0)
		return inputTokens;

	while(currentToken != NULL && tokenCount < MAXNUMBEROFARGUMENTS)
	{

		if(strcmp(currentToken, "<") == 0)
		{
			currentToken = strtok(NULL, tokenDelimiters);
			ci->inFileName = currentToken;
			
		}
		else if(strcmp(currentToken, ">") == 0)
		{
			currentToken = strtok(NULL, tokenDelimiters);
			ci->outFileName = currentToken;
		}
		else if(strcmp(currentToken, "&") == 0)
		{
			ci->isBackground = true;
			break;
		}
		else
			inputTokens[tokenCount++] = currentToken;	

		currentToken = strtok(NULL, tokenDelimiters);	
	}
	
	return inputTokens;
}

/*
 * Changes to the directory with the name in the spot of the second argument if it exists. 
 */ 

int shChangeDirectory(char** arguments)
{
	char* destination = arguments[1];
	if(destination == NULL)
	{
		destination = getenv("HOME");
	}
	if(chdir(destination) == -1)//Changes directories to the second argument. If fails error is reported.
	{
		perror("cd");//Send error info.
		fflush(stdout);
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
	return (sizeof(builtInCommands) / sizeof(char*));//Returns the number of built-in functions the shell supports. 
}

/*
 * Checks if commands given are supported. If they are, command is executed. If not, the shell is forked and a child process attempts to run the command. 
 */

int shExecuteArguments(char** arguments, struct CommandInfo* ci)
{
	int i;

	if(arguments[0] == NULL)//No arguments found. Exit.
		return(1);
	
	for(i = 0; i < shBuiltInFunctionListSize(); i++) 
	{
		if(strcmp(arguments[0], builtInCommands[i]) == 0)
			return (*builtInFunctions[i])(arguments); 
	}

	return shLaunch(arguments, ci);//Command is not supported by this shell. 
	
}

/*
 * Forks the process to run the given arguments if a child process is correctly created.
 */

int shLaunch(char** arguments, struct CommandInfo* ci)
{
	pid_t processID, waitPID;
	int status = 0;
	const char devNull[] = "/dev/null";

	if(ci->inFileName == NULL && ci->isBackground) 
		ci->inFileName = (char*)devNull;
	
	if(ci->outFileName == NULL && ci->isBackground)
		ci->outFileName = (char*)devNull;

	processID = fork();

	if(processID == 0)//Child process. Run the arguments.
	{	
		if(ci->inFileName != NULL)
		{
			ci->inStream = open(ci->inFileName, O_RDONLY);//Open the infile stream for reading from the specified path from the command.
			if(ci->inStream == -1)
			{
				perror("inStream");
				fflush(stdout);
			}
			else
				dup2(ci->inStream, STDIN_FILENO);
		}
		
		if(ci->outFileName != NULL)
		{
			ci->outStream = open(ci->outFileName, O_WRONLY);
			if(ci->outStream == -1)
			{
				perror("outStream");
				fflush(stdout);
			}
			else
				dup2(ci->outStream, STDOUT_FILENO);
		}
		fflush(stdout);
		if(execvp(arguments[0], arguments) == -1)//Child did not properly execute program.
		{
			fprintf(stderr, "Error executing argument.\n");
			exit(EXIT_FAILURE);
		}
	}
	else if(processID > 0)//Parent Process.
	{
		if(!ci->isBackground)//Not a background process so we need to wait for it.
		{	
			do
			{
				waitPID = waitpid(processID, &status, WUNTRACED);	
			}while(!WIFEXITED(status) && !WIFSIGNALED(status));//Waits for the child process until it exits or is killed by a signal.
		}
		else//Add the child to the list of children in ci.
			ci->children[ci->childCount++] = processID; 
	}
	else//Parent Process but child could not be created.
	{
		fprintf(stderr, "Error creating child process.\n.");
		exit(EXIT_FAILURE);
	}

	return(1);
}

/*
 * Main shell loop to call functions for user to interface with.
 */

void sh_loop()
{
	//Create CommandInformation object.
	struct CommandInfo* ci = CommandInfoInit();
	//Input text.
	char* input;
	//Array of arguments.
	char** arguments;
	//Status of command.
	int status = 1;

	do
	{
		printf(": ");
		fflush(stdout);
		input = shReadInput();
		arguments = shParseInput(input, ci);
		if(arguments[0] != NULL)
			status = shExecuteArguments(arguments, ci);
		memset(arguments, '\0', sizeof(arguments));
		memset(input, '\0', sizeof(input));
		ci->inFileName = NULL;
		ci->outFileName = NULL;
		ci->outStream = 0;
		ci->inStream = 0;
		ci->isBackground = false;
	}while(status);
}

int main(int argc, char** argv)
{
	//Shell loop.
	sh_loop();
}




