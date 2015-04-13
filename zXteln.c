#include <windows.h>

#include "defines.h"
#include "macros.h"
#include "types.h"
#include "globals.h"

#include "misc.h"
#include "command.h"


HANDLE				hProc;
HANDLE				hEv;

/*
rfc
855 sub negotionations
856 binary-transmission
857 echo option    (1)


/*
ctrl+shift+-  (^_)  0x1f
ctrl+shift+^  (^^)  0x1e
*/
/*
shell operation:

  on receiving 'str' of 'len' bytes

  if (at_prompt)
    for i=0 to len
      if str[i]!=^c
        cmdBuf[cmdLen++]=str[i]
      else
        break fgCmd
        flush all buffered input
  else
    for i=0 to len
      switch(str[i]){
      case '\r':
        runCmd(str+i);
      case '^C':
        break fgCmd;
        flush all buffered input

  */
int isConnType_teln(const char *str){
  register int i;
  /*
  if (isPrefix(chkConnType_teln,str))
    return 1;
   */
  for (i=0;str[i];++i){/*allow the '~' to be anywhere (not just in the beginning)*/
    if (str[i]=='~')
    //if (*((unsigned short *)(str+i))==0x1F7E/*~^_*/)
      return 1;
  }
  return 0;
}

int zXshowPrompt(pConnectionData pC){
//_send(pC->S,g_buf,wsprintf(g_buf,"prompt:cmdlen=%u,cmdstr=%x\r\n",pC->cmdLen,*((unsigned long *)pC->cmdStr)),0);
	pC->fgX_N=0xffff;/* to indicate that the user is currently at the prompt
                      * ie, no foreground command */
	_send(pC->S,pC->prompt,lstrlen(pC->prompt),0);/* show the prompt */
  if (pC->cmdLen)
	  _send(pC->S,pC->cmdStr,pC->cmdLen,0);/* show any keys that may have been 'buffered' */
	return 0;
}

int zXexecCmd(pCommandData pX)
{
	int				i,j;
	u_short   N;
	int				(*zXcmdFunc)(pCommandData);
	pConnectionData	pConn;

	pConn=pX->C;/*to prevent repeated dereferencing of pX->C*/
  pX->C_n=pConn->n;
  pX->C_N=pConn->N;

	for(i=0;i<MaxNumOfCommandThreadsPerConnection && pConn->X_N[i]!=0xffff;++i);
	pConn->X_N[pX->i=i]=N=pX->N;
    //save the i-value and N-value in the connectionData
  	//save the n-value; for use at the end (to check for zXprompt())
	
	if (pX->cmd[pX->cmdlen-1]=='&')//check for terminating '&' (to run in bg) else make fg
	{
		pX->cmd[--pX->cmdlen]=0;//strip off trailing '&'
		zXshowPrompt(pConn);//immediately show prompt
	}
	else
	{
		pConn->fgX_N=N;//set as foreground command;
		if (pX->cmd[pX->cmdlen-1]==' ')
			pX->cmd[--pX->cmdlen]=0;/*strip off trailing ' '; in case it was put to mask ending '&'*/
	}

	for(i=0;pX->cmd[i]!=' ' && pX->cmd[i]!=0 && i<4;++i);//find first space within the first four bytes
	pX->ncmd=((*((unsigned int *)pX->cmd))) & (0xffffffff>>(32-8*i));

	if (!(zXcmdFunc=zXcmd(pX->ncmd,0)))
		zXcmd_not_found(pX);
	else
	{
		lstrcpy(pX->argvBuf,pX->cmd);
		if (i>3)
			while(pX->argvBuf[i]!=' ' && pX->argvBuf[i])	++i;
		pX->argv[pX->argc=0]=pX->argvBuf;
		pX->argvBuf[i]=0;//stop string at first space;
		pX->argvlen[0]=i;
		while(pX->cmd[i] && pX->cmd[j=++i] && pX->argc<MaxNumOfCmdParams)//command not over (detect stray ' ')
		{
			while(pX->argvBuf[i]!='|' && pX->argvBuf[i]!=0)	++i;
			pX->argv[++(pX->argc)]=pX->argvBuf+j;
			pX->argvBuf[i]=0;
			pX->argvlen[pX->argc]=i-j;
			//	other possible separators   | : , % @ $ & * #
			//	MUST not split a "quoted argument"
			// must find option flags ex: ls -lR-f -h *.exe -> flags=lRfh argv[1]=*.exe
		}
		for(i=pX->argc;i<MaxNumOfCmdParams;pX->argv[++i]=0);//null all the other 'argv's
		pX->evSTOP=CreateEvent(0,1,1,0);//signalled; to close {DuplicateHandle;ResetEvent;CloseHandle;}
		pX->evPAUSE=CreateEvent(0,1,1,0);//signalled; //can PulseEvent this for a single-stepping effect
    pX->time=GetTickCount();
		zXcmdFunc(pX);//keep polling for state of c->evSTOP; break if non-signalled
		CloseHandle(pX->evPAUSE);
		CloseHandle(pX->evSTOP);
	}
	if (pConn->fgX_N==N /* && pConn->id==pX->id*/)//if this was the foreground command, show the prompt
		zXshowPrompt(pConn);
	CloseHandle(pX->hT);pX->hT=0;//close the thread handle (opened by wndproc)
	pConn->X_N[pX->i]=~0;/*mark 'unoccupied'*/
	--(pConn->numCommands);
	VirtualFree(pX,SizeCommandData,MEM_DECOMMIT);
	X[N]=0;/*free!*/
	return 0;
}
    
