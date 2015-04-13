
#include "defines.h"
#include "macros.h"
#include "types.h"
#include "globals.h"

#include "command.h"
#include "server.h"

#include "cmd_file.h"
#include "cmd_ftxr.h"
#include "cmd_help.h"
#include "cmd_notf.h"
#include "cmd_proc.h"
#include "cmd_serv.h"
#include "cmd_sys.h"
#include "cmd_temp.h"
#include "cmd_win.h"

#define MaxNumOfCommands 256
/*
#define _zXcmd_(X)	int zXcmd_##X	(pCommandData)

_zXcmd_(	pt	);
_zXcmd_(	ip	);
_zXcmd_(	mc	);
_zXcmd_(	up	);
_zXcmd_(	dn	);
_zXcmd_(	ux	);

_zXcmd_(	cd	);
_zXcmd_(	ls	);
_zXcmd_(	dl	);
_zXcmd_(	rm	);
_zXcmd_(	md	);
_zXcmd_(	mv	);
_zXcmd_(	cpmv);
_zXcmd_(	at	);
_zXcmd_(	ex	);

_zXcmd_(	hi	);
_zXcmd_(	qt	);
_zXcmd_(	zX	);

_zXcmd_(	co	);
_zXcmd_(	hp	);
_zXcmd_(	QQ	);
_zXcmd_(	bp	);
_zXcmd_(	mx	);
_zXcmd_(	eo	);
_zXcmd_(	sm	);
_zXcmd_(	wc	);
_zXcmd_(	ps	);
_zXcmd_(	kl	);
_zXcmd_(	ms	);
_zXcmd_(	cls	);
_zXcmd_(	test);

_zXcmd_(	f1	);
_zXcmd_(	Q	);
_zXcmd_(	t	);
_zXcmd_(	xo	);
*/
unsigned int	zXcmdTable_num;
int				(*zXcmd_not_found)(pCommandData);

unsigned int zXcmdTable_cmd[MaxNumOfCommands+1]=
{
	c_pt,		c_ip,		c_mc,		c_up,		c_dn,//		c_ux,

	c_dl,		c_cd,		c_md,		c_ls,		c_ex,
	
	c_rm,		c_mv,		c_cp,		c_at,

	c_bp,		c_mx,		c_sm,		c_wc,		c_ms,		c_ps,		c_kl,

	c_hp,		c_QQ,		c_hi,		c_eo,		c_qt,		c_zX,
	
	c_co,		c_test,		c_cls,		c_f1,		c_Q,		c_t, c_xo,

	0
};

int (*zXcmdTable_func[MaxNumOfCommands+1])(pCommandData)=
{
	zXcmd_pt,	zXcmd_ip,	zXcmd_mc,	zXcmd_up,	zXcmd_dn,//	zXcmd_ux,

	zXcmd_dl,	zXcmd_cd,	zXcmd_md,	zXcmd_ls,	zXcmd_ex,

	zXcmd_rm,	zXcmd_cpmv,	zXcmd_cpmv,	zXcmd_at,

	zXcmd_bp,	zXcmd_mx,	zXcmd_sm,	zXcmd_wc,	zXcmd_ms,	zXcmd_ps,	zXcmd_kl,

	zXcmd_hp,	zXcmd_hp,	zXcmd_hi,	zXcmd_eo,	zXcmd_qt,	zXcmd_zX,

	zXcmd_co,	zXcmd_test,	zXcmd_cls,	zXcmd_f1,	zXcmd_Q,	zXcmd_t, zXcmd_xo,

	0
};

int zXcmd_unrecognised(pCommandData X){
	wsprintf(X->sbuf,"= [%.8X] \"%s\" command unrecognized; type \"??\" for command list",X->ncmd,X->cmd);
	sendStr(X);
	return 0;
}

int init_commandTable()
{
  int i,j;
  /* bubble sort */
  for(i = 0; zXcmdTable_cmd[i + 1]; ++i)
    for(j= i + 1; zXcmdTable_cmd[j]; ++j)
      if (zXcmdTable_cmd[i] > zXcmdTable_cmd[j]){
        zXcmdTable_cmd[i] ^= zXcmdTable_cmd[j] ^= zXcmdTable_cmd[i] ^= zXcmdTable_cmd[j];
#define func_i ((int)zXcmdTable_func[i])
#define func_j ((int)zXcmdTable_func[j])
        func_i ^= func_j ^= func_i ^= func_j;
      }
  zXcmd_not_found = zXcmd_unrecognised;
  return zXcmdTable_num = i + 1;
}

int (*(zXcmd(unsigned int cmd,unsigned int *index)))(pCommandData)
{
	//binary search in the sorted cmdTable array
	int top,bot,mid;
	top=0;bot=zXcmdTable_num-1;
#ifdef caseSensitiveCommands
	while(top<=bot)
	{
		mid=(top+bot)/2;
		if (cmd==zXcmdTable_cmd[mid])
		{
			if (index)
				*index=mid;
			return zXcmdTable_func[mid];
		}
		if (cmd<zXcmdTable_cmd[mid])
			bot=mid-1;
		else
			top=mid+1;
	}
	if (index)
		*index=0;
	return 0;//zXcmd_not_found;
#else
  // ie, ignore case
	////// BUG BUG BUG /// some bug in this piece of code
	cmd&=0xDFDFDFDF;
	while(top<=bot)
	{
		mid=(top+bot)/2;
		if (!((cmd^zXcmdTable_cmd[mid])&0xDFDFDFDF))
		{
			if (index)
				*index=mid;
			return zXcmdTable_func[mid];
		}
		if (cmd<(zXcmdTable_cmd[mid]&0xDFDFDFDF))
			bot=mid-1;
		else
			top=mid+1;
	}
	////// BUG BUG BUG /// end
	if (index)
		*index=0;
	return 0;//zXcmd_not_found;
#endif
}
