#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>

/*
 * Pre-defined shell interface limitations.
 */

#define MAXCHILDREN 100
#define MAXNUMBEROFARGUMENTS 512
#define MAXCOMMANDLENGTH 2048
int shellStatus = 1;

/*
 * Background children.
 */

struct backgroundChildren
{
	int* ids;
	int maxSize;
	int count;
};

/*
 * Background children constructor.
 */

struct backgroundChildren* backgroundChildrenInit()
{
	struct backgroundChildren* bgc = malloc(sizeof(struct backgroundChildren));
	bgc->maxSize = MAXCHILDREN;
	bgc->count = 0;
bgc->ids = malloc(sizeof(int) * MAXCHILDREN);
	return bgc;
}

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

	return ci;
}

/*
 * Forward Declarations.
 */ 

int shLaunch(char** arguments, struct CommandInfo* ci, struct backgroundChildren* bgc, struct sigaction* CtrlCStopper);
void shLoop(struct sigaction* ctrlCStopper);
char* shReadInput();
char** shParseInput(char* userInput, struct CommandInfo* ci);
int shChangeDirectory(char** arguments);
int shStatus(char** arguments);
int shExit(char** arguments);
int shBuiltInFunctionListSize();
int shExecuteArguments(char** arguments, struct CommandInfo* ci, struct backgroundChildren* bgc, struct sigaction* CtrlCStopper);
int (*builtInFunctions[])(char** arguments);


/*
 * Strings of built-in command names.
 */

char *builtInCommands[] =
{
	"cd",
	"status",
	"exit"
};

/*
 * Array of addresses to built-in functions. Must match the ordering of builtInCommand string list.
 */

int (*builtInFunctions[])(char**) =
{
	&shChangeDirectory,
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
		exit(1);
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
	exit(1);
}

int shStatus(char** arguments)
{
	if(WIFEXITED(shellStatus))
		printf("Exit status: %d.\n", shellStatus);

	else if(WIFSIGNALED(shellStatus));
		printf("Terminated by signal: %d. \n", shellStatus);

	fflush(stdout);
	
	shellStatus = 0;
	
	return 1;
}

/*
 * Parses the user input into individual arguments using strtok.
 */

char** shParseInput(char* userInput, struct CommandInfo* ci)
{
	char* tokenDelimiters = " \n";
	int tokenCount = 0;
	
	//Allocate memory for the max number of arguments.
	char** inputTokens = malloc(sizeof(char*) * MAXNUMBEROFARGUMENTS);
	
	//Current string value.
	char* currentToken;

	if(!inputTokens)
	{
		fprintf(stderr, "Failed to allocate memory for inputTokens.\n");
		exit(1);
	}
	
	currentToken = strtok(userInput, tokenDelimiters);
	
	if(strncmp(currentToken, "#", 1) == 0)
		return inputTokens;

	while(currentToken != NULL && tokenCount < MAXNUMBEROFARGUMENTS)
	{
		if(strcmp(currentToken, "<") == 0)//InFile located, store name in structure.
		{
			currentToken = strtok(NULL, tokenDelimiters);
			ci->inFileName = currentToken;
			
		}
		else if(strcmp(currentToken, ">") == 0)//Outfile located, store name in structure.
		{
			currentToken = strtok(NULL, tokenDelimiters);
			ci->outFileName = currentToken;
		}
		else if(strcmp(currentToken, "&") == 0)//Background process, set flag.
		{
			ci->isBackground = true;
			break;
		}
		else
			inputTokens[tokenCount++] = currentToken;//Store in arguments array.	

		currentToken = strtok(NULL, tokenDelimiters);	
	}
	
	return inputTokens;//Return arguments.
}

/*
 * Changes to the directory with the name in the spot of the second argument if it exists. 
 */ 

