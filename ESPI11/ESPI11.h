

#include <WinSock2.h>
#include <Windows.h>
#include <WS2spi.h >
#include <SpOrder.h>
#include <atldef.h>
#include "CommonHeader.h"





class CESPI11
{

public:

	static int WSPAPI NewWSPStartup(WORD wVersionRequested,LPWSPDATA lpWSPData,LPWSAPROTOCOL_INFOW lpProtocolInfo,WSPUPCALLTABLE UpcallTable,LPWSPPROC_TABLE lpProcTable);
	static int WSPAPI NewWSPRecv(SOCKET s,LPWSABUF lpBuffers,DWORD dwBufferCount,LPDWORD lpNumberOfBytesRecvd,LPDWORD lpFlags,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,LPWSATHREADID lpThreadId,LPINT lpErrno);
	static int WSPAPI NewWSPCloseSocket(SOCKET s,LPINT lpErrno);
	static int WSPAPI NewWSPSend( SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno );
	static void __stdcall SetLayerGUID(__in GUID& stLayerGuid);
private:

	static WSPPROC_TABLE m_stNextWSPProcTable;

	static GUID m_stLayerGuid;

};