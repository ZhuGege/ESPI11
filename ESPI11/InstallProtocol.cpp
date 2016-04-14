#include "InstallProtocol.h"
#include "ZGGLog.h"

BOOL  CInstallProtocol::IsProcotolInstalled(__in const GUID& stLayerGuid)
{
	LPWSAPROTOCOL_INFOW lpstWSAProtocolInfo = NULL;
	DWORD dwNumOfProtocols = 0;
	bool bRet = FALSE;

	do 
	{
		if(EnumProto(lpstWSAProtocolInfo, dwNumOfProtocols) != ERROR_SUCCESS)
		{
			break;
		}
		//查找安装的协议
		for(DWORD dwIndex = 0; dwIndex < dwNumOfProtocols; dwIndex++)
		{
			if(memcmp(&lpstWSAProtocolInfo[dwIndex].ProviderId,&stLayerGuid,sizeof(GUID)) == 0)
			{
				bRet = TRUE;
				break;
				//已安装
			}
		}
	} while (FALSE);
	
	Free(lpstWSAProtocolInfo);

	return bRet;
}

DWORD CInstallProtocol::EnumProto(__inout LPWSAPROTOCOL_INFOW& lpstWSAProtocolInfo, __inout DWORD& dwNumberOfProtocols)
{
	DWORD dwErrorCode = ERROR_SUCCESS;

	do 
	{
		int nErrorCode	= 0;
		DWORD dwProtoInfoSize = 0; //协议信息结构缓冲区大小
		Free(lpstWSAProtocolInfo);
		//获取协议链缓冲大小
		if(WSCEnumProtocols(NULL,lpstWSAProtocolInfo,&dwProtoInfoSize,&nErrorCode) == SOCKET_ERROR)
		{
			if(nErrorCode != WSAENOBUFS)
			{
				dwErrorCode = ESPI_ERROR_CODE_FIRST_WSCEnumProtocols_FAILED;
				LOG_OUTPRINT("First WSCEnumProtocols Failed,ErrorCode:%d",nErrorCode);
				break;
			}
		}
		//分配内存空间
		if((lpstWSAProtocolInfo=(LPWSAPROTOCOL_INFOW)GlobalAlloc(GPTR,dwProtoInfoSize)) == NULL)
		{
			dwErrorCode = ESPI_ERROR_CODE_GlobalAlloc_FAILED;
			LOG_OUTPRINT("GlobalAlloc Failed,ErrorCode:%d",GetLastError());
			break;
		}
		//枚举协议链
		if((dwNumberOfProtocols = WSCEnumProtocols(NULL,lpstWSAProtocolInfo,&dwProtoInfoSize,&nErrorCode)) == SOCKET_ERROR)
		{
			dwErrorCode = ESPI_ERROR_CODE_SECOND_WSCEnumProtocols_FAILED;
			LOG_OUTPRINT("Second WSCEnumProtocols Failed,ErrorCode:%d",nErrorCode);
			break;
		}
	} while (FALSE);

	return dwErrorCode;
}

DWORD CInstallProtocol::InstallProtocol(__in const GUID& stLayerGuid, __in const GUID& stChainGuid, __in const WCHAR* pwszProtocolDllPath, __in const WCHAR* pwszProtocolName ,__in int nProtocolType)
{
	DWORD dwErrorCode = ERROR_SUCCESS;

	do 
	{
		LOG_OUTPRINT("Begin Install Protocol...");
		
		//GUID stGuid = { 0xd0d86306, 0x49bf, 0x49f6, { 0x9e, 0xf7, 0x3c, 0xf2, 0x38, 0x82, 0x92, 0x81 } };
		//GUID stChainGuid = { 0x6a7ce70, 0xf0fd, 0x4500, { 0x9d, 0x43, 0xd0, 0xa6, 0xf8, 0xa4, 0xe6, 0x77 } };
		DWORD dwLayerProtocolCataID = 0xFFFFFFFF;
		dwErrorCode = InstallLayerProtocol(stLayerGuid,pwszProtocolDllPath,pwszProtocolName,nProtocolType,dwLayerProtocolCataID);
		if (dwErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Install Layer Protocol Failed,ErrorCode:%d",dwErrorCode);
			break;
		}

		WCHAR wszProtocolChainName[MAX_PATH] = {0};
		_snwprintf_s(wszProtocolChainName,_TRUNCATE,L"%s%s",pwszProtocolName,L"Chain");
		dwErrorCode = InstallProtocolChain(stChainGuid,pwszProtocolDllPath,wszProtocolChainName,nProtocolType,dwLayerProtocolCataID);
		if (dwErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Install Protocol Chain Failed,ErrorCode:%d",dwErrorCode);
			break;
		}

		dwErrorCode = AdjustProtocolSequence(stLayerGuid,stChainGuid);
		if (dwErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Adjust Protocol Sequence Failed,ErrorCode:%d",dwErrorCode);
			break;
		}
		LOG_OUTPRINT("Install Protocol Sucess!");

	} while (FALSE);

	if (dwErrorCode != ERROR_SUCCESS)
	{
		RemoveProtocol(stLayerGuid,stChainGuid);
	}

	return dwErrorCode;
}



