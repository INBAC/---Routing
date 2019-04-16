#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>

pthread_t tids[100];
char *buffer;
char r_buffer[1024];
int thds;

int pid;
static void * handle(void *);
int srv()//int argc, char *argv[])
{
	int srv_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr1;
	int len;

	// arg parsing
	/*if (argc != 2) {
		printf("usage: srv port\n");
		return 0;
	}*/
	port_num = 3214;//atoi(argv[1]);

	// socket creation
	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1) {
		perror("Server socket CREATE fail!!");
		return 0;
	}

	// addr binding
	memset(&addr1, 0, sizeof(addr1));
	addr1.sin_family = AF_INET;
	addr1.sin_addr.s_addr = htons (INADDR_ANY); // 32bit IPV4 addr that not use static IP addr
	addr1.sin_port = htons (port_num); // using port num
	
	ret = bind (srv_sock, (struct sockaddr *)&addr1, sizeof(addr1));
	
	if (ret == -1) {
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}

	for (;;) {
	// Listen part
	ret = listen(srv_sock, 0);

	if (ret == -1) {
		perror("LISTEN stanby mode fail");
		close(srv_sock);
		return 0;
	}

	// Accept part ( create new client socket for communicate to client ! )
	cli_sock = accept(srv_sock, (struct sockaddr *)NULL, NULL); // client socket
	if (cli_sock == -1) {
		perror("cli_sock connect ACCEPT fail");
		close(srv_sock);
	}
	thds++;
	// cli handler
	pthread_create(&tids[thds], NULL, handle, &cli_sock);
	} // end for
	return 0;
}

static void * handle(void * arg)
{
	int cli_sockfd = *(int *)arg;
	int ret = -1;
	char *recv_buffer = (char *)malloc(1024);
	char *send_buffer = (char *)malloc(1024);
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
           
	/* get peer addr */
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_sockfd, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len, 
		hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), 
		NI_NUMERICHOST | NI_NUMERICSERV); 

	if (ret != 0) {
		ret = -1;
		pthread_exit(&ret);
	}
	/* read from client host:port */

	while (1) {
	int len = 0;

	printf("from client ----\n");
	memset(recv_buffer, 0, 1024);//sizeof(recv_buffer));
	len = recv(cli_sockfd, recv_buffer, sizeof(recv_buffer), 0);
	if (len == 0) continue;
	printf("%s\n len:%d\n", recv_buffer, len);
	memset(send_buffer, 0, 1024);//sizeof(send_buffer));
	sprintf(send_buffer, "[%s:%s]%s len:%d\n", 
				hbuf, sbuf, recv_buffer, len);
	len = strlen(send_buffer);

	ret = send(cli_sockfd, send_buffer, len, 0);
	if (ret == -1) break;
	printf("----\n");
	fflush(NULL);

	}
	close(cli_sockfd);
	ret = 0;
	pthread_exit(&ret);
}

int cli(char* argv)
{
        int fd_sock, cli_sock;
        int port_num, ret;
        struct sockaddr_in addr;
        int len;
        size_t getline_len;

        // arg parsing
        /*if (argc != 3) {
                printf("usage: cli srv_ip_addr port\n");
                return 0;
        }*/
        port_num = 3214;//atoi(argv[2]);

        // socket creation
        fd_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_sock == -1) {
                perror("socket");
                return 0;
        }

        // addr binding, and connect
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons (port_num);
        inet_pton(AF_INET, argv, &addr.sin_addr);
	while(1){
        ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1) {
                perror("connect");
                //close(fd_sock);
                //return 0;
        }
	else break;
	}

        while (1) {
                buffer = NULL;
                printf("send$ ");
                ret = getline(&buffer, &getline_len, stdin);
                if (ret == -1) { // EOF
                        perror("getline");
                        close(fd_sock);
                        break;
                }
                len = strlen(buffer);
                if (len == 0) {
                        free(buffer);
                        continue;
                }
                send(fd_sock, buffer, len, 0);
                free(buffer);

                memset(r_buffer, 0, 1024);//sizeof(r_buffer));
                len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
                if (len < 0) break;
                printf("server says $ %s\n", r_buffer);
                fflush(NULL);
        }
        // bye-bye
        close(fd_sock);
        return 0;
}


int main(int argc, char *argv[])
{
        pid_t pid;

        for(int i=0;i<10;i++)
        {
                pid = fork();
                if(pid == 0)
                {
                        //printf("child pid %d\n", getpid());
			cli(argv[1]);
                        exit(0);
                }
                else if(pid < 0)
                {
                        perror("fork error");
                        return 0;
                }
                else
                {
			srv();
                        //printf("parent pid %d\n", getpid());
                        wait(0);
                }
        }
        return 0;

}
