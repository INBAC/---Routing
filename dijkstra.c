/*
 * Please Copy
 * dijkstra()
 * printPath()
 * writeToFile()
 * #define INT_MAX_VALUE
 * FILE* stream
 * */

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

#define IP_LENGTH 16
#define INT_MAX_VALUE 2147483647

typedef struct
{
	char Source[IP_LENGTH];
	char Dest[IP_LENGTH];
	int cost;
}LSDB;

LSDB *lsdb;
int LSDB_NU = 0;
FILE* stream;

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
void writeToFile(int input, int type)
{
	char ip[IP_LENGTH + 1];
	char vertex = '0' + ++input;
	strcpy(ip, "220.149.244.21");
	ip[14] = vertex;
	if(type == 0)
	{
		ip[15] = ' ';
		ip[16] = '\0';
	}
	else
	{
		ip[15] = '\n';
		ip[16] = '\0';
	}
	fprintf(stream, "%s", ip);
    return;
}

void printPath(int currentVertex, int* parents, int* pathCheck)
{
	if (currentVertex == -1)
		return;
	printPath(parents[currentVertex], parents, pathCheck);
	printf("%d ", currentVertex + 1);
	(*pathCheck)++;
	if(*pathCheck == 2)
		writeToFile(currentVertex, 1);
}

void dijkstra()
{
	stream = fopen("./FIB.txt", "w");
	int i;
	int j;
	int adjacencyMatrix[100 / 2][100 / 2]; //change the number of vertices sometimes later!!!!
	int startVertex;
	char startIp[IP_LENGTH];
	int Source, Dest, cost;
	int nVertices = 0;
	int duplicateCheck[100] = {0};
	for(i = 0; i < LSDB_NU; i++)
	{
		Source = atoi(lsdb[i].Source + 12);
		Dest = atoi(lsdb[i].Dest + 12);
		cost = lsdb[i].cost;
		if(duplicateCheck[Source] == 0)
			nVertices++;
		if(duplicateCheck[Dest] == 0)
			nVertices++;
		duplicateCheck[Source] = 1;
		duplicateCheck[Dest] = 1;
	}
	for(i = 0; i < nVertices; i++)
		for(j = 0; j < nVertices; j++)
			if(i != j)
				adjacencyMatrix[i][j] = -1;
	for(i = 0; i < LSDB_NU; i++)
	{
		Source = atoi(lsdb[i].Source + 12);
		Dest = atoi(lsdb[i].Dest + 12);
		cost = lsdb[i].cost;
		Source = (Source % 10) - 1;
		Dest = (Dest % 10) - 1;
		adjacencyMatrix[Source][Dest] = cost;
	}
	strcpy(startIp, getIpAddress());
	startVertex = atoi(startIp + 12);
	startVertex = (startVertex % 10) - 1;
	int* shortestDistances = (int *)malloc(nVertices * sizeof(int));
	memset(shortestDistances, 0, nVertices * sizeof(int));
	int* added = (int *)malloc(nVertices * sizeof(int));;
	memset(added, 0, nVertices * sizeof(int));
	int* parents = (int *)malloc(nVertices * sizeof(int));;
	memset(parents, 0, nVertices * sizeof(int));
	int nearestVertex;
	int shortestDistance;
	int vertexIndex;
	int edgeDistance;
	int pathCheck;
	for(i = 0; i < nVertices; i++)
	{
		shortestDistances[i] = INT_MAX_VALUE;
		added[i] = 0;
	}
	shortestDistances[startVertex] = 0;
	parents[startVertex] = -1;
	for (i = 1; i < nVertices; i++)
	{
		nearestVertex = -1;
		shortestDistance = INT_MAX_VALUE;
		for (vertexIndex = 0; vertexIndex < nVertices; vertexIndex++)
		{
			if(added[vertexIndex] == 0 && shortestDistances[vertexIndex] < shortestDistance)
			{
				nearestVertex = vertexIndex;
				shortestDistance = shortestDistances[vertexIndex];
			}
		}
		added[nearestVertex] = 1;
		for(vertexIndex = 0; vertexIndex < nVertices; vertexIndex++)
		{
			edgeDistance = adjacencyMatrix[nearestVertex][vertexIndex];
			if(edgeDistance > 0  && ((shortestDistance + edgeDistance) <  shortestDistances[vertexIndex]))
			{
				parents[vertexIndex] = nearestVertex;
				shortestDistances[vertexIndex] = shortestDistance + edgeDistance;
			}
		}
	}
	for (vertexIndex = 0; vertexIndex < nVertices; vertexIndex++)
	{
		if (vertexIndex != startVertex)
		{
			printf("%d -> ", startVertex + 1);
			printf("%d\t\t", vertexIndex + 1);
			writeToFile(vertexIndex, 0);
			printf("%d\t\t", shortestDistances[vertexIndex]);
			pathCheck = 0;
			printPath(vertexIndex, parents, &pathCheck);
			printf("\n");
		}
	}
	fclose(stream);
	return;
}

