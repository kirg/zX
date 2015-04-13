#ifndef command_h
#define command_h

#define c_pt	0x00007470	//port
#define c_port  0x6F702000
#define c_ip	0x00007069	//ip_dotted_octet
#define c_mc	0x0000636D	//host name to ip
#define c_up	0x00007075	//upload
#define c_dn	0x00006E64	//download
#define c_ux	0x00007875	//upload and execute

#define c_cd	0x00006463	//change dir
#define c_ls	0x0000736C	//list files
#define c_dir   0x69642000
#define c_dl	0x00006C64	//drive list
#define c_rm	0x00006D72	//remove file
#define c_del   0x65642000
#define c_md	0x0000646D	//make directory
#define c_mv	0x0000766D	//move file
#define c_move  0x6F6D2000
#define c_cp	0x00007063	//copy file
#define c_copy  0x6F632000
#define c_at	0x00007461	//set attributes
#define c_attr  0x74612000
#define c_ex	0x00007865	//execute
#define c_exec  0x78652000

#define c_hi	0x00006968	//hi
#define c_qt	0x00007471	//quit
#define c_quit	0x74697571	//quit
#define c_zX	0x0000587A	//shutdown server 

#define c_co	0x00006F63	//command reference
#define c_hp	0x00007068	//help
#define c_help	0x706C6568	//help
#define c_QQ	0x00003F3F	//??:help
#define c_bp	0x00007062	//beep
#define c_beep	0x70656562	//beep
#define c_mx	0x0000786D	//message-box
#define c_eo	0x00006F65	//echo
#define c_echo  0x63652000
#define c_sm	0x00006D73	//system
#define c_wc	0x00006377	//window control
#define c_ps	0x00007370	//process status
#define c_kl	0x00006C6B	//kill process
#define c_kill  0x696B2000
#define c_ms	0x0000736D	//miscellaneous
#define c_misc  0x696D2000

#define c_cls	0x00736C63	//clear screen
#define c_test	0x74736574	//test
#define c_f1	0x00003166	//f1
#define c_Q		0x0000003F	//?
#define c_t		0x00000074	//t
#define c_xo	  0x00006F78

extern unsigned int zXcmdTable_num;
extern int (*zXcmd_not_found)(pCommandData);
extern unsigned int zXcmdTable_cmd[];
extern int (*zXcmdTable_func[])(pCommandData);

int zXcmd_unrecognised(pCommandData);
int init_commandTable();
int (*(zXcmd(unsigned int, unsigned int *)))(pCommandData);

#endif