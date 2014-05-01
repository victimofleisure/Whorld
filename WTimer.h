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

#ifndef WTIMER_INCLUDED
#define WTIMER_INCLUDED

class WTimer : public WObject {
public:
//
// Constructs a waitable timer.
//
	WTimer();
//
// Destroys the waitable timer.
//
	~WTimer();
//
// Creates the timer.
//
// Returns: True if successful.
//
	bool	Create(
		LPSECURITY_ATTRIBUTES lpTimerAttributes,	// Pointer to security attributes.
		BOOL	bManualReset,		// Flag for manual reset state.
		LPCTSTR	lpTimerName			// Pointer to timer object name.
	);
//
// Opens an existing timer.
//
// Returns: True if successful.
//
	bool	Open(
		DWORD	dwDesiredAccess,	// Access flag.
		BOOL	bInheritHandle,		// Inherit flag.
		LPCTSTR	lpTimerName			// Pointer to timer object name.
	);
//
// Sets the timer.
//
// Returns: True if successful.
//
	BOOL	Set(
		const LARGE_INTEGER *pDueTime,			// When timer will become signaled.
		LONG	lPeriod,						// Periodic timer interval.
		PTIMERAPCROUTINE pfnCompletionRoutine,	// Completion routine.
		LPVOID	lpArgToCompletionRoutine,		// Data for completion routine.
		BOOL	fResume							// Flag for resume state.
	);
//
// Cancels the timer.
//
// Returns: True if successful.
//
	BOOL	Cancel();
//
// Closes the timer handle.
//
	void	Close();
//
// Retrieves the timer's handle.
//
// Returns: The handle.
//
	operator HANDLE() const;

private:
	HANDLE	m_Timer;			// The timer handle.
};

inline WTimer::operator HANDLE() const
{
	return(m_Timer);
}

inline BOOL WTimer::Set(const LARGE_INTEGER *pDueTime, LONG lPeriod, 
						PTIMERAPCROUTINE pfnCompletionRoutine, 
						LPVOID lpArgToCompletionRoutine, BOOL fResume)
{
	return(SetWaitableTimer(m_Timer, pDueTime, lPeriod, pfnCompletionRoutine,
		lpArgToCompletionRoutine, fResume));
}

inline BOOL WTimer::Cancel()
{
	return(CancelWaitableTimer(m_Timer));
}

#endif
