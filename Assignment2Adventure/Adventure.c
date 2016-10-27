#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

struct room
{
	int numConnections;
	FILE* fileHandle;
	char* roomName;
	char* roomType;
	struct room** roomConnections;
};

int* chooseNameIndices()
{
	srand(time(NULL));
	bool flag = false;
	int* indices = malloc(sizeof(int) * 7);
	int i = 1;
	int j = 0;
	int temp;
	indices[0] = rand()%10;
	while(i < 7)
	{
		temp = rand()%10;
		while(j <= i)
		{
			if(indices[j] == temp)
			{
				flag = true;
				break;
			}
			else
				j++;
		}
		
		if(flag)
		{
			flag = false;
		}
		else
		{
			indices[i] = temp;
			i++;
		}
		
		j = 0;
	}
	
	return indices;
}

char* spawnDirectory()
{
	//Get the process ID.
	int pid = getpid();

	//Allocate memory for directory name to be stored in.
	char* directoryName = malloc(sizeof(char) * 20);

	//Combine the integer value of process ID and the hardcoded directory name into a string value.
	snprintf(directoryName, 19, "%s%d", "./chathamb.rooms.",  pid);
	
	//Create the directory.
	mkdir(directoryName, 755);

	//Return the directory name.
	return directoryName;
}

char** generateRoomNames()
{

	//Allocate memory for pointers to ten possible room names.
	char** roomNames = malloc(sizeof(char*) * 10);
	int i = 0;

	//Allocate memory for string of length 20 for each room name character pointer.
	for(i = 0; i < 10; i++)
	{
		roomNames[i] = malloc(sizeof(char) * 30);
	}
	
	//Harccode room names to be assigned and copied into the string array of possible names.
	char* room1 = "Seattle";
	char* room2 = "Library";
	char* room3 = "WoW";
	char* room4 = "Halo";
	char* room5 = "OASIS";
	char* room6 = "Nike";
	char* room7 = "Mario";
	char* room8 = "Starcraft";
	char* room9 = "Portland";
	char* room10 = "Warcraft";

	//Use strcpy to assign contents of hardcoded room names to the roomNames array of strings.
	strcpy(roomNames[0], room1);
	strcpy(roomNames[1], room2);
	strcpy(roomNames[2], room3);
	strcpy(roomNames[3], room4);
	strcpy(roomNames[4], room5);
	strcpy(roomNames[5], room6);
	strcpy(roomNames[6], room7);
	strcpy(roomNames[7], room8);
	strcpy(roomNames[8], room9);
	strcpy(roomNames[9], room10);

	//Return the array of room names.
	return roomNames;
	
}

FILE** createRoomFiles(char* directoryName)
{
	FILE** filePointers = malloc(sizeof(FILE*) * 7);
	
	/*These each with create a new text file for future use in the game.
	 *These files can be read and written but will be created in this instance.
	 *The file descriptor will be stored in an array of ints and returned.
	 *This will allow for future access to the file streams.
	 */
 
	char filePath[30]; 
	snprintf(filePath, 29, "%s/%s", directoryName, "room1.txt");
	filePointers[0] = fopen(filePath, "a+");
	
	snprintf(filePath, 29, "%s/%s", directoryName, "room2.txt");
	filePointers[1] = fopen(filePath, "a+");

	snprintf(filePath, 29, "%s/%s", directoryName, "room3.txt");
	filePointers[2] = fopen(filePath, "a+");

	snprintf(filePath, 29, "%s/%s", directoryName, "room4.txt");
	filePointers[3] = fopen(filePath, "a+");

	snprintf(filePath, 29, "%s/%s", directoryName, "room5.txt");
	filePointers[4] = fopen(filePath, "a+");

	snprintf(filePath, 29, "%s/%s", directoryName, "room6.txt");
	filePointers[5] = fopen(filePath, "a+");

	snprintf(filePath, 29, "%s/%s", directoryName, "room7.txt");
	filePointers[6] = fopen(filePath, "a+");

	return filePointers;
}


