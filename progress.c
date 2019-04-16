/*
	Written By: Jongwon Lee
	Purpose: To simulate routing table protocol
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

typedef struct ROUTING_TABLE_ENTRY
{
	char* dstip;
	int dstport;
	char* nextip;
	int nextport;
	int metric;
};

