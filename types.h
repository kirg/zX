#ifndef types_h
#define types_h

#include <winsock.h> /* for 'SOCKET',etc */

#include "defines.h"

#define u_short unsigned short
#define u_long  unsigned long
#define u_int   unsigned int

/////////// CONNECTION DATA STRUCTURE ///////////
#pragma pack(1)
typedef struct{
  u_int     connType;
  SOCKET    S;  /* the conn socket */
	u_short   N;  /* conn's N-value: index in ConnectionData structure */
  u_int     n;  /* conn id: GetTickCount() at connection accept to identify conn
                 *  for 'teln' connections/

/* the following are defined in (and required by) 'telnet' connections only */
  union{
    struct{
      u_long      	    cmdLen;
      char			        cmdStr[MaxCmdLen];
      char			        prevCmdStr[MaxCmdLen];
      u_int			        echo;
      u_int             ip;
      u_short       	  pt;
      char			        currentFile[MaxPath];
      CRITICAL_SECTION	cs_SetCurrentFile;//must initialize
      char			        currentDirectory[MaxPath];
      CRITICAL_SECTION  cs_SetCurrentDirectory;//must initialize
      char			        previousDirectory[MaxPath];
      char			        curRegKey[MaxPath];
      char			        prevRegKey[MaxPath];
      char			        prompt[MaxPromptLength];
      u_int       	    numCommands;/*currently running*/
      u_short	          fgX_N; /* n-value of the foreground command */
      u_short	          X_N[MaxNumOfCommandThreadsPerConnection];/* n-value: command indices in the CommandData array; */
      u_long		        threadWaitTimeout;/*time to wait for the thread to exit, after 'break'ing*/
      u_long		        aliasCmd[MaxNumOfAliases];
      char			        aliasStr[MaxAliasedCommandLength][MaxNumOfAliases];
    };
    char  httpBuf[HttpBufSize];
    char  connBuf[ConnBufSize];
  };
} ConnectionData, *pConnectionData;
#pragma pack()
/////////////////////////////////////////


#define MaxNumOfCmdParams 	6//+1 (for arv[0])

/////////// COMMAND DATA STRUCTURE //////
#pragma pack(1)
typedef struct{
	HANDLE			    hT;/*handle to the thread executing the command*/
	u_short	        N;/*index in the global cmd[] array*/
  u_short	        i;/*index in the connection's C->n[] array*/

  pConnectionData C;/*pointer to the command's 'parent' connection*/
	u_short	        C_N;/*index of C in C[] array; connection id when combined with the 'id' field*/
	u_int	          C_n;/*=GetTickCount() at connection accept; connection id*/

	u_int           time;/*may be used for obtaining exec times*/
	u_int           ncmd;/*the command integer, the first 'significant' (upto four) bytes*/
	char            cmd[MaxCmdLen];
	u_int           cmdlen;
	char            argvBuf[MaxCmdLen];
	char            argc;
	char            *argv[1+MaxNumOfCmdParams];
	u_short         argvlen[1+MaxNumOfCmdParams];

	HANDLE          evPAUSE;
	HANDLE    			evSTOP;
	char			      sbuf[zXbufSize];
	char			      currentFile[MaxPath];
	char			      currentDirectory[MaxPath];
	
	char			      str[MaxPath];	//300 bytes for temporary use
	char			      itoa0[20];		//20 bytes for temporary use
	char			      itoa1[20];		//20 bytes for temporary use
	char			      _reserved[3];	//space filler; round to pageSize=4k
} CommandData,*pCommandData;
#pragma pack()
/////////////////////////////////////////


