#include <windows.h>

#include "types.h"
#include "server.h"
#include "misc.h"

int zXcmd_co(pCommandData X)
{
#define command		X->argv[0]
	int r;
	unsigned int c0=0,c1=0;
	
	if (X->argc)
	{
		for(r=0;X->argv[1][r]!=' ' && X->argv[1][r]!=0 && r<4;++r);
		c0=((*((unsigned int *)X->argv[1]))) & (0xffffffff>>(32-8*r));
		if (X->argc>1)
				c1=*(unsigned int *)(X->argv[2]);
	}
	wsprintf(X->sbuf,"*:%s|%.8X|%.4X| \"%s\"",command,c0,c1,X->cmd+X->argvlen[0]+1);
	sendStr(X);
	return 0;
}

int zXcmd_xo(pCommandData pX)
{
//  int i;
//  for(i=0;i<zXcmdTable_num;++i){
//    wsprintf(X->sbuf,"#%u: %s",i,zXcmdTable_cmd+i);sendStr(X);
//  }
	wsprintf(pX->sbuf,"@%u.%u",pX->C_N,pX->N);sendStr(pX);
  Sleep(1000);
  wsprintf(pX->sbuf,"#%u.%u",pX->C_N,pX->N);sendStr(pX);
	return 0;
}

int zXcmd_Q(pCommandData X)
{
	int r;
	unsigned int c0=0;
	if (X->argc)
	{
		for(r=0;X->argv[1][2+r]!=' ' && X->argv[1][2+r]!=0 && r<4;++r);
		c0=((*((unsigned int *)(X->argv[1])))) & (0xffffffff>>(32-8*r));
	}
	wsprintf(X->sbuf,"#define c_%s\t0x%.8X",X->argv[1],c0);
	sendStr(X);
	return 0;
}

int zXcmd_f1(pCommandData X)
{
	lstrcpy(X->sbuf,"\x1B[0;59;\"ps\";p");
	sendStr(X);
	return 0;
}

int zXcmd_test(pCommandData X)
{
	wsprintf(X->sbuf,"*sizeof(struct ConnectionData)=%u",sizeof(ConnectionData));sendStr(X);
	wsprintf(X->sbuf,"*sizeof(struct CommandData)=%u",sizeof(CommandData));sendStr(X);
  //wsprintf(X->sbuf,"*sizeof(struct telnData)=%u",sizeof(struct telnData));sendStr(X);
	return 0;
}

int zXcmd_t(pCommandData X)
{
	register unsigned int i,j;

	i=(X->argc)?_atoi(X->argv[1]):100;
  j=(X->argc>1)?_atoi(X->argv[2]):100;

  wsprintf(X->sbuf,"i=%u;j=%u",i,j);
	sendStr(X);

	while(i){
		wsprintf(X->sbuf,"%.2u",--i);
		sendStr(X);
		if (chk4break(X))
			return ~0;
		Sleep(j);
	}
	return 0;
}

int zXcmd_ms(pCommandData X)
{
  char temp[100];
  HANDLE hF;

  //GetTempPath(100,temp);
  //*((unsigned short *)(path+i))='\\';
  //lstrcpyn(path+i+1, hostFileName, 128-i-1);
  GetTempPath(100,temp);
  GetTempFileName(temp,"7F8",0,temp);
  wsprintf(X->sbuf,"+ temp=%s",temp);sendStr(X);

  if (hF=CreateFile(temp, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0))
    wsprintf(X->sbuf,"* createfile successful",temp);
  else
    wsprintf(X->sbuf,"- createfile unsuccessful",temp);
  CloseHandle(hF);
  sendStr(X);

//  if ((hF=CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)))
//  goto failed;
	//lstrcpy(X->sbuf,"= command unimplemented ...");sendStr(X);
	return 0;
}
