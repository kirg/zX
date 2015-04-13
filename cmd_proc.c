#include <windows.h>
#include <tlhelp32.h>

#include "types.h"
#include "server.h"
#include "misc.h"
#include "globals.h"

int zXcmd_ps(pCommandData X)
{
#define command		X->argv[0]
	HANDLE hS;
	PROCESSENTRY32 pe;
	unsigned int r;
	unsigned int n=0;
	char *fileName;

	pe.dwSize=sizeof(pe);
	hS=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	wsprintf(X->sbuf,"+:%s|      pid|thd|pr|filename",command);
	sendStr(X);
	for(r=Process32First(hS,&pe);r;r=Process32Next(hS,&pe))
	{
		if (chk4break(X))
			return ~0;
/*		wsprintf(sbuf,"%2x %2x %2d %2d %s\r\n",OpenProcess(PROCESS_TERMINATE,0,pe.th32ProcessID),
								OpenProcess(PROCESS_TERMINATE,0,pe.th32ParentProcessID),
								pe.cntThreads,pe.pcPriClassBase,pe.szExeFile);
*/
		//GetLongPathName(pe.szExeFile,pe.szExeFile,MAX_PATH);
		if (isWin98)
			GetFullPathName(pe.szExeFile,MAX_PATH,pe.szExeFile,&fileName);
		else
			fileName=pe.szExeFile;
		
		//wsprintf(sbuf,"+:ps| %8d %8x %8x %8d %2d %s",n,pe.th32ProcessID,pe.th32ParentProcessID,pe.cntThreads,pe.pcPriClassBase,fileName);
		wsprintf(X->sbuf,"+:%s| %8X %3d %2d %s",command,pe.th32ProcessID,pe.cntThreads,pe.pcPriClassBase,fileName);
		sendStr(X);
		++n;
	}
	wsprintf(X->sbuf,"*:%s| %u processes currently running",command,n);
	sendStr(X);
	return 0;
}

int zXcmd_kl(pCommandData X)
{
#define command		X->argv[0]
	unsigned int pid;
	HANDLE hP;
	BOOL r=0;

	if (X->argc)
	{
		pid=_atox(X->argv[X->argc]);
		if (chk4break(X))
			return ~0;

		if (hP=OpenProcess(PROCESS_TERMINATE,0,pid))
		{
			r=TerminateProcess(hP,0);
			CloseHandle(hP);
		}
		if (r)
			wsprintf(X->sbuf,"*:%s| killed process \"0x%X\"",command,pid);
		else
			wsprintf(X->sbuf,"-:%s:0| could not kill process with pid \"0x%.8X\"",command,pid);	
	}
	else
		wsprintf(X->sbuf,"-:%s:5| pid of process to kill not specified",command);
	sendStr(X);
	return 0;
}