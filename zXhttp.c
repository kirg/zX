
#include "defines.h"
#include "macros.h"
#include "types.h"
#include "globals.h"

#include "zXhttp.h"
#include "misc.h"
#include "server.h"


#define HTTP_OK             200
#define HTTP_PartialContent 206
#define HTTP_NotFound       404

int isConnType_http(const char *str){
  if (isPrefix(ChkConnType_http,str))
    return ConnType_http;
  return 0;
}
/*
int wsErr(){
#define chkWSerr(X)  case X: msg=#X;break
  char *msg;
  switch(_WSAGetLastError()){
  chkWSerr(WSANOTINITIALISED);
  chkWSerr(WSAENETDOWN);
  chkWSerr(WSAEACCES);
  chkWSerr(WSAEINTR);
  chkWSerr(WSAEINPROGRESS);
  chkWSerr(WSAEFAULT);
  chkWSerr(WSAENETRESET);
  chkWSerr(WSAENOBUFS);
  chkWSerr(WSAENOTCONN);
  chkWSerr(WSAENOTSOCK);
  chkWSerr(WSAEOPNOTSUPP);
  chkWSerr(WSAESHUTDOWN);
  chkWSerr(WSAEWOULDBLOCK);
  chkWSerr(WSAEMSGSIZE);
  chkWSerr(WSAEINVAL);
  chkWSerr(WSAECONNABORTED);
  chkWSerr(WSAECONNRESET);
  default:
    msg="unknown";
  }
  return MessageBox(GetForegroundWindow(),msg,"winsock error!",MB_ICONERROR);
#undef chkWSerr
}
*/
char *getContentType(char *filename,int buflen,char *ContentType){
  HKEY		hK=0;
  register int i;
#define def_ContentType "application/octet-stream"
  
  for(i=lstrlen(filename)-2;filename[i]!='/' && filename[i]!='\\' && filename[i]!='.' && i;--i);
  if (filename[i]!='.'
      || _RegOpenKeyEx(HKEY_CLASSES_ROOT,filename+i,0,KEY_READ|KEY_WRITE,&hK)!=ERROR_SUCCESS
      || _RegQueryValueEx(hK,"Content Type",0,0,ContentType,&buflen)!=ERROR_SUCCESS)
      lstrcpyn(ContentType,def_ContentType,buflen);
  if (hK)
    _RegCloseKey(hK);
  return ContentType;
}

int zXhttp(pConnectionData pC,int len,char *str){
  register char *i,*l;

  if (!len)
    i=pC->httpBuf;
  else{
    i=pC->httpBuf+pC->n-3;
    if ((pC->n+len+1)>HttpBufSize){ /* request too long!; buffer (will) overflow; */
      _closesocket(pC->S);pC->S=INVALID_SOCKET;
      closeConnection(pC->N);
      return -1;
    }
    else{
      lstrcpyn(pC->httpBuf+pC->n,str,len+1);
      pC->n+=len;
    }
  }
  l=pC->httpBuf+pC->n-4;

  while(l>=i && *((unsigned int *)l)!=0x0a0d0a0d)
    --l;  /*search for "\r\n\r\n" <- end of http request*/
  if (l<i)/*not found*/
    return 1;
  /*else*/

  _WSAAsyncSelect(pC->S,hW,0,0);
  //_WSAAsyncSelect(pC->S,hW,zXmsg_socket+pC->N,FD_CLOSE);
  //_WSAAsyncSelect(pC->S,hW,0,0);/*un-async-ify socket*/

  *l=0;
  thread(doHttp,pC);
  return 0;
}

char *url_encode(char *dtn,char *src){
/* 'src' and 'dtn' MUST NOT point to the same buffer */
  register unsigned char *s,*d;
  for(s=src,d=dtn;*s;++s,++d){
    /* if (*s<'-' || (*s>'9' && *s<'A') || (*s>'Z' && *s<'a') || *s>'z'){ */
    /* RFC HTTP/1.1: unsafe = CTL | SP | <"> | "#" | "%" | "<" | ">" */
    if (*s<=' ' || *s=='#' || *s=='%' /*|| *s=='"' || *s=='<' || *s=='>'*/ || *s>=0x7F){
      *d='%';
      *++d=*s/16+'0';
      if (*d>'9')
        *d+=7;
      *++d=*s%16+'0';
      if (*d>'9')
        *d+=7;
    }
    else
      *d=*s;
  }
  *d=0;
  return dtn;
}

