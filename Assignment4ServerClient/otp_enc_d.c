#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXSIZE 1000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
void readFile(int socketFD, char* fileContents);
void encryptMessage(char* textFile, char* key, char* encryptedMessage);

int main(int argc, char *argv[])
{
	int textFileLength = 0;
	int keyFileLength = 0;
	int totlaBytes = 0;
	int charCount = 0;
	int connectionCount = 0;
	char* encryptedMessage;
	int currKey, currText;
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, PID;
	socklen_t sizeOfClientInfo;
	char textFile[MAXSIZE], key[MAXSIZE], encryptedMessage[MAXSIZE];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	
	while(connectionCount < 10)
	{
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		connectionCount++;
		
		if (establishedConnectionFD < 0) error("ERROR on accept");
		
		PID = fork();
		
		if(PID < 0)
			error("ERROR on forking");
	
		if(PID == 0)//Child process. Get both files and do encryption.
		{
			write(socketFD, "opt_enc_d", 9);//Confirms appropriate connection.

			do
			{
				charCount = recv(establishedConnectionFD, textFile, MAXSIZE - 1, 0);
				if(charCount < 0)
				{
					close(socketFD);
					error("ERROR on recv.");
				}	
		
				textFileLength += charCount;
			
			}while(textFile[textFileLength] != '\0');	

			charCount = write(establishedConnectionFD, "Success textfile.", 17);
		
			do
			{
				charCount = recv(establishedConnectionFD, key, MAXSIZE - 1, 0);
				if(charCount < 0)
				{
					close(socketFD);
					error("ERROR on recv.");
				}	
		
				keyFileLength += charCount;
			
			}while(key[totalBytes] != '\0');	
	
			charCount = write(establishedConnectionFD, "Success keyfile.", 16);
		
		
			if(keyFileLength != textFileLength)
				error("Non-matching file sizes.");

			//validate plaintext file contents.
			for(i = 0; i < textFileLength; i++)
			{
				if((int)textFile[i] < 65 || (int)textFile[i] > 90 && (int)textFile[i] != 32)
					error("Invalid textfile characters.");
			}
			
			//validate key file contents.
			for(i = 0; i < keyFileLength; i++)
			{
				if((int)key[i] < 65 || (int)key[i] > 90 && (int)key[i] != 32)
					error("Invalid keyfile characters.");
			}
			
				//Encrypt the message here.
			for(i = 0; i < textFileLength; i++)
			{
				//Handle spaces for the integer value conversions.
				if(textFile[i] == ' ')
					textFile[i] = '@';
				if(key[i] == ' ')
					key[i] = '@';
	
				currText = (int)textFile[i] - 64;
				currKey = (int)key[i] - 64;
	
				encryptedMessage[i] =(char)(((currText + currKey) % 27) + 64);
				
				if(encryptedMessage[i] = '@')
					encryptedMessage[i] = ' ';
			}
	
			charCount = write(socketFD, encryptedMessage, textFileLength);
	
			if(charCount < 0)
				error("ERROR on encrypted write.");
			
			close(socketFD);
			close(establishedConnectionFD);
		}
		else	
			close(socketFD); // Close the existing socket which is connected to the client.	
	}
	
	return 0;
}		
