#include <windows.h>

#include "types.h"

#include "server.h"
#include "misc.h"
#include "globals.h"
#include "command.h"

char *getAttrStr(unsigned long attr,char *s)
{
	*((unsigned long *)s)='---';//lstrcpy(s,"---");
	if (attr & FILE_ATTRIBUTE_READONLY)
		s[0]='r';
	if (attr & FILE_ATTRIBUTE_HIDDEN)
		s[1]='h';
	if (attr & FILE_ATTRIBUTE_SYSTEM)
		s[2]='s';
	return s;
}

int zXcmd_dl(pCommandData X)
{
#define command			X->argv[0]
#define driveList		X->str
#define driveType		X->itoa0
#define strTotalSpace	X->itoa1
#define strFreeSpace	X->currentFile
	
	char *sbuf=X->sbuf;
	unsigned int i=0;
	unsigned int numDrives=0;
	ULARGE_INTEGER total,free;

	GetLogicalDriveStrings(260,driveList);
	wsprintf(sbuf,"+:%s| root|type     |       free | total",command);
	while(X->str[i])
	{
		if (chk4break(X))
			return ~0;
		switch(GetDriveType(driveList+i))
		{
			case DRIVE_REMOVABLE:
				lstrcpy(driveType,"removable");
				break;
			case DRIVE_FIXED:
				lstrcpy(driveType,"fixed");
				break;
			case DRIVE_REMOTE:
				lstrcpy(driveType,"network");
				break;
			case DRIVE_CDROM:
				lstrcpy(driveType,"cdrom");
				break;
			case DRIVE_RAMDISK:
				lstrcpy(driveType,"ramdisk");
				break;
			default:
				lstrcpy(driveType,"unknown");
				break;
		}
    if (GetDriveType(driveList+i)==DRIVE_REMOVABLE /*dont getfreespace for floppies,etc*/
        || !GetDiskFreeSpaceEx(driveList+i,0,&total,&free))
      total.HighPart=free.HighPart=~0;
		sendStr(X);
		wsprintf(sbuf,"+:%s| %3s  %-9.9s %11s   %-11s",command,driveList+i,driveType,getSizeStr(&free,strFreeSpace),getSizeStr(&total,strTotalSpace));
		while(driveList[i++]);
		++numDrives;
	}
	if (numDrives)
		sendStr(X);
	wsprintf(sbuf,"*:%s| %d logical drive(s) available",command,numDrives);
	sendStr(X);
	return 0;
}


// file operations // ls cd md cp mv rm at ex //

int zXcmd_ls(pCommandData X)
{
#define command		X->argv[0]
#define fileAttr	X->itoa0
#define fileSize	X->itoa1
#define wildCardLen	files
#define param		folders

	unsigned int files,folders;
	char *sbuf=X->sbuf;//reduce the performance-hit on calculating 'X->sbuf' everytime
	int foundFile;
	char *wildCards;

	HANDLE hFF;
	WIN32_FIND_DATA	F;
//	FILETIME FT;SYSTEMTIME ST;
	
	if (param=X->argc)
	{
		//MUST use argv[1] to argv[param-1] as option flags
		/////// '....\' -> '....\*'
		wildCards=X->argv[param];
		wildCardLen=X->argvlen[param];
		if (wildCards[wildCardLen-1]=='\\' || wildCards[wildCardLen-1]=='/')
		{
			wildCards[wildCardLen]='*';
			wildCards[1+wildCardLen]=0;
		}
		//////////////////////////
	}
	else
	{
		X->str[0]='*';X->str[1]=0;
		wildCards=X->str;
	}
	files=folders=0;

	if (chk4break(X))
		return ~0;
	beginCurrentDirectory(X);
	hFF=FindFirstFile(wildCards,&F);
	endCurrentDirectory();
	
	wsprintf(sbuf,"+:%s|       size|attr |filename",command);
	
	if (hFF!=INVALID_HANDLE_VALUE && (foundFile=1))//initialising 'foundfile' to TRUE
	{
		while (foundFile)
		{
			if (chk4break(X))
				return ~0;
			if (F.cFileName[0]!='.')
			{
				getAttrStr(F.dwFileAttributes,fileAttr);
				fileAttr[3]=fileAttr[4]=' ';fileAttr[5]=0;
				if (F.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					++folders;
					fileAttr[4]='[';
					fileSize[0]=0;
				}
				else
				{
					++files;
					//itoa(F.nFileSizeLow,fileSize,10);
					wsprintf(fileSize,"%u",F.nFileSizeLow);
				}
//				FileTimeToLocalFileTime(&F.ftLastWriteTime,&FT);FileTimeToSystemTime(&FT,&ST);
//				wsprintf(sbuf,"+:ls| %.2u:%.2u %.2u/%s/%.4u %10s %s %s",ST.wHour,ST.wMinute,ST.wDay,month[ST.wMonth-1],ST.wYear,fileSize,attr,F.cFileName);
				//wsprintf(sbuf,"+:ls| %10s %s %s",fileSize,attr,F.cFileName);
				sendStr(X);
				wsprintf(sbuf,"+:%s| %10s %s %s",command,fileSize,fileAttr,F.cFileName);
			}
			foundFile=FindNextFile(hFF,&F);
		}
	}
	FindClose(hFF);
	if (files || folders)
		sendStr(X);
	if (X->argc)
		wsprintf(sbuf,"*:ls| %u file(s); %u folder(s) match criterion \"%s\"",files,folders,wildCards);
	else
		wsprintf(sbuf,"*:ls| %u file(s); %u folder(s) in \"%s\"",files,folders,X->C->currentDirectory);
	sendStr(X);
	return 0;
}

