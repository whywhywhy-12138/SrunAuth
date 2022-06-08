#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>  
#include <sys/socket.h>
#include <string.h>

#define __DEBUG

#ifdef __DEBUG
#define DEBUG(...)	printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif


#define BUFFER_SIZE 1024

/*
	@socket
	@remote_port
	@remote_ip
	@sockaddr_in
	@connected flag
*/
typedef struct _my_tcpclient{
    int socket;
    int remote_port;     
    char remote_ip[16];  
    struct sockaddr_in _addr; 
    int connected;       
} my_tcpclient;

int my_tcpclient_create(my_tcpclient *pclient,const char *host, int port);
int my_tcpclient_conn(my_tcpclient *pclient);
int my_tcpclient_recv(my_tcpclient *pclient,char **lpbuff,int size);
int my_tcpclient_send(my_tcpclient *pclient,char *buff,int size);
int my_tcpclient_close(my_tcpclient *pclient);
int my_http_post(my_tcpclient *pclient,char *page,char *request,char **response);
int my_http_get(my_tcpclient *pclient,char *page,char *request,char **response);


int my_tcpclient_create(my_tcpclient *pclient,const char *host, int port){
    struct hostent *he;

    if(pclient == NULL) 
		return -1;

    memset(pclient,0,sizeof(my_tcpclient));

    if((he = gethostbyname(host))==NULL){
        return -2;
    }

    pclient->remote_port = port;
    strcpy(pclient->remote_ip,inet_ntoa( *((struct in_addr *)he->h_addr) ));

    pclient->_addr.sin_family = AF_INET;
    pclient->_addr.sin_port = htons(pclient->remote_port);
    pclient->_addr.sin_addr = *((struct in_addr *)he->h_addr);

    if((pclient->socket = socket(AF_INET,SOCK_STREAM,0))==-1){
        return -3;
    }

    return 0;
}

int my_tcpclient_conn(my_tcpclient *pclient){

    if(pclient->connected)
        return 1;

    if(connect(pclient->socket, (struct sockaddr *)&pclient->_addr,sizeof(struct sockaddr))==-1){
        return -1;
    }

    pclient->connected = 1;

    return 0;
}

int my_tcpclient_recv(my_tcpclient *pclient,char **lpbuff,int size){
    int recvnum=0,tmpres=0;
    char buff[BUFFER_SIZE];

    *lpbuff = NULL;

    while(recvnum < size || size==0){
        tmpres = recv(pclient->socket, buff,BUFFER_SIZE,0);
        if(tmpres <= 0)
            break;
        recvnum += tmpres;

        if(*lpbuff == NULL){
            *lpbuff = (char*)malloc(recvnum);
            if(*lpbuff == NULL)
                return -2;
        }else{
            *lpbuff = (char*)realloc(*lpbuff,recvnum);
            if(*lpbuff == NULL)
                return -2;
        }

        memcpy(*lpbuff+recvnum-tmpres,buff,tmpres);
    }

    return recvnum;
}

int my_tcpclient_send(my_tcpclient *pclient,char *buff,int size){
    int sent=0,tmpres=0;

    while(sent < size){
        tmpres = send(pclient->socket,buff+sent,size-sent,0);
        if(tmpres == -1){
            return -1;
        }
        sent += tmpres;
    }
    return sent;
}

int my_tcpclient_close(my_tcpclient *pclient){
    close(pclient->socket);
    pclient->connected = 0;
}

void get_http_header(char *header){
	char userAgent[] = "User-Agent: QG1 Http 0.1\r\n";
	char cacheCtrl[] = "Cache-Control: no-cache\r\n";
	char contentType[] = "Content-Type: application/x-www-form-urlencoded\r\n";
	char postmanToken[] = "Postman-Token: 8d4d5783-4660-c219-8644-e2e7dad68500\r\n";
	char auth[] = "Authorization: Basic cm9hbTJmcmVlOmx5bmttYXhfdGVzdA==\r\n";
	char accept[] = "Accept: */*\r\n";

	strcpy(header,userAgent);
    strcat(header,cacheCtrl);
    strcat(header,contentType);
    strcat(header,postmanToken);
    strcat(header,auth);
	strcat(header,accept);
}

