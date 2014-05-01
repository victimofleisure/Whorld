// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		15aug04	initial version

		wrap Win32 event object

*/

#include "stdafx.h"
#include "WEvent.h"

WEvent::WEvent()
{
	m_Event = NULL;
}

WEvent::~WEvent()
{
	Close();
}

bool WEvent::Create(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, 
							BOOL bInitialState, LPCTSTR lpName)
{
	Close();
	m_Event = CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
	return(m_Event != NULL);
}

void WEvent::Close()
{
	if (m_Event != NULL) {
		CloseHandle(m_Event);
		m_Event = NULL;
	}
}
