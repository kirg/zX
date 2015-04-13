#include <windows.h>

#include "types.h"
#include "macros.h"
#include "server.h"
#include "globals.h"
#include "misc.h"

/* file transfer operations: pt ip mc dn up */

#define _zX_      0x5F587A5F
#define dxrCmd_up	0x00007075
#define dxrCmd_dn	0x00006E64

struct pkt_zXhdr{
  DWORD magic;  /* '_zX_'=0x5F587A5F */
  DWORD verReqd;/* versions above 'verReqd' can interpret this command (0=all_can)*/
  DWORD cmd;    /* the command 'up\0\0','dn\0\0',etc */
  WORD  cshLen; /* number of bytes in the command-specific header, that will follow */
  DWORD flags;  /* bit0: a x-header follows; the other bits could be command-specific */
};

/* command-specific header (csh) for Data tXR commands: 'up','dn'*/
struct pkt_zXcsh_dxr{
 	DWORD offset;   /* starting_offset (within the file) */
	WORD  pathLen;  /* path (to file) length */
/*char  path[..];*//* must contain the path with filename */
};

struct pkt_zXret{
  DWORD magic;    /* '_zX_'=0x5F587A5F */
	DWORD cmd;      /* the command (to which 'i' am reponding) */
	DWORD ver;      /* the version of zX (i am)*/
	DWORD retCode;  /*  0	:ready; will continue (open/create file successful)
                   *  1	:command not understood or donot meet version requirements
								   * ~0 :error; (no specific reason; that's it, bye!)
                   */
  DWORD flags;    /* general purpose; maybe convey if file already exists while uploading,etc */
  DWORD xhdrLen;  /* extended header length */
/*char  xhdr[..];*//*the extended header*/
};

struct pkt_zXcsh_dxrData{ /* the 'data format'; data length followed by the data */
  DWORD dataLen;  /* the number of bytes of data (in 'this' chunk) */
  /*char data[..];*/
};

/* just in case, the need comes for an 'upgrade' ... */
/* an outline for a command-specific extended header (csxh) */
/* contains extra information (specific to command and server version) */
struct pkt_zXcsxh{
  DWORD verReqd; /* versions above or equal to 'verReqd' can interpret this information */
  WORD  csxhLen; /* number of bytes (more) in this (x-)header */
  DWORD flags;   /* bit0: a(nother) x-header follows */
  /*char xcsh[..]*/
};

int zXcmd_pt(pCommandData X)
{
#define command		X->argv[0]
	char *param;
	int i;
	if (X->argc && X->argvlen[X->argc])
	{
		param=X->argv[X->argc];
		for(i=0;param[i] && param[i]!=':';++i);
		param[i]=0;
		X->C->ip=_htonl(X->C->ip=_atox(param));
		X->C->pt=_atoi(param+i+1);
		/*if (param[0]==':')
			sscanf(param+1,"%u",&(X->C->pt));
		else
		{
			sscanf(param,"%x:%u",&(X->C->ip),&(X->C->pt));
			X->C->ip=_htonl(X->C->ip);
		}*/
	}
	wsprintf(X->sbuf,"*:%s| %s[:%u] | data transfer addr:port",command,ip2a(X->C->ip),X->C->pt);
	sendStr(X);
	return 0;
}

int zXcmd_ip(pCommandData X)
{
#define command		X->argv[0]
	if (X->argc)
		X->C->ip=_inet_addr(X->argv[X->argc]);
	wsprintf(X->sbuf,"*:%s| %s[:%u] | data transfer addr:port",command,ip2a(X->C->ip),X->C->pt);
	if (!X->argc)
		X->sbuf[0]='-';
	sendStr(X);
	return 0;
}

int zXcmd_mc(pCommandData X)
{
#define command		X->argv[0]
	HOSTENT *heMC;

	if (X->argc)
	{
		heMC=_gethostbyname(X->argv[X->argc]);
		if (heMC)
		{
			X->C->ip=*(unsigned long *)heMC->h_addr_list[0];
			wsprintf(X->sbuf,"*:%s| %s[:%u] | data transfer addr:port",command,ip2a(X->C->ip),X->C->pt);
		}
		else
			wsprintf(X->sbuf,"-:%s:1| could not translate host name \"%s\"",command,X->argv[X->argc]);
	}
	else
		wsprintf(X->sbuf,"-:%s:5| machine name not specified!",command);
	sendStr(X);
	return 0;
}

