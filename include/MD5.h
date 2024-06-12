/*
* Copyright (c) 2018,hiro �з���ϵ
* All rights reserved.
*
* �ļ����ƣ�MD5.h
* �ļ���ʶ��
* ժ Ҫ��
*
* ��ǰ�汾��1.0
* �� �ߣ�����
* ������ڣ�
*
* ȡ���汾��
* ԭ���� ��
* ������ڣ�
*/

#ifndef __SRC_BASE_MD5_H__
#define __SRC_BASE_MD5_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned int count[2];
	unsigned int state[4];
	unsigned char buffer[64];   
} HRMD5_CTX;

static inline char i2hex(unsigned char i) { return i < 10 ? i + '0' : i - 10 + 'a'; }

void HRMD5_Init(HRMD5_CTX *context);
void HRMD5_Update(HRMD5_CTX *context, unsigned char *input, unsigned int inputlen);
void HRMD5_Final(HRMD5_CTX *context, unsigned char digest[16]);

void HRMD5_MD5SUM(unsigned char* input, unsigned int inputlen, unsigned char digest[16]);
void HRMD5_MD5SUM_HEX(unsigned char* input, unsigned int inputlen, char output[36]);


#ifdef __cplusplus
}
#endif

#endif

