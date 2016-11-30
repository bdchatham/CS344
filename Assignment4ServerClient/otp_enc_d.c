#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXSIZE 10000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
void readFile(int socketFD, char* fileContents);
void encryptMessage(char* textFile, char* key, char* encryptedMessage);

int main(int argc, char *argv[])
{
	int textFileLength = 0;
	int keyFileLength = 0;
	int charCount = 0;
	int connectionCount = 0;
	int currKey, currText;
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, PID, i;
	socklen_t sizeOfClientInfo;
	char textFile[MAXSIZE], key[MAXSIZE], encryptedMessage[MAXSIZE], response[15];
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
	
	while(1)
	{
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");
		
		PID = fork();

		
		if(PID < 0)
			error("ERROR on forking");
	
		if(PID == 0)//Child process. Get both files and do encryption.
		{
			write(establishedConnectionFD, "opt_enc_d", 9);//Confirms appropriate connection.

			do
			{
				charCount = recv(establishedConnectionFD, textFile, MAXSIZE - 1, 0);
				if(charCount < 0)
				{
					close(listenSocketFD);
					error("ERROR on recv.");
				}	
				
				//printf("current textfile data:%s.\n", textFile);//For Debugging
				
				textFileLength += charCount;
			
			}while(textFile[textFileLength] != '\0');	

			charCount = write(establishedConnectionFD, "Success textfile.", 17);
		
			do
			{
				charCount = recv(establishedConnectionFD, key, MAXSIZE - 1, 0);
				if(charCount < 0)
				{
					close(listenSocketFD);
					error("ERROR on recv.");
				}	
				
				//printf("current keyfile data:%s.\n", key);//For Debugging
		
				keyFileLength += charCount;
			
			}while(key[keyFileLength] != '\0');	
	
			charCount = write(establishedConnectionFD, "Success keyfile.", 16);
			
			//Wait for response before sending the ciphertext.
			memset(response, '\0', sizeof(response));
			charCount = recv(establishedConnectionFD, response, sizeof(response) - 1, 0);
		
			memset(encryptedMessage, '\0', sizeof(response));	
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
				//printf("Curr encryptMessage char:%c\n", encryptedMessage[i]);//For Debugging
				if(encryptedMessage[i] == '@')
					encryptedMessage[i] = ' ';
			}
			
			charCount = write(establishedConnectionFD, encryptedMessage, strlen(encryptedMessage));
	
			if(charCount < 0)
				error("ERROR on encrypted write.");
			
			close(listenSocketFD);
			close(establishedConnectionFD);

			exit(0);
		}
		else	
			close(establishedConnectionFD); // Close the existing socket which is connected to the client.	
	}
		
	return 0;
}		