/*
MUST DO:
http://host:2857/~/C:/WINDOWS?"*.exe"   //display *.exe
http://host:2857/~/C:/WINDOWS?"*.exe"&>1024&<1048576 //display *.exe greater that size 1kB & less than 1MB
http://host:2857/~/C:/WINDOWS?:f   //display files
http://host:2857/~/C:/WINDOWS?:F   //display folders
http://host:2857/~/C:/WINDOWS?:hF  //display hidden folders
http://host:2857/~/C:/WINDOWS?:r   //display read-only files and folders
http://host:2857/~/C:/WINDOWS?:sf  //display system files
http://host:2857/~/C:/WINDOWS?:R   //recursively!
http://host:2857/~/C:/WINDOWS/COMMAND.COM?19203 //start from 19203; must override "Range:" token
*/
int doHttp(pConnectionData pC){
  register char *c;
  register unsigned int i;
  unsigned int k,j,t;  
	HANDLE hF;
	WIN32_FIND_DATA	F;
	FILETIME FT;
	SYSTEMTIME ST;
  SOCKADDR_IN sockAddr;
  char attr[8];
	char size[40];
	char name[MaxPath];
	ULARGE_INTEGER totalSpace,freeSpace;
  SOCKET  s=pC->S;
  fd_set fds;
  char *buf=pC->httpBuf;

  for(c=buf;*c;++c)
    if (*((unsigned short *)c)==0x0a0d)/*"\r\n" -> 0 */
      *c=0;
  for(c=buf+lstrlen(ChkConnType_http),i=0;*c && *c!=' ';++c,++i){
    if (*c=='%' && ((*(c+1)>='0' && *(c+1)<='9') ||(*(c+1)>='A' && *(c+1)<='F'))){
      *++c-='0';
      if (*c>9)
        *c-=7;
      *++c-='0';
      if (*c>9)
        *c-=7;
      *c+=16**(c-1);
    }
    name[i]=*c;
  }
  name[i]=0;/* null terminate 'name'; i:length of 'name' */
  t=sizeof(sockAddr);_getsockname(s,(struct sockaddr *)&sockAddr,&t);
  if (!i)/* request was "GET /" ; show drive list */{
    sendHttpHeader(s,buf,HTTP_OK,~0,"text/html",~0,0);
    _send(s,buf,wsprintf(buf,
            "<HTML><HEAD><TITLE>/</TITLE><BASE HREF=\"http://%s:%u/~/\"></HEAD><BODY><PRE><A HREF=\"/\">/</a>\n"
            "\n\n\n"
            "         free           total          type        name\n",
            _inet_ntoa(*((struct in_addr *)&sockAddr.sin_addr.s_addr)),httpPort),0);
    for(k/*->numDrives*/=0,GetLogicalDriveStrings(MaxPath,(char *)(i=(unsigned int)name));*((char *)i);i=(unsigned int)c+1){
      switch(GetDriveType((char *)i)){
      case DRIVE_REMOVABLE:
        t=0; /*(unsigned long)"remov";*/
        break;
      case DRIVE_FIXED:
        t=(unsigned long)"fixed";
        break;
      case DRIVE_REMOTE:
        t=(unsigned long)"netwk";
        break;
      case DRIVE_CDROM:
        t=(unsigned long)"cdrom";
        break;
      case DRIVE_RAMDISK:
        t=(unsigned long)"ramdk";
        break;
      default:
        t=(unsigned long)"unkwn";
        /*break;*/
      }
      if (!t){
        t=(unsigned long)"remov";
        totalSpace.HighPart=freeSpace.HighPart=~0;
      }else
        if (!GetDiskFreeSpaceEx((char *)i,0,&totalSpace,&freeSpace))
          totalSpace.HighPart=freeSpace.HighPart=~0;
      for(c=(char*)i;*c;++c){
        if (*c>='a' && *c<='z') /* capitalize drive letters */
          *c^=32;
        if (*c=='\\')           /* convert the '\'s into '/'s */
          *c='/';
      }
      _send(s,buf,wsprintf(buf,
            "   %10s      %10s     %9s        <A HREF=\"./%s\">%s</A>\n",
            getSizeStr(&freeSpace,size),getSizeStr(&totalSpace,size+11),t,i,i),0);
      ++k;/*++numDrives*/
    }
    _send(s,buf,wsprintf(buf,"\n%u logical drive(s)",k),0);
    _send(s,"</PRE></BODY></HTML>",20,0);
  }
  else{
    if (~(k=GetFileAttributes(name))){ /* there exists 'something' called 'name' */
      if (k & FILE_ATTRIBUTE_DIRECTORY){ /* if it is a directory, show html page listing contents */
        if (name[i-1]!='/') /* i:length of name */
          *((unsigned short *)(name+i++))='/'; /* '/' must be the last char for dirName */
        sendHttpHeader(s,buf,HTTP_OK,~0,"text/html",~0,0);
        i=wsprintf(buf,"<HTML><HEAD><TITLE>%s</TITLE><BASE HREF=\"http://%s:%u/~/",
          name,_inet_ntoa(*((struct in_addr *)&sockAddr.sin_addr.s_addr)),httpPort);
        _send(s,buf,i+wsprintf(buf+i,"%s\"></HEAD><BODY><PRE>\n",url_encode(buf+i,name)),0);
        lstrcpy(c=buf,"<A HREF=\"");c+=9/*lstrlen("<a href=\"")*/;
        for(--i,j=0;i;--i)
          if (name[i]=='/')/*count no. of '/' in 'j'*/{
            *((unsigned long *)c)='/..';/*lstrcat(c,"../");*/
            c+=3/*lstrlen("../")*/;
            ++j;
          }
        lstrcpy(c,"\">/</A>");c+=7/*lstrlen("\">/</A>")*/; /* <A HREF="../../../ ... /../">/</A> */
        for(k=0/*,i=0*/;--j;k=i+1){
          lstrcpy(c,"<A HREF=\"");c+=9/*lstrlen("<A HREF=\"")*/;
          for(i=0;i<j;++i,c+=3/*lstrlen("../")*/)
            *((unsigned long *)c)='/..';/*lstrcat(c,"../");*/
          *((unsigned long *)c)='>\"';/*lstrcat(buf,"\">");*/
          c+=2/*lstrlen("\">")*/;
          for(i=k;name[i]!='/';++i);
          lstrcpyn(c,name+k,i-k+1);c+=(i-k);
          lstrcpy(c,"/</A>");c+=5/*lstrlen("/</A>")*/;
        }
        _send(s,buf,c-buf+wsprintf(c,"<A HREF=\"./\">%s</A>\n\n",name+k),0);
        _send(s,buf,wsprintf(buf,
            "  "/* "<A HREF=\"../\">..</A>"/*Opera spoils the page, otherwise*/
            "                                                 <A HREF=\"../\">parent directory</A>"
            "\n\n"
            "              date time            size   attr     name\n"),0);
        *((unsigned short *)(name+lstrlen(name)))='*';/*lstrcat(name,"*");*/
        if ((hF=FindFirstFile(name,&F))!=INVALID_HANDLE_VALUE){
          for(i=1,j=k=0;i;i=FindNextFile(hF,&F))/*j:numFolders;k:numFiles;*/{
            if (*((unsigned short *)F.cFileName)=='.' || (*((unsigned long *)F.cFileName) & 0x00FFFFFF)=='..') /* =="." && ==".." */
              continue; /* ignore "." and ".." entries */
            *((unsigned int *)attr)='----';i=F.dwFileAttributes;
            if (i & FILE_ATTRIBUTE_READONLY)
              *((unsigned int *)attr)^=0x0000005F; /* 'r' ^ '-' = 5fh */
            if (i & FILE_ATTRIBUTE_HIDDEN)
              *((unsigned int *)attr)^=0x00004500; /* 'h' ^ '-' = 45h */
            if (i & FILE_ATTRIBUTE_SYSTEM)
              *((unsigned int *)attr)^=0x005E0000; /* 's' ^ '-' = 5eh */
            if (i & FILE_ATTRIBUTE_ARCHIVE)
              *((unsigned int *)attr)^=0x4C000000; /* 'a' ^ '-' = 4ch */
            attr[4]=0;
            FileTimeToLocalFileTime(&F.ftLastWriteTime,&FT);
            FileTimeToSystemTime(&FT,&ST);
            wsprintf(size+12,"%s, %.2u %s %.4u %.2u:%.2u:%.2u",
              Day[ST.wDayOfWeek],ST.wDay,Month[ST.wMonth-1],
                ST.wYear,ST.wHour,ST.wMinute,ST.wSecond);
            if (F.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
              ++j;i='[';
              size[0]=0;
            }
            else{
              ++k;i=' ';
              getSizeStr((ULARGE_INTEGER *)&F.nFileSizeLow,size);
            }
            _send(s,buf,wsprintf(buf,
                "  %s  %10s   %s   %c <A HREF=\"%s\">%s</A>\n",
                size+12,size,attr,i,url_encode/*lstrcpy*/(name,F.cFileName),F.cFileName),0);
          }/*for each dir entry*/
        }/*findfirstfile!=invalid_handle_value*/
        _send(s,buf,wsprintf(buf,"\n%u folder(s), %u file(s)</PRE></BODY></HTML>",j,k),0);
      }/*fileattr==file_directory*/
      else /* request was for a file ...*/{
        if (hF=CreateFile(name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)){
          GetFileTime(hF,0,0,&FT);
          FileTimeToSystemTime(&FT,&ST);
          t=GetFileSize(hF,0);
          getContentType(name,40,size);
          k/*start offset*/=~0;
          for(c=buf;*c;c+=lstrlen(c)+2/*'\r\n'*/){ /* find "Range:" value in the request */
            if (*((unsigned long *)(c))=='gnaR' && *((unsigned short *)(c+4))==':e'){
              while(*c<'0'|| *c>'9') ++c; /* find the first number */
              for(i=c-buf;*c>='0' && *c<='9';++c); /* find the next non-numeric char */
              *c=0; /* null-terminate */
              k=_atoi(buf+i); /* get the value */
              break; /* done */
            }
          }
          if (~k){ /* a "Range:" was specified */
            wsprintf(name,"Content-Range: bytes %u-%u/%u",k,t-1,t);
            sendHttpHeader(s,buf,HTTP_PartialContent,&ST,size,t-k,name);
          }
          else{
            k=0;
            sendHttpHeader(s,buf,HTTP_OK,&ST,size,t,0);
          }
          SetFilePointer(hF,k,0,FILE_BEGIN);
          /*  i:  bytes in the 'buf' already sent
              j:  bytes sent by the last 'send'
              t:  bytes in the 'buf' left to be sent
              k:  number of attempts left, on failure
           */
          while(ReadFile(hF,buf,HttpBufSize,&t,0) && t)
            for(i=0;t;t-=i){
              k=50;/*max number of retries*/
retry:
              if ((j=_send(s,buf+i,t,0))==SOCKET_ERROR){
                if (_WSAGetLastError()==WSAEWOULDBLOCK){
                  FD_ZERO(&fds);
                  FD_SET(s,&fds);
                  _select(0,0,&fds,0,&tv);
                  if (--k)
				            goto retry;
                }
                //wsErr();
                //wsprintf(buf,"name:%s;\ni=%u;j=%u;\nk=%u;t=%u\nT=%u",name,i,j,k,t,T);imb(buf);
                goto error;
              }
              i+=j;
            }
error:
          CloseHandle(hF);
        }
        else /*null hF*/
          goto notFound;
      }
    }
    else
notFound:
      sendHttpHeader(s,buf,HTTP_NotFound,0,0,~0,0);
  }
  /* _closesocket(pC->S);pC->S=INVALID_SOCKET; */
  closeConnection(pC->N);
	return 0;
}

