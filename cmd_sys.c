#include <windows.h>

#include "types.h"
#include "server.h"
#include "globals.h"

int zXcmd_sm(pCommandData X)
{
#define command		X->argv[0]

#define s_sn 0x6E73	//shutdown
#define s_rt 0x7472 //reboot
#define s_sy 0x7973 //standby
#define s_lf 0x666C //log off
#define s_cl 0x6C63 //close all
#define s_mf 0x666D //monitor off
#define s_mn 0x6E6D //monitor on
#define s_ml 0x6C6D //monitor low power
#define s_sr 0x7273 //screen saver
#define s_sm 0x6D73 //start menu

	int ewx=0;
	char op='+';
	unsigned short smCmd;

	smCmd=X->argc?*(unsigned short *)(X->argv[X->argc]):0;
	switch(smCmd)
	{
		case s_sn:
			lstrcpy(X->str,"shutting down system ...");
			ewx=EWX_SHUTDOWN;
			break;
		case s_rt:
			lstrcpy(X->str,"rebooting system ...");
			ewx=EWX_REBOOT;
			break;
		case s_lf:
			lstrcpy(X->str,"logging off user ...");
			ewx=EWX_LOGOFF;
			break;
		case s_sy:
			lstrcpy(X->str,"putting system to standby ...");
			break;
		case s_cl:
			lstrcpy(X->str,"close all unimplemented !");
			break;
		case s_sm:
			op='*';
			DefWindowProc(hW,WM_SYSCOMMAND,SC_TASKLIST,0);
			lstrcpy(X->str,"activated task list ...");
			break;
		case s_ml:
			op='*';
			DefWindowProc(hW,WM_SYSCOMMAND,SC_MONITORPOWER,1);
			lstrcpy(X->str,"put monitor in low power ...");
			break;
		case s_mn:
			op='*';
			DefWindowProc(hW,WM_SYSCOMMAND,SC_MONITORPOWER,~0);
			lstrcpy(X->str,"switched on monitor ...");
			break;
		case s_mf:
			op='*';
			DefWindowProc(hW,WM_SYSCOMMAND,SC_MONITORPOWER,2);
			lstrcpy(X->str,"switched off monitor ...");
			break;
		//case s_sr:
		default:
			op='*';
			DefWindowProc(hW,WM_SYSCOMMAND,SC_SCREENSAVE,0);
			lstrcpy(X->str,"running screensaver ...");
			break;
	}
	wsprintf(X->sbuf,"%c:%s| %s",op,command,X->str);
	sendStr(X);
	switch(smCmd)
	{
		case s_sn:
		case s_rt:
		case s_lf:
			ExitWindowsEx(ewx,0);
			if (smCmd==s_lf)
			{
				wsprintf(X->sbuf,"*:%s| log off successful ...",command);
				sendStr(X);
			}
			break;
		case s_sy:
			SetSystemPowerState(0,1);
			wsprintf(X->sbuf,"*:%s| returned from standby ...",command);
			sendStr(X);
			break;
		//case s_sr:
		//case s_cl:
		//default:
			//break;
	}
	return 0;
}