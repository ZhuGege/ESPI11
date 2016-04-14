#pragma once
/**************************************************************************

Copyright:ZhuGege

Author: ZhuGege

Date:2016-03-13

Description:Inject DLL to Target Process

**************************************************************************/
#include <string>
#include <windows.h>
using namespace std;

class CZGGLog
{
public:
	static CZGGLog* GetZggLogInstance()
	{
		if (m_pobjZggLog == NULL)
		{
			m_pobjZggLog = new CZGGLog;
		}
		return m_pobjZggLog;
	}
	BOOL InitLog(string strLogFileName);
	BOOL WriteLog(char* fmt,...);
	void PrintBuf(const char* buf, int buf_len);

private:
	CZGGLog(){};
	string GetLogTime();
	BOOL WriteInternal(string strLog);

	HANDLE m_pFile;
	static CZGGLog* m_pobjZggLog;
	//ÄÚ²¿Ëø
	class CLock
	{
	public:
#define ZGGLOG_LOCK_EVENT_NAME	_T("ZggLogLockEvent")
		
		CLock()
		{
			lock();
		}
		~CLock()
		{
			unlock();
		}

		void lock()
		{
			WaitForSingleObject(m_hEvent,INFINITE);
			
		}
		void unlock()
		{
			SetEvent(m_hEvent);
		}
	private:
		static HANDLE m_hEvent;
	};
};

#define  LOG_OUTPRINT CZGGLog::GetZggLogInstance()->WriteLog

#define  LOG_OUTPRINT_HEXBUF  CZGGLog::GetZggLogInstance()->PrintBuf