int zXcmd_cd(pCommandData X)
{
#define command		X->argv[0]

	char *dirName;
	int dirNameLen;
	char *theCurrentDirectory;
	char r='*';
	if (X->argc)
	{
		dirName=X->argv[X->argc];
		dirNameLen=X->argvlen[X->argc];
		theCurrentDirectory=X->str;
tryAgain:
		beginCurrentDirectory(X);
		r=SetCurrentDirectory(dirName);
		GetCurrentDirectory(MaxPath,X->str);//to get the full path of the current directory
		endCurrentDirectory();

		if (chk4break(X))
			return ~0;//break!
		if (r)
		{
			lstrcpyn(X->C->previousDirectory,X->currentDirectory,MaxPath);//no need for critical_section, i guess
			if (isWin98)// api only in win98+
				GetLongPathName(theCurrentDirectory,X->currentDirectory,MaxPath);// get its lfn
			else
				lstrcpyn(X->currentDirectory,theCurrentDirectory,MaxPath);
			theCurrentDirectory=X->currentDirectory;
			if (!setCurrentDirectory(X,theCurrentDirectory))
				return ~0;//dead!
			r='*';
		}
		else
		{
			if (dirName[0]=='-' && dirNameLen==1)//folder '-' does not exist; so try prevDir
			{
				lstrcpyn(X->currentFile,X->C->previousDirectory,MaxPath);
				dirName=X->currentFile;
				dirNameLen=0;//to prevent inf_looping
				goto tryAgain;
			}
			r='-';
		}
	}
	else
		theCurrentDirectory=X->C->currentDirectory;

	wsprintf(X->sbuf,"%c:%s| %s",r,command,theCurrentDirectory);
	sendStr(X);
	return 0;
}

int zXcmd_md(pCommandData X)
{
#define command		X->argv[0]
#define dirName		X->argv[X->argc]
	if (X->argc)
	{
		if (chk4break(X))
			return ~0;//break!

		beginCurrentDirectory(X);
		if (CreateDirectory(dirName,0))
			wsprintf(X->sbuf,"*:%s| \"%s\" created",command,dirName);
		else
			wsprintf(X->sbuf,"-:%s:0| could not create directory \"%s\"",command,dirName);
		endCurrentDirectory();
	}
	else
		wsprintf(X->sbuf,"-:%s:5| directory name not specified ...",command);
	sendStr(X);
	return 0;
}

int zXcmd_cpmv(pCommandData X)
{
#define command			X->argv[0]
	char *source;
	char *destination;
	int r;
	char op='+';

	if (X->argc>1)
	{
		source=X->argv[X->argc-1];
		destination=X->argv[X->argc];
		if (!X->argvlen[X->argc-1])//==0
			source=X->currentFile;

		if (chk4break(X))
			return ~0;//break!

		beginCurrentDirectory(X);
		if (X->ncmd!=c_mv)
			r=CopyFile(source,destination,1);
		else
		{
			op='-';
			r=MoveFile(source,destination);
		}
		endCurrentDirectory();
		if (r)
		{
			wsprintf(X->sbuf,"*:%s| \"%s\" %c> \"%s\"",command,source,op,destination);
			setCurrentFile(X,destination);
		}
		else
			wsprintf(X->sbuf,"-:%s:0| \"%s\" %c> \"%s\" failed!",command,source,op,destination);
	}
	else
		wsprintf(X->sbuf,"-:%s:5| required parameters missing ...",command);
	sendStr(X);
	return 0;
}

