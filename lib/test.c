/*一个测试程序*/  
#include "base64.h"  
#include <stdio.h> 
#include <string.h> 

#include "md5.h"
#include "SHA1.h"

int main(int argc,char **argv)  
{  
    unsigned char * buf;
    unsigned char digest[20]={0};
	
    MD5_CTX md5c;

    MD5Init(&md5c);

    SHA1Context sha;

    SHA1Reset(&sha);

    if(strcmp(argv[1],"-d") == 0) 
    { 
	buf = base64_decode(argv[2]);
        printf("%s\n",digest); 

    }
    else  
    {
	buf = base64_encode(argv[1]);
        printf("%s\n",buf);
		
	int len = strlen(argv[1]);
	printf("%d\n",len);

	MD5Update(&md5c,(unsigned char *)(argv[1]),len);
	MD5Final(&md5c,digest);
	for(int i = 0 ; i< 16 ; i++){
		printf("%02x",*(digest+i));
	}
	
	printf("\n");
	SHA1Input(&sha , (const unsigned char *)(argv[1]),len);
	SHA1Result(&sha,digest);
	for(int i = 0 ; i < 20 ; i++){
		printf("%02x",*(digest+i));
	}
	printf("\n");

    }
 
    free(buf);
  
    return 0;  
} 
