#include "defines.h"
#include "globals.h"
#include "misc.h"

unsigned int _atox(const char *a){
/*__asm{
    xor eax,eax
    mov esi,a
next:
    movzx ecx,[byte ptr esi]
    and ecx,ecx
    jz finished
    shl eax,4
    and cl,0DFh
    sub ecx,10h
    add eax,ecx
    cmp ecx,0Ah
    jl skip
    sub eax,27h
skip:
    inc esi
    jmp next
finished:
*/
  register int i=0,j;
  while(j=*(a++)){
    i<<=4;
    j&=0xDF;/*upper-case-ify ; to make it case-insensitive*/
    j-=0x10;
    if (j>9)
      j-=0x27;
    i+=j;
  }
  return i;
}

unsigned int _atoi(const char *a){
/*__asm{
    xor eax,eax
    mov ecx,eax
    mov ebx,a
next:
    movzx ecx,[byte ptr esi]
    and ecx,ecx
    jz finished
    mov edx,0Ah
    mul edx
    sub ecx,30h
    add eax,ecx
    inc ebx
    jmp next
finished:
  };*/
  register int i=0;
	while(*a){
    i*=10;
    i+=*(a++)-'0';
  }
  return i;
}

unsigned int fillMemory(void *buf,unsigned int len,char fill){
/*__asm{
    mov al,fill
    mov edi,buf
    mov ecx,len
    rep stosb
	};
*/
	while(len--)
		*(((char *)buf)++)=fill;
	return len;
}

int createProcess(char *path){
  PROCESS_INFORMATION procInfo;
  STARTUPINFO startupInfo;
  zeroMemory(&startupInfo,sizeof(STARTUPINFO));
	startupInfo.cb=sizeof(STARTUPINFO);
	startupInfo.dwFlags=STARTF_FORCEOFFFEEDBACK;
  return CreateProcess(0,path,0,0,0,0,0,0,&startupInfo,&procInfo);
}

int isPrefix(const char *pfx,const char *str){
  register int i=0;
  for(i=0;pfx[i] && str[i] && (str[i]==pfx[i]);++i);
  return !pfx[i];
}

char *getSizeStr(ULARGE_INTEGER *n,char *str){
	if (~n->HighPart)/*n->HighPart!=0xffffffff*/
		if (n->HighPart || n->LowPart>=0x40000000)
			wsprintf(str,"%u.%.2u GB",((n->HighPart<<2)+((n->LowPart&0xC0000000)>>30)),((((n->LowPart & 0x3FF00000)>>20)*1000)>>10)/10);
		else
			if(n->LowPart>=0x100000)
				wsprintf(str,"%u.%.2u MB",((n->LowPart & 0x3FF00000)>>20),((((n->LowPart & 0xFFC00)>>10)*1000)>>10)/10);
			else
				if(n->LowPart>=0x400)
					wsprintf(str,"%u.%.2u kB",(((n->LowPart&0xFFC00)>>10)),(((n->LowPart & 0x3FF)*1000)>>10)/10);
				else
					wsprintf(str,"%u  B",n->LowPart & 0x3FF);
	else
		str[0]=0;
	return str;
}

int crackURL(const char *url,char *host,char *path,unsigned short *port,char *prot,char *user,char *pass){
	char str[MaxUrlLen];
	int i;
	int _prot,_user,_pass,_host,_port,_path;/* position markers within the url-string */

	lstrcpyn(str,url,MaxUrlLen);
	_prot=_user=_pass=_port=_path=~0;
	_host=i=0;

doNext:
	switch(str[i]){
	case '/':
		str[i]=0;
		_path=++i;
	case 0:
		goto finished;
		/*break;*/
	case ':':
		str[i++]=0;
		if (*((unsigned short *)(str+i))=='//'){
			if (~_prot)
				goto error;
			_prot=_host;
			_host=i+=2;
		}
		else{
			if (~_port)
				goto error;
			_port=i;
		}
		break;
	case '@':
		if (!~_user){
			str[i]=0;
			_user=_host;
			_pass=_port;
		}
		_host=++i;
		_port=~0;
		break;
	default:
		++i;
	}
	goto doNext;

finished:
	if (host)
		lstrcpy(host,str+_host);
	if (~_prot && prot)
		lstrcpy(prot,str+_prot);
	if (~_user && user)
		lstrcpy(user,str+_user);
	if (~_pass && pass)
		lstrcpy(pass,str+_pass);
	if (~_port && port)
		*port=(unsigned short)_atoi(str+_port);
	if (~_path && path){
		path[0]='/';
		lstrcpy(path+1,str+_path);
	}
	return 0;
error:
	return ~0;
}

SOCKET tcpConnect(char *svr,unsigned short port){
	SOCKET	s;
	struct sockaddr_in a;
	struct hostent *h;

 	a.sin_family=AF_INET;
	a.sin_port=_htons(port);

 	if ((s=_socket(AF_INET, SOCK_STREAM, 0))<0)
		goto failed;
  if (((a.sin_addr.s_addr=_inet_addr(svr))==INADDR_NONE))
    if (h=(struct hostent *)_gethostbyname(svr))
      a.sin_addr.s_addr=*((unsigned long *)h->h_addr);
    else
      goto failed;
	if (_connect(s, (struct sockaddr *)&a, sizeof(a))>=0)
		return s;
failed:
  _closesocket(s);
  return -1;
}
