#include "JCString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static JCString_conv_table_hash *sjiswin_to_utf8_hashtable = NULL;
static size_t sjiswin_to_utf8_hashtable_size = 0;

static char *string_each(unsigned char *p, unsigned char *end, int *mode);
static int string_charsize(unsigned char *p, unsigned char *end, int mode);
static int encconv_sjis_win_to_utf8(unsigned char *p, unsigned char *end, JCString_exec_info *info, int mode);
static JCSTRING_BOOL isend_string(unsigned char *p);

static int string_charsize(unsigned char *p, unsigned char *end, int mode)
{
	if((*p >= 0x00 && *p <= 0x7F) || (*p >= 0xA1 && *p <= 0xDF))
	{
		return 1;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && (*p >= 0x81 && *p <= 0x9F) || (*p >= 0xE0 && *p <= 0xFC) )
	{
		if((*(p+1) >= 0x40 && *(p+1) <= 0x7E) || (*(p+1) >= 0x80 && *(p+1) <= 0xFC))
		{
			return 2;
		}
	}

	return 0;
}
static char *string_each(unsigned char *p, unsigned char *end, int *mode)
{
	if( ((end != NULL) && (p > end)) || ((end == NULL) && (isend_string(p) == JCSTRING_TRUE)) )
	{
		return NULL;
	}

	if(*p >= 0x00 && *p <= 0x7F)
	{
		*mode = JCSTRING_CHAR_ASCII;
		p++;
	}
	else if(*p >= 0xA1 && *p <= 0xDF)
	{
		*mode = JCSTRING_CHAR_KANA;
		p++;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && ((*p >= 0x81 && *p <= 0x9F) || (*p >= 0xE0 && *p <= 0xFC)) && 
		((*(p+1) >= 0x40 && *(p+1) <= 0x7E) || (*(p+1) >= 0x80 && *(p+1) <= 0xFC)) )
	{
		*mode = JCSTRING_CHAR_KANJINEW;
		p+=2;
	}
	else
	{
		*mode = JCSTRING_CHAR_ASCII;
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

	if( ((*p >= 0x00 && *p <= 0x5B) || (*p >= 0x5D && *p <= 0x7D)) || (*p == 0x7F) || (string_charsize(p, end, mode) == 0) )
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

	sjiswin_to_utf8_hashtable_size = ((JCString_sjis_to_utf8_conv_table_size / sizeof(JCString_conv_table)) * 5);

	if(sjiswin_to_utf8_hashtable == NULL)
	{
		sjiswin_to_utf8_hashtable = JCString_GenConvTableHash(JCString_sjis_to_utf8_conv_table, JCString_sjis_to_utf8_conv_table_size, sjiswin_to_utf8_hashtable_size,
			string_each);
	}

	len = string_charsize(p, end, mode);

	convvalue = JCString_GetHashValue(sjiswin_to_utf8_hashtable, sjiswin_to_utf8_hashtable_size, p, len, JCString_Get_UTF8Each());
	
	
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

	count = JCString_StrByteLen((const char *)convvalue, JCString_Get_UTF8Each());

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
static void conv_sjis_kanji(char c1, char c2, unsigned char buff[])
{
	buff[0] = c1;
	buff[1] = c2;

	if (buff[0] >= 0xe0) { buff[0] = buff[0] - 0x40; }
	if (buff[1] >= 0x80) { buff[1] = buff[1] - 1; }
	if (buff[1] >= 0x9e) {
		buff[0] = (buff[0] - 0x70) * 2;
		buff[1] = buff[1] - 0x7d;
	} else {
		buff[0] = ((buff[0] - 0x70) * 2) - 1;
		buff[1] = buff[1] - 0x1f;
	}
}
static int encconv_sjis_to_jis(unsigned char *p, unsigned char *end, JCString_exec_info *info, int mode)
{
	int count=0;
	unsigned char buff[8];
	memset(buff, 0x00, sizeof(buff));

	if(info->header.exit == 1)
	{
		return 0;
	}

	if(string_charsize(p, end, mode) == 0)
	{
		return 0;
	}

	if((mode == JCSTRING_CHAR_DEFAULT) || (mode == JCSTRING_CHAR_ASCII))
	{
		if( (*p >= 0x00 && *p <= 0x7F) || (string_charsize(p, end, mode) == 0) )
		{
			count = 1;
			buff[0] = *p;
		}
		else if(*p >= 0xA1 && *p <= 0xDF)
		{
			count = 4;
			buff[0] = 0x1B;
			buff[1] = 0x28;
			buff[2] = 0x49;
			buff[3] = *p;
		}
		else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && ((*p >= 0x81 && *p <= 0x9F) || (*p >= 0xE0 && *p <= 0xFC)) && 
			((*(p+1) >= 0x40 && *(p+1) <= 0x7E) || (*(p+1) >= 0x80 && *(p+1) <= 0xFC)) )
		{
			count = 5;
			buff[0] = 0x1B;
			buff[1] = 0x24;
			buff[2] = 0x42;
			conv_sjis_kanji(*p, *(p+1), &buff[3]);
		}
	}
	else if(mode == JCSTRING_CHAR_KANA)
	{
		if(*p >= 0xA1 && *p <= 0xDF)
		{
			count = 1;
			buff[0] = *p;
		}
		else if( (*p >= 0x00 && *p <= 0x7F) || (string_charsize(p, end, mode) == 0) )
		{
			count = 4;
			buff[0] = 0x1B;
			buff[1] = 0x28;
			buff[2] = 0x42;
			buff[3] = *p;
		}
		else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && ((*p >= 0x81 && *p <= 0x9F) || (*p >= 0xE0 && *p <= 0xFC)) && 
			((*(p+1) >= 0x40 && *(p+1) <= 0x7E) || (*(p+1) >= 0x80 && *(p+1) <= 0xFC)) )
		{
			count = 5;
			buff[0] = 0x1B;
			buff[1] = 0x24;
			buff[2] = 0x42;
			conv_sjis_kanji(*p, *(p+1), &buff[3]);
		}
	}
	else if(mode == JCSTRING_CHAR_KANJINEW)
	{
		if( (*p >= 0x00 && *p <= 0x7F) || (string_charsize(p, end, mode) == 0) )
		{
			count = 4;
			buff[0] = 0x1B;
			buff[1] = 0x28;
			buff[2] = 0x42;
			buff[3] = *p;
		}
		else if(*p >= 0xA1 && *p <= 0xDF)
		{
			count = 4;
			buff[0] = 0x1B;
			buff[1] = 0x28;
			buff[2] = 0x49;
			buff[3] = *p;
		}
		else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && ((*p >= 0x81 && *p <= 0x9F) || (*p >= 0xE0 && *p <= 0xFC)) && 
			((*(p+1) >= 0x40 && *(p+1) <= 0x7E) || (*(p+1) >= 0x80 && *(p+1) <= 0xFC)) )
		{
			count = 2;
			conv_sjis_kanji(*p, *(p+1), buff);
		}
	}

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

	memmove(&(info->data.convert_data.buff[info->data.convert_data.count]), buff, count);

	return count;
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
JCString_String JCString_SjisWinToUTF8(JCString_String str, JCSTRING_ERR *err)
{
	JCString_String endmark;
	endmark = JCString_Get_StringEndMark(JCSTRING_ENC_UTF8, err);

	if(*err != JCSTRING_ERR_NONE)
	{
		return str;
	}

	return JCString_ConvEncodingCommon(str, string_each, encconv_sjis_win_to_utf8, isend_string, endmark, err);
}
JCString_String JCString_SjisToJIS(JCString_String str, JCSTRING_ERR *err)
{
	JCString_String endmark;
	endmark = JCString_Get_StringEndMark(JCSTRING_ENC_JIS, err);

	if(*err != JCSTRING_ERR_NONE)
	{
		return str;
	}

	return JCString_ConvEncodingCommon(str, string_each, encconv_sjis_to_jis, isend_string, endmark, err);
}
JCString_Each JCString_Get_SJISEach()
{
	return string_each;
}
JCString_IsEnd_String JCString_Get_SJISIsEndString()
{
	return isend_string;
}

