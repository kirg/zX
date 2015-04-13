#ifndef cmd_file_h
#define cmd_file_h

#include "types.h"

int zXcmd_dl(pCommandData);
int zXcmd_ls(pCommandData);
int zXcmd_cd(pCommandData);
int zXcmd_md(pCommandData);
int zXcmd_cpmv(pCommandData);
int zXcmd_rm(pCommandData);
int zXcmd_at(pCommandData);
int zXcmd_ex(pCommandData);

#endif