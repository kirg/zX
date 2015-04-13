#ifndef misc_h
#define misc_h

#include <windows.h>
#include "misc.h"

unsigned int _atox(const char *);
unsigned int _atoi(const char *);
unsigned int fillMemory(void *,unsigned int,char);
#define zeroMemory(buf, len) fillMemory(buf, len, 0)
int createProcess(char *);
int isPrefix(const char *,const char *);
char *getSizeStr(ULARGE_INTEGER *,char *);
int crackURL(const char *,char *,char *,unsigned short *,char *,char *,char *);
SOCKET tcpConnect(char *,unsigned short);

#endif