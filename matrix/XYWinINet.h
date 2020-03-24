#ifndef XYWININET_H
#define XYWININET_H
//---------------------------------------------------------------------------
#include <Windows.h>
#include <WinInet.h>
#include <iphlpapi.h>

#include <tchar.h>
//---------------------------------------------------------------------------
#pragma comment(lib, "wininet.lib")

#pragma comment(lib, "iphlpapi.lib")
//---------------------------------------------------------------------------
UINT HttpConversation(LPCTSTR agent, LPCTSTR headers, LPCTSTR verb, LPCTSTR url, LPCTSTR proxy, LPCTSTR username, LPCTSTR password, DWORD *receivestatus, BYTE *buffer, DWORD buffersize, const BYTE *postbuffer, UINT postbufferlength, int ssl);
//---------------------------------------------------------------------------
#endif