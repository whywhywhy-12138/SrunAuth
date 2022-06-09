#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <time.h>

#include "base64.h"
#include "hmac_md5.h"
#include "http.h"
#include "SHA1.h"
#include "xencode.h"

int getMatch(const char * regtext , const char * srctext , char * resulttext);

int get_chksum();

int get_ip(char * host);

int get_token();
int do_complex_work();
int login();
int get_usr_info();

int main(){

	char my_ip[16];
	char token[65]="";

	time_t t;
	t = time(NULL);
	long ii =time(&t);
	printf("%ld\n",ii);

	char *username="123456789456@xuesheng";
	char * host = "10.30.4.3";
	
	
	if(get_ip(host)<0){
		perror("get_ip");
		return -1;
	}
		
	if(get_token(host,username,token)<0){
		perror("get_token");
		return -1;
	}

	do_complex_work();
	login();
	get_usr_info();

	//my_tcpclient_close(&client);
	return 0;
}
int get_chksum(){

}

int get_ip(char * host){
	
	char response[2048]={0};

	char * not_online = "not_online_error";

	if(http_get(host,"/cgi-bin/rad_user_info?callback=0",response)<0){
		perror("IpGet");
		return -1;
	}

	const char * reg = "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";
	
	if(!strstr(response , not_online)){
		printf("Already Online!\n");
		exit(0);
	}

	if(getMatch(reg , response , host)<0){
		perror("IpMatch");
		return -1;
	}

	return 0;
}

int get_token(char * host , char * username ,char *token){

	char response[2048]={0};
	char tokenurl[128]={0};

	sprintf(tokenurl,"/cgi-bin/get_challenge?username=%s&ip=%s&callback=0",username,host);
	
	if(http_get(host , tokenurl , response)<0){
		perror("TokenGet");
		return -1;
	}

	const char * reg = "[0-9a-z]{64}";
	if(getMatch(reg , response , token)<0){
		perror("TokenMatch");
		return -1;
	}
	return 0;
}

int do_complex_work(){}
int login(){}
int get_usr_info(){}

int getMatch(const char * regtext , const char * srctext , char * resulttext){
	int i;
	int ret;
	int cflags;
	regex_t reg;
	regmatch_t rm[5];

	cflags = REG_EXTENDED |REG_ICASE;

	ret= regcomp(&reg,regtext,cflags);
	if(ret){
		perror("regcomp:");
		regfree(&reg);
		return -1;
	}

	ret = regexec(&reg , srctext , 5 ,rm , 0);
	if(ret){
		perror("regexec:\n");
		regfree(&reg);
		return -1;
	}

	if(rm[0].rm_so > -1){
		resulttext = strndup(srctext+rm[0].rm_so,rm[0].rm_eo-rm[0].rm_so);
		printf("%s\n",resulttext);
	}
	regfree(&reg);
	return 0;

}