int zXcmd_dn(pCommandData X)
{
/*	char file[MAX_PATH];
	char sbuf[zXbufSize];
	HANDLE hF;
	unsigned long fileSize,readBytes,totalRead;
	SOCKET sD;
	SOCKADDR_IN saD;
	unsigned long attr;

	if (!X->C->ip)
		lstrcpy(sbuf,"-:dn:8| use \"PT:[iphex]:[port]\" to specify data transfer port");
	else
	{
		if (X->cmdlen>3)
		{
			lstrcpy(file,X->cmd+3);
			if (!~(attr=GetFileAttributes(file)))
				wsprintf(sbuf,"-:dn:1| \"%s\" :file not found",file);
			else
			{
				if (attr & FILE_ATTRIBUTE_DIRECTORY)
					wsprintf(sbuf,"-:dn:2| \"%s\" is a directory !",file);
				else
				{
					if (hF=CreateFile(file,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0))
					{
						SetFilePointer(hF,0,0,FILE_BEGIN);
						fileSize=GetFileSize(hF,0);
						wsprintf(sbuf,"+:dn:0| %d bytes | total file size\r\n",fileSize);sendStr(X->C->S,sbuf);
						sD=_socket(AF_INET,SOCK_STREAM,0);
						saD.sin_family=AF_INET;
						saD.sin_port=_htons(X->C->pt);
						saD.sin_addr.s_addr=X->C->ip;//X->C->ip=_htonl(X->C->ip);//done during PT command.
						if (!_connect(sD,(SOCKADDR *)&saD,sizeof(saD)))
						{
							lstrcpy(sbuf,"+:1| connected; sending data ...\r\n");sendStr(X,sbuf);
							readBytes=dataBufSize;totalRead=0;
							while (readBytes==dataBufSize)
							{
								ReadFile(hF,sbuf,dataBufSize,&readBytes,0);
								totalRead+=_send(sD,sbuf,readBytes,0);
							}
							if (fileSize==totalRead)
								wsprintf(sbuf,"*:dn| %u bytes transferred; file transfer complete!",totalRead);
							else
								wsprintf(sbuf,"-:dn:3|%u off %u bytes transferred; file transfer incomplete!",totalRead,fileSize);
						}
						else
							wsprintf(sbuf,"-:dn:6|could not connect to %s:%u",ip2a(X->C->ip),X->C->pt);
						_closesocket(sD);
						CloseHandle(hF);
					}
					else
						wsprintf(sbuf,"-:dn:0|could not open file \"%s\"",file);
				}
			}
		}
		else
			wsprintf(sbuf,"-:5|file name not specified ...");
	}
*/
	lstrcpy(X->sbuf,"= command unimplemented ...");
	sendStr(X);
	return 0;
}



