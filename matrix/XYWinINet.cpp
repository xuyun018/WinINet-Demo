#include "XYWinINet.h"
//---------------------------------------------------------------------------
BOOL ParseURL(const TCHAR *url, TCHAR *host, UINT hostsize, USHORT *port, TCHAR *urlpath, UINT urlpathsize)
{
	TCHAR canonicalizeurl[INTERNET_MAX_URL_LENGTH];
	DWORD canonicalizeurllength = INTERNET_MAX_URL_LENGTH;
	URL_COMPONENTS uc;
	BOOL result;

	// ICU_BROWSER_MODE
	result = InternetCanonicalizeUrl(url, canonicalizeurl, &canonicalizeurllength, ICU_ESCAPE);
	if (result)
	{
		ZeroMemory(&uc, sizeof(uc));

		uc.dwStructSize = sizeof(uc);
		uc.lpszHostName = host;
		uc.dwHostNameLength = hostsize;
		uc.lpszUrlPath = urlpath;
		uc.dwUrlPathLength = urlpathsize;
		result = InternetCrackUrl(canonicalizeurl, canonicalizeurllength, 0, &uc);
		if (result)
		{
			*port = uc.nPort;
		}
	}

	return(result);
}

UINT HttpConversation(LPCTSTR agent, LPCTSTR headers, LPCTSTR verb, LPCTSTR url, LPCTSTR proxy, LPCTSTR username, LPCTSTR password, DWORD *receivestatus, BYTE *buffer, DWORD buffersize, const BYTE *postbuffer, UINT postbufferlength, int ssl)
{
	UINT headerslength;
	HINTERNET hinternet;
	HINTERNET hconnect;
	HINTERNET hrequest;
	DWORD accesstype;
	LPCTSTR lpproxy = NULL;
	LPCTSTR lpproxybypass = NULL;
	DWORD flags;
	TCHAR host[INTERNET_MAX_HOST_NAME_LENGTH];
	USHORT port;
	TCHAR urlpath[INTERNET_MAX_PATH_LENGTH];
	DWORD bufferlength;
	DWORD status = 0;
	DWORD statussize = sizeof(status);
	DWORD contentlength = 0;
	DWORD contentlengthsize = sizeof(contentlength);
	DWORD numberofbytes;
	BOOL httpflag;
	UINT retrycount = 2;
	UINT proxyauthretrycount = 2;
	BOOL forceretry;
	INTERNET_PROXY_INFO ipi;
	DWORD datasize;
	INT errorcode = 0;
	DWORD timeout0 = 9000;
	DWORD timeout1 = 9000;
	UINT result = 0;

	if (receivestatus != NULL)
	{
		*receivestatus = 0;
	}

	httpflag = FALSE;

	// Retrieves the proxy or direct configuration from the registry.
	if (proxy != NULL && proxy[0] != _T('\0'))
	{
		accesstype = INTERNET_OPEN_TYPE_PROXY;

		lpproxy = proxy;
		//lpproxybypass = proxy;

		//OutputDebugValue(proxy, accesstype);
	}
	else
	{
		accesstype = INTERNET_OPEN_TYPE_PRECONFIG;

		//OutputDebugValue(_T("accesstype"), accesstype, INTERNET_OPEN_TYPE_PRECONFIG);
	}
	flags = 0;
	hinternet = InternetOpen(agent, accesstype, lpproxy, lpproxybypass, flags);
	if (hinternet != NULL)
	{
		if (ParseURL(url, host, sizeof(host) / sizeof(TCHAR), &port, urlpath, sizeof(urlpath) / sizeof(TCHAR)))
		{
			if (proxy != NULL && proxy[0] != _T('\0'))
			{
				ipi.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
				ipi.lpszProxy = lpproxy;
				ipi.lpszProxyBypass = lpproxybypass;
				InternetSetOption(hinternet, INTERNET_OPTION_PROXY, &ipi, sizeof(INTERNET_PROXY_INFO));
				//InternetSetOption(hinternet, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);

				_tcscpy(urlpath, url);
			}

			timeout0 = 108000;
			InternetSetOption(hinternet, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &timeout0, sizeof(DWORD));
			timeout1 = 108000;
			InternetSetOption(hinternet, INTERNET_OPTION_CONTROL_SEND_TIMEOUT, &timeout1, sizeof(DWORD));

			hconnect = InternetConnect(hinternet, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 2);
			if (hconnect)
			{
				flags = INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_UI | INTERNET_FLAG_DONT_CACHE;
				if (ssl)
				//if (FALSE && port == 443)
				{
					flags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
					flags |= SECURITY_FLAG_IGNORE_REVOCATION | SECURITY_FLAG_IGNORE_UNKNOWN_CA;
				}
				hrequest = HttpOpenRequest(hconnect, verb, urlpath, NULL, NULL, NULL, flags, NULL);
			
				if (hrequest)
				{
					headerslength = 0;
					if (headers != NULL)
					{
						headerslength = _tcslen(headers);
					}

					do
					{
						forceretry = FALSE;
						if (HttpSendRequest(hrequest, headers, headerslength, (LPVOID)postbuffer, postbufferlength))
						{
							if (HttpQueryInfo(hrequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &status, &statussize, NULL))
							{
								//OutputDebugValue(_T("Get Data"), 6, status);

								if (status == HTTP_STATUS_PROXY_AUTH_REQ)
								{
									//result |= WININET_FLAG_CONNECTED;

									//  Proxy  Authentication  Required
									//  Insert  code  to  set  strUsername  and  strPassword.    
									//  cchUserLength  is  the  length  of  strUsername  and      
									//  cchPasswordLength  is  the  length  of  strPassword.  
									//printf("ÇëÊäÈë´úÀíÓÃ»§Ãû£º");
									//scanf("%s", username);
									//printf("ÇëÊäÈë´úÀíÓÃ»§ÃÜÂë£º");
									//scanf("%s", password);
									//  Insert  code  to  safely  determine  cchUserLength  and    
									//  cchPasswordLength.  Insert  appropriate  error  handling  code.    
									if (username != NULL && username[0] != _T('\0') && password != NULL && password[0] != _T('\0'))
									{
										if (proxyauthretrycount > 0)
										{
											proxyauthretrycount--;

											InternetSetOption(hrequest, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)username, _tcslen(username));
											InternetSetOption(hrequest, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)password, _tcslen(password));
											//InternetSetOption(hrequest, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)username, (_tcslen(username)+1)*sizeof(TCHAR));
											//InternetSetOption(hrequest, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)password, (_tcslen(password)+1)*sizeof(TCHAR));
											//InternetSetOption(hinternet, INTERNET_OPTION_PROXY_SETTINGS_CHANGED, NULL, 0);

											//continue;
											forceretry = TRUE;
										}
									}
								}

								//OutputDebugValue(_T("Return Query Status"), status);

								if (receivestatus != NULL)
								{
									*receivestatus = status;
								}
								if (status == HTTP_STATUS_OK)
								//if (status == HTTP_STATUS_OK || status == HTTP_STATUS_PROXY_AUTH_REQ || status == HTTP_STATUS_DENIED || status == HTTP_STATUS_BAD_REQUEST || status == HTTP_STATUS_FORBIDDEN || status == HTTP_STATUS_NOT_FOUND)
									//if (status != HTTP_STATUS_PROXY_AUTH_REQ && status != HTTP_STATUS_DENIED && status != HTTP_STATUS_BAD_REQUEST && status != HTTP_STATUS_FORBIDDEN && status != HTTP_STATUS_NOT_FOUND)
									//if (status == HTTP_STATUS_OK)
								{
									if (status == HTTP_STATUS_PROXY_AUTH_REQ)
									{
										//SampleCodeOne(psfs, hrequest);

										//result |= WININET_FLAG_CONNECTED;
									}

									httpflag = TRUE;
								}
								else
								{
									//OutputDebugValue(_T("Query Status"), status);
								}
							}
							else
							{
								errorcode = GetLastError();
							}
						}
						else
						{
							errorcode = GetLastError();

							// ÓÐÊ±ºòÒ²¿ÉÄÜÊÇ ERROR_INTERNET_CANNOT_CONNECT Õâ¸ö´íÎó
							//if (errorcode == ERROR_INTERNET_INVALID_CA || errorcode == ERROR_INTERNET_CANNOT_CONNECT || errorcode == 0x80096004 || errorcode == 0x80072F0D || errorcode == ERROR_HTTP_INVALID_SERVER_RESPONSE)
							if (errorcode == ERROR_INTERNET_INVALID_CA || errorcode == ERROR_INTERNET_CANNOT_CONNECT || errorcode == 0x80096004 || errorcode == 0x80072F0D)
							{
								//OutputDebugValue(_T("GETÖØÐÂ´ò¿ª"), errorcode, ERROR_INTERNET_FORCE_RETRY);

								bufferlength = sizeof(flags);

								InternetQueryOption(hrequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&flags, &bufferlength);

								flags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_REVOCATION;
								InternetSetOption(hrequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));

								forceretry = TRUE;
							}
						}
						retrycount--;
					} while (forceretry && retrycount > 0);

					//OutputDebugValue(_T("Get Data"),7,result);

					//MessageBox(NULL, host, urlpath, MB_OK);

					//  Insert  code  to  read  the  data  from  the  hResourceHandle    
					//  at  this  point. 
					//if (error == 0)
					if (httpflag)
					{
						datasize = 0;

						//if (!HttpQueryInfo(hrequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&contentlength, &contentlengthsize, NULL))
						{
							contentlength = 0;
						}
						//printf("ÄÚÈÝ³¤¶È[%d]\n", contentlength);

						//OutputDebugValue(_T("Get Data"),8,contentlength);

						//if (contentlength > 0)
						//if (status == HTTP_STATUS_OK)
						{
							bufferlength = 0;
							while (datasize < buffersize && (httpflag = InternetReadFile(hrequest, buffer + datasize, buffersize - datasize, &bufferlength)) && bufferlength > 0)
							{
								datasize += bufferlength;

								bufferlength = 0;

								//OutputDebugValue(_T("Get Data"),9);
							}

							if (httpflag)
							{
								result = datasize;
							}

							if (!httpflag)
							{
								errorcode = GetLastError();
							}
						}
						//else
						{
							//if (status == HTTP_STATUS_NOT_FOUND)
							{
								//
							}
						}
					}

					InternetCloseHandle(hrequest);
				}
				else
				{
					//printf("HttpOpenRequestÊ§°Ü!\n");
					//error = GetLastError();
				}

				InternetCloseHandle(hconnect);
			}
			else
			{
				//printf("InternetConnectÊ§°Ü!\n");
				//error = GetLastError();
			}
		}
		else
		{
			//
		}

		InternetCloseHandle(hinternet);
	}
	else
	{
	}

	return(result);
}
//---------------------------------------------------------------------------