void main()
{
	int i;
	lsdb = (LSDB *)malloc(sizeof(LSDB) * 100);

	strcpy(lsdb[0].Source, "220.149.244.211");
	strcpy(lsdb[0].Dest, "220.149.244.212");
	lsdb[0].cost = 1;

	strcpy(lsdb[1].Source, "220.149.244.212");
	strcpy(lsdb[1].Dest, "220.149.244.211");
	lsdb[1].cost = 1;

	strcpy(lsdb[2].Source, "220.149.244.211");
	strcpy(lsdb[2].Dest, "220.149.244.213");
	lsdb[2].cost = 2;

	strcpy(lsdb[3].Source, "220.149.244.213");
	strcpy(lsdb[3].Dest, "220.149.244.211");
	lsdb[3].cost = 2;

	strcpy(lsdb[4].Source, "220.149.244.213");
	strcpy(lsdb[4].Dest, "220.149.244.215");
	lsdb[4].cost = 3;

	strcpy(lsdb[5].Source, "220.149.244.215");
	strcpy(lsdb[5].Dest, "220.149.244.213");
	lsdb[5].cost = 3;

	strcpy(lsdb[6].Source, "220.149.244.212");
	strcpy(lsdb[6].Dest, "220.149.244.214");
	lsdb[6].cost = 4;

	strcpy(lsdb[7].Source, "220.149.244.214");
	strcpy(lsdb[7].Dest, "220.149.244.212");
	lsdb[7].cost = 4;

	strcpy(lsdb[8].Source, "220.149.244.212");
	strcpy(lsdb[8].Dest, "220.149.244.215");
	lsdb[8].cost = 5;

	strcpy(lsdb[9].Source, "220.149.244.215");
	strcpy(lsdb[9].Dest, "220.149.244.212");
	lsdb[9].cost = 5;

	strcpy(lsdb[10].Source, "220.149.244.213");
	strcpy(lsdb[10].Dest, "220.149.244.214");
	lsdb[10].cost = 6;

	strcpy(lsdb[11].Source, "220.149.244.214");
	strcpy(lsdb[11].Dest, "220.149.244.213");
	lsdb[11].cost = 6;

	strcpy(lsdb[12].Source, "220.149.244.212");
	strcpy(lsdb[12].Dest, "220.149.244.211");
	lsdb[12].cost = 1;

	strcpy(lsdb[13].Source, "220.149.244.211");
	strcpy(lsdb[13].Dest, "220.149.244.213");
	lsdb[13].cost = 2;

	strcpy(lsdb[14].Source, "220.149.244.213");
	strcpy(lsdb[14].Dest, "220.149.244.211");
	lsdb[14].cost = 2;

	strcpy(lsdb[15].Source, "220.149.244.213");
	strcpy(lsdb[15].Dest, "220.149.244.215");
	lsdb[15].cost = 3;

	LSDB_NU = 16;
	dijkstra();
	return;
}
