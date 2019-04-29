/*
	Edit By: Dokyung Lee
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
#include <ifaddrs.h>

#define NODE_NUMBER 5
#define BUFFER_SIZE 1024
#define PORT_NUMBER 3155
#define IP_LENGTH 16

#define ROUTER 1
#define CLIENT 0
#define SERVER 0

typedef struct
{
	char* destinationIp;
	int destinationPort;
	char* nextIp;
	int nextPort;
	int metric;
}ROUTING_TABLE_ENTRY;

typedef struct
{	
	char sourceIp[IP_LENGTH];
	char destinationIp[IP_LENGTH];
	char message[BUFFER_SIZE];
}PACKET;

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
char nextHopIp[NODE_NUMBER][IP_LENGTH];
char destinationIp[NODE_NUMBER][IP_LENGTH];

PACKET sendPacket;
PACKET* receivePacket;
int flag = 0;
char* sendBuffer;
char* receiveBuffer;
size_t ipLength = IP_LENGTH;
size_t getlineLength;
struct sockaddr_in clientSocketAddress[NODE_NUMBER];
int clientSocket[NODE_NUMBER];
int connectionCheck[NODE_NUMBER] = {0};
char destination[16];

void* routerThreadFunction(void* arg){
	int serverNumber = 0;
	while(1){
		while(flag != ROUTER);
		for(int i = 0; i < NODE_NUMBER - 1; i++){
			if(strcmp(destinationIp[i], receivePacket->destinationIp) == 0){
				serverNumber = i;
				break;
			}
		}
		if(connectionCheck[serverNumber] == 0){
			while(connect(clientSocket[serverNumber], (struct sockaddr *)&clientSocketAddress[serverNumber], sizeof(clientSocketAddress[serverNumber])) == -1);
		}

		if(connectionCheck[serverNumber] == 0) connectionCheck[serverNumber] = 1;

		send(clientSocket[serverNumber], receivePacket, sizeof(PACKET), 0);
		printf("ROUTING packet from %s :: %s -> %s\n", receivePacket->sourceIp, getIpAddress(), nextHopIp[serverNumber]);
		flag = SERVER;
	}

}

void *clientThreadFunction(void *arg)
{
	int serverNumber;	
	for(int i = 0; i < NODE_NUMBER - 1; i++){ 

		clientSocket[i] = socket(AF_INET, SOCK_STREAM, 0);

		if(clientSocket[i] == -1)
		{
			perror("Client Socket Creation Failure");
			pthread_exit(NULL);
		}

		memset(&clientSocketAddress[i], 0, sizeof(clientSocketAddress[i]));
		clientSocketAddress[i].sin_family = AF_INET;
		clientSocketAddress[i].sin_port = htons(PORT_NUMBER);
		inet_pton(AF_INET, nextHopIp[i], &clientSocketAddress[i].sin_addr);
	}
 
	
	//sending
	while(1)
	{	
		gets(destination);

		for(int i = 0; i < NODE_NUMBER - 1; i++){
			if(strcmp(destinationIp[i], destination) == 0){
				strcpy(sendPacket.destinationIp, destination);
				strcpy(sendPacket.sourceIp, getIpAddress());
				serverNumber = i;
				break;
			}
		}

		if(connectionCheck[serverNumber] == 0){
			while(connect(clientSocket[serverNumber], (struct sockaddr *)&clientSocketAddress[serverNumber], sizeof(clientSocketAddress[serverNumber])) == -1);
		}

		if(connectionCheck[serverNumber] == 0) connectionCheck[serverNumber] = 1;
		gets(sendPacket.message);

		send(clientSocket[serverNumber], &sendPacket, sizeof(PACKET), 0);
		printf("CLIENT packet sent to %s :: %s\n", sendPacket.destinationIp, sendPacket.message);
	}
	//sending

	for(int i = 0; i < NODE_NUMBER; i++)close(clientSocket[i]);
	pthread_exit(NULL);
}

void *serverThreadFunction(void *arg)
{
	receiveBuffer = (char *)malloc(sizeof(PACKET));
	int clientSocket = *(int*)arg;
//receive
	
	flag = SERVER;

	while(1)
	{	
		recv(clientSocket, receiveBuffer, sizeof(PACKET), 0);
		receivePacket = (PACKET*)receiveBuffer;

		if(strcmp(receivePacket->destinationIp, getIpAddress()) != 0){
			flag = ROUTER;
		}
		if(flag == SERVER){
			printf("SERVER received packet from %s :: %s\n", receivePacket->sourceIp, receivePacket->message);
		}
	}
	pthread_exit(NULL);
}

void main(int argc, char* args[])
{
	char* line = NULL;
	FILE* fp = fopen("FIB.txt", "r");
	int threadNumber = 1;	
	pthread_t clientThread;
	pthread_t serverThread[NODE_NUMBER];
	pthread_t routerThread;
	struct sockaddr_in serverSocketAddress;
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	int clientSocket;
	size_t len = 0;
	int i = 0;
	char* temp;

	while(getline(&line, &len, fp) != -1){
		temp = strtok(line, " ");
		strcpy(destinationIp[i], temp);
		strcpy(nextHopIp[i], strtok(NULL, "\n"));
		i++;		
	}

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
	pthread_create(&clientThread, NULL, clientThreadFunction, args[1]);
	pthread_create(&routerThread, NULL, routerThreadFunction, NULL);
	while(1){
	
		if(listen(serverSocket, 0) == -1){
			perror("Server Listen Standby Mode Failure");
			close(serverSocket);
			pthread_exit(NULL);
		}
	
		clientSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
		if(clientSocket == -1){
			perror("Server Connect Accept Failure");
			close(serverSocket);
			pthread_exit(NULL);
		}
		pthread_create(&serverThread[threadNumber++], NULL, serverThreadFunction, &clientSocket);
	
	}
	return;
}
