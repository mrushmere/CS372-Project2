/*************************************************************************************
 * Mark Rushmere
 * CS 372
 * Project 2
 * Decription: This program (ftserver) opens a sock and listens for incoming requests. 
 * ***********************************************************************************/


#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>

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
		char *buffer = (char*) malloc(1024 * sizeof(char));
		char *com = (char*) malloc(4 * sizeof(char));
		char *fileName = (char*) malloc(512 * sizeof(char));
		char *dataPortNoStr = (char*) malloc(16 * sizeof(char));
		int dataPortNo;	
		int serverDataSock, clientSock, clientDataSock;
		//printf("server running on port %d\n", portNo);
		
		clientSock = connectSocket(serverSock);
		if(clientSock < 0) {
			perror("connecting");
			break;
		}

		
		// Get the command from the client
		if(read(clientSock,buffer, sizeof(buffer)) == -1) {
			perror("could not get command from client\n");
			exit(1);
		}

		printf("buffer contents %s\n", buffer);
		char *temp;
		temp = strtok(buffer, " \n\0");
		strcpy(com, temp);
		printf("com: %s", com);
		temp = strtok(NULL, " \n\0");
		strcpy(dataPortNoStr, temp);
		dataPortNo = atoi(temp);
		if(com[1] == 'g') {
			printf("parsing the filename\n");
			temp = (char*) strtok(NULL, " \n\0");
			strcpy(fileName, temp);
		}

		// Set up the data socket
		serverDataSock =  setSocket(dataPortNo);
		printf("5\n");
		clientDataSock = connectSocket(serverDataSock);

	
		// send confirmation messagge to client
		char *dirMessage = "OK";
		int writeStatus;
		writeStatus = write(clientSock, dirMessage, strlen(dirMessage));
		if(writeStatus < 0) {
			perror("sending confirmation message");
			exit(1);
		}

		


		// Call appropriate functions based on command
		if(com[1] == 'l') {
				printf("calling list command");
				listCommand(clientSock, clientDataSock);
		}
		else if(com[1] == 'g') {
			printf("calling get command");
			getCommand(clientSock, clientDataSock, fileName);
		}
		close(clientDataSock);
		close(clientSock);
		close(serverDataSock);
		
	}
	return 0;
}

int setSocket(int port) {
	printf("1\n");
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

	printf("2\n");
	if(setsockopt(socketNum, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
		perror("setsocketopt");
		exit(1);
	}

	printf("3\n");
	if(bind(socketNum, (struct sockaddr*) &sa, sizeof(struct sockaddr_in)) < 0) {
			printf("Error binding the socket\n");
			exit(1);
	}

	printf("listening on port: %d\n", port);
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
		return clientSocket;
	}

}


void listCommand(int clientSock, int clientDataSock) {


	printf("sending file");
	// Allocate memory for the message to to be sent
	char *send = (char*) malloc(1024 * sizeof(char));

	// http://pubs.opengroup.org/onlinepubs/009695399/functions/opendir.html
	DIR *dir;
	struct dirent *dp;

	// Get current directory 
	if((dir = opendir(".")) == NULL) {
		perror("error opening directory");
		exit(1);
	}

	// Get the contents of the current directory
	while((dp = readdir(dir)) != NULL) {
		strcat(send, dp->d_name);
		strcat(send, "\n");
	}
	closedir(dir);

	if(strlen(send) < 1) {
		strcpy(send, "The directory is empty");
	}

	if(write(clientDataSock, send, strlen(send)) < 0) {
		perror("error sending directory contents");
	}
	free(send);
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
		if(write(clientSock, fnf, strlen(fnf)) < 0) {
			perror("writing file send message");
		}
	}

	// If file was succesfully opened
	else {
		if(write(clientSock, ff, strlen(ff)) < 0) {
			perror("writing file send message");
			exit(1);
		}
	}

	int read;
	do {
		if((read = fread(buf, sizeof(buf), 1, fp)) < 0) {
			perror("reading from file");
		}
		if(write(clientDataSock, buf, read) < 0) {
			perror("writing to client");
		}
	} while(read > 0);
	fclose(fp);

}