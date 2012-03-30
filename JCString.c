#include "JCString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static int alloc_count = 0;
static JCString_allocated_memory_list * allocated_memory_list = NULL;
static JCString_allocated_memory_hash * allocated_memory_hash = NULL;

static int remove_memaddr_hash(void *p);
static int get_table_hash(unsigned char *key,  unsigned int keylen, size_t hashtable_size);

static int get_memaddr_table_hash(void *p)
{
	static const int multipliter = 37;
	int i=0;
	int len=0;
	void *tmp = 0;
	unsigned int h;

	h = 0;
	tmp = p;
	len = sizeof(long);

	for(i=0; i < len ; i++)
	{
		h = multipliter * h + ((long)tmp & 0xFF);
		tmp = (void *)((long)(tmp) >> 8);
	}
	
	return h % MEMADDR_HASH_SIZE;
}
static int release_memaddr_hash()
{
	JCString_allocated_memory_list *entry = NULL;

	if(allocated_memory_list == NULL)
	{
		return 1;
	}

	entry = allocated_memory_list;

	remove_memaddr_hash(entry->p);
	free(entry->p);
	alloc_count--;

	JCString_DebugLog( __FILE__, __LINE__, "Memory free success");
	JCString_DebugLog( __FILE__, __LINE__, "Allocated memory count is %d", alloc_count);

	while(entry->next != NULL)
	{
		entry = entry->next;
		remove_memaddr_hash(entry->p);
		free(entry->p);
		alloc_count--;

		JCString_DebugLog( __FILE__, __LINE__, "Memory free success");
		JCString_DebugLog( __FILE__, __LINE__, "Allocated memory count is %d", alloc_count);
	}

	return 1;
}
static int set_memaddr_hash(void *p, JCString_allocated_memory_list *entry)
{
	int hash = 0;
	JCString_allocated_memory_hash *hashentry = NULL;

	if(allocated_memory_hash == NULL)
	{
		JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory hash is null!!"); 
		return -1;
	}

	hash = get_memaddr_table_hash(p);
	hashentry = &allocated_memory_hash[hash];

	if(hashentry->p != NULL)
	{
		while(hashentry->next != NULL)
		{
			hashentry = hashentry->next;
		}

		hashentry = (JCString_allocated_memory_hash *)malloc(sizeof(JCString_allocated_memory_hash));

		if(hashentry == NULL)
		{
			JCString_DebugLog( __FILE__, __LINE__ , "Allocation failure of entries in the hash of the allocated memory!!");
			return -1;
		}

		alloc_count++;

		JCString_DebugLog( __FILE__, __LINE__ , "Success to allocate memory for entry in the hash table of allocated memory!!");
		JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory count is %d", alloc_count);		
		memset((void *)hashentry, 0x00, sizeof(JCString_allocated_memory_hash));
	}

	hashentry->p = p;
	hashentry->listentry = entry;

	return 1;
}
static int remove_memaddr_hash(void *p)
{
	int hash = 0;

	JCString_allocated_memory_hash *entry = NULL;
	JCString_allocated_memory_hash *prev = NULL;
	JCString_allocated_memory_list *listentry = NULL;

	if(allocated_memory_hash == NULL)
	{
		JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory hash is null!!"); 
		return -1;
	}

	hash = get_memaddr_table_hash(p);

	
	if(allocated_memory_hash[hash].p != p)
	{
		entry = &allocated_memory_hash[hash];

		while(entry->p != p)
		{
			if(entry->next == NULL)
			{
				JCString_DebugLog( __FILE__, __LINE__ , "Entry of the allocated memory can not be found in the hash table!!"); 
				return -1;
			}

			prev = &allocated_memory_hash[hash];
			entry = entry->next;
		}

		if(prev == NULL)
		{
			JCString_DebugLog( __FILE__, __LINE__ , 
				"prev entry is null!! There may be a problem with the process of release of the hash table entry"); 
		}
		else
		{
			prev->next = entry->next;
		}

		free((void *)entry);

		alloc_count--;
		
		JCString_DebugLog( __FILE__, __LINE__ , "Success to remove the hash table entry of the allocated memory!!");
		JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory count is %d", alloc_count);
	}
	else
	{
		memset((void *)&allocated_memory_hash[hash], 0x00, sizeof(JCString_allocated_memory_hash));
	}

	return 1;
}
static int release_memaddr_list()
{
	JCString_allocated_memory_list *entry = NULL;

	if(allocated_memory_list == NULL)
	{
		return 1;
	}

	entry = allocated_memory_list;

	while(entry->next != NULL)
	{
		entry = entry->next;
		entry->prev->next = NULL;
		entry->prev->p = NULL;
		free(entry->prev);
		entry->prev = NULL;
		alloc_count--;

		JCString_DebugLog( __FILE__, __LINE__ , "Success to remove the list entry of the allocated memory!!");
		JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory count is %d", alloc_count);
	}

	free(entry);
	alloc_count--;

	JCString_DebugLog( __FILE__, __LINE__ , "Success to remove the list entry of the allocated memory!!");
	JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory count is %d", alloc_count);

	return 1;
}
static int add_memaddr_list(JCString_allocated_memory_list *entry)
{
	if(allocated_memory_list == NULL)
	{
		allocated_memory_list = entry;
	}
	else
	{
		allocated_memory_list->prev = entry;
		entry->prev = NULL;
		entry->next = allocated_memory_list;
		allocated_memory_list = entry;
	}

	return 1;
}
static int remove_memaddr_list(JCString_allocated_memory_list *entry)
{
	if(allocated_memory_list == NULL)
	{
		JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory list is null!!"); 
		return -1;
	}

	if((entry->prev == NULL) && (entry->next == NULL))
	{
		allocated_memory_list = NULL;
	}
	else if(entry->prev == NULL)
	{
		entry->next->prev = NULL;
		allocated_memory_list = entry->next;
	}
	else if(entry->next == NULL)
	{
		entry->prev->next = NULL;
	}
	else
	{
		entry->prev->next = entry->next;
		entry->next->prev = entry->prev;
	}

	free((void *)entry);
	alloc_count--;

	JCString_DebugLog( __FILE__, __LINE__ , "Success to remove the list entry of the allocated memory!!");
	JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory count is %d", alloc_count);

	return 1;
}
static int get_table_hash(unsigned char *key,  unsigned int keylen, size_t hashtable_size)
{
	static const int multipliter = 251;
	unsigned int h;
	unsigned char *p;
	unsigned char *end;

	h = 0;
	end = (unsigned char *)((unsigned int)key + keylen);

	for(p = key; p < end; p++)
	{
		h = multipliter * h + *p;
	}

	return h % hashtable_size;
}
static void JCString_DebugLog(char file[], int line, char format[], ...)
{
	va_list ap;
	va_start(ap, format);

	printf("FILE=%s: LINE=%d:", file, line);
	printf("MSG=");
	vprintf(format, ap);
	printf("\n");

	va_end(ap);

	return ;
}
size_t JCString_StrByteLen(const char *p, JCString_Each string_each_func)
{
	unsigned char *it = NULL;
	unsigned char *end = NULL;
	
	it = (unsigned char *)p;	

	for(end = it; (it = (unsigned char *)string_each_func(it, NULL)) != NULL; end = it);

	return end - (unsigned char *)p;
}
void * JCString_Malloc(size_t size, char file[], int line)
{
	void * p = NULL;
	JCString_allocated_memory_list *entry = NULL;

	if(allocated_memory_hash == NULL)
	{
		allocated_memory_hash = (JCString_allocated_memory_hash *)malloc(sizeof(JCString_allocated_memory_hash) * MEMADDR_HASH_SIZE);
		
		if(allocated_memory_hash == NULL)
		{
			return NULL;
		}

		alloc_count++;
		memset((void *)allocated_memory_hash, 0x00, sizeof(JCString_allocated_memory_hash) * MEMADDR_HASH_SIZE);
		
		JCString_DebugLog( __FILE__, __LINE__ , "Success to allocate memory to allocated memory hash table!!");
		JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory count is %d", alloc_count);
	}

	entry = (JCString_allocated_memory_list *)malloc(sizeof(JCString_allocated_memory_list));

	if(entry == NULL)
	{
		JCString_DebugLog( __FILE__, __LINE__ , "Failed to allocate memory of entries in the list of allocated memory!!");
		return NULL;
	}

	alloc_count++;
	JCString_DebugLog( __FILE__, __LINE__ , "Success to allocate memory to allocated memory list entry!!");
	JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory count is %d", alloc_count);

	memset((void *)entry, 0x00, sizeof(JCString_allocated_memory_list));

	p = malloc(size);

	if(p == NULL)
	{
		JCString_DebugLog(file, line, "Memory allocate failed!!");
		return NULL;
	}
	
	if(set_memaddr_hash(p, entry) == -1)
	{
		free(p);
		return NULL;
	}

	entry->p = p;
	add_memaddr_list(entry);

	alloc_count++;

	JCString_DebugLog(file, line, "Memory allocate success");
	JCString_DebugLog(file, line, "Allocated memory count is %d", alloc_count);

	return p;
}
int JCString_Free(void *p, char file[], int line)
{
	JCString_allocated_memory_list *entry = NULL;
	JCString_allocated_memory_hash *hashentry = NULL;
	int hash = 0;

	hash = get_memaddr_table_hash(p);
	hashentry = &allocated_memory_hash[hash];

	if(hashentry->p != p)
	{
		while(hashentry->p != p)
		{
			if(hashentry->next == NULL)
			{
				JCString_DebugLog(file, line, "Is not the memory allocated by JCString...(memory address %p)...", p);
				return -1;
			}
			hashentry = hashentry->next;
		}
	}
	
	entry = hashentry->listentry;

	if(remove_memaddr_list(entry) == -1)
	{
		JCString_DebugLog( __FILE__, __LINE__, "Allocated memory list entry remove failed!!");
	}

	if(remove_memaddr_hash(p) == -1)
	{
		JCString_DebugLog( __FILE__, __LINE__, "Allocated memory hash entry remove failed!!");
	}

	free(p);

	alloc_count--;
	
	JCString_DebugLog(file, line, "Memory free success");
	JCString_DebugLog(file, line, "Allocated memory count is %d", alloc_count);

	return 1;
}
int JCString_Realloc(void **p, size_t size, char file[], int line)
{
	void *tmp;
	JCString_allocated_memory_list *entry = NULL;
	JCString_allocated_memory_hash *hashentry = NULL;
	int hash = 0;

	hash = get_memaddr_table_hash(*p);
	hashentry = &allocated_memory_hash[hash];

	if(hashentry->p != *p)
	{
		while(hashentry->p != *p)
		{
			if(hashentry->next == NULL)
			{
				JCString_DebugLog(file, line, "Is not the memory allocated by JCString...(memory address %p)...", p);
				return -1;
			}
			hashentry = hashentry->next;
		}
	}
	
	tmp = realloc(*p, size);

	if(tmp == NULL)
	{
		JCString_DebugLog(file, line, "Memory allocate failed!!");
		return -1;
	}

	entry = hashentry->listentry;

	if(remove_memaddr_hash(*p) == -1)
	{
		JCString_DebugLog( __FILE__, __LINE__, "Allocated memory hash entry remove failed!!");
		free(tmp);
		return -1;
	}

	if(set_memaddr_hash(tmp, entry) == -1)
	{
		free(tmp);
		return -1;
	}

	*p = tmp;
	entry->p = *p;

	JCString_DebugLog(file, line, "Memory reallocate success");
	
	return 1;
}
unsigned char *JCString_GetHashValue(JCString_conv_table_hash *hashtable, size_t hashtable_size, unsigned char *key, int keylen, 
	JCString_Each string_each_func)
{
	int hashkey = 0;
	JCString_conv_table_hash *hashentry = NULL;

	hashkey = get_table_hash(key, keylen, hashtable_size);
	hashentry = &hashtable[hashkey];
	
	if(hashentry->key== NULL)
	{
		return NULL;
	}

	while((keylen != JCString_StrByteLen(hashentry->key, string_each_func)) || memcmp((const char *)key, (const char *)hashentry->key, keylen) != 0)
	{
		if(hashentry->next == NULL)
		{
			return NULL;
		}

		hashentry = hashentry->next;
	}
	
	return hashentry->val;
}
JCString_conv_table_hash *JCString_GenConvTableHash(const JCString_conv_table table[], size_t table_size, size_t hashtable_size,
	JCString_Each string_each_func)
{
	int count = 0;
	int i=0;
	char *key = NULL;
	char *val = NULL;
	unsigned int len = 0;
	int hash = 0;
	JCString_conv_table_hash *hashentry = NULL;

	JCString_conv_table_hash *hashtable = NULL;
	hashtable = (JCString_conv_table_hash *)JCString_Malloc(hashtable_size * sizeof(JCString_conv_table_hash), __FILE__, __LINE__ );
	memset(hashtable, 0x00, sizeof(JCString_conv_table_hash) * hashtable_size);

	count = table_size / sizeof(JCString_conv_table);

	for(i=0; i < count; i++)
	{
		key = (char *)&(table[i].key[0]);
		val = (char *)&(table[i].val[0]);

		len = JCString_StrByteLen(key, string_each_func);

		hash = get_table_hash((unsigned char *)key, len, hashtable_size);

		hashentry = &hashtable[hash];

		if(hashentry->key != NULL)
		{
			while(hashentry->next != NULL)
			{
				hashentry = hashentry->next;
			}	
			hashentry->next = (JCString_conv_table_hash *)JCString_Malloc(sizeof(JCString_conv_table_hash), __FILE__, __LINE__ );
			hashentry = hashentry->next;
		}

		hashentry->key = key;
		hashentry->val = val;
		hashentry->next = NULL;
	}

	return hashtable;
}
JCString_conv_table_hash *JCString_GenConvInvertedTableHash(const JCString_conv_table table[], size_t table_size, size_t hashtable_size,
	JCString_Each string_each_func)
{
	int count = 0;
	int i=0;
	char *key = NULL;
	char *val = NULL;
	unsigned int len = 0;
	int hash = 0;
	JCString_conv_table_hash *hashentry = NULL;

	JCString_conv_table_hash *hashtable = NULL;
	hashtable = (JCString_conv_table_hash *)JCString_Malloc(hashtable_size * sizeof(JCString_conv_table_hash), __FILE__, __LINE__ );
	memset(hashtable, 0x00, sizeof(JCString_conv_table_hash) * hashtable_size);

	count = table_size / sizeof(JCString_conv_table);

	for(i=0; i < count; i++)
	{
		key = (char *)&(table[i].key[0]);
		val = (char *)&(table[i].val[0]);

		/* 本関数では、キーと値を逆にしてハッシュを生成するため、val（値）の長さを求める。*/
		len = JCString_StrByteLen(val, string_each_func);

		/* 本関数では、キーと値を逆にしてハッシュを生成するため、val（値）を元にハッシュを生成する。 */
		hash = get_table_hash((unsigned char *)val, len, hashtable_size);

		hashentry = &hashtable[hash];

		if(hashentry->key != NULL)
		{
			while(hashentry->next != NULL)
			{
				hashentry = hashentry->next;
			}	
			hashentry->next = (JCString_conv_table_hash *)JCString_Malloc(sizeof(JCString_conv_table_hash), __FILE__, __LINE__ );
			hashentry = hashentry->next;
		}

		/* 本関数では、キーと値を逆にしてハッシュを生成するため、val（値）とkey（キー）を逆にしてハッシュに設定する。 */
		hashentry->key = val;
		hashentry->val = key;
		hashentry->next = NULL;
	}

	return hashtable;
}
JCString_String JCString_ConvEncoding(JCString_String str, 
	JCString_Each string_each_func, JCString_ConvertEncode convfunc, JCString_IsEnd_String isstrend_func)
{
	int len = 0;
	unsigned char *p = NULL;
	unsigned char *end = NULL;
	JCString_exec_info info;
	JCString_String result;
	memset(&result, 0x00, sizeof(JCString_String));
	result.use_length = JCSTRING_TRUE;

	memset(&info, 0x00, sizeof(info));

	p = (unsigned char*)str.value;

	if(str.use_length == JCSTRING_FALSE)
	{
		if(isstrend_func(p) == JCSTRING_TRUE)
		{
			return result;
		}

		len = JCString_StrByteLen((const char *)str.value, string_each_func) * 2;
		end = (p + (JCString_StrByteLen((const char *)str.value, string_each_func))) - 1;
	}
	else if(str.use_length != JCSTRING_TRUE)
	{
		return result;
	}
	else if(str.length > 0)
	{
		end = p + str.length - 1;
		len = str.length * 2;
	}
	else
	{
		return result;
	}
	
	if(len <= 0)
	{
		return result;
	}

	info.data.convert_data.buff = (unsigned char *)JCString_Malloc(len, __FILE__, __LINE__ );
	info.data.convert_data.size = len;
	memset(info.data.convert_data.buff, 0x00, len);

	if(info.data.convert_data.buff == NULL)
	{
		JCString_DebugLog(__FILE__, __LINE__, "memory allocate failed!!");
		return str;
	}

	memset(info.data.convert_data.buff, 0x00, len);

	do
	{
		info.data.convert_data.count += convfunc(p, end, &info);

		if(info.header.exit == 1)
		{
			return result;
		}
	}while((p = (unsigned char*)string_each_func(p, end)) != NULL);

	info.data.convert_data.buff[info.data.convert_data.count] = '\0';
	result.length = info.data.convert_data.count;
	result.value = (char *)info.data.convert_data.buff;

	return result;
}
int JCString_Release()
{
	int i=0;
	int ret=0;

	release_memaddr_hash();
	release_memaddr_list();
	free(allocated_memory_hash);
	alloc_count--;

	JCString_DebugLog( __FILE__, __LINE__ , "Success to remove the hash table of the allocated memory!!");
	JCString_DebugLog( __FILE__, __LINE__ , "Allocated memory count is %d", alloc_count);		

	return 1;
}
char * JCString_FileRead(FILE *fp)
{
	char * buff;
	size_t size = 0;

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );

	buff = (char *)JCString_Malloc(size + 1, __FILE__, __LINE__ );
	rewind(fp);
	fread(buff, sizeof(char), size, fp);
	fclose(fp);
	buff[size] = '\0';

	return buff;
}

