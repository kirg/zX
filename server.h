#ifndef server_h
#define server_h

/* int fputs(pConnectionData,char *); */
int sendStr(pCommandData);
__inline int chk4break(pCommandData);
void closeConnection(int);
void closeAllConnections(char *);
int zXstart();
int zXstop(char *);
void zXshutDown(char *);
int beginCurrentDirectory(pCommandData);
void endCurrentDirectory();
unsigned int setCurrentDirectory(pCommandData,char *);
unsigned int setCurrentFile(pCommandData,char *);

#endif