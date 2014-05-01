// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		05mar03	initial version

		wrap Win32 critical section

*/

#include "stdafx.h"
#include "WCritSec.h"

WCritSec::WCritSec()
{
	InitializeCriticalSection(&m_CritSec);
}

WCritSec::~WCritSec()
{
	DeleteCriticalSection(&m_CritSec);
}
