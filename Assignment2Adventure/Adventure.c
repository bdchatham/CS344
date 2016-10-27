#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

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
	for(i = 0; i < 7; i++)
		fprintf(gameRooms[i]->fileHandle, "ROOM NAME: %s\n", gameRooms[i]->roomName);
	
	for(i = 0; i < 7; i++)
	{
		while(gameRooms[i]->roomConnections[j] != NULL && j < gameRooms[i]->numConnections)
		{
			fprintf(gameRooms[i]->fileHandle, "CONNECTION %d: %s\n", j+1, gameRooms[i]->roomConnections[j]->roomName);
			j++;
		}
	
		j=0;
	}
	
	for(i = 0; i < 7; i++)
		fprintf(gameRooms[i]->fileHandle, "ROOM TYPE: %s", gameRooms[i]->roomType);
}


struct room** setupRooms(char** roomNames, int* randomIndices)
{
	int i, j;
	struct room** roomHolder = malloc(sizeof(struct room*) * 7);

	for(i = 0; i < 7; i++)
	{
		roomHolder[i] = malloc(sizeof(struct room));
		roomHolder[i]->roomName = malloc(sizeof(char) * 30);
		strcpy(roomHolder[i]->roomName, roomNames[randomIndices[i]]);

		roomHolder[i]->numConnections = 0;

		if(i == 0)
			roomHolder[i]->roomType = "START_ROOM";
		else if( i == 5)
			roomHolder[i]->roomType = "END_ROOM";
		else
			roomHolder[i]->roomType = "MID_ROOM"; 
		
		roomHolder[i]->roomConnections = malloc(sizeof(struct room) * 7);
		for(j = 0; j < 7; j++)
			roomHolder[i]->roomConnections[j] = NULL;	
	}

	return roomHolder;

}

int isGraphFull(struct room** gameRooms)
{
	int i;
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
	int i = 0;
	while((room1->roomConnections[i] != NULL) && (i < 7))
	{
		if(strcmp(room1->roomConnections[i]->roomName, room2->roomName) == 0)
			return false;
		else
			i++;
	}
	
	return true;
}

void addConnection(struct room* roomToAddTo, struct room** gameRooms)
{
	int randomIndex;
	while(true)
	{
		srand(time(NULL));
		randomIndex = rand()%7;
		if((gameRooms[randomIndex]->numConnections < 7) && (gameRooms[randomIndex] != roomToAddTo) && roomsAreNotConnected(gameRooms[randomIndex], roomToAddTo))
		{	
			connect(roomToAddTo, gameRooms[randomIndex]);
			break;
		}
	}
}

void connectRooms(struct room** gameRooms, int* randomIndices)
{
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
		gameRooms[i]->fileHandle = filePointers[i];
	}
}

bool checkRoomType(struct room* currentRoom)
{
	if(strcmp(currentRoom->roomType, "END_ROOM") == 0)
		return true;
	else
		return false; 
}

void readRoomFiles(char* directoryName, struct room** gameRooms)
{
	
	//Could all probably be done in about 4 lines of a for-loop but won't try it unless I have time after I finish.
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
		memset(inputFromFile, '\0', sizeof(inputFromFile));
		fgets(inputFromFile, 30, gameRooms[i]->fileHandle);
		newLineRemover = strchr(inputFromFile, '\n');
		if(newLineRemover)
			*newLineRemover = '\0';
		pertinentInfo = strtok(inputFromFile, spaceDelimiter);
		pertinentInfo = strtok(NULL, spaceDelimiter);
		pertinentInfo = strtok(NULL, spaceDelimiter);
		
		//Copy the string read from the file into the room struct name.
		strcpy(gameRooms[i]->roomName, pertinentInfo);
		
		for(j = 0; j < gameRooms[i]->numConnections; j++)
		{
			memset(inputFromFile, '\0', sizeof(inputFromFile));
			fgets(inputFromFile, 30, gameRooms[i]->fileHandle);
			newLineRemover = strchr(inputFromFile, '\n');
			if(newLineRemover)
				*newLineRemover = '\0';
			pertinentInfo = strtok(inputFromFile, spaceDelimiter);
			pertinentInfo = strtok(NULL, spaceDelimiter);
			pertinentInfo = strtok(NULL, spaceDelimiter);
	
			//Copt the string read from the file into the room struct connection slot.
			strcpy(gameRooms[i]->roomConnections[j]->roomName, pertinentInfo);	
		}
	
	}
}

void  playGame(FILE** filePointers, struct room* currentRoom)
{
	//char** pathToVictory
	int i;
	char userResponse[30];
	while(true)
	{
		printf("\n");
		if(checkRoomType(currentRoom))
		{
			printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
			//winning description.
			break;
		}
		
		printf("CURRENT LOCATION: %s\n", currentRoom->roomName);
		printf("POSSIBLE CONNECTIONS: ");
		
		for(i = 0; i < currentRoom->numConnections; i++)
		{
			printf("%s", currentRoom->roomConnections[i]->roomName);
			if(i+1 ==  currentRoom->numConnections)
				printf(".\n");
			else
				printf(", ");
		}
		
		printf("WHERE TO?  ");
		memset(userResponse, '\0', sizeof(userResponse));
		scanf("%s", userResponse);
		for(i = 0; i < currentRoom->numConnections; i++)
		{
			if(strcmp(userResponse, currentRoom->roomConnections[i]->roomName) == 0)
			{
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

int main(int argc, char **argv)
{
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
	readRoomFiles(directoryName, gameRooms);
	
	struct room* startRoom = gameRooms[0];
	
		
	playGame(filePointers, startRoom);
	
	return 0;
}


/*primary thread, lock mutex
 * then 
 *maybe use join
 */
