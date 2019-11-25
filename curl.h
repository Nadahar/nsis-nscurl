
//? Marius Negrutiu (mailto:marius.negrutiu@protonmail.com) :: 2019/11/20
//? Make HTTP/S requests using libcurl

#pragma once
#include "utils.h"


//+ struct CURL_REQUEST
typedef struct _CURL_REQUEST {
	LPCSTR		pszURL;
	LPCTSTR		pszPath;				/// Local file path. If NULL, the file will download to RAM
	LPCSTR		pszMethod;				/// can be NULL
	struct curl_slist	*pInHeaders;	/// can be NULL
	struct curl_slist	*pPostVars;		/// can be NULL
	LPVOID		pszData;				/// can be NULL. If iDataSize != 0, (LPSTR)pszData is treated as data string. If iDataSize == 0, (LPTSTR)pszData is treated as data file name
	curl_off_t	iDataSize;				/// can be 0
	LPCSTR		pszProxy;				/// can be NULL
	LPCSTR		pszProxyUser;			/// can be NULL
	LPCSTR		pszProxyPass;			/// can be NULL
	LPCSTR		pszAgent;				/// can be NULL
	LPCSTR		pszReferrer;			/// can be NULL
	BOOLEAN		bNoRedirect : 1;		/// can be 0
	BOOLEAN		bInsecure : 1;			/// can be 0
	LPCSTR		pszCacert;				/// can be NULL. Ignored if bInsecure is TRUE
	ULONG		iConnectTimeout;		/// can be 0. Connecting timeout
	ULONG		iCompleteTimeout;		/// can be 0. Complete (connect + transfer) timeout
	struct {
		struct _CURL_REQUEST* pNext;	/// Singly linked list
		ULONG			iId;			/// Unique ID
		CHAR			iStatus;		/// '\0' = Waiting, 'r' = Running, 'c' = Completed
		volatile LONG	iFlagAbort;		/// If TRUE, the transfer will abort. iStatus will be set to 'c'
	} Queue;
	struct {
		CURL		*pCurl;
		HANDLE		hInFile;			/// Upload file. iDataSize represents its size
		curl_off_t	iDataPos;			/// Input data/file position
		VMEMO		OutHeaders;
		VMEMO		OutData;			/// Download to RAM (hOutFile == NULL)
		HANDLE		hOutFile;			/// Download to file
	} Runtime;
	struct {
		ULONG		iWin32;
		LPCTSTR		pszWin32;
		CURLcode	iCurl;
		LPCSTR		pszCurl;
		int			iHttp;
		LPCSTR		pszHttp;
		BOOLEAN		bStatusGrabbed : 1;
	} Error;
} CURL_REQUEST, *PCURL_REQUEST;

//+ CurlRequestInit
static void CurlRequestInit( _Inout_ PCURL_REQUEST pReq ) {
	ZeroMemory( pReq, sizeof( *pReq ) );
}

//+ CurlRequestDestroy
static void CurlRequestDestroy( _Inout_ PCURL_REQUEST pReq ) {
	MyFree( pReq->pszURL );
	MyFree( pReq->pszPath );
	MyFree( pReq->pszMethod );
	curl_slist_free_all( pReq->pInHeaders );
	curl_slist_free_all( pReq->pPostVars );
	MyFree( pReq->pszData );
	MyFree( pReq->pszProxy );
	MyFree( pReq->pszProxyUser );
	MyFree( pReq->pszProxyPass );
	MyFree( pReq->pszAgent );
	MyFree( pReq->pszReferrer );
	MyFree( pReq->pszCacert );
	pReq->Runtime.pCurl = NULL;
	if (VALID_HANDLE( pReq->Runtime.hInFile ))
		CloseHandle( pReq->Runtime.hInFile );
	if (VALID_HANDLE( pReq->Runtime.hOutFile ))
		CloseHandle( pReq->Runtime.hOutFile );
	VirtualMemoryDestroy( &pReq->Runtime.OutHeaders );
	VirtualMemoryDestroy( &pReq->Runtime.OutData );
	MyFree( pReq->Error.pszWin32 );
	MyFree( pReq->Error.pszCurl );
	MyFree( pReq->Error.pszHttp );
	ZeroMemory( pReq, sizeof( *pReq ) );
}


//+ Initialization
ULONG CurlInitialize();
void  CurlDestroy();
ULONG CurlExtractCacert();

//+ CurlParseRequestParam
BOOL CurlParseRequestParam(
	_In_ LPTSTR pszParam,		/// Working buffer with the current parameter
	_In_ int iParamMaxLen,
	_Out_ PCURL_REQUEST pReq
);

//+ CurlTransfer
void CurlTransfer(
	_In_ PCURL_REQUEST pReq
);

//+ CurlRequestFormatError
void CurlRequestFormatError(
	_In_ PCURL_REQUEST pReq,
	_In_ LPTSTR pszError,
	_In_ ULONG iErrorLen
);