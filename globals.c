#include "defines.h"
#include "globals.h"

HWND    hW;
char    g_buf[zXbufSize];
//u_int i;
DWORD   threadID;

HMODULE hADVAPI32_DLL;
HMODULE hWSOCK32_DLL;

p_RegOpenKeyEx    _RegOpenKeyEx;
p_RegQueryValueEx _RegQueryValueEx;
p_RegSetValueEx   _RegSetValueEx;
p_RegCloseKey	    _RegCloseKey;


int     isWinNT;
int     isWin98;/*compatible;ie atleast Win98 (not Win95), maybe WinMe*/
int     runSafe;/*run safe ...*/

HOSTENT *h_this;
char    g_hostName[MaxHostNameLen];


u_int   I;

SOCKET            sL = INVALID_SOCKET;/*INVALID_SOCKET indicates not_listening*/

CRITICAL_SECTION	g_cs_SetCurrentDirectory;

char	            *memConnectionData;
char	            *memCommandData;

pConnectionData   C[MaxNumOfConnections];
pCommandData      X[MaxNumOfCommandThreads];

u_int		connectionTimeOut;
u_int		maxNumOfCommandThreads;
u_int		maxNumOfConnections;
u_int		numOfConnections;

#define SendTimeOut	200000/*usecs*//*200ms*/
struct timeval tv={0,SendTimeOut};

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

const char Month[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char Day  [ 7][4]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
