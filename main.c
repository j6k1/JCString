#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "j6string.h"

int main(int argc, char *argv[])
{
	FILE *fp;
	long size;
	size_t retsize = 0;
	char *buff = NULL;
	char *result = NULL;
	
	if(argc < 2)
	{
		printf("ファイル名を指定してください。\n");
		return -1;
	}
	
	fp = fopen(argv[1], "r");
	
	if(!fp)
	{
		printf("ファイル%sを開けませんでした。\n", argv[1]);
		return -1;
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );

	buff = (char *)j6string_malloc(size, __FILE__, __LINE__ );
	rewind(fp);
	fread(buff, sizeof(char), size, fp);
	fclose(fp);

	result = j6string_sjis_to_utf8(buff);
	j6string_free(buff, __FILE__, __LINE__ );

	fp = fopen("C:/download_file/tmp/utf8.txt", "wb");

	if(!fp)
	{
		printf("ファイル%sを開けませんでした。\n", argv[1]);
		return -1;
	}

	retsize = fwrite(result, sizeof(char), strlen(result), fp);
	j6string_free(result, __FILE__, __LINE__ );
	
	if(retsize < strlen(result))
	{
		printf("ファイルへの書き込みでエラーが発生しました。\n");
	}

	fclose(fp);

	j6string_release();

	return 0;
}