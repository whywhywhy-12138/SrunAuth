
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "url_parse.h"

typedef struct {
	int sockfd;
	int remote_port;
	char remote_ip[16];
	struct sockaddr_in _addr;
	int connected;
}tcpclient;

int http_get(char * host,char *url,char response[2048]);
int tcpclient_con(tcpclient * client);
int httpstr_init(char * result,char * url , char * host);
int tcpclient_init(tcpclient *client , char ip[16] , int port);
int tcpclient_handle(tcpclient * client,char * url,char response[2048]);

