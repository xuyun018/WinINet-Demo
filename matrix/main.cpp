#include <Windows.h>
#include <tchar.h>

#include <stdio.h>

#include "XYWinINet.h"

//---------------------------------------------------------------------------
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
//---------------------------------------------------------------------------
unsigned char *ReadFileBuffer(const TCHAR *filename, unsigned int *filesize)
{
	HANDLE hfile;
	DWORD numberofbytes;
	unsigned char *result = NULL;

	hfile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		*filesize = GetFileSize(hfile, NULL);

		result = (unsigned char *)MALLOC(*filesize);
		if (result != NULL)
		{
			ReadFile(hfile, result, *filesize, &numberofbytes, NULL);
		}

		CloseHandle(hfile);
	}
	return(result);
}

int _tmain(int argc, WCHAR *argv[])
{
	unsigned char *buffer;
	unsigned char *postbuffer;
	unsigned int buffersize;
	unsigned int postsize;
	unsigned int l;
	DWORD receivestatus = 0;

	postbuffer = ReadFileBuffer(L"C:\\Bin\\ssss.txt", &postsize);
	if (postbuffer)
	{
		buffersize = 1024 * 1024;
		buffer = (unsigned char *)MALLOC(buffersize);
		if (buffer)
		{
			l = HttpConversation(NULL, NULL, L"POST", L"https://wbslot-fd.scwanding.com/jsfpk/Login", NULL, NULL, NULL, &receivestatus, buffer, buffersize, postbuffer, postsize, 1);

			printf("%s\r\n", buffer);

			FREE(buffer);
		}
		printf("%d, %d\r\n", l, receivestatus);

		FREE(postbuffer);
	}
	getchar();

	return(0);
}