int zXcmd_rm(pCommandData X)
{
#define command			X->argv[0]
	char *param;
	unsigned long attr;
	BOOL r;

	if (X->argc)
	{
		param=X->argv[X->argc];

		if (chk4break(X))
			return ~0;//break!

		beginCurrentDirectory(X);
		if (~(attr=GetFileAttributes(param)))
		{
			if (attr & FILE_ATTRIBUTE_DIRECTORY)
				r=RemoveDirectory(param);
			else
				r=DeleteFile(param);
			if (r)
				wsprintf(X->sbuf,"*:%s| \"%s\" deleted",command,param);
			else
				wsprintf(X->sbuf,"-:%s:0| could not delete \"%s\"",command,param);
		}
		else
			wsprintf(X->sbuf,"-:%s:1| file not found : \"%s\"",command,param);
		endCurrentDirectory();
	}
	else
		wsprintf(X->sbuf,"-:%s:5| filename not specified ...",command);
	sendStr(X);
	return 0;
}

int zXcmd_at(pCommandData X)
{
#define command			X->argv[0]
#define sattr0			X->itoa0
#define sattr1			X->itoa1
	char *file;
	char *attr;
	int i;
	unsigned long attr0,attr1;

	if (X->argc)
	{
		if (X->argvlen[2])
			file=X->argv[2];
		else
			file=X->currentFile;
		if (~(attr0=GetFileAttributes(file)))
		{
			attr=X->argv[1];
			attr1=0;
			for(i=0;i<X->argvlen[1];++i)
				switch(attr[i])
				{
				case 'r':
					attr1|=FILE_ATTRIBUTE_READONLY;
					break;
				case 'h':
					attr1|=FILE_ATTRIBUTE_HIDDEN;
					break;
				case 's':
					attr1|=FILE_ATTRIBUTE_SYSTEM;
					//break;
				}

			getAttrStr(attr0,sattr0);
			getAttrStr(attr1,sattr1);

			if (chk4break(X))
				return ~0;//break!

			beginCurrentDirectory(X);
			if (SetFileAttributes(file,attr1))
				wsprintf(X->sbuf,"*:%s| %s -> %s for \"%s\"",command,sattr0,sattr1,file);
			else
				wsprintf(X->sbuf,"-:%s:0| could not change attributes of \"%s\" from %s to %s",command,file,sattr0,sattr1);
			endCurrentDirectory();
		}
		else
			wsprintf(X->sbuf,"-:%s:1| file not found :\"%s\"",command,file);
	}
	else
		wsprintf(X->sbuf,"-:%s:5| attr and filename not specified ...",command);
	sendStr(X);
	return 0;
}


int zXcmd_ex(pCommandData X)
{
#define command		X->argv[0]

/*
todo: must lookup in the registry for the executable path translation
HKLM\Software\Microsoft\Windows\CurrentVersion\App Paths\THE_EXE
HKLM\Software\Microsoft\Windows\CurrentVersion\App Paths\THE_EXE.EXE
*/
	int r;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char *file;

	if (X->argc)
	{
		file=X->argv[X->argc];
		zeroMemory(&si,sizeof(STARTUPINFO));
		si.cb=sizeof(STARTUPINFO);
		si.dwFlags=STARTF_FORCEOFFFEEDBACK;

		if (chk4break(X))
				return ~0;//break!

		beginCurrentDirectory(X);
		r=CreateProcess(0,file,0,0,0,0,0,0,&si,&pi);
		//r=(int)ShellExecute(0,"open",file,fileName+i,0,SW_SHOWNORMAL);
		endCurrentDirectory();
		if (r)
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);//free up my copy of the handles
			wsprintf(X->sbuf,"*:%s| pid=0x%X | \"%s\" executed successfully !",command,pi.dwProcessId,file);
		}
		else
			wsprintf(X->sbuf,"-:%s:0| could not execute \"%s\"",command,file);
/*		if (r<=32)
		{
			switch(r)
			{
				case 0:
					wsprintf(sbuf,"-:ex:4| could not execute \"%s\" : out of memory!",fileName);
					break;
				case ERROR_FILE_NOT_FOUND:
					wsprintf(sbuf,"-:ex:1| could not execute \"%s\" : file not found!",fileName);
					break;
				default:
					wsprintf(sbuf,"-:ex:0| could not execute \"%s\"",fileName);
					break;
			}
		}
		else
			lstrcpy(sbuf,"*:ex| executed command successfully !");
*/	}
	else
		wsprintf(X->sbuf,"-:%s:5| file name not specified ...",command);
	sendStr(X);
	return 0;
}