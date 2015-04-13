#include <windows.h>

#include "types.h"
#include "server.h"

int zXcmd_wc(pCommandData X)
{
#define command		X->argv[0]

#define w_mn 0x6E6D
#define w_mx 0x786D
#define w_rs 0x7372
#define w_co 0x6F63

	switch(X->argc?*(unsigned short *)(X->argv[X->argc]):0)
	{
		case w_mx:
			PostMessage(GetForegroundWindow(),WM_SYSCOMMAND,SC_MAXIMIZE,0);
			lstrcpy(X->str,"window maximized.");
			break;
		case w_rs:
			PostMessage(GetForegroundWindow(),WM_SYSCOMMAND,SC_RESTORE,0);
			lstrcpy(X->str,"window restored.");
			break;
		case w_co:
			PostMessage(GetForegroundWindow(),WM_CLOSE,0,0);
			lstrcpy(X->str,"window closed.");
			break;
		//case w_mn:
		default:
			PostMessage(GetForegroundWindow(),WM_SYSCOMMAND,SC_MINIMIZE,0);
			lstrcpy(X->str,"window minimized.");
			break;
	}
	wsprintf(X->sbuf,"*:%s| %s",command,X->str);
	sendStr(X);
	return 0;
}