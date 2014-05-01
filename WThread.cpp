// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		15aug04	initial version

		wrap Win32 thread object

*/

#include "stdafx.h"
#include <process.h>
#include "WThread.h"

WThread::WThread()
{
	m_Thread = NULL;
}

WThread::~WThread()
{
	Close();
}

bool WThread::Create(void *Security, unsigned StackSize, THRFUNCP StartAddress, 
					 void *ArgList, unsigned InitFlag, unsigned *ThreadID)
{
	Close();
	m_Thread = (HANDLE)_beginthreadex(Security, StackSize, 
		StartAddress, ArgList, InitFlag, ThreadID);
	return(m_Thread != NULL);
}

void WThread::Close()
{
	if (m_Thread != NULL) {
		CloseHandle(m_Thread);
		m_Thread = NULL;
	}
}
