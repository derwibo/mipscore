/* dllmain.cxx
 *
 */

#include <Python.h>
//#include <windows.h>
#include "dllmain.h"
#include "mips.h"

/*
BOOL WINAPI DllMain(HINSTANCE hinstDLL,	DWORD fdwReason, LPVOID lpvReserved)
{
  if(fdwReason == DLL_PROCESS_ATTACH)
  {
    TApplication::hInstance = hinstDLL;
    TApplication::hPrevInstance = 0;
    TApplication::nCmdShow = SW_SHOWNORMAL;
    TApplication::lpszCmdLine = 0;
  }
  return TRUE;
}
*/

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_mips(void)
{
  return InitMips();
}
#else
PyMODINIT_FUNC initmips(void)
{
  InitMips();
}
#endif

