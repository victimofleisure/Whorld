// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		15aug04	initial version

		wrap Win32 waitable timer object

*/

#include "stdafx.h"
#include "WTimer.h"

WTimer::WTimer()
{
	m_Timer = NULL;
}

WTimer::~WTimer()
{
	Close();
}

bool WTimer::Create(LPSECURITY_ATTRIBUTES lpTimerAttributes, 
					BOOL bManualReset, LPCTSTR lpTimerName)
{
	Close();
	m_Timer = CreateWaitableTimer(lpTimerAttributes, bManualReset, lpTimerName);
	return(m_Timer != NULL);
}

bool WTimer::Open(DWORD	dwDesiredAccess, BOOL bInheritHandle, LPCTSTR lpTimerName)
{
	Close();
	m_Timer = OpenWaitableTimer(dwDesiredAccess, bInheritHandle, lpTimerName);
	return(m_Timer != NULL);
}

void WTimer::Close()
{
	if (m_Timer != NULL) {
		CancelWaitableTimer(m_Timer);
		CloseHandle(m_Timer);
		m_Timer = NULL;
	}
}
