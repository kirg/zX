#include <windows.h>
#include "defines.h"
#include "macros.h"
#include "types.h"
#include "globals.h"
#include "misc.h"

/*
int fputs(pConnectionData C,char *str){
	fd_set fds;
	int l_str;
	int attempts=50;

	l_str=lstrlen(str);
retry:
	if (_send(C->S,str,strlen,0)==SOCKET_ERROR){
		if (_WSAGetLastError()==WSAEWOULDBLOCK{//if wsock buffer full, retry
			FD_ZERO(&fds);
			FD_SET(C->S,&fds);
			_select(0,0,&fds,0,&tv);//wait for some time to see if block removed
			if (--attempts)
				goto retry;
		}
		return 0;//error!
	}
	return ~0;//done!
}
*/
int sendStr(pCommandData X)
{
	fd_set fds;
	int msglen;
	int attempts=50;
	char *msg=X->sbuf;

	msglen=lstrlen(msg);
	msg[msglen++]='\r';/*cat a CRLF*/
	msg[msglen++]='\n';
retry:
	if (_send(X->C->S,msg,msglen,0)==SOCKET_ERROR)
	{
		if (_WSAGetLastError()==WSAEWOULDBLOCK)//if wsock buffer full, retry ...
		{
			FD_ZERO(&fds);
			FD_SET(X->C->S,&fds);
			_select(0,0,&fds,0,&tv);//wait for some time to see if block removed
			if (--attempts)
				goto retry;
		}
		return 0;//error!
	}
	return ~0;//done!
}

int chk4break(pCommandData X)
{
	if (WaitForSingleObject(X->evSTOP,0)==WAIT_TIMEOUT){
    lstrcpy(X->sbuf,"^C");
    sendStr(X);
    return 1;
  }
	else
		return 0;
}

void closeConnection(int i){
  HANDLE hProc,hEv;
  HANDLE hT[MaxNumOfCommandThreadsPerConnection];
  int m=0,n=0;
  if (C[i]->S!=INVALID_SOCKET){
    _closesocket(C[i]->S);
    C[i]->S=INVALID_SOCKET;/*mark as unused*/
  }
  if (C[i]->connType==ConnType_teln){
    if (C[i]->numCommands){
      while(m<MaxNumOfCommandThreadsPerConnection){
        if (C[i]->X_N[m]!=0xffff && !IsBadReadPtr(X[C[i]->X_N[m]],SizeCommandData)){
          hT[n++]=X[C[i]->X_N[m]]->hT;/*C[i]->thread[m];*//*prepare handle array for the 'wait'*/
          DuplicateHandle(hProc,X[C[i]->X_N[m]]->evSTOP,hProc=GetCurrentProcess(),&hEv,0,0,DUPLICATE_SAME_ACCESS);
          ResetEvent(hEv);/*reset (non-signalled state)*/
          CloseHandle(hEv);
        }
        ++m;
      }
      if (n && WaitForMultipleObjects(n,hT,1,C[i]->threadWaitTimeout)==WAIT_TIMEOUT){
        while(n--){
				  MessageBeep(~0);
				  TerminateThread(hT[n],0);
				  CloseHandle(hT[n]);//hT[n]=0;
			  }
		  }
	  }
	  DeleteCriticalSection(&C[i]->cs_SetCurrentFile);
	  DeleteCriticalSection(&C[i]->cs_SetCurrentDirectory);
  }
	VirtualFree(C[i],SizeConnectionData,MEM_DECOMMIT);C[i]=0;
	--numOfConnections;
}

void closeAllConnections(char *msg){
	int i,j=0;
	if (numOfConnections){

closeTheConnections:
		for (i=0;i<MaxNumOfConnections;++i){
    /*close the sockets in the first round(j==0); free up the connections in the second round(j!=0)*/
			if (C[i]){
				if (!j){/*first round; close sockets*/
					_send(C[i]->S,msg,lstrlen(msg),0);
					_closesocket(C[i]->S);
          C[i]->S=INVALID_SOCKET;
				}
				else/*second roundl free up the connections*/
					closeConnection(i);//thread(closeConnection,i);
			}
		}
		if (!j){
			j=~0;
			goto closeTheConnections;
		}
	}
}

HMODULE hWSOCK32_DLL;

