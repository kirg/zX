#ifndef macros_h
#define macros_h

#include "defines.h"

#define fileMonRunning  FindWindow("FileMonClass",0) /* if filemon running! */
#define regMonRunning   FindWindow("RegMonClass",0)	 /* if regmon running! */

#ifdef noSilentCrash
#define onErr(M)  {MessageBeep(~0);MessageBox(GetForegroundWindow(),M,"please report this error!",MB_ICONERROR);}
#else
#define onErr(M)  {;}
#endif

#define thread(func,arg)  CreateThread(0,0,(LPTHREAD_START_ROUTINE)&func,(void *)arg,0,&threadID)

#define emb(M)			MessageBox(0,M,"error!",MB_ICONERROR)
#define imb(M)			{MessageBeep(~0);MessageBox(GetForegroundWindow(),M,"info!",MB_ICONINFORMATION);}
#define IMB(M)			MessageBox(GetForegroundWindow(),M,"info!",MB_ICONINFORMATION)
//#define _(M)			  MessageBox(0,#M,"test message!",MB_ICONINFORMATION)

#define ip2a(ip)		_inet_ntoa(*((struct in_addr *)&(ip)))
//#define sendStr(s,buf)	_send(s,buf,lstrlen(buf),0)

//#define connectionAlive	(!IsBadReadPtr(X->C,SizeConnectionData) && X->C->id==X->id)

//#define chk4break		if (WaitForSingleObject(X->evSTOP,0)==WAIT_TIMEOUT)\
//							{lstrcpy(X->sbuf,"^C");sendStr(X,X->sbuf);return ~0;}

#endif