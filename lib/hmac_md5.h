#include <stdio.h>
#include <stdlib.h>
#include "md5.h"
#include <string.h>
  
#define LENGTH_MD5_RESULT 16
#define LENGTH_BLOCK 64


void hmac_md5(unsigned char* out, unsigned char* data, int dlen, unsigned char* key, int klen);