int zXstart(){ /* start server */
  WSADATA       wsaStartupData;
  SOCKADDR_IN   saL;
/*SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);*/
  if (!(hWSOCK32_DLL=LoadLibrary("wsock32.dll")))
    onErr("error! LoadLibrary(..) of wsock32.dll");
  if (!(_accept		=(p_accept)				GetProcAddress(hWSOCK32_DLL,"accept"))
      || !(_bind		=(p_bind)				GetProcAddress(hWSOCK32_DLL,"bind"))
      || !(_closesocket	=(p_closesocket)		GetProcAddress(hWSOCK32_DLL,"closesocket"))
      || !(_connect		=(p_connect)			GetProcAddress(hWSOCK32_DLL,"connect"))
      || !(_htonl		=(p_htonl)				GetProcAddress(hWSOCK32_DLL,"htonl"))
      || !(_htons		=(p_htons)				GetProcAddress(hWSOCK32_DLL,"htons"))
      || !(_inet_addr	=(p_inet_addr)			GetProcAddress(hWSOCK32_DLL,"inet_addr"))
      || !(_inet_ntoa	=(p_inet_ntoa)			GetProcAddress(hWSOCK32_DLL,"inet_ntoa"))
      || !(_listen		=(p_listen)				GetProcAddress(hWSOCK32_DLL,"listen"))
      || !(_ntohl		=(p_ntohl)				GetProcAddress(hWSOCK32_DLL,"ntohl"))
//    || !(_ntohs		=(p_ntohs)				GetProcAddress(hWSOCK32_DLL,"ntohs"))
      || !(_recv		=(p_recv)				GetProcAddress(hWSOCK32_DLL,"recv"))
      || !(_send		=(p_send)				GetProcAddress(hWSOCK32_DLL,"send"))
      || !(_select		=(p_select)				GetProcAddress(hWSOCK32_DLL,"select"))
      || !(_socket		=(p_socket)				GetProcAddress(hWSOCK32_DLL,"socket"))
//    || !(_gethostbyaddr=(p_gethostbyaddr)		GetProcAddress(hWSOCK32_DLL,"gethostbyaddr"))
      || !(_gethostbyname=(p_gethostbyname)		GetProcAddress(hWSOCK32_DLL,"gethostbyname"))
      || !(_gethostname	=(p_gethostname)		GetProcAddress(hWSOCK32_DLL,"gethostname"))
      || !(_getpeername	=(p_getpeername)		GetProcAddress(hWSOCK32_DLL,"getpeername"))
      || !(_getsockname	=(p_getsockname)		GetProcAddress(hWSOCK32_DLL,"getsockname"))
      || !(_WSAStartup	=(p_WSAStartup)			GetProcAddress(hWSOCK32_DLL,"WSAStartup"))
      || !(_WSACleanup	=(p_WSACleanup)			GetProcAddress(hWSOCK32_DLL,"WSACleanup"))
      || !(_WSAGetLastError=(p_WSAGetLastError)	GetProcAddress(hWSOCK32_DLL,"WSAGetLastError"))
      || !(_WSAAsyncSelect=(p_WSAAsyncSelect)	GetProcAddress(hWSOCK32_DLL,"WSAAsyncSelect")))
		onErr("error! GetProcAddress(..) in wsock32.dll");
  InitializeCriticalSection(&g_cs_SetCurrentDirectory);
  memConnectionData=VirtualAlloc(0,MaxNumOfConnections*SizeConnectionData,MEM_RESERVE,PAGE_READWRITE);
  memCommandData=VirtualAlloc(0,MaxNumOfCommandThreads*SizeCommandData,MEM_RESERVE,PAGE_READWRITE);
  numOfConnections=0;
  maxNumOfConnections=MaxNumOfConnections;//leave a small gap; so that if the init key is root and pass is provided this connection is allocated
  maxNumOfCommandThreads=MaxNumOfCommandThreads;
  connectionTimeOut=ConnectionTimeOut;
  fillMemory(C,MaxNumOfConnections*sizeof(pConnectionData),0);/*for (i=0;i<MaxNumOfConnections;++i) C[i]=0;*/
  if (_WSAStartup(0x101,&wsaStartupData))
    onErr("WSAStartup() error!");
  sL=_socket(AF_INET,SOCK_STREAM,0);
  saL.sin_family=AF_INET;
  saL.sin_addr.s_addr=INADDR_ANY;
  saL.sin_port=_htons(zXport);
  if (_bind(sL,(SOCKADDR *)&saL,sizeof(saL)))
    onErr("error! bind(..)");
  if (_listen(sL,5))
    onErr("error! listen(..)");
  _WSAAsyncSelect(sL,hW,zXmsg_newConn,FD_ACCEPT);
  return sL;
}

int zXstop(char *msg){  /* stop server */
  _closesocket(sL);sL=INVALID_SOCKET; /* stop listening for connections */
  closeAllConnections(msg); /* MUST make sure all connections are closed */
  VirtualFree(memCommandData,0,MEM_RELEASE);
  VirtualFree(memConnectionData,0,MEM_RELEASE);
  DeleteCriticalSection(&g_cs_SetCurrentDirectory);
  _WSACleanup();
  FreeLibrary(hWSOCK32_DLL);
/*SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);*/
  return 0;
}


void zXshutDown(char *msg){	/* shutdown server */
  zXstop(msg);
/*DestroyWindow(hW);*/
/*ExitProcess(0);*/
  PostMessage(hW,WM_QUIT,0,0); /* exit from message loop */
}

int beginCurrentDirectory(pCommandData X){
/* must 'endCurrentDirectory()' if ret_value is non-zero */
	EnterCriticalSection(&g_cs_SetCurrentDirectory);
	if (!SetCurrentDirectory(X->currentDirectory))
		return ~0; /* invalid_directory; must endCurrentDirectory() */
	return 1; /* no_error; must endCurrentDirectory() */
}

void endCurrentDirectory(){
	LeaveCriticalSection(&g_cs_SetCurrentDirectory);
}

unsigned int setCurrentDirectory(pCommandData X,char *directory){ /* 1 if no_error */
	int r=1; /* no_error */
	EnterCriticalSection(&X->C->cs_SetCurrentDirectory);
	if (GetFileAttributes(directory) & FILE_ATTRIBUTE_DIRECTORY)
		lstrcpyn(X->C->currentDirectory,directory,MaxPath);
	else
		r=~0; /* invalid_directory */
	LeaveCriticalSection(&X->C->cs_SetCurrentDirectory);
	return r;
}

unsigned int setCurrentFile(pCommandData X,char *file){ /* 1 if no_error */
	int r=1; /* no_error */
	EnterCriticalSection(&X->C->cs_SetCurrentFile);
	if (~GetFileAttributes(file)) /* should i make sure if it is a file? */
		lstrcpyn(X->C->currentFile,file,MaxPath);
	else
		r=~0; /* invalid_file */
	LeaveCriticalSection(&X->C->cs_SetCurrentFile);
	return r;
}