void prepareRooms(struct room** gameRooms)
{
	
	int i, j;
	
	//Formats the room file for the room name.
	for(i = 0; i < 7; i++)
		fprintf(gameRooms[i]->fileHandle, "ROOM NAME: %s\n", gameRooms[i]->roomName);
	
	//Formats the room file for the room connections.
	for(i = 0; i < 7; i++)
	{
		//While not the end of the room connection array.
		while(gameRooms[i]->roomConnections[j] != NULL && j < gameRooms[i]->numConnections)
		{
			fprintf(gameRooms[i]->fileHandle, "CONNECTION %d: %s\n", j+1, gameRooms[i]->roomConnections[j]->roomName);
			j++;
		}
		//Reset j for looping.
		j=0;
	}
	
	//Formats the room file for the room type.
	for(i = 0; i < 7; i++)
		fprintf(gameRooms[i]->fileHandle, "ROOM TYPE: %s", gameRooms[i]->roomType);
}


struct room** setupRooms(char** roomNames, int* randomIndices)
{
	int i, j;
	struct room** roomHolder = malloc(sizeof(struct room*) * 7);

	//For each room that will go in the room holder array....
	for(i = 0; i < 7; i++)
	{
		//Allocate memory for the rooms and set first member variables.
		roomHolder[i] = malloc(sizeof(struct room));
		roomHolder[i]->roomName = malloc(sizeof(char) * 30);
		roomHolder[i]->roomType = malloc(sizeof(char) * 30);
		strcpy(roomHolder[i]->roomName, roomNames[randomIndices[i]]);

		roomHolder[i]->numConnections = 0;

		if(i == 0)
			roomHolder[i]->roomType = "START_ROOM";
		else if( i == 5)
			roomHolder[i]->roomType = "END_ROOM";
		else
			roomHolder[i]->roomType = "MID_ROOM"; 
		
		//Allocates memory for pointers to other rooms. Maybe not necessary but wanted to be sure.
		roomHolder[i]->roomConnections = malloc(sizeof(struct room) * 7);
		
		//Sets all connections to NULL to start.
		for(j = 0; j < 7; j++)
			roomHolder[i]->roomConnections[j] = NULL;	
	}

	//Return the rooms
	return roomHolder;

}

int isGraphFull(struct room** gameRooms)
{
	int i;

	//Return index of incomplete room if one exists, if not, return -1.
	for(i = 0; i < 7; i++)
	{
		if(gameRooms[i]->numConnections < 4)
		{
			return i;
		}
	}
	
	return -1;
}

void connect(struct room* room1, struct room* room2)
{
	//Connect the rooms passed into function.
	int i = 0;
	while(room1->roomConnections[i] != NULL)
		i++;
	room1->roomConnections[i] = room2;
	room1->numConnections++;

	i = 0;
	while(room2->roomConnections[i] != NULL)
		i++;
	room2->roomConnections[i] = room1;
	room2->numConnections++;
}

bool roomsAreNotConnected(struct room* room1, struct room* room2)
{
	//Iterate over the room connections and check if they are equal. 
	int i = 0;
	while((room1->roomConnections[i] != NULL) && (i < 7))
	{
		if(strcmp(room1->roomConnections[i]->roomName, room2->roomName) == 0)
			return false;
		else
			i++;
	}
	
	//Returns true if the rooms were not found to be a match based on room name.
	return true;
}

void addConnection(struct room* roomToAddTo, struct room** gameRooms)
{
	//Connects the two rooms once it is verified that they are not connected and they are not the same room. 
	int randomIndex;
	while(true)
	{
		srand(time(NULL));
		randomIndex = rand()%7;
		
		//Checks if the rooms are not compatible.
		if((gameRooms[randomIndex]->numConnections < 7) && (gameRooms[randomIndex] != roomToAddTo) && roomsAreNotConnected(gameRooms[randomIndex], roomToAddTo))
		{	
			connect(roomToAddTo, gameRooms[randomIndex]);
			break;
		}
	}
}

