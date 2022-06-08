#include "hmac_md5.h"

/**
*作者：			小姚
*时间：			2020-08-11
*功能：			基于md5实现hmac。
*注意事项：			
*				1. 代码中多次出现64、16，应该定义为宏，便于修改。
*				2. 
*参数：
*		data(in): 需要加密的数据
*		dlen(in): 加密数据的长度
*		key(in) : 密钥
*		klen(in): 密钥长度
*		out(out): 加密之后的串
*/
void hmac_md5(unsigned char* out, unsigned char* data, int dlen, unsigned char* key, int klen)
{
	int i;
 
	unsigned char tempString16[LENGTH_MD5_RESULT];
	unsigned char OneEnding[LENGTH_BLOCK];
	unsigned char TwoEnding[LENGTH_BLOCK];
	unsigned char ThreeEnding[LENGTH_BLOCK + dlen];
	unsigned char FourEnding[LENGTH_MD5_RESULT];	/*如下步骤四生成的结果*/
	unsigned char FiveEnding[LENGTH_BLOCK];    /*步骤五生成的结果*/
	unsigned char SixEnding[LENGTH_BLOCK+ LENGTH_MD5_RESULT];
	
	char ipad;
	char opad;
	MD5_CTX md5;
	
	ipad = 0x36;
	opad = 0x5c;
	
	
/*(1) 在密钥key后面添加0来创建一个长为B(64字节)的字符串(OneEnding)。如果key的长度klen大于64字节，则先进行md5运算，使其长度klen=16字节。 */
 
	for ( i = 0; i < LENGTH_BLOCK; i++)
	{
			OneEnding[i] = 0;
	}
 
	if ( klen > LENGTH_BLOCK)
	{
		MD5Init(&md5);         		
		MD5Update(&md5,key,klen);
		MD5Final(&md5,tempString16);   
		for (i = 0; i < LENGTH_MD5_RESULT; i++)
			OneEnding[i] = tempString16[i];
	}	
	else
	{
	   for (i = 0; i < klen; i++)
			OneEnding[i] = key[i] ;
	}
	
	
/*(2) 将上一步生成的字符串(OneEnding)与ipad(0x36)做异或运算，形成结果字符串(TwoEnding)。*/
	for ( i = 0; i < LENGTH_BLOCK; i++)
	{
		TwoEnding[i] = OneEnding[i] ^ ipad; 
	}
/*(3) 将数据流data附加到第二步的结果字符串(TwoEnding)的末尾。*/
	for ( i = 0; i < LENGTH_BLOCK; i++ )
	{
		ThreeEnding[i] = TwoEnding[i];
	}
	for ( ; i < dlen + LENGTH_BLOCK; i++)
	{
		ThreeEnding[i] = data[i - LENGTH_BLOCK];
	}
/*(4) 做md5运算于第三步生成的数据流(ThreeEnding)。*/
	MD5Init(&md5);         		
	MD5Update( &md5, ThreeEnding, LENGTH_BLOCK + dlen);
	MD5Final(&md5,FourEnding);
	
/*(5) 将第一步生成的字符串(OneEnding)与opad(0x5c)做异或运算，形成结果字符串(FiveEnding)。*/
	for ( i = 0 ; i < LENGTH_BLOCK; i++ )
	{
			FiveEnding[i] = OneEnding[i] ^ opad;
	}
/*(6) 再将第四步的结果(FourEnding)附加到第五步的结果字符串(FiveEnding)的末尾。*/
	for (i = 0; i < LENGTH_BLOCK; i++)
	{
		SixEnding[i] = FiveEnding[i];
	}
	for ( ; i < (LENGTH_BLOCK + LENGTH_MD5_RESULT); i++)
	{
		SixEnding[i] = FourEnding[i - LENGTH_BLOCK];
	}
/*(7) 做md5运算于第六步生成的数据流(SixEnding)，输出最终结果(out)。*/
	MD5Init(&md5);         		
	MD5Update( &md5, SixEnding, LENGTH_BLOCK + LENGTH_MD5_RESULT);
	MD5Final(&md5,out);	
} 
