/**************************************************************************

Copyright:ZhuGege

Author: ZhuGege

Date:2016-03-13

Description:log

**************************************************************************/

#include "ZGGLog.h"
#include <time.h>
#include <tchar.h>

HANDLE CZGGLog::CLock::m_hEvent = CreateEvent(NULL,TRUE,TRUE,ZGGLOG_LOCK_EVENT_NAME);


CZGGLog* CZGGLog::m_pobjZggLog = NULL;

//获取当前时间
std::string CZGGLog::GetLogTime()
{
	time_t stLogTime;
	char szLogTime[21] = {0};

	time(&stLogTime);
#pragma warning(disable:4996)
	strftime(szLogTime, 20, "%Y-%m-%d %H:%M:%S", localtime(&stLogTime));

	return string(szLogTime);
}

//创建日志
BOOL CZGGLog::InitLog(string strLogFileName)
{
	m_pFile = INVALID_HANDLE_VALUE;
	
	m_pFile = CreateFileA(strLogFileName.c_str(),FILE_WRITE_DATA|FILE_READ_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	return m_pFile != INVALID_HANDLE_VALUE;
}

BOOL CZGGLog::WriteInternal(string strLog)
{
	DWORD dwRet;
	return WriteFile(m_pFile,strLog.c_str(),strLog.length(),&dwRet,NULL);
}

BOOL CZGGLog::WriteLog(char* fmt,...)
{
	if(m_pFile == INVALID_HANDLE_VALUE)
		return FALSE;

	static char* sprint_buf = new(std::nothrow) char[1024*1024];
	memset(sprint_buf,0,1024*1024);
	va_list args; 
	va_start(args, fmt);//初始化参数指针 
	vsprintf(sprint_buf, fmt, args);/*函数放回已经处理的字符串长度*/ 

	string strLogTime = GetLogTime();
	string strTemp = strLogTime + "  " + sprint_buf + "\r\n";

	CLock lock;

	return WriteInternal(strTemp);
}


VOID CZGGLog::PrintBuf(const char* buf, int buf_len) 
{
	if (buf == NULL || buf_len == 0)
	{
		LOG_OUTPRINT("Buf is NULL or Buf Len is 0");
		return;
	}

	std::string strOut;
	
	char szTmp[20]={0};
	memset(szTmp,0,_countof(szTmp));
	sprintf(szTmp,"buf_len:%d\r\n", buf_len);
	strOut.assign(szTmp);

	for (int i = 0; i < buf_len; ++i) 
	{
		memset(szTmp,0,20);
		sprintf(szTmp,"0x%02x ", static_cast<unsigned char>(buf[i]));
		strOut.append(szTmp);
		if (i > 0 && i % 16 == 15)
		{
			strOut.append("\r\n");
		}
	}
	strOut.append("");
	LOG_OUTPRINT("Buf:%p,%s",buf,strOut.c_str());

}