int executeCommand(pConnectionData pC){
  register int i;
  if (pC->numCommands<MaxNumOfCommandThreadsPerConnection){
    for(i=0;X[i] && i<MaxNumOfCommandThreads;++i);/*search for a free 'slot'*/
    if (i<MaxNumOfCommandThreads){
      if (X[i]=(CommandData *)VirtualAlloc(memCommandData+i*SizeCommandData,SizeCommandData,MEM_COMMIT,PAGE_READWRITE)){
        X[i]->N=i;
        X[i]->C=pC;
        lstrcpyn(X[i]->cmd,pC->cmdStr,1+(X[i]->cmdlen=pC->cmdLen));
        lstrcpyn(X[i]->currentFile,pC->currentFile,MaxPath);
        lstrcpyn(X[i]->currentDirectory,pC->currentDirectory,MaxPath);
        if (X[i]->hT=thread(zXexecCmd,X[i])){
          ++(pC->numCommands);
          i=0;
        }
        else{
          VirtualFree(X[i],SizeCommandData,MEM_DECOMMIT);
	        X[i]=0;/*free!*/
          _send(pC->S,g_buf,
            wsprintf(g_buf,"#%.2u.%.2u: command execution failed! (while creating thread)\r\n",
              pC->N,pC->numCommands),0);
          i=4;
        }
      }
      else{
        _send(pC->S,g_buf,
          wsprintf(g_buf,"#%.2u.%.2u: command execution failed! (while committing memory)\r\n",
            pC->N,pC->numCommands),0);
        i=3;
      }
    }
    else{
      _send(pC->S,g_buf,
        wsprintf(g_buf,"#%.2u.%.2u: server's command buffer is full!\r\n",
          pC->N,pC->numCommands),0);
      i=2;
    }
  }
  else{
    _send(pC->S,g_buf,
      wsprintf(g_buf,"#%.2u.%.2u: connection's command buffer is full!\r\n",
        pC->N,pC->numCommands),0);
    i=1;
  }
  pC->cmdLen=0;/*command buffer empty; ready for new command*/
  return i;
}

