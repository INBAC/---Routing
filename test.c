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

int srv(){
	printf("recieve\n");
}

int cli(){
	while(1){
		printf("send\n");
		sleep(1);
	}
}

int main(int argc, char* argv[]){

  pthread_t sender;
  int threadId;
  int status;

  threadId = pthread_create(&sender, NULL, cli, NULL);

  if(threadId < 0){
	perror("thread create error : ");
	exit(0);
  }

  while(1){
	srv();
	sleep(1);
  }
  
  return 0;
}
