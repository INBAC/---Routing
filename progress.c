/*
	Written By: Jongwon Lee
	Purpose: To simulate routing table protocol
	useful functions (socket, pthread, sigaction, timer, open, close, send, recv, bind, accept, listen, connect, memset, htons/l, ntohs/l, memset, malloc/free)
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
#define BUFFER_SIZE 8

typedef struct
{
	char* destinationIp;
	int destinationPort;
	char* nextIp;
	int nextPort;
	int metric;
}ROUTING_TABLE_ENTRY;

void *sendThreadFunction(void *args)
{
	pthread_exit(NULL);
}

void *receiveThreadFunction(void *args)
{
	pthread_exit(NULL);
}

void main(int argc, char* args[])
{
	pthread_t sendThread;
	pthread_t receiveThread;
	pthread_create(&sendThread, NULL, sendThreadFunction, NULL);
	pthread_create(&receiveThread, NULL, receiveThreadFunction, NULL);

	pthread_join(sendThread, NULL);
	pthread_join(receiveThread, NULL);
	return;
}
