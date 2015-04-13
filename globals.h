#ifndef globals_h
#define globals_h

#include "types.h"

extern HWND hW;
extern char g_buf[];

extern DWORD threadID;

extern HMODULE hADVAPI32_DLL;
extern HMODULE hWSOCK32_DLL;

extern p_RegOpenKeyEx _RegOpenKeyEx;
extern p_RegQueryValueEx _RegQueryValueEx;
extern p_RegSetValueEx _RegSetValueEx;
extern p_RegCloseKey _RegCloseKey;


extern int isWinNT;
extern int isWin98;/*compatible;ie atleast Win98 (not Win95), maybe WinMe*/
extern int runSafe;/*run safe ...*/

extern HOSTENT *h_this;
extern char g_hostName[];

extern unsigned int I;

extern SOCKET sL;

extern CRITICAL_SECTION g_cs_SetCurrentDirectory;

extern char *memConnectionData;
extern char *memCommandData;

extern pConnectionData C[];
extern pCommandData X[];

extern unsigned int connectionTimeOut;
extern unsigned int maxNumOfCommandThreads;
extern unsigned int maxNumOfConnections;
extern unsigned int numOfConnections;

extern struct timeval tv;

extern p_accept          _accept;
extern p_bind            _bind;
extern p_closesocket     _closesocket;
extern p_connect         _connect;
extern p_htonl           _htonl;
extern p_htons           _htons;
extern p_inet_addr       _inet_addr;
extern p_inet_ntoa       _inet_ntoa;
extern p_listen          _listen;
extern p_ntohl           _ntohl;
/*extern p_ntohs         _ntohs;*/
extern p_recv            _recv;
extern p_send            _send;
extern p_select          _select;
extern p_socket          _socket;
/*extern p_gethostbyaddr _gethostbyaddr;*/
extern p_gethostbyname   _gethostbyname;
extern p_gethostname     _gethostname;
extern p_getpeername     _getpeername;
extern p_getsockname     _getsockname;
extern p_WSAStartup      _WSAStartup;
extern p_WSACleanup      _WSACleanup;
extern p_WSAGetLastError _WSAGetLastError;
extern p_WSAAsyncSelect  _WSAAsyncSelect;

extern const char Month[][4];
extern const char Day[][4];

#endif
