#include "JCString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static JCSTRING_BOOL isend_string(unsigned char *p);
static int string_charsize(unsigned char *p, unsigned char *end, int mode);
static JCSTRING_BOOL isescape_sequence(unsigned char *p, unsigned char *end);
static int changemode(unsigned char *p, unsigned char *end, int mode);
static char *string_each(unsigned char *p, unsigned char *end, int *mode);
static JCSTRING_BOOL isend_string(unsigned char *p);
static int encconv_jis_to_sjis(unsigned char *p, unsigned char *end, JCString_exec_info *info, int mode);

static int string_charsize(unsigned char *p, unsigned char *end, int mode)
{
	int newmode = mode;

	if(mode == JCSTRING_CHAR_DEFAULT)
	{
		newmode = changemode(p, end, mode);

		if(newmode != mode)
		{
			return 0;
		}
		else if((*p >= 0x00 && *p <= 0x1F) || (*p == 0x7F))
		{
			return 1;
		}
	}
	else if((mode == JCSTRING_CHAR_ASCII) || (mode == JCSTRING_CHAR_ROMAN))
	{
		newmode = changemode(p, end, mode);

		if(newmode != mode)
		{
			return 0;
		}
		else if(*p >= 0x20 && *p <= 0x7E)
		{
			return 1;
		}
	}
	else if(mode == JCSTRING_CHAR_KANA)
	{
		newmode = changemode(p, end, mode);

		if(newmode != mode)
		{
			return 0;
		}
		else if(*p >= 0x21 && *p <= 0x5F)
		{
			return 1;
		}
	}
	else if((mode == JCSTRING_CHAR_KANJIOLD) || (mode == JCSTRING_CHAR_KANJINEW))
	{
		newmode = changemode(p, end, mode);

		if(newmode != mode)
		{
			return 0;
		}
		else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && ((*p >= 0x21 && *p <= 0x7E) && (*(p+1) >= 0x21 && *(p+1) <= 0x7E) ) )
		{
			return 2;
		}
	}

	return 0;
}
static JCSTRING_BOOL isescape_sequence(unsigned char *p, unsigned char *end)
{
	if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x28 && *(p+2) == 0x42) )
	{
		return JCSTRING_TRUE;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x28 && *(p+2) == 0x4A) )
	{
		return JCSTRING_TRUE;
	}	
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x28 && *(p+2) == 0x49) )
	{
		return JCSTRING_TRUE;
	}	
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x40) )
	{
		return JCSTRING_TRUE;
	}	
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x42) )
	{
		return JCSTRING_TRUE;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+5) <= end))) && (*p == 0x1B && *(p+1) == 0x26 && *(p+2) == 0x40 && 
		*(p+3) == 0x1B && *(p+4) == 0x24 && *(p+5) == 0x42) )
	{
		return JCSTRING_TRUE;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x28 && 
		*(p+3) == 0x44) )
	{
		return JCSTRING_TRUE;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x28 && 
		*(p+3) == 0x4F) )
	{
		return JCSTRING_TRUE;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x28 && 
		*(p+3) == 0x51) )
	{
		return JCSTRING_TRUE;
	}else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x28 && 
		*(p+3) == 0x50) )
	{
		return JCSTRING_TRUE;
	}

	return JCSTRING_FALSE;
}
static int changemode(unsigned char *p, unsigned char *end, int mode)
{
	if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x28 && *(p+2) == 0x42) )
	{
		mode = JCSTRING_CHAR_ASCII;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x28 && *(p+2) == 0x4A) )
	{
		mode = JCSTRING_CHAR_ROMAN;
	}	
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x28 && *(p+2) == 0x49) )
	{
		mode = JCSTRING_CHAR_KANA;
	}	
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x40) )
	{
		mode = JCSTRING_CHAR_KANJIOLD;
	}	
	else if( ((end == NULL) || ((end != NULL) && ((p+2) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x42) )
	{
		mode = JCSTRING_CHAR_KANJINEW;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+5) <= end))) && (*p == 0x1B && *(p+1) == 0x26 && *(p+2) == 0x40 && 
		*(p+3) == 0x1B && *(p+4) == 0x24 && *(p+5) == 0x42) )
	{
		mode = JCSTRING_CHAR_KANJI_JIS_X_0208_1990;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x28 && 
		*(p+3) == 0x44) )
	{
		mode = JCSTRING_CHAR_KANJI_JIS_X_0212_1990;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x28 && 
		*(p+3) == 0x4F) )
	{
		mode = JCSTRING_CHAR_KANJI_JIS_X_0213_2000_1;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x28 && 
		*(p+3) == 0x51) )
	{
		mode = JCSTRING_CHAR_KANJI_JIS_X_0213_2004_1;
	}else if( ((end == NULL) || ((end != NULL) && ((p+3) <= end))) && (*p == 0x1B && *(p+1) == 0x24 && *(p+2) == 0x28 && 
		*(p+3) == 0x50) )
	{
		mode = JCSTRING_CHAR_KANJI_JIS_X_0213_2000_2;
	}

	return mode;
}
static char *string_each(unsigned char *p, unsigned char *end, int *mode)
{
	int newmode = *mode;

	if( ((end != NULL) && ((p + string_charsize(p, end, *mode)) > end)) || ((end == NULL) && (isend_string(p) == JCSTRING_TRUE)) )
	{
		return NULL;
	}

	if(*mode == JCSTRING_CHAR_DEFAULT)
	{
		newmode = changemode(p, end, *mode);

		if(newmode != *mode)
		{
			*mode = newmode;

			switch(newmode)
			{
				case JCSTRING_CHAR_ASCII:
				case JCSTRING_CHAR_ROMAN:
				case JCSTRING_CHAR_KANA:
				case JCSTRING_CHAR_KANJIOLD:
				case JCSTRING_CHAR_KANJINEW:
					p+=3;
					break;
				case JCSTRING_CHAR_KANJI_JIS_X_0208_1990:
					p+=6;
					break;
				case JCSTRING_CHAR_KANJI_JIS_X_0212_1990:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2000_1:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2004_1:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2000_2:
					p+=4;
					break;
			}
		}
		else if((*p >= 0x00 && *p <= 0x1F) || (*p == 0x7F))
		{
			p++;
		}
		else
		{
			p++;
		}
	}
	else if((*mode == JCSTRING_CHAR_ASCII) || (*mode == JCSTRING_CHAR_ROMAN))
	{
		newmode = changemode(p, end, *mode);

		if(newmode != *mode)
		{
			*mode = newmode;
			
			switch(newmode)
			{
				case JCSTRING_CHAR_ASCII:
				case JCSTRING_CHAR_ROMAN:
				case JCSTRING_CHAR_KANA:
				case JCSTRING_CHAR_KANJIOLD:
				case JCSTRING_CHAR_KANJINEW:
					p+=3;
					break;
				case JCSTRING_CHAR_KANJI_JIS_X_0208_1990:
					p+=6;
					break;
				case JCSTRING_CHAR_KANJI_JIS_X_0212_1990:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2000_1:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2004_1:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2000_2:
					p+=4;
					break;
			}
		}
		else if(*p >= 0x20 && *p <= 0x7E)
		{
			p++;
		}
		else
		{
			p++;
		}
	}
	else if(*mode == JCSTRING_CHAR_KANA)
	{
		newmode = changemode(p, end, *mode);

		if(newmode != *mode)
		{
			*mode = newmode;

			switch(newmode)
			{
				case JCSTRING_CHAR_ASCII:
				case JCSTRING_CHAR_ROMAN:
				case JCSTRING_CHAR_KANA:
				case JCSTRING_CHAR_KANJIOLD:
				case JCSTRING_CHAR_KANJINEW:
					p+=3;
					break;
				case JCSTRING_CHAR_KANJI_JIS_X_0208_1990:
					p+=6;
					break;
				case JCSTRING_CHAR_KANJI_JIS_X_0212_1990:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2000_1:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2004_1:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2000_2:
					p+=4;
					break;
			}
		}
		else if(*p >= 0x21 && *p <= 0x5F)
		{
			p++;
		}
		else
		{
			p++;
		}
	}
	else if((*mode == JCSTRING_CHAR_KANJIOLD) || (*mode == JCSTRING_CHAR_KANJINEW) || (*mode == JCSTRING_CHAR_KANJI_JIS_X_0208_1990) ||
		(*mode == JCSTRING_CHAR_KANJI_JIS_X_0212_1990) || (*mode == JCSTRING_CHAR_KANJI_JIS_X_0213_2000_1) || 
		(*mode == JCSTRING_CHAR_KANJI_JIS_X_0213_2004_1) || (*mode == JCSTRING_CHAR_KANJI_JIS_X_0213_2000_2))
	{
		newmode = changemode(p, end, *mode);

		if(newmode != *mode)
		{
			*mode = newmode;

			switch(newmode)
			{
				case JCSTRING_CHAR_ASCII:
				case JCSTRING_CHAR_ROMAN:
				case JCSTRING_CHAR_KANA:
				case JCSTRING_CHAR_KANJIOLD:
				case JCSTRING_CHAR_KANJINEW:
					p+=3;
					break;
				case JCSTRING_CHAR_KANJI_JIS_X_0208_1990:
					p+=6;
					break;
				case JCSTRING_CHAR_KANJI_JIS_X_0212_1990:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2000_1:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2004_1:
				case JCSTRING_CHAR_KANJI_JIS_X_0213_2000_2:
					p+=4;
					break;
			}
		}
		else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && ((*p >= 0x21 && *p <= 0x7E) && (*(p+1) >= 0x21 && *(p+1) <= 0x7E) ) )
		{
			p+=2;
		}
		else
		{
			p++;
		}
	}

	return (char *)p;
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
static int encconv_jis_to_sjis(unsigned char *p, unsigned char *end, JCString_exec_info *info, int mode)
{
	int count=0;
	unsigned char c1,c2;

	if(info->header.exit == 1)
	{
		return 0;
	}

	if(isescape_sequence(p, end) == JCSTRING_TRUE)
	{
		return 0;
	}

	if( ((mode == JCSTRING_CHAR_DEFAULT) || (mode == JCSTRING_CHAR_ASCII) || (mode == JCSTRING_CHAR_ROMAN))
		&& (!(*p >= 0x00 && *p <= 0x7F)) )
	{
		return 0;
	}
	else if( (mode == JCSTRING_CHAR_KANA) && (!(*p >= 0x21 && *p <= 0x5F)) )
	{
		return 0;
	}

	if((mode == JCSTRING_CHAR_DEFAULT) || (mode == JCSTRING_CHAR_ASCII) || 
		(mode == JCSTRING_CHAR_ROMAN) || (mode == JCSTRING_CHAR_KANA))
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
	else if( ((mode == JCSTRING_CHAR_KANJIOLD) || (mode == JCSTRING_CHAR_KANJINEW) || (mode == JCSTRING_CHAR_KANJI_JIS_X_0208_1990)) &&
		((end == NULL) || ((end != NULL) && ((p+1) <= end))) && ((*p >= 0x21 && *p <= 0x7E) && (*(p+1) >= 0x21 && *(p+1) <= 0x7E) ) )
	{
		count = 2;

		c1 = *p;
		c2 = *(p+1);
		if (c1 % 2) {
			c1 = ((c1 + 1) / 2) + 0x70;
			c2 = c2 + 0x1f;
		} else {
			c1 = (c1 / 2) + 0x70;
			c2 = c2 + 0x7d;
		}
		if (c1 >= 0xa0) { c1 = c1 + 0x40; }
		if (c2 >= 0x7f) { c2 = c2 + 1; }
	}
	else
	{
		return 0;
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

	info->data.convert_data.buff[info->data.convert_data.count] = c1;
	info->data.convert_data.buff[info->data.convert_data.count+1] = c2; 

	return count;
}
JCString_String JCString_JISToSJIS(JCString_String str, JCSTRING_ERR *err)
{
	return JCString_ConvEncodingCommon(str, string_each, encconv_jis_to_sjis, isend_string, err);
}
JCString_Each JCString_Get_JISEach()
{
	return string_each;
}