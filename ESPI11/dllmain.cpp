// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ESPI11.h"
#include "InstallProtocol.h"
#include "ZGGLog.h"

HMODULE g_hModdule;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			char szParentProcessName[MAX_PATH] = {0};
			if (GetModuleFileNameA(NULL,szParentProcessName,_countof(szParentProcessName)) != 0)
			{
				char* pszProcName = strrchr(szParentProcessName, '\\');
				if (pszProcName != NULL)
				{
					pszProcName++;
					if(_strnicmp(pszProcName,"chrome.exe",strlen(pszProcName)) == 0
						|| _strnicmp(pszProcName,"test.exe",strlen(pszProcName)) == 0)
					{		
						DWORD dwPID = GetCurrentProcessId();
						char szLogName[MAX_PATH] = {0};
						sprintf_s(szLogName,"%s%d%s",pszProcName,dwPID,".log");
						CZGGLog::GetZggLogInstance()->InitLog(szLogName);
					}
				}
			}
			g_hModdule = hModule;

		}

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

