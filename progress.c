/*
	Written By: Jongwon Lee
	Purpose: To simulate routing table protocol
	useful functions (socket, pthread, sigaction, timer, open, close, client, recv, bind, accept, listen, connect, memset, htons/l, ntohs/l, memset, malloc/free)
*/
#include <stdio.h>
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
#include <ifaddrs.h>

#define NODE_NUMBER 5
#define BUFFER_SIZE 1024
#define PORT_NUMBER 3605

typedef struct
{
	char* destinationIp;
	int destinationPort;
	char* nextIp;
	int nextPort;
	int metric;
}ROUTING_TABLE_ENTRY;

char receiveBuffer[BUFFER_SIZE];
char* destinationIp;

char* getIpAddress()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;
    getifaddrs (&ifap);
    for(ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family==AF_INET)
        {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
        }
    }
    freeifaddrs(ifap);
    return addr;
}

void *clientThreadFunction(void *arg)
{
	char* nextIpAddress = (char *)arg;
	char* sendBuffer;
	size_t getlineLength;
	struct sockaddr_in clientSocketAddress;
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket == -1)
	{
		perror("Client Socket Creation Failure");
		pthread_exit(NULL);
	}
	memset(&clientSocketAddress, 0, sizeof(clientSocketAddress));
	clientSocketAddress.sin_family = AF_INET;
	clientSocketAddress.sin_port = htons(PORT_NUMBER);
	inet_pton(AF_INET, nextIpAddress, &clientSocketAddress.sin_addr);
	while(connect(clientSocket, (struct sockaddr *)&clientSocketAddress, sizeof(clientSocketAddress)) == -1);
	if(destinationIp == NULL)
	{
		while(1)
		{
			while(strlen(receiveBuffer) == 0);
			sendBuffer = NULL;
			send(clientSocket, sendBuffer, BUFFER_SIZE, 0);
			if(strcmp(sendBuffer, "exit\n") == 0)
				break;
			free(sendBuffer);
		}
	}
	else
	{
		while(1)
		{
			sendBuffer = NULL;
			getline(&sendBuffer, &getlineLength, stdin);
			send(clientSocket, sendBuffer, BUFFER_SIZE, 0);
			if(strcmp(sendBuffer, "exit\n") == 0)
				break;
			free(sendBuffer);
		}
	}
	close(clientSocket);
	pthread_exit(NULL);
}

void *serverThreadFunction(void *arg)
{
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
	serverSocketAddress.sin_port = htons(PORT_NUMBER);
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
	if(destinationIp == NULL)
	{
		while(1)
		{
			recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
			printf("%s", receiveBuffer);
			if(strcmp(receiveBuffer, "exit\n") == 0)
				break;
			memset(receiveBuffer, 0, sizeof(receiveBuffer));
		}
	}
	else
	{
		while(1)
		{
			recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
			printf("%s", receiveBuffer);
			if(strcmp(receiveBuffer, "exit\n") == 0)
				break;
			memset(receiveBuffer, 0, sizeof(receiveBuffer));
		}
	}
	close(serverSocket);
	pthread_exit(NULL);
}

void main(int argc, char* args[])
{
	int i;
	char* input = NULL;
	destinationIp = NULL;
	ssize_t inputSize;
	pthread_t clientThread[NODE_NUMBER - 1];
	pthread_t serverThread[NODE_NUMBER - 1];
	printf("Please enter 'r' for router or destination IP address: ");
	getline(&input, &inputSize, stdin);
	if(strcmp(input, "r\n") == 0)
	{
		for(i = 0; i < argc - 1; i ++)
		{
			pthread_create(&clientThread[i], NULL, clientThreadFunction, args[i]);
			pthread_create(&serverThread[i], NULL, serverThreadFunction, NULL);
		}
		for(i = 0; i < argc - 1; i ++)
		{
			pthread_join(clientThread[i], NULL);
			pthread_join(serverThread[i], NULL);
		}
		return;
	}
	else
	{
		destinationIp = strtok(input, "\n");
		for(i = 0; i < argc - 1; i ++)
		{
			pthread_create(&clientThread[i], NULL, clientThreadFunction, args[i]);
			pthread_create(&serverThread[i], NULL, serverThreadFunction, NULL);
		}
		for(i = 0; i < argc - 1; i ++)
		{
			pthread_join(clientThread[i], NULL);
			pthread_join(serverThread[i], NULL);
		}
		return;
	}
}
