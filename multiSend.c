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
#define CLIENT_SERVER 0

typedef struct
{
	char* destinationIp;
	int destinationPort;
	char* nextIp;
	int nextPort;
	int metric;
}ROUTING_TABLE_ENTRY;

typedef struct
{	//char sourceIp[IP_LENGTH];
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
const char address[NODE_NUMBER][16]={"220.149.244.211", "220.149.244.212", "220.149.244.213", "220.149.244.214", "220.149.244.215"};
//const int port[5] = {3155, 3156, 3157, 3158, 3159};
char* nextIp; 
PACKET sendPacket;
int flag = 0;


void *clientThreadFunction(void *arg)
{
	//char* ipAddress = nextIp;//(char *)arg;
	char* sendBuffer;
	PACKET sendPacket;
	size_t getlineLength;
	struct sockaddr_in clientSocketAddress[NODE_NUMBER];
	int clientSocket[NODE_NUMBER];
	int connectionCheck[NODE_NUMBER] = {0};

	for(int i = 0; i < NODE_NUMBER; i++){ 

		clientSocket[i] = socket(AF_INET, SOCK_STREAM, 0);

		if(clientSocket[i] == -1)
		{
			perror("Client Socket Creation Failure");
			pthread_exit(NULL);
		}

		memset(&clientSocketAddress[i], 0, sizeof(clientSocketAddress[i]));
		clientSocketAddress[i].sin_family = AF_INET;
		clientSocketAddress[i].sin_port = htons(PORT_NUMBER);
		inet_pton(AF_INET, address[i], &clientSocketAddress[i].sin_addr);
	}
 
	int num =3;
	//sending
	while(1)
	{
		printf("server num : %s\n", address[num]);
	
		if(connectionCheck[num] == 0){
			while(connect(clientSocket[num], (struct sockaddr *)&clientSocketAddress[num], sizeof(clientSocketAddress[num])) == -1);
		}

		if(connectionCheck[num] == 0) connectionCheck[num] = 1;

		//if(num == 2) num=3;
		//else num = 2;	
		//memset(&sendBuffer, 0, sizeof(PACKET));
		sendBuffer = NULL;
	
		//if(flag == ROUTER){

		//}else{
		printf("message : ");
		getline(&sendBuffer, &getlineLength, stdin);
	
		//}
		//strcpy(sendPacket.message, sendBuffer);
		//strcpy(sendPacket.destinationIp, address[num]);
	
		if(flag != ROUTER){
			//strcpy(sendPacket.sourceIp, getIpAddress());
		}
		send(clientSocket[num], sendBuffer, 1024, 0);
		printf("send : %s", sendBuffer);
		
		if(strcmp(sendBuffer, "exit\n") == 0)
			break;
		if(num == 3) num = 4;
		else if(num == 4)num = 3;
	}
//sending
	for(int i = 0; i < NODE_NUMBER; i++)close(clientSocket[i]);
	pthread_exit(NULL);
}

void *serverThreadFunction(void *arg)
{
	char* receiveBuffer = (char *)malloc(sizeof(1024));
	PACKET* receivePacket;
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
//receive
	//memset(&receivePacket, 0, sizeof(PACKET));
	while(1)
	{
//		if(strcmp(receivePacket.destinationIp, getIpAddress()) != 0){
//			strcpy(sendPacket.message, receivePacket.message);
//			//memcpy(sendPacket, receivePacket, sizeof(PACKET));
//		
//			flag = ROUTER;
//			nextIp = receivePacket.destinationIp;
//		}
		//memset(receiveBuffer, 0, 1024);
		recv(clientSocket, receiveBuffer, 1024, 0);

		//receivePacket = (PACKET*)receiveBuffer;
	
		//if(strcmp(receivePacket->destinationIp, getIpAddress()) != 0){
		//	flag = ROUTER;
		//	nextIp = receivePacket->destinationIp;
		//	strcpy(sendPacket.message, receivePacket->message);
		//}
		printf("receive : %s", receiveBuffer);
		if(strcmp(receiveBuffer, "exit\n") == 0)
			break;
	}
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
