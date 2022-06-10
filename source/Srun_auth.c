#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "base64.h"
#include "hmac_md5.h"
#include "http.h"
#include "SHA1.h"
#include "xencode.h"


typedef struct _user_info{
	char  name[32];
	char  pwd[32];
	char  epwd[64];
	char  chksum[64];
	char  info[512];
	char  ip[16];
	char  token[65];
}user_info;

int getMatch(const char * regtext , const char * srctext , char * resulttext);

int get_ip(char * host,char * ip);
int get_token(char * host , user_info * user);
int encode_info(user_info * user);
int login(char * host,user_info * user);

int main(int argc,char * argv[]){

	user_info user={0};

	char * host = "10.30.4.3";
	int ret;

	char opt;
	char *optstring = "u:p:m:";

	if(argc < 5){
		printf("usage: %s -u username -p password [-m mode]\n",argv[0]);
		return -1;
	}

	while((opt = getopt(argc ,argv , optstring)) != -1){
		switch(opt){
			case 'u':
				strcpy(user.name,optarg);
				break;
			case 'p':
				strcpy(user.pwd,optarg);
				break;
			case 'm':
				break;
		}
	}

	if(strlen(user.name)==0||strlen(user.pwd)==0){
		printf("input error\n");
		return -1;
	}
	
	printf("name:%s\npassword:%s\n",user.name,user.pwd);

	ret = get_ip(host,user.ip);
	if(ret<0){
		perror("get_ip");
		return -1;
	}else if(ret == 1){
		printf("Already online!\n");
		exit(0);
	}
		
	if(get_token(host,&user)<0){
		perror("get_token");
		return -1;
	}

	encode_info(&user);
	
	ret =login(host,&user);
	if(ret<0){
		perror("login");
		return -1;	
	}else if(ret == 0){
		printf("LoginSuccess!\n");
	}else if(ret == 1){
		printf("LoginFild!\n");
	}

	//printf("name:%s\npwd:%s\nepwd:%s\nchksum:%s\ninfo:%s\nip:%s\ntoken:%s\n",user.name,user.pwd,user.epwd,user.chksum ,user.info, user.ip,user.token);
	return 0;
}


int get_ip(char * host , char * ip){
	
	char response[2048]={0};

	char * not_online = "not_online_error";

	if(http_get(host,"/cgi-bin/rad_user_info?callback=jQuery112405889573243661126_1654760393331",response)<0){
		perror("IpGet");
		return -1;
	}

	const char * reg = "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";
	
	if(!strstr(response , not_online)){
		return 1;
	}

	if(getMatch(reg , response , ip)<0){
		perror("IpMatch");
		return -1;
	}

	return 0;
}

int get_token(char * host , user_info * user){

	char response[2048]={0};
	char tokenurl[128]={0};

	
	struct timeval s;
	gettimeofday(&s,NULL);
	//printf("%ld\n",s.tv_sec * 1000 +s.tv_usec/1000);
	
	char tmp[128]="";

	strcpy(tmp , user->name);
	//printf("%s\n\n",tmp);
	urlencode(tmp);

	sprintf(tokenurl,"/cgi-bin/get_challenge?username=%s&ip=%s&callback=jQuery112405889573243661126_1654760393331&_=%ld",tmp,user->ip,s.tv_sec * 1000 +s.tv_usec/1000);
	
	//printf("%s\n\n\n\n",tokenurl);
	if(http_get(host , tokenurl , response)<0){
		perror("TokenGet");
		return -1;
	}
	
	const char * reg = "[0-9a-z]{64}";
	if(getMatch(reg , response , user->token)<0){
		perror("TokenMatch");
		return -1;
	}

	return 0;
}

int encode_info(user_info * user){
	unsigned char tmp[256]={0};
	unsigned char tmp1[1024]={0};

	sprintf(tmp,"{\"username\":\"%s\","
		   "\"password\":\"%s\","
		   "\"ip\":\"%s\","
		   "\"acid\":\"4\",\"enc_ver\":\"srun_bx1\"}",user->name,user->pwd,user->ip);
	
	
	get_xencode(tmp,user->token,tmp1);
	//printf("\t%s\n\n\t%s\n\n",tmp,user->token);

	strcat(user->info, "{SRBX1}");
	strcat(user->info, base64_encode(tmp1));
	

	memset(tmp,0,sizeof(tmp));
	memset(tmp1,0,sizeof(tmp1));
	
	hmac_md5(tmp,user->pwd,strlen(user->pwd),user->token,strlen(user->token));
	for(int i = 0;i < 16 ; i++){
		sprintf(((user->epwd)+(2*i)),"%02x",tmp[i]);
	}
	
	memset(tmp,0,sizeof(tmp));
	memset(tmp1,0,sizeof(tmp1));

	sprintf(tmp1,"%s%s%s%s%s%d%s%s%s%d%s%d%s%s",user->token,user->name,\
						user->token,user->epwd,\
						user->token,4,\
						user->token,user->ip,\
						user->token,200,\
						user->token,1,\
						user->token,user->info);
	SHA1Context sha1;
	SHA1Reset(&sha1);
	SHA1Input(&sha1,tmp1,strlen(tmp1));
	SHA1Result(&sha1,tmp);
	for(int i = 0;i < SHA1HashSize; i++){
		sprintf(((user->chksum)+(2*i)),"%02x",tmp[i]);
	}
	

}
int login(char * host , user_info * user){
	
	char login_url[2048];
	char response[2048];

	const char * auth_url_data = "/cgi-bin/srun_portal?callback=jQuery112405889573243661126_1654760393331&action=login&username=%s&password=%%7BMD5%%7D%s&os=Windows+10&name=Windows&double_stack=0&chksum=%s&info=%s&ac_id=4&ip=%s&n=200&type=1&_=%ld";

	char tempinfo[1024];
	char tempname[64];

	strcpy(tempinfo,user->info);
	strcpy(tempname,user->name);

	urlencode(tempinfo);
	urlencode(tempname);
	
	struct timeval s;
	gettimeofday(&s,NULL);
	
	//printf("%%7B%s\n\n",tempinfo);


	sprintf(login_url,auth_url_data,tempname,user->epwd,user->chksum,tempinfo,user->ip,s.tv_sec * 1000 +s.tv_usec/100);

	//printf("%s\n\n",login_url);
	if(http_get(host,login_url,response)<0){
		perror("login");
		return -1;
	}

	if(strstr(response,"successful"))
		return 0;
	else
		return 1;

//	printf("\n\n%s\n\n%s\n\n",login_url,response);

}

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
		strcpy(resulttext,strndup(srctext+rm[0].rm_so,rm[0].rm_eo-rm[0].rm_so));
	}

	regfree(&reg);
	
	return 0;

}
