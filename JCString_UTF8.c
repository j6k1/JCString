#include "JCString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static char *utf8_each(unsigned char *p);

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
static char *utf8_each(unsigned char *p, unsigned char *end)
{
	if( ((end != NULL) && (p >= end)) || ((end == NULL) && (isend_string(p) == JCSTRING_TRUE)) )
	{
		return NULL;
	}

	if((*p >= 0x00 && *p <= 0x2E) || (*p >= 0x30 && *p <= 0x7F))
	{
		p++;
	}
	else if( ((end == NULL) || ((end != NULL) && ((p+1) <= end))) && 
		((end != NULL) && (p+1 <= end)) && (*p >= 0xC0 && *p <= 0xDF) && (*(p+1) >= 0x80 && *(p+1) <= 0xBF) )
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
JCString_Each JCString_Get_UTF8Each()
{
	return utf8_each;
}