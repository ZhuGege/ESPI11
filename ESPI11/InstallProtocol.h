#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <WS2spi.h >
#include <SpOrder.h>
#include <atldef.h>
#include "CommonHeader.h"

class CInstallProtocol
{
public:
	static BOOL  _stdcall IsProcotolInstalled(
		__in const GUID& stLayerGuid
		);

	static DWORD _stdcall InstallProtocol(
		__in const GUID& stLayerGuid, 
		__in const GUID& stChainGuid, 
		__in const WCHAR* pwszProtocolDllPath, 
		__in const WCHAR* pwszProtocolName, 
		__in int nProtocolType
		);

	static BOOL  _stdcall RemoveProtocol(
		__in const GUID& stLayerGuid, 
		__in const GUID& stChainGuid
		);

	static void  _stdcall Free(
		__inout LPWSAPROTOCOL_INFOW& lpstWSAProtocolInfo
		);

	static DWORD _stdcall EnumProto(
		__inout LPWSAPROTOCOL_INFOW& lpstWSAProtocolInfo,
		__inout DWORD& dwNumberOfProtocols
		);

private:
	static DWORD _stdcall InstallLayerProtocol(
		__in const GUID& stGuid,
		__in const WCHAR* pwszDllPath,
		__in const WCHAR* pwszLayerProtocolName,
		__in int nProtocolType,
		__out DWORD& dwInstalledLayerProtocolCataID
		);
	static DWORD __stdcall InstallProtocolChain(
		__in const GUID& stGuid, 
		__in const WCHAR* pwszDllPath, 
		__in const WCHAR* pwszProtocolChainName, 
		__in int nProtocolType,
		__in const DWORD dwInstalledLayerCataID
		);
	static DWORD _stdcall AdjustProtocolSequence(
		__in const GUID& stLayerGuid, 
		__in const GUID& stChainGuid
		);
};