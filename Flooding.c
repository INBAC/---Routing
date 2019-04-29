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

#define PORT_NUMBER 3667
#define IP_LENGTH 16

int number = 0;
int LSDB_NU = 0;
pthread_mutex_t lock;
pthread_cond_t cond;
int socket_OK = 0;
FILE * Final;

typedef struct
{
	char link_state_Id[IP_LENGTH];
	char link_Id[5][IP_LENGTH];
	int cost[5];
	int number;
}LSA;

typedef struct
{
	char Source[IP_LENGTH];
	char Dest[IP_LENGTH];
	int cost;
}LSDB;

LSA sendPacket;
LSA *receivePacket;
int receiveflag = 0;
LSDB *lsdb;

void sigpipe_handler()
{
    printf("SIGPIPE caught\n");
    socket_OK = 0;
}

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
	LSA routerPacket;
	memset(&routerPacket, 0, sizeof(LSA));
	struct sockaddr_in clientSocketAddress[number];
	int clientSocket[number];
	int connection[number];
	int End = 0;

	for(int i = 0; i < number; i++)
	{
		clientSocket[i] = socket(AF_INET, SOCK_STREAM, 0);
		connection[i] = 0;
		if(clientSocket[i] == -1)
		{
			perror("Client Socket Creation Failure");
			pthread_exit(NULL);
		}
		memset(&clientSocketAddress[i], 0, sizeof(clientSocketAddress[i]));
		clientSocketAddress[i].sin_family = AF_INET;
		clientSocketAddress[i].sin_port = htons(PORT_NUMBER);
		inet_pton(AF_INET, sendPacket.link_Id[i], &clientSocketAddress[i].sin_addr);
	}
	while(1)
	{
		for(int m=0; m < number; m++)
		{	if(connection[m] == 0)
			{
				while(connect(clientSocket[m], (struct sockaddr *)&clientSocketAddress[m], sizeof(clientSocketAddress[m])) == -1);
				connection[m] = 1;
			}
			send(clientSocket[m], &sendPacket, sizeof(LSA), 0);
		}
		pthread_mutex_lock(&lock);
		while(receiveflag == 0)
		{
			pthread_cond_wait(&cond, &lock);
		}
		strcpy(routerPacket.link_state_Id, receivePacket->link_state_Id);
		routerPacket.number = receivePacket->number;
		for(int j=0; j < receivePacket->number; j++)
		{
			strcpy(routerPacket.link_Id[j], receivePacket->link_Id[j]);
			routerPacket.cost[j] = receivePacket->cost[j];
		}
		for(int l = 0; l < number; l++)
		{
			if(connection[l] == 0)
			{
				while(connect(clientSocket[l], (struct sockaddr *)&clientSocketAddress[l], sizeof(clientSocketAddress[l])) == -1);
				connection[l] = 1;
			}
			send(clientSocket[l], &routerPacket, sizeof(LSA), 0);
		}
		receiveflag = 0;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock);

		if(End == 1000) break;
		End++;
	}
	//exit(0);
		
}

void *serverThreadFunction(void *arg)
{
	int clientSocket = *(int *)arg;
	char* receiveBuffer = (char *)malloc(sizeof(LSA));
	int i = 0;
	int happyFlag = 0;
	int MAX;
	receivePacket = (LSA *)malloc(sizeof(LSA));
	int End = 0;

	while(1)
	{
		MAX = LSDB_NU;
		happyFlag = 0;
		memset(receiveBuffer, 0, sizeof(LSA));
		recv(clientSocket, receiveBuffer, sizeof(LSA), 0);
		pthread_mutex_lock(&lock);
		receivePacket = (LSA*)receiveBuffer;
		receiveflag = 1;
		for(int j=0; j < MAX; j++)
		{
			if(strncmp(receivePacket->link_state_Id, lsdb[j].Source, 16) == 0)
			{
						happyFlag = 1;
			}
		}

		if(happyFlag == 0)
		{
			for(int n=0; n < receivePacket->number; n++)
			{
				strcpy(lsdb[LSDB_NU].Source, receivePacket->link_state_Id);
				strcpy(lsdb[LSDB_NU].Dest, receivePacket->link_Id[n]);
				lsdb[LSDB_NU].cost = receivePacket->cost[n];
				LSDB_NU++;
				//write file here
			}
		}
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock);
		printf("table\n");
		for(int k=0; k<LSDB_NU; k++)
		{
			printf("%s, %s, %d\n", lsdb[k].Source, lsdb[k].Dest, lsdb[k].cost);
		}
		if(End == 1000) break;
		End++;
	}
	for(int n=0; n < LSDB_NU; n++)
	{
		fprintf(Final, "%s %s %d\n", lsdb[n].Source, lsdb[n].Dest, lsdb[n].cost);
		//fwrite((char *)&lsdb[n], sizeof(LSDB), 1, Final);
	}
	exit(0);
}


void main(int argc, char* args[])
{
	FILE * fp;
	//FILE * Final;
	fp = fopen("router1.txt", "r");
	Final = fopen("LSDB.txt", "w");
	size_t len = 0;
	ssize_t read;
	char * line = NULL;
	char * result;
	int * ret;
	int rc;

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);

	memset(&sendPacket, 0, sizeof(LSA));
	lsdb = (LSDB *)malloc(sizeof(LSDB) * 100);

	while((read = getline(&line, &len, fp) != -1))
	{	
		result = strtok(line, " ");
		strcpy(sendPacket.link_Id[number], result);
		result = strtok(NULL, "\n\0");
		sendPacket.cost[number] = atoi(result);
		number++;
		sendPacket.number = number;
	}
	strcpy(sendPacket.link_state_Id, getIpAddress());

	for(int n=0; n < number; n++)
	{
		strcpy(lsdb[n].Source, sendPacket.link_state_Id);
		strcpy(lsdb[n].Dest, sendPacket.link_Id[n]);
		lsdb[n].cost = sendPacket.cost[n];
		LSDB_NU++;
	}
	int threadNumber = 0;
	pthread_t clientThread;
	pthread_t serverThread[number];
	pthread_create(&clientThread, NULL, clientThreadFunction, NULL);
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
	signal(SIGPIPE,sigpipe_handler);
	socket_OK=1;
	//int nSockOpt = 1;
	//setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &nSockOpt, sizeof(nSockOpt));
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

	/*for(int i=0; i < number; i++)
	{
			rc = pthread_join(serverThread[i], (void **) &ret);
	}
	rc = pthread_join(clientThread, (void **) &ret);

	for(int n=0; n < LSDB_NU; n++)
	{
		fwrite((char *)&lsdb[n], sizeof(LSDB), 1, Final);
	}*/
	fclose(fp);
	fclose(Final);
}
