#ifndef zXteln_h
#define zXteln_h

int isConnType_teln(const char *);
int zXshowPrompt(pConnectionData);
int zXexecCmd(pCommandData);
int executeCommand(pConnectionData);
int zXteln(pConnectionData,int,char *);

#endif