void connectRooms(struct room** gameRooms, int* randomIndices)
{
	//Checks if the graph is full and adds a connection to the returned index if it is not.
	int i, j, roomIndex;
	while((roomIndex = isGraphFull(gameRooms)) != -1)
	{
		addConnection(gameRooms[roomIndex], gameRooms);
	}
}

void setRoomFilePointers(FILE** filePointers, struct room** gameRooms)
{
	int i;
	for(i = 0; i < 7; i++)
	{
		//Sets the file handle to the appropriate handle in the file pointer array.
		gameRooms[i]->fileHandle = filePointers[i];
	}
}

bool checkRoomType(struct room* currentRoom)
{
	//Checks if the room is the end room to end the game.
	if(strcmp(currentRoom->roomType, "END_ROOM") == 0)
		return true;
	else
		return false; 
}

void readRoomFiles(char* directoryName, struct room** gameRooms)
{
	
	//Could all probably be done in about 4 lines of a for-loop but won't try it unless I have time after I finish.
	//Does the same thing for connecting to files as previously done except it is for reading only this time.
	int i, j;
	const char spaceDelimiter[2] = " ";
	char* newLineRemover;
	char* pertinentInfo;
	char inputFromFile[30];
	char filePath[30];
       	snprintf(filePath, 29, "%s/%s", directoryName, "room1.txt");
       	gameRooms[0]->fileHandle = fopen(filePath, "r");
	
        snprintf(filePath, 29, "%s/%s", directoryName, "room2.txt");
        gameRooms[1]->fileHandle = fopen(filePath, "r");

        snprintf(filePath, 29, "%s/%s", directoryName, "room3.txt");
        gameRooms[2]->fileHandle = fopen(filePath, "r");
	
        snprintf(filePath, 29, "%s/%s", directoryName, "room4.txt");
        gameRooms[3]->fileHandle = fopen(filePath, "r");

        snprintf(filePath, 29, "%s/%s", directoryName, "room5.txt");
        gameRooms[4]->fileHandle = fopen(filePath, "r");

        snprintf(filePath, 29, "%s/%s", directoryName, "room6.txt");
        gameRooms[5]->fileHandle = fopen(filePath, "r");
	
        snprintf(filePath, 29, "%s/%s", directoryName, "room7.txt");
        gameRooms[6]->fileHandle = fopen(filePath, "r"); 

	for(i = 0; i < 7; i++)
	{
		//Clears the string and starts parsing through the file.
		memset(inputFromFile, '\0', sizeof(inputFromFile));
		fgets(inputFromFile, 29, gameRooms[i]->fileHandle);

		//This little trick I found on stackoverflow to replace the endline character of the string with a null character for string comparisons.
		newLineRemover = strchr(inputFromFile, '\n');
		if(newLineRemover)
			*newLineRemover = '\0';
		
		//Clears out the formatting and takes out the room name to restore it into the structure.
		pertinentInfo = strtok(inputFromFile, spaceDelimiter);
		pertinentInfo = strtok(NULL, spaceDelimiter);
		pertinentInfo = strtok(NULL, spaceDelimiter);
		
		//Copy the string read from the file into the room struct name.
		strcpy(gameRooms[i]->roomName, pertinentInfo);
		
		for(j = 0; j < gameRooms[i]->numConnections; j++)
		{
			//Clears out the string again.
			memset(inputFromFile, '\0', sizeof(inputFromFile));
			fgets(inputFromFile, 29, gameRooms[i]->fileHandle);
			//Same trick as before. 
			printf("%s\n", inputFromFile);
			newLineRemover = strchr(inputFromFile, '\n');
			if(newLineRemover)
				*newLineRemover ='\0';
			//clears out the formatting and takes out the room name from the room connection to store it back in the strcture.
			pertinentInfo = strtok(inputFromFile, spaceDelimiter);
			pertinentInfo = strtok(NULL, spaceDelimiter);
			pertinentInfo = strtok(NULL, spaceDelimiter);
		}

		
		memset(inputFromFile, '\0', sizeof(inputFromFile));
		fgets(inputFromFile, 29, gameRooms[i]->fileHandle);
		
		newLineRemover = strchr(inputFromFile, '\n');
		if(newLineRemover)
			*newLineRemover = '\0';
		pertinentInfo = strtok(inputFromFile, spaceDelimiter);
		pertinentInfo = strtok(NULL, spaceDelimiter);
                pertinentInfo = strtok(NULL, spaceDelimiter);
	}
}