BOOL  CInstallProtocol::RemoveProtocol(__in const GUID& stLayerGuid, __in const GUID& stChainGuid)
{
	int nErrorCode = 0;
	BOOL bRemoveSuccess = TRUE;
	do 
	{
		if (WSCDeinstallProvider((LPGUID)&stChainGuid,&nErrorCode) == SOCKET_ERROR)
		{
			LOG_OUTPRINT("Remove Spi Chain Failed,ErrorCode:%d",nErrorCode);
			bRemoveSuccess = FALSE;
		}
		if(WSCDeinstallProvider((LPGUID)&stLayerGuid,&nErrorCode) == SOCKET_ERROR)
		{
			LOG_OUTPRINT("Remov Spi layer Failed,ErrorCode:%d",nErrorCode);
			bRemoveSuccess = FALSE;
		}
	} while (FALSE);

	return bRemoveSuccess;
}

//释放内存
void  CInstallProtocol::Free(__inout LPWSAPROTOCOL_INFOW& lpstWSAProtocolInfo)
{
	if (lpstWSAProtocolInfo != NULL)
	{
		GlobalFree(lpstWSAProtocolInfo);
		lpstWSAProtocolInfo = NULL;

	}
}



DWORD CInstallProtocol::InstallLayerProtocol(__in const GUID& stGuid,
											 __in const WCHAR* pwszDllPath,
											 __in const WCHAR* pwszLayerProtocolName, 
											 __in int nProtocolType,
											 __out DWORD& dwInstalledLayerProtocolCataID
											 )
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	int nErrorCode    = ERROR_SUCCESS;
	LPWSAPROTOCOL_INFOW lpstWSAProtocolInfo = NULL;

	do 
	{
		LOG_OUTPRINT("Enum Exist Protocol");
		int nNumOfProtocols = 0;
		dwErrorCode = EnumProto(lpstWSAProtocolInfo,(DWORD&)nNumOfProtocols);
		if(dwErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Enum Exist Protocol Failed");
			break;
		}
		LOG_OUTPRINT("Look For TCP Protocol");
		//查找对应协议
		int nIndex = 0;
		WSAPROTOCOL_INFOW stInstalledLayerInfo = {0};
		
		for(nIndex = 0; nIndex < nNumOfProtocols; nIndex++)
		{

			if(lpstWSAProtocolInfo[nIndex].iAddressFamily == AF_INET 
				//Internet协议
				&& lpstWSAProtocolInfo[nIndex].iProtocol == nProtocolType)
				//TCP协议
			{

				memcpy(&stInstalledLayerInfo,&lpstWSAProtocolInfo[nIndex],sizeof(WSAPROTOCOL_INFOW));
				stInstalledLayerInfo.dwServiceFlags1=lpstWSAProtocolInfo[nIndex].dwServiceFlags1 & (~XP1_IFS_HANDLES);

				break;

			}
		}
		if (nIndex >= nNumOfProtocols)
		{
			LOG_OUTPRINT("Not Find Appropriate Protocol");
			dwErrorCode = ESPI_ERROR_CODE_NOT_FIND_APPROPRIATE_PROTOCOL;
			break;
		}
		wcscpy_s(stInstalledLayerInfo.szProtocol,pwszLayerProtocolName);
		stInstalledLayerInfo.ProtocolChain.ChainLen = LAYERED_PROTOCOL;
		LOG_OUTPRINT("Install Protocol");
		//安装协议
		if(WSCInstallProvider((LPGUID)&stGuid,pwszDllPath,&stInstalledLayerInfo,1,&nErrorCode) == SOCKET_ERROR)
		{
			dwErrorCode = ESPI_ERROR_CODE_INSTALL_LAYERED_PROTOCOL_FAILED;
			LOG_OUTPRINT("Install Layered Protocol Failed,ErrorCode:%d",nErrorCode);
			break;
		}
		//重新枚举，找到安装的协议
		LOG_OUTPRINT("Enum Protocol Again,and Look For Installed Protocol ID");
		dwErrorCode = EnumProto(lpstWSAProtocolInfo,(DWORD&)nNumOfProtocols);
		if (dwErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Enum Installed Protocol Failed,ErrorCode:%d",dwErrorCode);
			break;
		}
		//查找安装的协议
		for(int nIndex = 0; nIndex < nNumOfProtocols; nIndex++)
		{
			if(memcmp(&lpstWSAProtocolInfo[nIndex].ProviderId, &stGuid, sizeof(GUID)) == 0)
			{
				dwInstalledLayerProtocolCataID = lpstWSAProtocolInfo[nIndex].dwCatalogEntryId;
				break;
			}
		}
		if (nIndex >= nNumOfProtocols)
		{
			LOG_OUTPRINT("Not Find Installed Ptotocol");
			dwErrorCode = ESPI_ERROR_CODE_NOT_FIND_INSTALLED_PROTOCOL;
			break;
		}
	} while (FALSE);

	Free(lpstWSAProtocolInfo);
	return dwErrorCode;
}

