#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>


int getMatch(const char * regtext , const char * srctext , char * resulttext);

int main(){
	char  * regtext="[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";
     	char * srcchar="jQuery112409595426690201216_1654650481897({\"client_ip\":\"10.50.11.83\",\"ecode\":0,\"error\":\"not_online_error\",\"error_msg\":\"\",\"online_ip\":\"10.50.11.83\",\"res\":\"not_online_error\",\"srun_ver\":\"SRunCGIAuthIntfSvr V1.18 B20210607\",\"st\":1654654544})";

	char * res = NULL;

	if(getMatch((const char *)regtext , (const char *)srcchar,res)<0){
		perror("getMatch");
	}
	free(res);
		
	return 0;
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
		resulttext = strndup(srctext+rm[0].rm_so,rm[0].rm_eo-rm[0].rm_so);
		printf("%s\n",resulttext);
	}
	regfree(&reg);
	return 0;

}
