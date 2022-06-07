#include <math.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


int ordat(char * msg , int idx){
	if(strlen(msg) > idx)
		return (int)msg[idx];
	return 0;
}

int sencode(char *msg,int key,int *res){
	int l = strlen(msg);
	int i,j;
	for(i = 0 ,j = 0 ; i < l ; i+=4,j++){
		res[j] = ordat(msg,i) | ordat(msg,i+1) << 8| ordat(msg , i+2) << 16| ordat(msg,i+3) << 24;
		printf("%d\n",res[j]);
	}
	if(key)
		res[j]=l;
	return j;
}

int lencode(int * msg,int l,int key,char *res){
	int ll = (l - 1) << 2;
	if(key){
		int m = msg[l - 1];
		if(m < (ll - 3) || m > ll)
			return 0;
		ll = m;
	}
	int i;	
	char tmp[5]= {0};
	for(i = 0 ; i < l ; i++){
		sprintf(tmp,"%c%c%c%c",(msg[i]&0xff) ,(msg[i] >> 8 & 0xff ) , (msg[i] >> 16 & 0xff) ,(msg[i] >> 24 & 0xff));
		strcat(res , (const char *)tmp);
	}
	return i;
}

int get_xencode(char * msg, char * key,char * res){
	if(strlen(msg)==0)
		return 0;

	int pwd[256] = {0};
	int pwdk[256] = {0};
	
	int lpwd = sencode(msg , 1 ,  pwd) + 1;
	int lpwdk = sencode(key , 0 , pwdk) + 1;

	if(lpwdk < 4)
		lpwdk = 4;
	
	int n =lpwd - 1;
	printf("%d\n", n);
	int z = pwd[n];
	int y = pwd[0];
//	printf("y:%d\n",y);
	int c = 0x86014019 | 0x183639A0;
	int m = 0;
	int e = 0;
	int p = 0;
	int q = (int)floor(6+52/(n-1));
	int d = 0;

	while(0 < q){
		d = d + c & (0x8ce0d9bf | 0x731f2640);
		//printf("%ld\n",d);
		e = d >> 2 & 3;
		//printf("%ld\n",e);
		p = 0 ;
		while(p < n){
			y = pwd[p+1];
			m = (z >> 5 ^ y << 2 );
		//	printf("%d,%d,%ld\n",z,y,m);
			m = m +(( y >> 3 ^ z << 4 ) ^ ( d ^ y ));
			m = m +(pwdk[p&3^e] ^ z);
			pwd[p] = pwd[p]+((int)m) & (0xEFB8D130 | 0x10472FCF);
			z = pwd[p];
			p = p + 1;
		}
		y = pwd[0];
		m = (z >> 5 ^ y << 2 );
		printf("%lf\n",m);
		m = m +(( y >> 3 ^ z << 4 ) ^ ( d ^ y ));
		m = m +(pwdk[p&3^e] ^ z);
		pwd[n] = pwd[n]+((int)m) & (0xBB290742 | 0x44C6F8BD);
		z = pwd[n];
		q = q - 1; 
	}	


	lencode(pwd ,lpwd, 0 ,res);
}

int main(){

	char * msg="{'username':'201626203044@cmcc','password':'15879684798qq','ip':'10.128.96.249','acid':'1','enc_ver':'srun_bx1'}";
	char * key="e6843f26b8544327a3a25978dd3c5f89e6b745df1732993b88fe082c13a34cb9";

	char res[256] = {0};

	get_xencode(msg , key ,res);

//	printf("%d\n",ordat(msg,2));
	printf("%s\n",res);
	return 0;
}