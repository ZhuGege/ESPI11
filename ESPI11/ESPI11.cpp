#include "stdafx.h"
#include "ESPI11.h"
#include "ZGGLog.h"
#include "InstallProtocol.h"

#pragma comment(lib,"Ws2_32.lib")



#pragma data_seg("Shared")
__declspec (allocate("Shared")) GUID CESPI11::m_stLayerGuid = {0};
#pragma data_seg()

WSPPROC_TABLE CESPI11::m_stNextWSPProcTable = {0};


void __stdcall CESPI11::SetLayerGUID(__in GUID& stLayerGuid)
{
	m_stLayerGuid = stLayerGuid;
}

int WSPAPI CESPI11::NewWSPStartup(WORD wVersionRequested,
								  LPWSPDATA lpWSPData,
								  LPWSAPROTOCOL_INFOW lpProtocolInfo,
								  WSPUPCALLTABLE UpcallTable,
								  LPWSPPROC_TABLE lpProcTable
								  )
{

	LOG_OUTPRINT("COME IN");

	LPWSAPROTOCOL_INFOW lpstWSAProtocolInfo = NULL;
	int nErrorCode = ERROR_SUCCESS;

	do 
	{
		if(lpProtocolInfo->ProtocolChain.ChainLen <= 1)
		{
			LOG_OUTPRINT("Protocol Chain too Short");
			break;
		}

		int nProtocolCount = 0;
		if(CInstallProtocol::EnumProto(lpstWSAProtocolInfo,(DWORD&)nProtocolCount) != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Enum Protocol Failed");
			break;
		}
		//找到我们的协议
		//我们的协议在第一个，因此下层协议在第二个
/*
		DWORD dwLayerID=0;
		for(int nIndex=0; nIndex < nProtocolCount; nIndex++)
		{
			if(memcmp(&lpstWSAProtocolInfo[nIndex].ProviderId,&m_stLayerGuid,sizeof(GUID))==0)
			{
				dwLayerID = lpstWSAProtocolInfo[nIndex].dwCatalogEntryId;
				break;
			}
		}
		//协议链中下一个Chain
		DWORD dwNextLayerId = 0;
		for(int nIndex = 0;nIndex < lpProtocolInfo->ProtocolChain.ChainLen; nIndex++)
		{
			if(lpProtocolInfo->ProtocolChain.ChainEntries[nIndex] == dwLayerID)
			{
				dwNextLayerId = lpProtocolInfo->ProtocolChain.ChainEntries[nIndex+1];
				break;
			}
		}
*/
		//下一个协议的文件路径
		int nNextSpiPathLen = MAX_PATH;
		TCHAR* ptszNextSpiPath = (TCHAR*)GlobalAlloc(GPTR,nNextSpiPathLen);
		DWORD dwNextLayerId = lpProtocolInfo->ProtocolChain.ChainEntries[1];
		for(int nIndex = 0; nIndex < nProtocolCount; nIndex++)
		{
			if(dwNextLayerId == lpstWSAProtocolInfo[nIndex].dwCatalogEntryId)
			{
				if(WSCGetProviderPath(&lpstWSAProtocolInfo[nIndex].ProviderId,ptszNextSpiPath,&nNextSpiPathLen,&nErrorCode) == SOCKET_ERROR)
				{
					nErrorCode =  WSAEPROVIDERFAILEDINIT;
				}
				break;
			}
		}
		if (nErrorCode != ERROR_SUCCESS)
		{
			break;
		}
		//扩展环境变量
		if(!ExpandEnvironmentStrings(ptszNextSpiPath,ptszNextSpiPath,nNextSpiPathLen))
		{
			nErrorCode = WSAEPROVIDERFAILEDINIT;
			break;
		}
		//加载协议文件
		HINSTANCE hDll=NULL;
		if((hDll = LoadLibrary(ptszNextSpiPath)) == NULL)
		{
			nErrorCode = WSAEPROVIDERFAILEDINIT;
			break;
		}
		//获取 WSPStartup 函数地址
		LPWSPSTARTUP pfnWSPStartup=NULL;
		if((pfnWSPStartup = (LPWSPSTARTUP)GetProcAddress(hDll,"WSPStartup")) == NULL)
		{
			nErrorCode = WSAEPROVIDERFAILEDINIT;
			break;
		}
		//调用下一个协议链的 WSPStartup 函数
		if((nErrorCode = pfnWSPStartup(wVersionRequested,lpWSPData,lpProtocolInfo,UpcallTable,lpProcTable)) != ERROR_SUCCESS)
		{
			break;
		}
		m_stNextWSPProcTable = *lpProcTable;
		//处理函数
		lpProcTable->lpWSPRecv=NewWSPRecv;
		lpProcTable->lpWSPCloseSocket=NewWSPCloseSocket;
		lpProcTable->lpWSPSend = NewWSPSend;
		
	} while (FALSE);
	
	CInstallProtocol::Free(lpstWSAProtocolInfo);
	return nErrorCode;
}

int WSPAPI CESPI11::NewWSPRecv(SOCKET s,
							   LPWSABUF lpBuffers,
							   DWORD dwBufferCount,
							   LPDWORD lpNumberOfBytesRecvd,
							   LPDWORD lpFlags,
							   LPWSAOVERLAPPED lpOverlapped,
							   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
							   LPWSATHREADID lpThreadId,
							   LPINT lpErrno
							   )
{
	int nResult = m_stNextWSPProcTable.lpWSPRecv(s,lpBuffers,dwBufferCount,lpNumberOfBytesRecvd,lpFlags,lpOverlapped,lpCompletionRoutine,lpThreadId,lpErrno);
	LOG_OUTPRINT("Recv:%s",lpBuffers->buf);
	return nResult;
}

//关闭连接
int WSPAPI CESPI11::NewWSPCloseSocket(SOCKET s,LPINT lpErrno)
{

	return m_stNextWSPProcTable.lpWSPCloseSocket(s,lpErrno);
}


int WSPAPI CESPI11::NewWSPSend(
								 SOCKET s,
								 LPWSABUF lpBuffers,
								 DWORD dwBufferCount,
								 LPDWORD lpNumberOfBytesSent,
								 DWORD dwFlags,
								 LPWSAOVERLAPPED lpOverlapped,
								 LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
								 LPWSATHREADID lpThreadId,
								 LPINT lpErrno
								 )
{
	LOG_OUTPRINT("Send:%s",lpBuffers->buf);
	return m_stNextWSPProcTable.lpWSPSend(s,lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags,lpOverlapped,lpCompletionRoutine,lpThreadId,lpErrno);
}






