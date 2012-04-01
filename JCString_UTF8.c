#include "JCString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static JCString_conv_table_hash *utf8_to_sjiswin_hashtable = NULL;
static size_t utf8_to_sjiswin_hashtable_size = 0;
static int string_charsize(unsigned char *p, unsigned char *end, int mode);
static char *string_each(unsigned char *p, unsigned char *end, int *mode);
static int encconv_sjis_win_to_utf8(unsigned char *p, unsigned char *end, JCString_exec_info *info, int mode);

static int string_charsize(unsigned char *p, unsigned char *end, int mode)
{
	if((*p >= 0x00 && *p <= 0x2E) || (*p >= 0x30 && *p <= 0x7F))
	{
		return 1;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && 
		(*p >= 0xC0 && *p <= 0xDF) && (*(p+1) >= 0x80 && *(p+1) <= 0xBF) )
	{
		return 2;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && 
		(*p >= 0xE0 && *p <= 0xEF) && (*(p+1) >= 0x80 && *(p+1) <= 0xBF) && (*(p+2) >= 0x80 && *(p+2) <= 0xBF) )
	{
		return 3;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) &&
		(*p >= 0xF0 && *p <= 0xF7) && (*(p+1) >= 0x80 && *(p+1) <= 0xBF) && 
		(*(p+2) >= 0x80 && *(p+2) <= 0xBF) && (*(p+3) >= 0x80 && *(p+3) <= 0xBF))
	{
		return 4;
	}

	return 0;
}
static JCSTRING_BOOL isend_string(unsigned char *p)
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
static char *string_each(unsigned char *p, unsigned char *end, int *mode)
{
	if( ((end != NULL) && ((p + string_charsize(p, end, *mode)) > end)) || ((end == NULL) && (isend_string(p) == JCSTRING_TRUE)) )
	{
		return NULL;
	}

	if((*p >= 0x00 && *p <= 0x2E) || (*p >= 0x30 && *p <= 0x7F))
	{
		p++;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && 
		(*p >= 0xC0 && *p <= 0xDF) && (*(p+1) >= 0x80 && *(p+1) <= 0xBF) )
	{
		p+=2;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && 
		(*p >= 0xE0 && *p <= 0xEF) && (*(p+1) >= 0x80 && *(p+1) <= 0xBF) && (*(p+2) >= 0x80 && *(p+2) <= 0xBF) )
	{
		p+=3;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) &&
		(*p >= 0xF0 && *p <= 0xF7) && (*(p+1) >= 0x80 && *(p+1) <= 0xBF) && 
		(*(p+2) >= 0x80 && *(p+2) <= 0xBF) && (*(p+3) >= 0x80 && *(p+3) <= 0xBF))
	{
		p+=4;
	}
	else
	{
		p++;
	}

	return (char *)p;
}
static int encconv_sjis_win_to_utf8(unsigned char *p, unsigned char *end, JCString_exec_info *info, int mode)
{
	unsigned char *convvalue = NULL;
	int len=0;
	int i=0;
	int count=0;

	if(info->header.exit == 1)
	{
		return 0;
	}

	if(string_charsize(p, end, mode) == 0)
	{
		return 0;
	}

	if( ((*p >= 0x00 && *p <= 0x5B) || (*p >= 0x5D && *p <= 0x7D)) || (string_charsize(p, end, mode) == 0) )
	{
		count = 1;

		if((info->data.convert_data.count + count) > info->data.convert_data.size)
		{
			if(JCString_Realloc((void **)(&info->data.convert_data.buff), info->data.convert_data.size * 2, __FILE__, __LINE__ ) != JCSTRING_SUCCESS)
			{
				info->header.exit = 1;
				info->header.err = JCSTRING_ERR_BAD_ALLOC;
				return 0;
			}
			info->data.convert_data.size = info->data.convert_data.size * 2;
		}

		info->data.convert_data.buff[info->data.convert_data.count] = *p;
		return count;
	}

	utf8_to_sjiswin_hashtable_size = ((JCString_sjis_to_utf8_conv_table_size / sizeof(JCString_conv_table)) * 5);

	if(utf8_to_sjiswin_hashtable == NULL)
	{
		utf8_to_sjiswin_hashtable = JCString_GenConvInvertedTableHash(JCString_sjis_to_utf8_conv_table, JCString_sjis_to_utf8_conv_table_size, utf8_to_sjiswin_hashtable_size,
			string_each);
	}

	len = string_charsize(p, end, mode);

	convvalue = JCString_GetHashValue(utf8_to_sjiswin_hashtable, utf8_to_sjiswin_hashtable_size, p, len, JCString_Get_SJISEach());
	
	
	if(convvalue == NULL)
	{
		for(i=0; i < len; i++)
		{
			count += 4;

			if((info->data.convert_data.count + count) > info->data.convert_data.size)
			{
				if(JCString_Realloc((void **)(&info->data.convert_data.buff), info->data.convert_data.size * 2, __FILE__, __LINE__ ) != JCSTRING_SUCCESS)
				{
					info->header.exit = 1;
					info->header.err = JCSTRING_ERR_BAD_ALLOC;
					return 0;
				}
				info->data.convert_data.size = info->data.convert_data.size * 2;
			}

			sprintf((char *)&(info->data.convert_data.buff[info->data.convert_data.count]), "\\x%02X", *(p+i));
		}
		return count;
	}

	count = JCString_StrByteLen((const char *)convvalue, JCString_Get_SJISEach());

	if((info->data.convert_data.count + count) > info->data.convert_data.size)
	{
		if(JCString_Realloc((void **)(&info->data.convert_data.buff), info->data.convert_data.size * 2, __FILE__, __LINE__ ) != JCSTRING_SUCCESS)
		{
			info->header.exit = 1;
			info->header.err = JCSTRING_ERR_BAD_ALLOC;
			return 0;
		}
		info->data.convert_data.size = info->data.convert_data.size * 2;
	}

	strncpy((char *)&(info->data.convert_data.buff[info->data.convert_data.count]), (char *)convvalue, count);

	return count;
}
JCString_String JCString_UTF8ToSJISWin(JCString_String str, JCSTRING_ERR *err)
{
	return JCString_ConvEncodingCommon(str, string_each, encconv_sjis_win_to_utf8, isend_string, err);
}
JCString_Each JCString_Get_UTF8Each()
{
	return string_each;
}
JCString_IsEnd_String JCString_Get_UTF8IsEndString()
{
	return isend_string;
}
