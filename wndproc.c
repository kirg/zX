#include <windows.h>
#include <winsock.h>
#include "defines.h"
#include "macros.h"
#include "types.h"
#include "globals.h"
#include "misc.h"
#include "server.h"
#include "zXteln.h"
#include "zXhttp.h"
#include "chk4net.h"
#include "login.h"

SOCKET  s_newConn;
u_int   R;

//char t_s[100];int t_i;
LRESULT CALLBACK zXwndProc(HWND hWl,UINT msg,WPARAM wp,LPARAM lp){
  if (msg>=zXmsg_base){/*'special' message?*/

    if (msg>=zXmsg_socket && msg<=zXmsg_socket_max){/*message from a connection*/
			I=msg-zXmsg_socket;/*I->connection index*/
			switch(WSAGETSELECTEVENT(lp)){
			case FD_CLOSE:
				if (C[I]->connType==ConnType_none)
					KillTimer(hW,msg);
				thread(closeConnection,I);
				break;
			case FD_READ:
        R=_recv(C[I]->S,g_buf,zXbufSize-1,0);
        if (/*!~*/R==SOCKET_ERROR || !R/*socket closed*/)
          return 0;/*ignore ...*/
        g_buf[R]=0;/*null terminate*/
        switch(C[I]->connType){
        case ConnType_teln:
          zXteln(C[I],R,g_buf);
          break;
        case ConnType_data:
          //zXdata(C[I],R,g_buf);
          break;
        case ConnType_http:
          zXhttp(C[I],R,g_buf);
          break;
        case ConnType_none:
          //for(t_i=0;t_i<R;++t_i)  _send(C[I]->S,t_s,wsprintf(t_s,"[%.3u]",g_buf[t_i]&0xff),0);
          C[I]->n+=lstrlen(lstrcpyn(C[I]->connBuf+C[I]->n,g_buf,ConnBufSize-C[I]->n));
          if (isConnType_http(C[I]->connBuf)){
            C[I]->connType=ConnType_http;
            zXhttp(C[I],0,C[I]->connBuf);
            //imb("http!");
          }
          else
/*            if (isConnType_data(C[I]->connBuf)){
              C[I]->connType=ConnType_data;
              //zXdata(C[I],0,C[I]->connBuf);
            }
            else*/
              if (isConnType_teln(C[I]->connBuf)){
                C[I]->connType=ConnType_teln;
                zXteln(C[I],0,C[I]->connBuf);
              }
              /*else
                  C[I]->connType=ConnType_none;*/
          if (C[I]->connType!=ConnType_none)
            KillTimer(hW,msg);
          /*break;*/
				}
        /*break;*/
      }
      return 0;
		}

    if (msg==zXmsg_newConn && WSAGETSELECTEVENT(lp)==FD_ACCEPT){/* new connection(?) */
			if (/*!~*/(s_newConn=_accept(sL,0,0))==INVALID_SOCKET)
					return 0;
      for(I=0;I<maxNumOfConnections;++I)/* search for a free connection 'slot' */{
				if (!C[I]/*if a free connection 'slot' is found*/){
					/* allocate ('commit')memory for the connection's data */
					if (C[I]=(pConnectionData)VirtualAlloc(memConnectionData+I*SizeConnectionData,SizeConnectionData,MEM_COMMIT,PAGE_READWRITE)){
						++numOfConnections;/* global_count of the number of current connections */
						_WSAAsyncSelect(s_newConn,hW,zXmsg_socket+I,FD_READ|FD_CLOSE); /* allocate a 'unique' message */
            C[I]->connType=ConnType_none;
						C[I]->S=s_newConn;  /* connection socket */
						C[I]->N=I;          /* the connection index */
						C[I]->n=0;          /* length of string in 'connBuf' */
						SetTimer(hW,zXmsg_socket+I,connectionTimeOut,0);
              /* if the connType is not identified within the 'connectionTimeOut' period,
               *  the connection is terminated */
						return 0;
					}
					//wsprintf(g_buf,"#%.2u: could not allocate memory !\r\n",numOfConnections);
					goto sendErrMessage;
				}
			}
			//wsprintf(g_buf,"#%.2u: connection buffer full ! please try after sometime ...\r\n",numOfConnections);
sendErrMessage:
      /* [BUG?] someone unaware of the server would also receive this message! */
			//_send(s_newConn,g_buf,lstrlen(g_buf),0);
			_closesocket(s_newConn);
			return 0;
		}

    if (msg==zXmsg_server){
			switch(wp){
				case zXmsg_getVersion:
					return zXver_num;
          /*break;*/
				case zXmsg_svrShutdown:
					lstrcpy(g_buf,"\r\n#: bye! server shutdown ...\r\n");
					thread(zXshutDown,g_buf);
					/*break;*/
			}
			return 0;
		}
	}

  if (msg==WM_TIMER){
    switch(wp){
    case zXtimer_checkForNet:
#ifndef noCheck4Connection
      if (!(I=checkForNet()) && ~sL/*sL!=INVALID_SOCKET*/){
        zXstop(0);
        loginStatus=0;
      }
      else
        !~I && /*!~*/sL==INVALID_SOCKET && ~zXstart() && thread(zXlogin,0);
          /* [todo] decrease the chk4net timer delay when connected */
#else
      if (checkForNet()/* if online ... */){
        if (!loginStatus) /* if not logged in ... */
          thread(zXlogin,0);
      }
      else
        loginStatus=0; /* set to zero if disconnected; ie not logged in */
#endif
      break;
    case zXtimer_relogin:
      thread(zXlogin,0);
      break;
    default:
      if (wp>=zXmsg_socket && wp<=zXmsg_socket_max){
        KillTimer(hW,wp);
        if (C[I=wp-zXmsg_socket]->connType==ConnType_none)
          closeConnection(I);
      }
    }
    return 0;
  }

  if (msg==WM_COPYDATA && wp==zXmsg_newVersion){
    wsprintf(g_buf,"\r\n#: installation of %s/%.8X in progress ...\r\n",
      ((COPYDATASTRUCT *)lp)->lpData,((COPYDATASTRUCT *)lp)->dwData);
    thread(zXshutDown,g_buf);
    return 0;
  }

  return DefWindowProc(hWl,msg,wp,lp);
}
