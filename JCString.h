#ifndef __JCSTRING_H__
#include <stdlib.h>
#include <stdio.h>

enum {
	MEMADDR_HASH_SIZE = 1000
};

typedef struct _JCString_allocated_memory_list JCString_allocated_memory_list;

struct _JCString_allocated_memory_list {
	JCString_allocated_memory_list *prev;
	JCString_allocated_memory_list *next;
	void * p;
};

typedef struct _JCString_allocated_memory_hash JCString_allocated_memory_hash;

struct _JCString_allocated_memory_hash {
	void *p;
	JCString_allocated_memory_list * listentry;
	JCString_allocated_memory_hash *next;
} ;

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
enum JCSTRING_BOOL {
	JCSTRING_FALSE = 0,
	JCSTRING_TRUE = 1
};
typedef struct {
	enum JCSTRING_BOOL use_length;
	size_t length;
	char * value;
} JCSTRING_String;
typedef char * (*JCString_Each)(unsigned char *p);
typedef enum JCSTRING_BOOL (*JCString_IsEnd_String)(unsigned char *p);
typedef int (*JCString_ConvertEncode)(unsigned char *p, JCString_exec_info *info);

extern const JCString_conv_table JCString_sjis_to_utf8_conv_table[];
extern size_t JCString_sjis_to_utf8_conv_table_size;

void JCString_DebugLog(char file[], int line, char format[], ...);
size_t JCString_StrLen(const char *p, JCString_Each string_each_func);
void * JCString_Malloc(size_t size, char file[], int line);
int JCString_Realloc(void **p, size_t size, char file[], int line);
int JCString_Free(void *p, char file[], int line);
int JCString_Release();
unsigned char *JCString_GetHashValue(JCString_conv_table_hash *hashtable, size_t hashtable_size, unsigned char *key, int keylen, 
	JCString_Each string_each_func);
JCString_conv_table_hash *JCString_GenConvTableHash(const JCString_conv_table table[], size_t table_size, size_t hashtable_size,
	JCString_Each string_each_func);
JCString_conv_table_hash *JCString_GenConvInvertedTableHash(const JCString_conv_table table[], size_t table_size, size_t hashtable_size,
	JCString_Each string_each_func);
JCSTRING_String JCString_ConvEncoding(JCSTRING_String str, 
	JCString_Each string_each_func, JCString_ConvertEncode convfunc, JCString_IsEnd_String isstrend_func);
JCSTRING_String JCString_SjisToUTF8(JCSTRING_String str);
JCString_Each JCString_Get_UTF8Each();
char *JCString_FileRead(FILE *fp);

#define __JCSTRING_H__
#endif
