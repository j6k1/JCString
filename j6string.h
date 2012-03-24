#ifndef __J6STRING_H__
#include <stdlib.h>

typedef struct {
	int size;
	int count;
	unsigned char *buff;
} j6string_enc_convert_data;

typedef union {
	j6string_enc_convert_data convert_data;
} j6string_exec_info;


typedef struct {
	char key[5];
	char val[5];
} j6string_conv_table;

typedef struct _j6string_j6string_conv_table_hash j6string_conv_table_hash;

struct _j6string_j6string_conv_table_hash {
	unsigned char *key;
	unsigned char *val;
	j6string_conv_table_hash * next;
};
void * j6string_malloc(size_t size, char file[], int line);
void j6string_free(void *p, char file[], int line);
unsigned char *j6string_gethashvalue(j6string_conv_table_hash *hashtable, size_t hashtable_size, unsigned char *key, int keylen);
j6string_conv_table_hash *j6string_gen_convtable_hash(const j6string_conv_table table[], size_t table_size, size_t hashtable_size);
char * j6string_sjis_to_utf8(char str[]);
int j6string_release();

#define __J6STRING_H__
#endif