int http_post(my_tcpclient *pclient,char *page,char *request,char **response){

    char post[300],host[100],content_len[100];
    char *lpbuf,*ptmp;
    int len=0;

	char header2[1024];

	memset(header2, sizeof(header2), 0);
	get_http_header(header2);

	//DEBUG("header2 : %s \n", header2);

    lpbuf = NULL;

    sprintf(post,"POST %s HTTP/1.0\r\n",page);
    sprintf(host,"HOST: %s:%d\r\n",pclient->remote_ip,pclient->remote_port);
    sprintf(content_len,"Content-Length: %d\r\n\r\n",strlen(request));

    len = strlen(post)+strlen(host)+strlen(header2)+strlen(content_len)+strlen(request)+1;
    lpbuf = (char*)malloc(len);
    if(lpbuf==NULL){
        return -1;
    }

    strcpy(lpbuf,post);
    strcat(lpbuf,host);
    strcat(lpbuf,header2);
    strcat(lpbuf,content_len);
    strcat(lpbuf,request);

	//printf("liuyan: http data= \r\n%s liuyan end \n", lpbuf);

    if(!pclient->connected){
        my_tcpclient_conn(pclient);
    }

    if(my_tcpclient_send(pclient,lpbuf,len)<0){
        return -1;
    }
	printf("发送请求:\n%s\n",lpbuf);

    /*释放内存*/
    if(lpbuf != NULL) 
		free(lpbuf);
	
    lpbuf = NULL;

    /*it's time to recv from server*/
    if(my_tcpclient_recv(pclient,&lpbuf,0) <= 0){
        if(lpbuf) 
			free(lpbuf);
        return -2;
    }
	printf("接收响应:\n%s\n",lpbuf);

    /*响应代码,|HTTP/1.0 200 OK|
     *从第10个字符开始,第3位
     * */
    memset(post,0,sizeof(post));
    strncpy(post,lpbuf+9,3);
    if(atoi(post)!=200){
        if(lpbuf) free(lpbuf);
        return atoi(post);
    }

    ptmp = (char*)strstr(lpbuf,"\r\n\r\n");
    if(ptmp == NULL){
        free(lpbuf);
        return -3;
    }
    ptmp += 4;/*跳过\r\n*/

    len = strlen(ptmp)+1;
    *response=(char*)malloc(len);
    if(*response == NULL){
        if(lpbuf) free(lpbuf);
        return -1;
    }
    memset(*response,0,len);
    memcpy(*response,ptmp,len-1);

    /*从头域找到内容长度,如果没有找到则不处理*/
    ptmp = (char*)strstr(lpbuf,"Content-Length:");
    if(ptmp != NULL){
        char *ptmp2;
        ptmp += 15;
        ptmp2 = (char*)strstr(ptmp,"\r\n");
        if(ptmp2 != NULL){
            memset(post,0,sizeof(post));
            strncpy(post,ptmp,ptmp2-ptmp);
            if(atoi(post)<len)
                (*response)[atoi(post)] = '\0';
        }
    }

    if(lpbuf) free(lpbuf);

    return 0;
}

int http_get(my_tcpclient *pclient,char *page,char *request,char **response){

    char get[300],host[100],content_len[100];
    char *lpbuf,*ptmp;
    int len=0;

	char header2[1024];

	memset(header2, sizeof(header2), 0);
	get_http_header(header2);

	//DEBUG("header2 : %s \n", header2);

    lpbuf = NULL;

    sprintf(get,"GET %s HTTP/1.0\r\n",page);
    sprintf(host,"HOST: %s:%d\r\n",pclient->remote_ip,pclient->remote_port);
    sprintf(content_len,"Content-Length: %d\r\n\r\n",strlen(request));

    len = strlen(get)+strlen(host)+strlen(header2)+strlen(content_len)+strlen(request)+1;
    lpbuf = (char*)malloc(len);
    if(lpbuf==NULL){
        return -1;
    }

    strcpy(lpbuf,get);
    strcat(lpbuf,host);
    strcat(lpbuf,header2);
    strcat(lpbuf,content_len);
    strcat(lpbuf,request);

	//printf("liuyan: http data= \r\n%s liuyan end \n", lpbuf);

    if(!pclient->connected){
        my_tcpclient_conn(pclient);
    }

    if(my_tcpclient_send(pclient,lpbuf,len)<0){
        return -1;
    }
	printf("发送请求:\n%s\n",lpbuf);

    /*释放内存*/
    if(lpbuf != NULL) 
		free(lpbuf);
	
    lpbuf = NULL;

    /*it's time to recv from server*/
    if(my_tcpclient_recv(pclient,&lpbuf,0) <= 0){
        if(lpbuf) 
			free(lpbuf);
        return -2;
    }
	printf("接收响应:\n%s\n",lpbuf);

    /*响应代码,|HTTP/1.0 200 OK|
     *从第10个字符开始,第3位
     * */
    memset(get,0,sizeof(get));
    strncpy(get,lpbuf+9,3);
    if(atoi(get)!=200){
        if(lpbuf) free(lpbuf);
        return atoi(get);
    }

    ptmp = (char*)strstr(lpbuf,"\r\n\r\n");
    if(ptmp == NULL){
        free(lpbuf);
        return -3;
    }
    ptmp += 4;/*跳过\r\n*/

    len = strlen(ptmp)+1;
    *response=(char*)malloc(len);
    if(*response == NULL){
        if(lpbuf) free(lpbuf);
        return -1;
    }
    memset(*response,0,len);
    memcpy(*response,ptmp,len-1);

    /*从头域找到内容长度,如果没有找到则不处理*/
    ptmp = (char*)strstr(lpbuf,"Content-Length:");
    if(ptmp != NULL){
        char *ptmp2;
        ptmp += 15;
        ptmp2 = (char*)strstr(ptmp,"\r\n");
        if(ptmp2 != NULL){
            memset(get,0,sizeof(get));
            strncpy(get,ptmp,ptmp2-ptmp);
            if(atoi(get)<len)
                (*response)[atoi(get)] = '\0';
        }
    }

    if(lpbuf) free(lpbuf);

    return 0;
}
