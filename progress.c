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

#define MAX_NODE_NUMBER 10
#define BUFFER_SIZE 1024
#define PORT_NUMBER 3605
#define IP_LENGTH 16

typedef struct
{
	char sourceIp[IP_LENGTH];
	char neighborIp[MAX_NODE_NUMBER][IP_LENGTH];
	int cost[MAX_NODE_NUMBER];
	char mark;
}LSA;

typedef struct
{
	char sourceIp[IP_LENGTH];
	char neighborIp[IP_LENGTH];
	int cost;
	char mark;
}LSDB_ENTRY;

int neighborNodeNumber;
int lsdbEntryNumber = 0;
int receiveFlag = 0;
LSA myInfoPacket;
LSA neighborInfoPacket;
LSA* receivePacket;
LSDB_ENTRY* LSDB;

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

void initialize(char* fileName)
{
	int i = 0;
	FILE* fileptr = fopen(fileName, "r");
	char* buffer = NULL;
	size_t bufferSize;
	memset(&myInfoPacket, 0, sizeof(LSA));
	strcpy(myInfoPacket.sourceIp, getIpAddress());
	while(getline(&buffer, &bufferSize, fileptr) != -1)
	{
		strcpy(myInfoPacket.neighborIp[i], strtok(buffer, " "));
		myInfoPacket.cost[i++] = atoi(strtok(NULL, "\n"));
	}
	neighborNodeNumber = i;
	myInfoPacket.mark = 's';
	LSDB = (LSDB_ENTRY *)malloc(sizeof(LSDB_ENTRY) * ((MAX_NODE_NUMBER * (MAX_NODE_NUMBER - 1))));
	for(i = 0; i < neighborNodeNumber; i++)
	{
		strcpy(LSDB[i].sourceIp, myInfoPacket.sourceIp);
		strcpy(LSDB[i].neighborIp, myInfoPacket.neighborIp[i]);
		LSDB[i].cost = myInfoPacket.cost[i];
		LSDB[i].mark = 's';
		lsdbEntryNumber++;
	}
	fclose(fileptr);
	return;
}

void *serverThreadFunction(void *arg)
{
	int i;
	int duplicateIpCheck;
	int clientSocket = *(int *)arg;
	char* receiveBuffer = (char *)malloc(sizeof(LSA));
	while(1)
	{
		recv(clientSocket, receiveBuffer, sizeof(LSA), 0);
		while(receiveFlag == 1);
		receivePacket = (LSA*)receiveBuffer;
		receiveFlag = 1;
		duplicateIpCheck = 1;
		for(i = 0; i < lsdbEntryNumber; i++)
		{
			if(strcmp(receivePacket->sourceIp, LSDB[i].sourceIp) == 0)
			{
				duplicateIpCheck = 0;
				break;
			}
		}
		if(duplicateIpCheck == 1)
		{
			for(i = 0; strlen(receivePacket->neighborIp[i]) > 0; i++)
			{
				strcpy(LSDB[lsdbEntryNumber + i].sourceIp, receivePacket->sourceIp);
				strcpy(LSDB[lsdbEntryNumber + i].neighborIp, receivePacket->neighborIp[i]);
				LSDB[lsdbEntryNumber + i].cost = receivePacket->cost[i];
				LSDB[lsdbEntryNumber + i].mark = receivePacket->mark;
			}
			lsdbEntryNumber = lsdbEntryNumber + i;
		}
		for(i = 0; i < lsdbEntryNumber; i++)
		{
			printf("%c %s -> %s :::: %d\n", LSDB[i].mark, LSDB[i].sourceIp, LSDB[i].neighborIp, LSDB[i].cost);
		}
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}
}

void *clientThreadFunction(void *arg)
{
	int i;
	struct sockaddr_in clientSocketAddress[neighborNodeNumber];
	int clientSocket[neighborNodeNumber];
	for(i = 0; i < neighborNodeNumber; i++)
	{
		clientSocket[i] = socket(AF_INET, SOCK_STREAM, 0);
		if(clientSocket[i] == -1)
		{
			perror("Client Socket Creation Failure");
			pthread_exit(NULL);
		}
		memset(&clientSocketAddress[i], 0, sizeof(clientSocketAddress[i]));
		clientSocketAddress[i].sin_family = AF_INET;
		clientSocketAddress[i].sin_port = htons(PORT_NUMBER);
		inet_pton(AF_INET, myInfoPacket.neighborIp[i], &clientSocketAddress[i].sin_addr);
		while(connect(clientSocket[i], (struct sockaddr *)&clientSocketAddress[i], sizeof(clientSocketAddress[i])) == -1);
	}
	while(1)
	{
		for(i = 0; i < neighborNodeNumber; i++)
		{
			myInfoPacket.mark = 'd';
			send(clientSocket[i], &myInfoPacket, sizeof(LSA), 0);
		}
	}
}

void *routerThreadFunction(void *arg)
{
	int i;
	struct sockaddr_in clientSocketAddress[neighborNodeNumber];
	int clientSocket[neighborNodeNumber];
	for(i = 0; i < neighborNodeNumber; i++)
	{
		clientSocket[i] = socket(AF_INET, SOCK_STREAM, 0);
		if(clientSocket[i] == -1)
		{
			perror("Client Socket Creation Failure");
			pthread_exit(NULL);
		}
		memset(&clientSocketAddress[i], 0, sizeof(clientSocketAddress[i]));
		clientSocketAddress[i].sin_family = AF_INET;
		clientSocketAddress[i].sin_port = htons(PORT_NUMBER);
		inet_pton(AF_INET, myInfoPacket.neighborIp[i], &clientSocketAddress[i].sin_addr);
		while(connect(clientSocket[i], (struct sockaddr *)&clientSocketAddress[i], sizeof(clientSocketAddress[i])) == -1);
	}
	while(1)
	{
		while(receiveFlag == 0);
		strcpy(neighborInfoPacket.sourceIp, receivePacket->sourceIp);
		for(i = 0; strlen(receivePacket->neighborIp[i]) > 0; i++)
		{
			strcpy(neighborInfoPacket.neighborIp[i], receivePacket->neighborIp[i]);
			neighborInfoPacket.cost[i] = receivePacket->cost[i];
			neighborInfoPacket.mark = 'i';
		}
		for(i = 0; i < neighborNodeNumber; i++)
		{
			send(clientSocket[i], &neighborInfoPacket, sizeof(LSA), 0);
		}
		receiveFlag = 0;
	}
}

void startThreads()
{
	int threadNumber = 0;
	pthread_t clientThread;
	pthread_t routerThread;
	pthread_t serverThread[MAX_NODE_NUMBER * 2];
	pthread_create(&clientThread, NULL, clientThreadFunction, NULL);
	pthread_create(&routerThread, NULL, routerThreadFunction, NULL);
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
	while(1)
	{
		if(listen(serverSocket, 0) == -1)
		{
			perror("Server Listen Standby Mode Failure");
			close(serverSocket);
			exit(0);
		}
		clientSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
		if(clientSocket == -1)
		{
			perror("Server Connect Accept Failure");
			close(serverSocket);
			exit(0);
		}
		pthread_create(&serverThread[threadNumber++], NULL, serverThreadFunction, &clientSocket);
	}
}

void main(int argc, char* args[])
{
	if(argc < 2)
	{
		printf("Please Enter The File\n");
		exit(0);
	}
	initialize(args[1]);
	startThreads();
}