DWORD CInstallProtocol::InstallProtocolChain(__in const GUID& stChainGuid,
											 __in const WCHAR* pwszDllPath,
											 __in const WCHAR* pwszProtocolChainName,
											 __in int nProtocolType,
											 __in const DWORD dwInstalledLayerCataID
											 )
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	int nErrorCode	  = ERROR_SUCCESS;

	LPWSAPROTOCOL_INFOW lpstWSAProtocolInfo = NULL;
	do 
	{
		int nNumOfProtocols = 0;
		dwErrorCode = EnumProto(lpstWSAProtocolInfo,(DWORD&)nNumOfProtocols);
		if(dwErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Enum Exist Protocol Failed");
			break;
		}
		LOG_OUTPRINT("Insert Protocol To TcpChain");
		WSAPROTOCOL_INFOW stTcpChainInfo = {0};
		DWORD dwTcpOrigCataID = 0;
		int nIndex = 0;
		for(nIndex = 0; nIndex < nNumOfProtocols; nIndex++)
		{

			if(lpstWSAProtocolInfo[nIndex].iAddressFamily == AF_INET 
				//Internet协议
				&& lpstWSAProtocolInfo[nIndex].iProtocol == nProtocolType)
				//TCP协议
			{
				dwTcpOrigCataID = lpstWSAProtocolInfo[nIndex].dwCatalogEntryId;

				memcpy(&stTcpChainInfo,&lpstWSAProtocolInfo[nIndex],sizeof(WSAPROTOCOL_INFOW));
				stTcpChainInfo.dwServiceFlags1=lpstWSAProtocolInfo[nIndex].dwServiceFlags1 & (~XP1_IFS_HANDLES);

				break;
			}
		}

		int nProtocolListLen=0;
		WCHAR wszChainName[WSAPROTOCOL_LEN + 1] = {0};
		WSAPROTOCOL_INFOW stChainArray[1];
		wcscpy_s(wszChainName,pwszProtocolChainName);
		wcscpy_s(stTcpChainInfo.szProtocol,wszChainName);
		
		if(stTcpChainInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)
		{
			stTcpChainInfo.ProtocolChain.ChainEntries[1] = dwTcpOrigCataID;
		}
		else
		{
			for(int i = stTcpChainInfo.ProtocolChain.ChainLen; i > 0; i--)
			{
				stTcpChainInfo.ProtocolChain.ChainEntries[i] = stTcpChainInfo.ProtocolChain.ChainEntries[i-1];
			}
		}
		stTcpChainInfo.ProtocolChain.ChainLen++;
		stTcpChainInfo.ProtocolChain.ChainEntries[0] = dwInstalledLayerCataID;
		memcpy(&stChainArray[nProtocolListLen++],&stTcpChainInfo,sizeof(WSAPROTOCOL_INFOW));

		LOG_OUTPRINT("Install Protocol Chain");
		//安装Chain
		if(WSCInstallProvider((LPGUID)&stChainGuid,pwszDllPath,stChainArray,nProtocolListLen,&nErrorCode) == SOCKET_ERROR)
		{
			LOG_OUTPRINT("Install Protocol Chain Failed,ErrorCode:%d",nErrorCode);
			dwErrorCode = ESPI_ERROR_CODE_INSTALL_PROTOCOL_CHAIN_FAILED;
			break;
		}
		LOG_OUTPRINT("Enum Protocol Chain");
		dwErrorCode = EnumProto(lpstWSAProtocolInfo,(DWORD&)nNumOfProtocols);
		if(dwErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Enum Protocol Chain Failed,ErrorCode:%d",dwErrorCode);
			break;
		}
	} while (FALSE);

	Free(lpstWSAProtocolInfo);
	return dwErrorCode;
}
DWORD CInstallProtocol::AdjustProtocolSequence(__in const GUID& stLayerGuid, 
											   __in const GUID& stChainGuid
											   )
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	int nErrorCode	  = ERROR_SUCCESS;
	LPWSAPROTOCOL_INFOW lpstWSAProtocolInfo = NULL;

	do 
	{
		int nNumOfProtocols = 0;
		dwErrorCode = EnumProto(lpstWSAProtocolInfo,(DWORD&)nNumOfProtocols);
		if(dwErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Enum Exist Protocol Failed");
			break;
		}
		//调整顺序
		LOG_OUTPRINT("Ready To Adjust Sequeece");
		LPDWORD lpDwCateaEntries;
		if((lpDwCateaEntries = (LPDWORD)GlobalAlloc(GPTR,nNumOfProtocols*sizeof(WSAPROTOCOL_INFOW))) == NULL)
		{
			LOG_OUTPRINT("Alloc Mem Failed");
			dwErrorCode = ESPI_ERROR_CODE_GlobalAlloc_FAILED;
			break;
		}

		int nCataEntryIndex = 0;
		for(int nIndex = 0; nIndex < nNumOfProtocols; nIndex++)
		{
			//将安装的协议放在第一个
			if(memcmp(&lpstWSAProtocolInfo[nIndex].ProviderId,&stLayerGuid,sizeof(GUID)) == 0
				|| memcmp(&lpstWSAProtocolInfo[nIndex].ProviderId,&stChainGuid,sizeof(GUID)) == 0)
			{
				lpDwCateaEntries[nCataEntryIndex++]=lpstWSAProtocolInfo[nIndex].dwCatalogEntryId;
			}
		}
		for(int nIndex = 0; nIndex < nNumOfProtocols; nIndex++)
		{
			//其他协议往后挪
			if(memcmp(&lpstWSAProtocolInfo[nIndex].ProviderId,&stLayerGuid,sizeof(GUID)) != 0
				&& memcmp(&lpstWSAProtocolInfo[nIndex].ProviderId,&stChainGuid,sizeof(GUID)) != 0)
			{
				lpDwCateaEntries[nCataEntryIndex++]=lpstWSAProtocolInfo[nIndex].dwCatalogEntryId;
			}
		}

		//写入系统
		nErrorCode = WSCWriteProviderOrder(lpDwCateaEntries,nNumOfProtocols);
		if(nErrorCode != ERROR_SUCCESS)
		{
			LOG_OUTPRINT("Ajust Sequeese Failed");
			dwErrorCode = ESPI_ERROR_CODE_ADJUST_SEQUEESE_FAILED;
			break;
		}
	} while (FALSE);

	Free(lpstWSAProtocolInfo);
	return dwErrorCode;
}
