#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#define MAXSIZE 100000


int main(int argc, char *argv[])
{
	char ch;
	FILE *keyFD, *textFD; 
	int i, keyFileSize, encodedSize;
	int socketFD, portNumber, charsRead, fileSize, charsSent;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char encodedText[MAXSIZE], key[MAXSIZE], response[MAXSIZE], plainText[MAXSIZE];
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	//Validate port number.
	if(portNumber < 0 || portNumber > 65535)
	{
		fprintf(stderr, "Port Invalid.\n");
		exit(2);
	}
	
	//Read file information in.
	textFD = fopen(argv[1], "r");
	if(textFD < 0)
	{
		fprintf(stderr, "Failed to open text file.");
		exit(1);
	}
	
	keyFD = fopen(argv[2], "r");
	if(keyFD < 0)
	{
		fprintf(stderr, "Failed to open key file");
		exit(1);
	}

	i = 0;	

	//Read the file character by character.
	while((ch = getc(textFD)) != EOF )
	{
		if(ch == '\n')
			break;
		encodedText[i++] = (char)ch;
	}
	encodedText[i++] = '\0';
	encodedSize = i;
	
	//Read the file character by character.
	i = 0;
	while((ch = getc(keyFD)) != EOF)
	{
		if(ch == '\n')
			break;
		key[i++] = (char)ch;
	}
	key[i++] = '\0';
	keyFileSize = i;

	if(keyFileSize < encodedSize)
	{
		fprintf(stderr, "Invalid filesizes.");
		exit(1);
	}
	fclose(textFD);
	fclose(keyFD);

	//Validate file contents.
	for(i = 0; i < encodedSize - 1; i++)
	{
		if((int)encodedText[i] > 90 || (int)encodedText[i] < 65 && (int)encodedText[i] != 32)
		{
			fprintf(stderr, "Text file contains invalid characters.");
			exit(1); 
		}
	}
	
	//Validate file contents.
	for(i = 0; i < keyFileSize - 1; i++)
	{
		if((int)key[i] > 90 || (int)key[i] < 65 && (int)key[i] != 32)
		{	
			fprintf(stderr, "Key file contains invalid characters.");
			exit(1);
		}
	}

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		fprintf(stderr, "CLIENT: ERROR connecting.");

	//Response from server validating connection
	memset(response, '\0', sizeof(response));
	charsRead = recv(socketFD, response, sizeof(response) - 1, 0); 	
	if(charsRead < 0)
	{
		fprintf(stderr, "Did not receive response from server.");
		exit(1);
	}
	//Do a strcmp on response to check that the response was the correct server.
	if(strstr(response, "otp_dec_d") == NULL)
	{
		strcmp(encodedText, "NULL\0");
		charsSent = write(socketFD, encodedText, strlen(encodedText));
		fprintf(stderr, "Connected to incorrect server.");
		exit(1);
	}
	
	//Send the plaintext contents.	
	charsSent = write(socketFD, encodedText, strlen(encodedText));
	
	if(charsSent < encodedSize - 1)
	{
		fprintf(stderr, "Not all of data written to server.");
		exit(1);
	}

	//Receive response from server before proceeding.
	memset(response, '\0', sizeof(response));
	charsRead = recv(socketFD, response, sizeof(response) -1, 0);

	//Send the key contents.
	charsSent = write(socketFD, key, strlen(key));
	
	if(charsSent < keyFileSize - 1)
	{
		fprintf(stderr, "Not all of data written to server.");
		exit(1);
	}

	//Receive response from server before proceeding.
	memset(response, '\0', sizeof(response));
	charsRead = recv(socketFD, response, sizeof(response) - 1, 0);
	
	//Send reponse to make the server wait until ready for ciptertext transfer.
	charsSent = write(socketFD, "otp_enc", 7);
	
	//Receive encrypted ciphertext.
	memset(plainText, '\0', sizeof(plainText));

	do
	{
		charsRead = recv(socketFD, plainText, sizeof(plainText) - 1, 0);//Reads teh plaintext from the daemon and prints it.
	}
	while(charsRead > 0);

	printf("%s\n", plainText);

	close(socketFD); // Close the socket
	return 0;
}
