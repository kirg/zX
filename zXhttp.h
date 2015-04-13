#ifndef zXhttp_h
#define zXhttp_h

#include <winsock.h>
#include "types.h"

#define httpSer_ver  zXver " <zXhttp_v0.4.9>"
#define httpPort     zXport

int sendHttpHeader(SOCKET,char *,int,SYSTEMTIME *,char *,unsigned long,char *);
int doHttp(pConnectionData);

int isConnType_http(const char *);
char *getContentType(char *, int, char *);
int zXhttp(pConnectionData,int,char *);
char *url_encode(char *, char *);
int doHttp(pConnectionData);
int sendHttpHeader(SOCKET, char *, int, SYSTEMTIME *, char *, unsigned long, char *);

#endif