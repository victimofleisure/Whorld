// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16jul05	initial version

		self-correcting timer
 
*/

#ifndef CREALTIMER_INCLUDED
#define CREALTIMER_INCLUDED

#include "WEvent.h"
#include "WThread.h"
#include "WTimer.h"

class CRealTimer : public WObject {
public:
	typedef	void (*LPCALLBACK)(LPVOID Cookie);
	CRealTimer();
	~CRealTimer();
	bool	Launch(LPCALLBACK Callback, LPVOID Cookie, int Priority);
	bool	Kill();
	void	Run(bool Enable);
	void	SetFreq(float Freq, bool Resync = FALSE);
	float	GetFreq() const;
	void	Resync();

private:
	enum {	// timer states
		STOP,
		RUN,
		DIE
	};
	LPCALLBACK	m_Callback;	// if non-null, pointer to user's callback function
	LPVOID		m_Cookie;	// user-defined data passed to callback function
	WTimer		m_Timer;	// timer object
	WEvent		m_Event;	// event flag
	WThread		m_Thread;	// thread instance
	volatile	float	m_PrevFreq;	// previous target frequency, in Hz
	volatile	float	m_Freq;		// target frequency, in Hz
	volatile	int		m_Reset;	// if changed, thread resets timer
	volatile	int		m_State;	// timer's current state
	volatile	int		m_NewState;	// timer's new state

	void	DoTimer();
	static	UINT __stdcall ThreadFunc(LPVOID pParam);
};

inline float CRealTimer::GetFreq() const
{
	return(m_Freq);
}

#endif
