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

#ifndef WCRITSEC_INCLUDED
#define WCRITSEC_INCLUDED

class WCritSec : public WObject {
public:
//
// Constructs a critical section.
//
	WCritSec();
//
// Destroys the critical section.
//
	~WCritSec();
//
// Enters the critical section.
//
	void	Enter();
//
// Leaves the critical section.
//
	void	Leave();
//
// Tries to enter the critical section.
//
// Returns: True if the critical section was entered.
//
	bool	TryEnter();

private:
	CRITICAL_SECTION	m_CritSec;
};

inline void WCritSec::Enter()
{
	EnterCriticalSection(&m_CritSec);
}

inline void WCritSec::Leave()
{
	LeaveCriticalSection(&m_CritSec);
}

inline bool WCritSec::TryEnter()
{
	return(TryEnterCriticalSection(&m_CritSec) != 0);
}

#endif