pthread_mutex_t mutex;

void* writeTimeFile()
{
	//Writes the current time to a file called time.txt
	time_t currTime;
	currTime = time(NULL);
	FILE* timeFile;
	timeFile = fopen("time.txt", "w+");
	fprintf(timeFile, "%s", ctime(&currTime));
	fclose(timeFile);
}

void printTimeFile()
{
	//Create local varaibles.
	FILE* fp;
	char buffer[255];
	memset(buffer, '\0', sizeof(buffer));
	int c;
	int pos = 0;

	fp = fopen("time.txt", "r"); //open file
	
	do
	{
		c = fgetc(fp); //get a character from the file
		
		if (c != EOF) //if not end of file
		{
			buffer[pos] = (char)c;
			pos++;
		}
	} while (c != EOF && c != '\n'); //reads one line
	printf("\n\n");
	printf("%s", buffer);

	fclose(fp); //closes file pointer.
}

void threadTwo()
{
        pthread_t thread2;
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_lock(&mutex);
        int threadID = pthread_create(&thread2, NULL, writeTimeFile, NULL);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_destroy(&mutex);
        sleep(1);
}



void  playGame(FILE** filePointers, struct room* currentRoom)
{
	char** pathToVictory = malloc(sizeof(char) * 10000);
	memset(pathToVictory, '\0', sizeof(pathToVictory)); 
	int stepCounter = 0;
	int i;
	char userResponse[30];
	while(true)
	{
		//Formatting
		printf("\n");
		if(checkRoomType(currentRoom))
		{
			printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
			printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCounter);
			for(i = 0; i < stepCounter; i++)
				printf("%s\n", pathToVictory[i]);
			break;
		}
		
		//Write interface formatting.
		printf("CURRENT LOCATION: %s\n", currentRoom->roomName);
		printf("POSSIBLE CONNECTIONS: ");
		
		//Iterate over all of the connections the room has.
		for(i = 0; i < currentRoom->numConnections; i++)
		{
			printf("%s", currentRoom->roomConnections[i]->roomName);
			if(i+1 ==  currentRoom->numConnections)
				printf(".\n");
			else
				printf(", ");
		}
		
		//Formatting
		printf("WHERE TO?  ");
		memset(userResponse, '\0', sizeof(userResponse));
		scanf("%s", userResponse);
		if(strcmp(userResponse,"time") == 0)
		{
			threadTwo();
			printTimeFile();
		}
		else
		{
			for(i = 0; i < currentRoom->numConnections; i++)
			{
				if(strcmp(userResponse, currentRoom->roomConnections[i]->roomName) == 0)
				{
					pathToVictory[stepCounter++] = currentRoom->roomName;
					currentRoom = currentRoom->roomConnections[i];
					break;
				}
				
				if(i+1 == currentRoom->numConnections)
				{
					printf("\n");
					printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
				}
			}
		}
	}		
}

int main(int argc, char **argv)
{
	//Runs all of the functions in the appropriate order and starts up the game! Have fun :)
	int i;
	char* directoryName = spawnDirectory();
	char** roomNames = generateRoomNames();
	FILE** filePointers = createRoomFiles(directoryName);
	int* randomIndices = chooseNameIndices();
	struct room** gameRooms = setupRooms(roomNames, randomIndices); 
	connectRooms(gameRooms, randomIndices);
	setRoomFilePointers(filePointers, gameRooms);
	prepareRooms(gameRooms);
	for(i = 0; i < 7; i++)
		fclose(filePointers[i]);
	//readRoomFiles(directoryName, gameRooms);
	for(i = 0; i < 7; i++)
		gameRooms[i]->fileHandle = NULL;
	struct room* startRoom = gameRooms[0];
			
	playGame(filePointers, startRoom);

	return 0;
}



