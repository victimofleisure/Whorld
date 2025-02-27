// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		05mar03		initial version
		01		13mar03		inline functions were missing inline keyword
		02		24apr03		some void functions were missing void keyword
		03		18oct04		ck: remove set debug name
		04		12jun08		ck: add lock nested class

		wrap Win32 critical section

*/

#ifndef WCRITSEC_INCLUDED
#define WCRITSEC_INCLUDED

// The WCritSec object wraps a Win32 critical section.  Initialization and
// cleanup are automated; the underlying object is not directly accessible.
//
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
//
// The WCritSec::Lock object automates the process of using a critical section.
// The critical section is entered when a lock instance is created, and exited
// when the lock instance is destroyed. The lock instance is typically created
// on the stack, so that the critical section is automatically exited when the
// lock instance goes out of scope.
//
	class Lock : public WObject {
	public:
//
// Creates the lock object, entering the specified critical section.
//
		Lock(
			WCritSec&	CritSec		// The critical section to lock.
		);
//
// Destroys the lock object, exiting the critical section.
//
		~Lock();

	protected:
		WCritSec&	m_CritSec;		// Reference to the critical section.
	};

private:
	CRITICAL_SECTION	m_CritSec;
};

inline WCritSec::WCritSec()
{
	InitializeCriticalSection(&m_CritSec);
}

inline WCritSec::~WCritSec()
{
	DeleteCriticalSection(&m_CritSec);
}

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

inline WCritSec::Lock::Lock(WCritSec& CritSec) :
	m_CritSec(CritSec)
{
	m_CritSec.Enter();
}

inline WCritSec::Lock::~Lock()
{
	m_CritSec.Leave();
}

#endif
