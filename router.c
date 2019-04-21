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
#include <ifaddrs.h>

#define NODE_NUMBER 5
#define BUFFER_SIZE 1024
#define PORT_NUM 3155

typedef struct
{
	char* destinationIp;
	int destinationPort;
	char* nextIp;
	int nextPort;
	int metric;
}ROUTING_TABLE_ENTRY;

char receiveBuffer[BUFFER_SIZE];
char* sendBuffer;
int flag = 0;
char* destIp = "220.149.244.212";
char* nextIp = "220.149.244.213";

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
	char* ipAddress = nextIp;//(char *)arg;
	size_t getlineLength;
	struct sockaddr_in clientSocketAddress;
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);


	if(clientSocket == -1)
	{
		perror("Client Socket Creation Failure");
		pthread_exit(NULL);
	}

	//if(flag == 1) ipAddress = nextIp;

	memset(&clientSocketAddress, 0, sizeof(clientSocketAddress));
	clientSocketAddress.sin_family = AF_INET;
	clientSocketAddress.sin_port = htons(PORT_NUM);
	inet_pton(AF_INET, ipAddress, &clientSocketAddress.sin_addr);
	while(connect(clientSocket, (struct sockaddr *)&clientSocketAddress, sizeof(clientSocketAddress)) == -1);
//sending
	while(1)
	{
		sendBuffer = NULL;
		if(flag == 0){ getline(&sendBuffer, &getlineLength, stdin);
		}
		else strcpy(sendBuffer, receiveBuffer); 
		send(clientSocket, sendBuffer, BUFFER_SIZE, 0);
		if(strcmp(sendBuffer, "exit\n") == 0)
			break;
		free(sendBuffer);
	}
//sending
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
	flag = 0;
	while(1)
	{
		if(strcmp(getIpAddress(), destIp) != 0) flag = 1;
		recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
		printf("%s", receiveBuffer);
		if(strcmp(receiveBuffer, "exit\n") == 0)
			break;
		memset(receiveBuffer, 0, sizeof(receiveBuffer));
	}
//receive
	close(serverSocket);
	pthread_exit(NULL);
}

void main(int argc, char* args[])
{
	pthread_t clientThread;
	pthread_t serverThread;
	pthread_create(&clientThread, NULL, clientThreadFunction, nextIp);
	pthread_create(&serverThread, NULL, serverThreadFunction, NULL);
	pthread_join(clientThread, NULL);
	pthread_join(serverThread, NULL);
	return;
}
