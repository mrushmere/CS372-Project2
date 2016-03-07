/*************************************************************************************
 * Mark Rushmere
 * CS 372
 * Project 2
 * Decription: This program (ftserver) opens a sock and listens for incoming requests. 
 * ***********************************************************************************/


#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>
#include <arpa/inet.h>


 // Function prototypes
 void listCommand(int clientSock, int clientDataSock);
 void getCommand(int clientSock, int clientDataSock, char *fileName);
 int setSocket(int port);
 int connectSocket(int socket);


int main(int argc, char* argv[]) {

	// Check if the port number was provided
	int portNo, serverSock;
	if(argc != 2) {
		printf("Proper usage: ftserver [port number]");
		exit(1);
	}
	else {
		portNo = atoi(argv[1]);
	}

	serverSock = setSocket(portNo);

	// Loop until user sends SIGINT
	while(1) {
		// Set buffers and variables
		char buffer[512];
		char res[1024];
		char fileName[512];
		char dataPortNoStr[16];
		char type[4];
		int dataPortNo;	
		int serverDataSock, clientSock, clientDataSock;
		//printf("server running on port %d\n", portNo);
		
		clientSock = connectSocket(serverSock);
		if(clientSock < 0) {
			perror("connecting");
			break;
		}
		printf("connected to the client");

		memset(buffer, '\0', 512);
		memset(type, '\0', 4);
		memset(fileName, '\0', 512);
		memset(dataPortNoStr, '\0', 16);
		memset(res, '\0', 1024);

		if(read(clientSock, buffer, sizeof(buffer)) < 0) {
			perror("error reading command");
			exit(1);
		}

		printf("buffer contents %s\n", buffer);
		char *temp;
		temp = strtok(buffer, " \n\0");
		strcpy(type, temp);
		printf("com: %s\n", type);
		temp = strtok(NULL, " \n\0");
		strcpy(dataPortNoStr, temp);
		dataPortNo = atoi(temp);
		if(type[0] == 'g') {
			printf("parsing the filename\n");
			temp = strtok(NULL, " \n\0");
			strcpy(fileName, temp);
		}

		// Set up the data socket
		serverDataSock =  setSocket(dataPortNo);
		clientDataSock = connectSocket(serverDataSock);

		// Call appropriate functions based on command
		if(type[0] == 'l') {
				printf("calling list command");
				listCommand(clientSock, clientDataSock);
		}
		else if(type[0] == 'g') {
			printf("calling get command");
			getCommand(clientSock, clientDataSock, fileName);
		}
		else {
			strcpy(res, "unknown command");
			if(write(clientSock, res, sizeof(res)) < 0) {
				perror("error sending error message");
			}
		}
		close(clientDataSock);
		close(serverDataSock);
		close(clientSock);
		
	}
	return 0;
}

int setSocket(int port) {

	int socketNum;
	int one = 1;
	struct sockaddr_in sa;

	socketNum = socket(AF_INET, SOCK_STREAM, 0);

	if(socketNum < 0) {
		perror("socket");
		exit(1);
	}

	memset(&sa, '\0', sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(port);


	if(setsockopt(socketNum, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
		perror("setsocketopt");
		exit(1);
	}

	if(bind(socketNum, (struct sockaddr*) &sa, sizeof(struct sockaddr_in)) < 0) {
			printf("Error binding the socket\n");
			exit(1);
	}

	if(listen(socketNum, 10) == -1) {
		perror("listen");
		exit(1);
	}
	return socketNum;

}

int connectSocket(int socket) {
	struct sockaddr_in clientAddr;
	int clientSocket;
	int clientLen = sizeof(clientAddr);
	clientSocket = accept(socket, (struct sockaddr*) &clientAddr, &clientLen);

	if(clientSocket < 0) {
		perror("accept");
		return -1;
	}
	else {
		printf("client connected");
		return clientSocket;
	}


}


void listCommand(int clientSock, int clientDataSock) {
	// Allocate memory for the message to to be sent
	char send[1024];
	memset(send, '\0', 1024);
	char *ok = "OK";
	if(write(clientSock, ok, strlen(ok)) < 0) {
		perror("error sending OK response");
	}

	// http://pubs.opengroup.org/onlinepubs/009695399/functions/opendir.html
	DIR *dir;
	struct dirent *dp;

	// Get current directory 
	dir = opendir(".");
	if(dir) {
		// Get the contents of the current directory
		while((dp = readdir(dir)) != NULL) {
			strcat(send, dp->d_name);
			strcat(send, "\n");
		}
	}
	
	closedir(dir);

	if(strlen(send) <= 1) {
		strcpy(send, "The directory is empty");

	}

	if(write(clientDataSock, send, strlen(send)) < 0) {
		perror("error sending directory contents");
	}

	printf("done sending directory");
}

void getCommand(int clientSock, int clientDataSock, char *fileName) {

	// Messages for the status of file
	char *ff = "receving file from flip\n";
	char *fnf = "file not found\n"; 
	char *buf = (char*) malloc(512 * sizeof(char));

	// Open the file to be sent
	FILE* fp;
	if((fp = fopen(fileName, "r")) == NULL) {
		// send message that file was not found
		perror("writing file send message");

	}

	int read;
	do {
		if((read = fread(buf, 1, 512, fp)) < 0) {
			perror("reading from file");
		}
		if(write(clientDataSock, buf, read) < 0) {
			perror("writing to client");
		}
	} while(read > 0);
	fclose(fp);

}