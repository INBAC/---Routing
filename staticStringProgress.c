/*
	Written By: Jongwon Lee
	Purpose: To simulate routing table protocol
	useful functions (socket, pthread, sigaction, timer, open, close, client, recv, bind, accept, listen, connect, memset, htons/l, ntohs/l, memset, malloc/free)
*/
#include<stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>

#define NODE_NUMBER 5
#define BUFFER_SIZE 1024
#define PORT_NUM 3605

typedef struct
{
	char* destinationIp;
	int destinationPort;
	char* nextIp;
	int nextPort;
	int metric;
}ROUTING_TABLE_ENTRY;

void *clientThreadFunction(void *arg)
{
	char* ipAddress = (char *)arg;
	char* sendBuffer;
	int sendBufferLength;
	struct sockaddr_in clientSocketAddress;
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket == -1)
	{
		perror("Client Socket Creation Failure");
		pthread_exit(NULL);
	}
	memset(&clientSocketAddress, 0, sizeof(clientSocketAddress));
	clientSocketAddress.sin_family = AF_INET;
	clientSocketAddress.sin_port = htons(PORT_NUM);
	inet_pton(AF_INET, ipAddress, &clientSocketAddress.sin_addr);
	printf("Client Waiting For Connection...\n");
	while(connect(clientSocket, (struct sockaddr *)&clientSocketAddress, sizeof(clientSocketAddress)) == -1);
//sending
	sendBuffer = "Im from Test2";
	sendBufferLength = strlen(sendBuffer);
	send(clientSocket, sendBuffer, BUFFER_SIZE, 0);
	printf("Client Sent\n");
//sending
	close(clientSocket);
	pthread_exit(NULL);
}

void *serverThreadFunction(void *arg)
{
	char receiveBuffer[BUFFER_SIZE];
	int receiveBufferLength;
	struct sockaddr_in serverSocketAddress;
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	int clientSocket;
	if(serverSocket == -1)
	{
		perror("Server Socket Creation Failure");
		pthread_exit(NULL);
	}
	memset(&serverSocketAddress, 0, sizeof(serverSocketAddress));
	serverSocketAddress.sin_family = AF_INET;
	serverSocketAddress.sin_addr.s_addr = htons(INADDR_ANY);
	serverSocketAddress.sin_port = htons(PORT_NUM);
	if(bind(serverSocket, (struct sockaddr *)&serverSocketAddress, sizeof(serverSocketAddress)) == -1)
	{
		perror("Server Bind Failure");
		close(serverSocket);
		pthread_exit(NULL);
	}
	if(listen(serverSocket, 0) == -1)
	{
		perror("Server Listen Standby Mode Failure");
		close(serverSocket);
		pthread_exit(NULL);
	}
	clientSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
	if(clientSocket == -1)
	{
		perror("Server Connect Accept Failure");
		close(serverSocket);
		pthread_exit(NULL);
	}
//receive
	recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
	printf("%s\n", receiveBuffer);
//receive
	close(serverSocket);
	pthread_exit(NULL);
}

void main(int argc, char* args[])
{
	pthread_t clientThread;
	pthread_t serverThread;
	pthread_create(&clientThread, NULL, clientThreadFunction, args[1]);
	pthread_create(&serverThread, NULL, serverThreadFunction, NULL);
	pthread_join(clientThread, NULL);
	pthread_join(serverThread, NULL);
	return;
}
