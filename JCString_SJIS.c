#include "JCString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static JCString_conv_table_hash *sjis_to_utf8_hashtable = NULL;
static size_t sjis_to_utf8_hashtable_size = 0;

static char *sjis_each(unsigned char *p);
static int sjis_charsize(unsigned char *p);
static int encconv_sjis_to_utf8(unsigned char *p, JCString_exec_info *info);
static enum JCSTRING_BOOL isend_string(unsigned char *p);

static int sjis_charsize(unsigned char *p)
{
	if((*p >= 0x00 && *p <= 0x7F) || (*p >= 0xA1 && *p <= 0xDF))
	{
		return 1;
	}
	else if((*p >= 0x81 && *p <= 0x9F) || (*p >= 0xE0 && *p <= 0xFC))
	{
		if((*(p+1) >= 0x40 && *(p+1) <= 0x7E) || (*(p+1) >= 0x80 && *(p+1) <= 0xFC))
		{
			return 2;
		}
	}

	return 0;
}
static char *sjis_each(unsigned char *p)
{
	if(isend_string(p) == JCSTRING_TRUE)
	{
		return NULL;
	}

	if((*p >= 0x00 && *p <= 0x7F) || (*p >= 0xA1 && *p <= 0xDF))
	{
		p++;
	}
	else if((*p >= 0x81 && *p <= 0x9F) || (*p >= 0xE0 && *p <= 0xFC))
	{
		if((*(p+1) >= 0x40 && *(p+1) <= 0x7E) || (*(p+1) >= 0x80 && *(p+1) <= 0xFC))
		{
			p+=2;
		}
	}
	else
	{
		p++;
	}

	return (char *)p;
}
static int encconv_sjis_to_utf8(unsigned char *p, JCString_exec_info *info)
{
	unsigned char *convvalue = NULL;
	int len=0;
	int i=0;
	int count=0;

	if(info->header.exit == 1)
	{
		return 0;
	}

	if((*p >= 0x00 && *p <= 0x5B) || (*p >= 0x5D && *p <= 0x7D))
	{
		count = 1;

		if((info->data.convert_data.count + count) > info->data.convert_data.size)
		{
			if(JCString_Realloc((void **)(&info->data.convert_data.buff), info->data.convert_data.size * 2, __FILE__, __LINE__ ) == -1)
			{
				info->header.exit = 1;
				return 0;
			}
			info->data.convert_data.size = info->data.convert_data.size * 2;
		}

		info->data.convert_data.buff[info->data.convert_data.count] = *p;
		return count;
	}

	sjis_to_utf8_hashtable_size = ((JCString_sjis_to_utf8_conv_table_size / sizeof(JCString_conv_table)) * 5);

	if(sjis_to_utf8_hashtable == NULL)
	{
		sjis_to_utf8_hashtable = JCString_GenConvTableHash(JCString_sjis_to_utf8_conv_table, JCString_sjis_to_utf8_conv_table_size, sjis_to_utf8_hashtable_size,
			sjis_each);
	}

	len = sjis_charsize(p);

	convvalue = JCString_GetHashValue(sjis_to_utf8_hashtable, sjis_to_utf8_hashtable_size, p, len, JCString_Get_UTF8Each());
	
	
	if(convvalue == NULL)
	{
		for(i=0; i < len; i++)
		{
			count += 4;

			if((info->data.convert_data.count + count) > info->data.convert_data.size)
			{
				if(JCString_Realloc((void **)(&info->data.convert_data.buff), info->data.convert_data.size * 2, __FILE__, __LINE__ ) == -1)
				{
					info->header.exit = 1;
					return 0;
				}
				info->data.convert_data.size = info->data.convert_data.size * 2;
			}

			sprintf((char *)&(info->data.convert_data.buff[info->data.convert_data.count]), "\\x%02X", *(p+i));
		}
		return count;
	}

	count = JCString_StrLen((const char *)convvalue, JCString_Get_UTF8Each());

	if((info->data.convert_data.count + count) > info->data.convert_data.size)
	{
		if(JCString_Realloc((void **)(&info->data.convert_data.buff), info->data.convert_data.size * 2, __FILE__, __LINE__ ) == -1)
		{
			info->header.exit = 1;
			return 0;
		}
		info->data.convert_data.size = info->data.convert_data.size * 2;
	}

	strncpy((char *)&(info->data.convert_data.buff[info->data.convert_data.count]), (char *)convvalue, count);

	return count;
}
static enum JCSTRING_BOOL isend_string(unsigned char *p)
{
	if(*p == '\0')
	{
		return JCSTRING_TRUE;
	}
	else
	{
		return JCSTRING_FALSE;
	}
}
JCSTRING_String JCString_SjisToUTF8(JCSTRING_String str)
{
	return JCString_ConvEncoding(str, sjis_each, encconv_sjis_to_utf8, isend_string);
}