int sendHttpHeader(SOCKET s,char *buf,int HttpRetCode,SYSTEMTIME *LastModified,char *ContentType,unsigned long ContentLength,char *extra){
  char date[32];
  SYSTEMTIME ST;
  register int l;

  GetSystemTime(&ST);
  wsprintf(date,"%s, %.2u %s %.4u %.2u:%.2u:%.2u",
    Day[ST.wDayOfWeek],ST.wDay,Month[ST.wMonth-1],
    ST.wYear,ST.wHour,ST.wMinute,ST.wSecond);
  l=wsprintf(buf, "HTTP/1.0 %u hi!\r\n"
                  "Server: " httpSer_ver "\r\n"
                  "Date: %s GMT\r\n",HttpRetCode,date);
  if (ContentType)
    l+=wsprintf(buf+l,"Content-Type: %s\r\n",ContentType);
  if (LastModified){
    if (~(unsigned long)LastModified)
      wsprintf(date,"%s, %.2u %s %.4u %.2u:%.2u:%.2u",
        Day[LastModified->wDayOfWeek],LastModified->wDay,Month[LastModified->wMonth-1],
        LastModified->wYear,LastModified->wHour,LastModified->wMinute,LastModified->wSecond);
    l+=wsprintf(buf+l,"Last-Modified: %s GMT\r\n",date);
  }
  if (~ContentLength)
    l+=wsprintf(buf+l,"Content-Length: %u\r\n",ContentLength);
  if (extra)
    l+=wsprintf(buf+l,"%s\r\n",extra);
  return _send(s,buf,l+wsprintf(buf+l,"Connection: close\r\n\r\n"),0);
}
