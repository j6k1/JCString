#ifndef __JCSTRING_H__
#include <stdlib.h>
#include <stdio.h>


typedef struct {
	int size;
	int count;
	unsigned char *buff;
} JCString_enc_convert_data;

typedef struct {
	int exit;
} JCString_exec_info_header ;
typedef struct {
	JCString_exec_info_header header;
	union _JCString_exec_info_by_feature {
		JCString_enc_convert_data convert_data;
	} data;
} JCString_exec_info;

typedef struct {
	char key[5];
	char val[5];
} JCString_conv_table;

typedef struct _JCString_conv_table_hash JCString_conv_table_hash;

struct _JCString_conv_table_hash {
	unsigned char *key;
	unsigned char *val;
	JCString_conv_table_hash * next;
};
void JCString_DebugLog(char file[], int line, char format[], ...);
void * JCString_Malloc(size_t size, char file[], int line);
int JCString_Realloc(void **p, size_t size, char file[], int line);
void JCString_Free(void *p, char file[], int line);
int JCString_Release();
unsigned char *JCString_GetHashValue(JCString_conv_table_hash *hashtable, size_t hashtable_size, unsigned char *key, int keylen);
JCString_conv_table_hash *JCString_GenConvTableHash(const JCString_conv_table table[], size_t table_size, size_t hashtable_size);
char * JCString_SjisToUTF8(char str[]);
char *JCString_FileRead(FILE *fp);

#define __JCSTRING_H__
#endif