int shChangeDirectory(char** arguments)
{
	char* destination = arguments[1];
	if(destination == NULL)
	{
		destination = getenv("HOME");//Sets destination to the root directory.
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
	return(0);//Returns 0 to exit the shLoop.
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

int shExecuteArguments(char** arguments, struct CommandInfo* ci, struct backgroundChildren* bgc, struct sigaction* CtrlCStopper)
{
	int i, j;

	if(arguments[0] == NULL)//No arguments found. Exit.
	{
		shellStatus = 1;
		return 1;
	}
	for(i = 0; i < shBuiltInFunctionListSize(); i++) 
	{
		if(strcmp(arguments[0], builtInCommands[i]) == 0)//Checks to see if the given command is in the predefined function list.
		{	if(strcmp(arguments[0], "exit") == 0)
			{
				for(j =0; j < bgc->count; j++)
					kill(bgc->ids[j], SIGKILL);//Exiting the program - kill all children lololololololol.
			}	
			shellStatus = 1;
			return(*builtInFunctions[i])(arguments);//Return the return value of the built-in function.
		} 
	}
 
	shellStatus = 1;
	return shLaunch(arguments, ci, bgc, CtrlCStopper);//Command is not supported by this shell. 
}

/*
 * Forks the process to run the given arguments if a child process is correctly created.
 */

int shLaunch(char** arguments, struct CommandInfo* ci, struct backgroundChildren* bgc, struct sigaction* CtrlCStopper)
{
	pid_t processID, waitPID;
	int status = 0;
	const char devNull[] = "/dev/null";

	if(ci->inFileName == NULL && ci->isBackground)//Background process with no inFile. Set to dev null.
		ci->inFileName = (char*)devNull;
	
	if(ci->outFileName == NULL && ci->isBackground)//Background process with no outFile. Set to dev null.
		ci->outFileName = (char*)devNull;

	processID = fork();//Fork process.

	if(processID == 0)//Child process. Run the arguments.
	{	
		//Handle the case of Ctrl-C
		CtrlCStopper->sa_handler = SIG_DFL;
		CtrlCStopper->sa_flags = 0;
		sigaction(SIGINT, (&(*CtrlCStopper)), NULL);
	
		if(ci->inFileName != NULL)
		{
			ci->inStream = open(ci->inFileName, O_RDONLY);//Open the infile stream for reading from the specified path from the command.
			if(ci->inStream == -1)
			{
				perror("inStream");
				fflush(stdout);
				shellStatus = 1;
				exit(1);
			}
			else
				dup2(ci->inStream, STDIN_FILENO);//Copy the file stream to stdin to redirect input.
		}
		
		if(ci->outFileName != NULL)
		{
			ci->outStream = open(ci->outFileName, O_WRONLY);//Open the outfile stream for writing from the specific path from the command.

			if(ci->outStream == -1)
			{
				perror("outStream");
				fflush(stdout);
				shellStatus = 1;
				exit(1);
			}
			else
				dup2(ci->outStream, STDOUT_FILENO);//Copy the file stream to stdout to redirect output.
		}
		fflush(stdout);
		if(execvp(arguments[0], arguments) == -1)//Child did not properly execute program.
		{
			fprintf(stderr, "Error executing argument.\n");
			fflush(stdout);
			shellStatus = 1;
           		exit(EXIT_FAILURE);//Exit because execvp failed and therefore the program should stop.
		}
	}
	else if(processID > 0)//Parent Process.
	{
		//Handle the case of Ctrl-C
		CtrlCStopper->sa_handler = SIG_IGN;
		sigaction(SIGINT, (&(*CtrlCStopper)), NULL);
		
		if(!ci->isBackground)//Not a background process so we need to wait for it.
		{	
			do
			{
				waitPID = waitpid(processID, &status, WUNTRACED);	
			}while(!WIFEXITED(status) && !WIFSIGNALED(status));//Waits for the child process until it exits or is killed by a signal.
		
			shellStatus = status;
		}
		else//Add the child to the list of children in ci.
		{
			printf("Background process %d is being executed.\n", processID);
			fflush(stdout);
			bgc->ids[bgc->count++] = processID;//Adds the child to the array and increments ALL IN ONE LINE SO NIFTY.
		}
	}
	else//Parent Process but child could not be created.
	{
		fprintf(stderr, "Error creating child process.\n.");
		shellStatus = 1;
		exit(EXIT_FAILURE);
	}
	
	return 1;
}

/*
 * Main shell loop to call functions for user to interface with.
 */

void sh_loop(struct sigaction* CtrlCStopper)
{
	int i;
	//Create CommandInformation object.
	struct CommandInfo* ci = CommandInfoInit();
	//Create backgroundChildren object.
	struct backgroundChildren* bgChildren = backgroundChildrenInit();
	//Input text.
	char* input;
	//Array of arguments.
	char** arguments;
	//Looper
	int status = 1;
	
	do
	{
		printf(": ");//Prompt for input.
		fflush(stdout);
		input = shReadInput();
		arguments = shParseInput(input, ci);//Parse the input and set the structure flags and file specific information.
		if(arguments[0] != NULL)
			status = shExecuteArguments(arguments, ci, bgChildren, CtrlCStopper);//Execute the given argumemts.
		//Clear the variables for input.
		memset(arguments, '\0', MAXNUMBEROFARGUMENTS);
		memset(input, '\0', MAXCOMMANDLENGTH);
		//Reset the structure values.
		ci->inFileName = NULL;
		ci->outFileName = NULL;
		ci->outStream = 0;
		ci->inStream = 0;
		ci->isBackground = false;
		//Check if background processes have exited or were terminated.
		if(bgChildren->count > 0)
		{
			int exitValue;
			pid_t waitPID = -1;
			waitPID = waitpid(waitPID, &exitValue, WNOHANG);

			if(waitPID != 0)
			{
				if(WIFEXITED(exitValue))//Process exited.
				{
					printf("Background process %d is completed with the exit value %d.\n", waitPID, WEXITSTATUS(exitValue));
					fflush(stdout);
				}
				if(WIFSIGNALED(exitValue))
				{
					printf("Background process %d was terminated by signal %d.\n", waitPID, exitValue);
					fflush(stdout);
				}
				//Removing child from array.
				for(i = 0; i < bgChildren->count; i++)
				{
					if(bgChildren->ids[i] == waitPID)//This removes the child from the array by taking the value from the back and putting it into the removed spot, setting final value to 0, and decrement.
					{
						bgChildren->ids[i] = bgChildren->ids[bgChildren->count];
						bgChildren->ids[bgChildren->count] = 0;
						bgChildren->count--;
					}
				}
			}
		}
	}while(status);//Check the return status - will only exit if exit is called and exit returns a value of 0.
}

int main(int argc, char** argv)
{
	struct sigaction* ctrlCStopper = malloc(sizeof(struct sigaction));//Creates the struct for handling ctrl-c functionality.
	ctrlCStopper->sa_handler = SIG_IGN;
	sigaction(SIGINT, (&(*ctrlCStopper)), NULL);
		
	//Shell loop.
	sh_loop(ctrlCStopper);
    return 0;
}