/*
int zXcmd_dn(pCommandData X)
{
	struct pkt_zX_default	pkt0;
	struct pkt_zX_up		pkt1;
	//struct pkt_zX_up_ret	pktR;
	//struct pkt_zX_up_end	pktE;
#define pktR pkt1
#define pktE pkt1

	char file[MAX_PATH];
	char sbuf[zXbufSize];
	HANDLE hF;
	unsigned long fileSize,readBytes,totalRead;
	SOCKET sD;
	SOCKADDR_IN saD;
	unsigned long attr;
	int i;

	if (X->cmdlen>3)
	{
		lstrcpy(file,X->cmd+3);
		if (!~(attr=GetFileAttributes(file)))
			wsprintf(sbuf,"-:dn:1| \"%s\" :file not found",file);
		else
		{
			if (attr & FILE_ATTRIBUTE_DIRECTORY)
				wsprintf(sbuf,"-:dn:2| \"%s\" is a directory !",file);
			else
			{
				if (hF=CreateFile(file,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0))
				{
					SetFilePointer(hF,0,0,FILE_BEGIN);
					fileSize=GetFileSize(hF,0);
					wsprintf(sbuf,"+:dn:0| %d bytes | total file size\r\n",fileSize);sendStr(X->C->S,sbuf);

					pkt1.Flags=0;//ignore fileTimes and fileAttributes and don't overwrite existing file
					pkt1.FileSize=fileSize;
					pkt1.StartOffset=0;
					pkt1.DataSize=fileSize;
					pkt1.ExtraHdrLen=0;
					pkt1.FilePathLen=lstrlen(file);
					
					pkt0.MagicNum=_zX_;
					pkt0.MinVerReqd=0;
					pkt0.Command=auxCmd_up;
					pkt0.HdrLen=sizeof(pkt1)+pkt1.FilePathLen;//+lstrlen(file);
					
					sD=_socket(AF_INET,SOCK_STREAM,0);
					saD.sin_family=AF_INET;
					saD.sin_port=htons(X->C->pt);
					saD.sin_addr.s_addr=X->C->ip;//X->C->ip=_htonl(X->C->ip);//done during PT command.
					if (!connect(sD,(SOCKADDR *)&saD,sizeof(saD)))
					{
						lstrcpy(sbuf,"+:1| connected; initialising data transfer ...\r\n");sendStr(X->C->S,sbuf);
						if (_send(sD,&pkt0,sizeof(pkt0),0)!=SOCKET_ERROR &&
							_send(sD,&pkt1,sizeof(pkt1),0)!=SOCKET_ERROR   &&
							_send(sD,&file,pkt1.FilePathLen,0)!=SOCKET_ERROR &&
							_recv(sD,&pktR,sizeof(pktR),0)!=SOCKET_ERROR )
						{
							switch(((pkt_zX_up_ret *)pktR).retCode)
							{
							case 0:
								lstrcpy(sbuf,"+:2| transferring data ...\r\n");sendStr(X->C->S,sbuf);
								totalRead=0;//readBytes=dataBufSize;
								//crc32=0;
								ReadFile(hF,sbuf,dataBufSize,&readBytes,0);
								while (readBytes==dataBufSize && (i=_send(sD,sbuf,readBytes,0)!=SOCKET_ERROR)
								{
									totalRead+=i;
									//CRC32(&crc32,sbuf,readBytes);
									ReadFile(hF,sbuf,dataBufSize,&readBytes,0);
								}
								if (_recv(sD,&pktR,sizeof(pktR),0)!=SOCKET_ERROR)
								{
									if (((pkt_zX_up_end *)pktE).BytesReceived!=fileSize)
										wsprintf(sbuf,"-:dn:3|%u off %u bytes transferred; file transfer incomplete!",((pkt_zX_up_end *)pktE).BytesReceived,fileSize);
									else
										if (((pkt_zX_up_end *)pktE).CRC32!=crc32)
											wsprintf(sbuf,"-:dn:4| data corrupt!",fileSize);
										else
											wsprintf(sbuf,"*:dn|%u bytes|crc32=0x%X |file transfer complete!",fileSize,((pkt_zX_up_end *)pktE).CRC32);
								}
								closezXsocket(sD);
								break;
							case 1:
								closezXsocket(sD);
								lstrcpy(sbuf,"-:3| file already exists ...\r\n");
								break;
							//case ~0:
							default:
								closezXsocket(sD);
								lstrcpy(sbuf,"-:4| data transfer failed! reason unknown ...\r\n");
								//break;
							}
						}
						else
						{
							closezXsocket(sD);
							lstrcpy(sbuf,"-:3| error! initialising data transfer ...\r\n");
						}
					}
					else
						wsprintf(sbuf,"-:dn:6|could not connect to %s:%u",ip2a(X->C->ip),X->C->pt);
					closezXsocket(sD);
					CloseHandle(hF);
				}
				else
					wsprintf(sbuf,"-:dn:0|could not open file \"%s\"",file);
			}
		}
	}
	else
		wsprintf(sbuf,"-:5|file name not specified ...");
	lstrcat(sbuf,"\r\n");sendStr(X->C->S,sbuf);
	return 0;
}
*/
//int zXcmd_dn(pCommandData X){return 0;}
//int zXcmd_up(pCommandData X){return 0;}

int zXcmd_up(pCommandData X)
{
/*	char sbuf[zXbufSize];
	SOCKADDR_IN sa_up;

	if (!X->C->ip)
		lstrcpy(sbuf,"-:8|use \"PT:[iphex]:[port]\" to specify data transfer port");
	else
	{
		if (X->cmdlen>3)
		{
			lstrcpy(file_up,X->cmd+3);
			if (GetFileAttributes(file_up)!=~0)
				wsprintf(sbuf,"-:6|file \"%s\" already exists",file_up);
			else
			{
				s_up=_socket(AF_INET,SOCK_STREAM,0);
		s=X->C->S;
		pt_up=X->C->pt;
		ip_up=X->C->ip;
				sa_up.sin_family=AF_INET;
				sa_up.sin_port=_htons(pt_up);
				sa_up.sin_addr.s_addr=ip_up;//remIPaddr=_htonl(remIPaddr);//done during PT command.
				if (!_connect(s_up,(SOCKADDR *)&sa_up,sizeof(sa_up)))
				{
					if (hF_up=CreateFile(file_up,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0))
					{
						SetFilePointer(hF_up,0,0,FILE_BEGIN);
						bytesRead_up=bytesRead_up=0;
						lstrcpy(sbuf,"+ connected; receiving data ...");
						startTimer=GetTickCount();
						_WSAAsyncSelect(s_up,hW,WSA_ASYNC_up,FD_READ|FD_CLOSE);
					}
					else
					{
						wsprintf(sbuf,"-:0|could not create file \"%s\"",file_up);
						_closesocket(s_up);
					}
				}
				else
					wsprintf(sbuf,"-:6|could not connect to %s:%u",ip2a(ip_up),pt_up);
			}
		}
		else
			wsprintf(sbuf,"-:5|file name not specified ...");
	}
*/
	lstrcpy(X->sbuf,"= command unimplemented ...");
	sendStr(X);
	return 0;
}
