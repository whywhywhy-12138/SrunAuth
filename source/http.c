#include "http.h"


int tcpclient_init(tcpclient *client , char ip[16] , int port){

	if((client->sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1){
		perror("socket init");
		return -1;
	} 

	client -> remote_port = port;
	strcpy(client->remote_ip,(const char *)ip);

	client -> _addr.sin_family = AF_INET; 
	client -> _addr.sin_addr.s_addr = inet_addr(ip); 
	client -> _addr.sin_port = htons(port);

	return 0;
}

int tcpclient_con(tcpclient * client){
	if(connect(client->sockfd,(struct sockaddr *)&(client->_addr),sizeof(client->_addr))<-1){ 
		perror("oops: client"); 
		return -1; 
	}
	return 0;
}

int httpstr_init(char * result,char * url , char * host){
	sprintf(result,"GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Connection: close\r\n\r\n",url,host);
	return 0;
}

int tcpclient_handle(tcpclient * client,char * url,char * response){

	char httpstr[1024]={0};
	char ch[64]={0};
	int ret;

	httpstr_init(httpstr , url , client->remote_ip);

	write(client->sockfd,httpstr,strlen(httpstr)); 

	do{

		ret = read(client->sockfd,ch,sizeof(ch));
		if(ret < 0){
			perror("result");
			return -1;
		}

		memcpy(response,ch,ret);
		response+=ret;

	}while(ret);

}

int http_get(char * host,char *url,char * response){

	tcpclient client;
	if(tcpclient_init(&client,host,80)<0){
		perror("client_init");
		return -1;
	}
	if(tcpclient_con(&client)<0){
		perror("client_con");
		return -1;
	}
	if(tcpclient_handle(&client,url,response)<0){
		perror("client_handle");
		return -1;
	}

	return 0; 
}