typedef SOCKET    (__stdcall *p_accept)		  (SOCKET,struct sockaddr FAR *,int FAR *);
typedef int       (__stdcall *p_bind)			  (SOCKET,const struct sockaddr FAR *,int);
typedef int       (__stdcall *p_closesocket)(SOCKET);
typedef int       (__stdcall *p_connect)    (SOCKET,const struct sockaddr FAR *,int);
typedef u_long    (__stdcall *p_htonl)      (u_long);
typedef u_short   (__stdcall *p_htons)      (u_short);
typedef u_long    (__stdcall *p_inet_addr)	(const char FAR *);
typedef char FAR *(__stdcall *p_inet_ntoa)	(struct in_addr);
typedef int       (__stdcall *p_listen)     (SOCKET, int);
typedef u_long    (__stdcall *p_ntohl)      (u_long);
typedef u_short   (__stdcall *p_ntohs)      (u_short);
typedef	u_short   (__stdcall *p_htons)      (u_short);
typedef int       (__stdcall *p_recv)       (SOCKET,char FAR *,int,int);
typedef int       (__stdcall *p_send)       (SOCKET,const char FAR *,int,int);
typedef	long      (__stdcall *p_select)     (int,fd_set FAR *,fd_set FAR *,fd_set FAR *,const struct timeval FAR *);
typedef SOCKET    (__stdcall *p_socket)     (int,int,int);
typedef struct hostent FAR *	(__stdcall *p_gethostbyaddr)	(const char FAR *,int,int);
typedef struct hostent FAR *	(__stdcall *p_gethostbyname)	(const char FAR *);
typedef int       (__stdcall *p_gethostname)(char FAR *,int);
typedef int       (__stdcall *p_getpeername)(SOCKET,struct sockaddr FAR *,int FAR *);
typedef int       (__stdcall *p_getsockname)(SOCKET,struct sockaddr FAR *,int FAR *);
typedef int       (__stdcall *p_WSAStartup) (WORD,LPWSADATA);
typedef int       (__stdcall *p_WSACleanup) ();
typedef int       (__stdcall *p_WSAGetLastError)();
typedef int       (__stdcall *p_WSAAsyncSelect) (SOCKET,HWND,u_int,long);

typedef LONG      (__stdcall *p_RegOpenKeyEx)   (HKEY,LPCTSTR,DWORD,REGSAM,PHKEY);
typedef LONG      (__stdcall *p_RegCloseKey)    (HKEY);
typedef LONG      (__stdcall *p_RegSetValueEx)  (HKEY,LPCTSTR,DWORD,DWORD,CONST BYTE *,DWORD);
typedef	LONG      (__stdcall *p_RegQueryValueEx)(HKEY,LPTSTR,LPDWORD,LPDWORD,LPBYTE,LPDWORD);

typedef DWORD     (__stdcall *p_RasEnumConnections)   (LPRASCONN,LPDWORD,LPDWORD);
typedef DWORD     (__stdcall *p_RasGetConnectStatus)  (HRASCONN,LPRASCONNSTATUS);
typedef DWORD     (__stdcall *p_WNetOpenEnum)         (DWORD,DWORD,DWORD,LPNETRESOURCE,LPHANDLE);
typedef DWORD     (__stdcall *p_WNetEnumResource)     (HANDLE,LPDWORD,LPVOID,LPDWORD);
typedef DWORD     (__stdcall *p_WNetCloseEnum)        (HANDLE);
typedef BOOL      (__stdcall *p_InternetGetConnectedState)  (LPDWORD,DWORD);

#pragma pack(1)
typedef struct{
  HWND              hW;
  u_int             isWinNT;
  u_int             isWin98;
  SOCKET            sL;
  u_int             numOfConnections;
  u_int             maxNumOfConnections;
  pConnectionData   C[MaxNumOfConnections];
  u_int             maxNumOfCommandThreads;
  pCommandData      X[MaxNumOfCommandThreads];

  HMODULE           hADVAPI32_DLL;
  p_RegOpenKeyEx    _RegOpenKeyEx;
  p_RegQueryValueEx _RegQueryValueEx;
  p_RegSetValueEx   _RegSetValueEx;
  p_RegCloseKey	    _RegCloseKey;

  HMODULE           hWSOCK32_DLL;
  p_accept          _accept;
  p_bind            _bind;
  p_closesocket     _closesocket;
  p_connect         _connect;
  p_htonl           _htonl;
  p_htons           _htons;
  p_inet_addr       _inet_addr;
  p_inet_ntoa       _inet_ntoa;
  p_listen          _listen;
  p_ntohl           _ntohl;
  /*p_ntohs         _ntohs;*/
  p_recv            _recv;
  p_send            _send;
  p_select          _select;
  p_socket          _socket;
  /*p_gethostbyaddr _gethostbyaddr;*/
  p_gethostbyname   _gethostbyname;
  p_gethostname     _gethostname;
  p_getpeername     _getpeername;
  p_getsockname     _getsockname;
  p_WSAStartup      _WSAStartup;
  p_WSACleanup      _WSACleanup;
  p_WSAGetLastError _WSAGetLastError;
  p_WSAAsyncSelect  _WSAAsyncSelect;
} ServerData, *pServerData;
#pragma pack()

#endif