int zXteln(pConnectionData pC,int len,char *str){
  register int i;
  SOCKADDR_IN sa;
  int sa_len;
  
  if (!len){ /*initialize connection for zXteln*/
    pC->n=GetTickCount();/*connection id; used to identify the connection*/;
                          /*save connection id; used along with 'N' to identify connection*/
    fillMemory(pC->connBuf,ConnBufSize,0);/*pC->numCommands=0; pC->cmdLen=0;... etc*/
/*  pC->cmdLen=0;
    pC->numCommands=0;
    pC->echo=0;
    pC->currentFile[0]=0;
    pC->currentDirectory[0]=0;
    pC->curRegKey[0]=0;
    pC->prevRegKey[0]=0;
    pC->aliasCmd[i]=0;
    pC->aliasStr[i][0]=0;*/
    fillMemory(pC->X_N,MaxNumOfCommandThreadsPerConnection*sizeof(u_short),~0);/*for(r=0;r<MaxNumOfCommandThreadsPerConnection;++r)  pC->cmd_n[r]=~0;*/
    pC->threadWaitTimeout=DefaultThreadWaitTimeout;
    lstrcpyn(pC->prompt,zXdefaultPrompt,MaxPromptLength);
    lstrcpyn(pC->currentDirectory,initCurrentDirectory,MaxPath);
    lstrcpyn(pC->previousDirectory,initCurrentDirectory,MaxPath);
    lstrcpyn(pC->prevCmdStr,initPrevCommand,MaxCmdLen);
    InitializeCriticalSection(&pC->cs_SetCurrentDirectory);
    InitializeCriticalSection(&pC->cs_SetCurrentFile);

    _gethostname(g_hostName,MaxHostNameLen);h_this=_gethostbyname(g_hostName);
    sa_len=sizeof(SOCKADDR_IN);_getpeername(pC->S,(struct sockaddr *)&sa,&sa_len);
    pC->ip=sa.sin_addr.s_addr;pC->pt=zXport;
    _send(pC->S,g_buf,
      wsprintf(g_buf,"\r#%.2u: hi! you're connected to \"%s\" [%s] from [%s]\r\n",
        pC->N,g_hostName,ip2a(*(h_this->h_addr_list[0])),ip2a(sa.sin_addr.s_addr)),0);
    /*pC->fgX_N=0xffff;*//*done within 'zXshowPrompt(..)'*/
    zXshowPrompt(pC);
    return 0;
  }

  for(i=0;i<len;++i){
    switch (str[i]){
    case 0x1B://escape
    case 0x03://^C
      if (pC->fgX_N!=0xffff)/* if there is a command is running in the foreground */{
        DuplicateHandle(hProc,X[pC->fgX_N]->evSTOP,hProc=GetCurrentProcess(),&hEv,0,0,DUPLICATE_SAME_ACCESS);
        ResetEvent(hEv);/* reset (non-signal) */
        CloseHandle(hEv);
      }
      else{
        _send(pC->S,g_buf,wsprintf(g_buf,"^C\r\n#%.2u: command ignored ...\r\n",pC->N),0);
        pC->cmdStr[pC->cmdLen=0]=0;
        zXshowPrompt(pC);
      }
      return 0;/*don't process whatever else may have been received ...*/
      /*break;*/
      //case 0x09://TAB
      //auto-complete filename; use FindFirstFile(str+"*");
      //break;
    case 0x05://^E
      pC->echo=!pC->echo;
      break;
    case 0x0C://^L
      lstrcpy(pC->cmdStr,"cls");pC->cmdLen=3;
      executeCommand(pC);
      break;
    case 0x04://^D
    case 0x11://^Q
    case 0x1A://^Z
      lstrcpy(pC->cmdStr,"qt");pC->cmdLen=2;
      executeCommand(pC);
      break;
    case 0x01://^A
      goto runPreviousCommand;
      //lstrcpy(pC->cmdStr,"hi");pC->cmdLen=2;
      //executeCommand(pC);
      //break;
    case 0x02://^B
      lstrcpy(pC->cmdStr,"asdf");pC->cmdLen=2;
      executeCommand(pC);
      break;
    /*case 0x1A://^Z
      lstrcpy(pC->cmdStr,"zX");pC->cmdLen=2;
      goto executeCommand;
      break;*/
    case 0x08:/*<bkSpace>*/
    case 0x7F:/*del*/
        if (pC->cmdLen){/*there is something 'typed' at the prompt*/
          if (pC->echo || str[i]==0x7F)
            *((unsigned long *)g_buf)=0x00082008;/*<bkSpace><space><bkSpace>*/
          else
            *((unsigned long *)g_buf)=0x00000820;/*<space><bkSpace>*/
          _send(pC->S,g_buf,lstrlen(g_buf),0);
          pC->cmdStr[--pC->cmdLen]=0;/*remove trailing character*/
        }
        else{/*there was nothing at the prompt (to delete); so show previous command*/
          *((unsigned short *)g_buf)=0x000D;
          _send(pC->S,g_buf,1,0);
          zXshowPrompt(pC);/*OR show the last char of prompt*/
          _send(pC->S,pC->cmdStr,pC->cmdLen=lstrlen(lstrcpy(pC->cmdStr,pC->prevCmdStr)),0);
        }
        break;
    case 0x0D/*'\r'*/:/*<enter>*/
runPreviousCommand:
      if (!pC->cmdLen){
        //zXshowPrompt(C);/*ignore; show the prompt*/
        _send(pC->S,pC->cmdStr,pC->cmdLen=lstrlen(lstrcpy(pC->cmdStr,pC->prevCmdStr)),0);
        _send(pC->S,"\r\n",2,0);/*send a new line*/
        executeCommand(pC);
        break;
      }
      if (pC->echo)
        _send(pC->S,"\r\n",2,0);/*send a new line*/
      lstrcpyn(pC->prevCmdStr,pC->cmdStr,1+pC->cmdLen);
      executeCommand(pC);
      break;
    case 0x0A:
      /*ignore ...*/
      break;
    default:/*alphabets, numbers, etc ...*/
      if (pC->cmdLen<MaxCmdLen)
        pC->cmdStr[(pC->cmdLen)++]=str[i];
      else{
        _send(pC->S,g_buf,
          wsprintf(g_buf,"\r\n#%.2u.%.2u: command too long!\r\n",
            pC->N,pC->numCommands),0);
        return 0;
      }
      if (pC->echo)
        _send(pC->S,str+i,1,0);
      /* also add echo beep; ie send back beep (0x07), too, for every char*/
      //_send(pC->S,"\x07",1,0);
    }
  }
  return 0;
}

/*  
<ESC>D Move cursor down to next line 
<ESC>E Move cursor to first column and down one line 
<ESC>M Move cursor up one line 
<ESC>7 Save cursor position, attributes and colors 
<ESC>8 Restore saved cursor position, attributes and colors 
<ESC>[nA Move cursor up n lines 
<ESC>[nB Move cursor down n lines 
<ESC>[nC Move cursor forward n spaces 
<ESC>[nD Move cursor backward n spaces 
<ESC>[nE Move cursor to beginning of line, down n lines 
<ESC>[nF Move cursor to beginning of line, up n lines 
<ESC>[xG Move cursor to column x 
<ESC>[y;xH Move cursor to line y, column x 
<ESC>[nI Move cursor forward n tabstops 
<ESC>[nZ Move cursor backwards n tabstops 
<ESC>[na Move cursor forward n spaces 
<ESC>[yd Move cursor to row y 
<ESC>[ne Move cursor down n lines 
<ESC>[y;xf Move cursor to line y, column x 
<ESC>[s Save cursor position 
<ESC>[u Return to saved cursor position 
<ESC>[x` Move cursor to column x 
*/