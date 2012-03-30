#ifndef __JCSTRING_H__
#include <stdlib.h>
#include <stdio.h>

enum {
	MEMADDR_HASH_SIZE = 10000
};
typedef enum {
	JCSTRING_FALSE = 0,
	JCSTRING_TRUE = 1
} JCSTRING_BOOL;
typedef enum {
	JCSTRING_ENC_INTERNAL = 0,
	JCSTRING_ENC_SJIS,
	JCSTRING_ENC_SJISWIN,
	JCSTRING_ENC_UTF8,
	JCSTRING_ENC_END
} JCSTRING_ENCODING;
typedef enum {
	JCSTRING_ERR_NONE = 0,
	JCSTRING_SUCCESS,
	JCSTRING_ERR_PRMERR,
	JCSTRING_ERR_BAD_ALLOC,
	JCSTRING_ERR_SYSTEMERR
} JCSTRING_ERR;
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
	JCSTRING_ERR err;
	int exit;
} JCString_exec_info_header ;
typedef struct {
	JCString_exec_info_header header;
	union _JCString_exec_info_by_feature {
		JCString_enc_convert_data convert_data;
	} data;
} JCString_exec_info;

typedef struct {
	char key[8];
	char val[8];
} JCString_conv_table;

typedef struct _JCString_conv_table_hash JCString_conv_table_hash;

struct _JCString_conv_table_hash {
	unsigned char *key;
	unsigned char *val;
	JCString_conv_table_hash * next;
};
typedef struct {
	JCSTRING_BOOL use_length;
	size_t length;
	char * value;
} JCString_String;
typedef char * (*JCString_Each)(unsigned char *p, unsigned char *end);
typedef JCSTRING_BOOL (*JCString_IsEnd_String)(unsigned char *p);
typedef int (*JCString_ConvertEncode)(unsigned char *p, unsigned char *end, JCString_exec_info *info);

extern const JCString_conv_table JCString_sjis_to_utf8_conv_table[];
extern size_t JCString_sjis_to_utf8_conv_table_size;

void JCString_DebugLog(char file[], int line, char format[], ...);
size_t JCString_StrByteLen(const char *p, JCString_Each string_each_func);

void * JCString_Malloc(size_t size, char file[], int line);
JCSTRING_ERR JCString_Realloc(void **p, size_t size, char file[], int line);
JCSTRING_ERR JCString_Free(void *p, char file[], int line);
JCSTRING_ERR JCString_Release();

unsigned char *JCString_GetHashValue(JCString_conv_table_hash *hashtable, size_t hashtable_size, unsigned char *key, int keylen, 
	JCString_Each string_each_func);
JCString_conv_table_hash *JCString_GenConvTableHash(const JCString_conv_table table[], size_t table_size, size_t hashtable_size,
	JCString_Each string_each_func);
JCString_conv_table_hash *JCString_GenConvInvertedTableHash(const JCString_conv_table table[], size_t table_size, size_t hashtable_size,
	JCString_Each string_each_func);

JCSTRING_ERR JCString_SetInternalEncoding(JCSTRING_ENCODING encoding);
JCSTRING_ENCODING JCString_GetInternalEncoding();
JCSTRING_BOOL JCString_IsDefinedEncType(int encoding);
JCString_String JCString_CreateString(char *p, JCSTRING_ENCODING encoding, JCSTRING_ERR *err);

JCString_String JCString_ConvEncodingCommon(JCString_String str, 
	JCString_Each string_each_func, JCString_ConvertEncode convfunc, JCString_IsEnd_String isstrend_func, JCSTRING_ERR *err);
JCString_String JCString_SjisWinToUTF8(JCString_String str, JCSTRING_ERR *err);
JCString_String JCString_UTF8ToSJISWin(JCString_String str, JCSTRING_ERR *err);
JCString_String JCString_ToSJISWin(JCString_String str, JCSTRING_ENCODING encoding, JCSTRING_ERR *err);
JCString_String JCString_ToUTF8(JCString_String str, JCSTRING_ENCODING encoding, JCSTRING_ERR *err);
JCString_IsEnd_String JCString_Get_SJISIsEndString();

JCString_Each JCString_Get_SJISEach();
JCString_Each JCString_Get_UTF8Each();
JCString_Each JCString_Get_Each(JCSTRING_ENCODING encoding);

JCString_IsEnd_String JCString_Get_UTF8IsEndString();
JCString_String JCString_CreateStringFromFile(FILE *fp, JCSTRING_ENCODING encoding, JCSTRING_ERR *err);

#define __JCSTRING_H__
#endif
