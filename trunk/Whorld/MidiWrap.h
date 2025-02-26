// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda
 
		revision history:
		rev		date	comments
		00		01sep13	initial version
		01		07may14	in CMMTimer::Create, fix dwUser type
 
		wrap system MIDI and multimedia timer APIs
 
*/

#ifndef CMIDIWRAP_INCLUDED
#define	CMIDIWRAP_INCLUDED

#include "mmsystem.h"

#define MIDI_SUCCEEDED(mr) (mr == MMSYSERR_NOERROR)
#define MIDI_FAILED(mr) (mr != MMSYSERR_NOERROR)

#define TIMER_SUCCEEDED(mr) (mr == TIMERR_NOERROR)
#define TIMER_FAILED(mr) (mr != TIMERR_NOERROR)

class CMidiIn : public WObject {
public:
// Construction
	CMidiIn();
	~CMidiIn();

// Types
	typedef void (CALLBACK *PROC_PTR)(HMIDIIN hMidiIn, UINT wMsg, W64UINT dwInstance, W64UINT dwParam1, W64UINT dwParam2);

// Attributes
	bool	IsOpen() const;
	HMIDIIN	GetHandle() const;

// Operations
	MMRESULT	Open(UINT uDeviceID, W64UINT dwCallback, W64UINT dwCallbackInstance, DWORD dwFlags);
	MMRESULT	Close();
	MMRESULT	Reset();
	MMRESULT	Start();
	MMRESULT	Stop();
	MMRESULT	AddBuffer(LPMIDIHDR lpMidiInHdr);
	MMRESULT	PrepareHeader(LPMIDIHDR lpMidiInHdr);
	MMRESULT	UnprepareHeader(LPMIDIHDR lpMidiInHdr);
	static	MMRESULT	GetDeviceNames(CStringArray& DevList);
	static	MMRESULT	GetDeviceInterfaceName(int DevID, CString& Name);

protected:
// Data members
	HMIDIIN	m_hMidiIn;		// MIDI input handle
};

inline bool CMidiIn::IsOpen() const
{
	return(m_hMidiIn != NULL);
}

inline HMIDIIN CMidiIn::GetHandle() const
{
	return(m_hMidiIn);
}

inline MMRESULT CMidiIn::Reset()
{
	return(midiInReset(m_hMidiIn));
}

inline MMRESULT CMidiIn::Start()
{
	return(midiInStart(m_hMidiIn));
}

inline MMRESULT CMidiIn::Stop()
{
	return(midiInStop(m_hMidiIn));
}

inline MMRESULT CMidiIn::AddBuffer(LPMIDIHDR lpMidiInHdr)
{
	return(midiInAddBuffer(m_hMidiIn, lpMidiInHdr, sizeof(MIDIHDR)));
}

inline MMRESULT CMidiIn::PrepareHeader(LPMIDIHDR lpMidiInHdr)
{
	return(midiInPrepareHeader(m_hMidiIn, lpMidiInHdr, sizeof(MIDIHDR)));
}

inline MMRESULT CMidiIn::UnprepareHeader(LPMIDIHDR lpMidiInHdr)
{
	return(midiInUnprepareHeader(m_hMidiIn, lpMidiInHdr, sizeof(MIDIHDR)));
}

class CMidiOut : public WObject {
public:
// Construction
	CMidiOut();
	~CMidiOut();

// Types
	typedef void (CALLBACK *PROC_PTR)(HMIDIOUT hMidiOut, UINT wMsg, W64UINT dwInstance, W64UINT dwParam1, W64UINT dwParam2);

// Attributes
	bool	IsOpen() const;
	HMIDIOUT	GetHandle() const;

// Operations
	MMRESULT	Open(UINT uDeviceID, W64UINT dwCallback, W64UINT dwCallbackInstance, DWORD dwFlags);
	MMRESULT	Close();
	MMRESULT	Reset();
	MMRESULT	OutShortMsg(DWORD dwMsg);
	MMRESULT	OutLongMsg(LPMIDIHDR lpMidiOutHdr);
	MMRESULT	OutLongMsg(LPSTR lpData, DWORD dwLength);
	MMRESULT	OutLongMsg(const BYTE *lpData, DWORD dwLength);
	MMRESULT	PrepareHeader(LPMIDIHDR lpMidiOutHdr);
	MMRESULT	UnprepareHeader(LPMIDIHDR lpMidiOutHdr);
	static	MMRESULT	GetDeviceNames(CStringArray& DevList);
	static	MMRESULT	GetDeviceInterfaceName(int DevID, CString& Name);
	static	CString	GetErrorString(MMRESULT mmrError);
 
protected:
// Data members
	HMIDIOUT	m_hMidiOut;		// MIDI output handle
};

inline bool CMidiOut::IsOpen() const
{
	return(m_hMidiOut != NULL);
}

inline HMIDIOUT CMidiOut::GetHandle() const
{
	return(m_hMidiOut);
}

inline MMRESULT CMidiOut::Reset()
{
	return(midiOutReset(m_hMidiOut));
}

inline MMRESULT CMidiOut::OutShortMsg(DWORD dwMsg)
{
	return(midiOutShortMsg(m_hMidiOut, dwMsg));
}

inline MMRESULT CMidiOut::OutLongMsg(LPMIDIHDR lpMidiOutHdr)
{
	return(midiOutLongMsg(m_hMidiOut, lpMidiOutHdr, sizeof(MIDIHDR)));
}

inline MMRESULT CMidiOut::PrepareHeader(LPMIDIHDR lpMidiOutHdr)
{
	return(midiOutPrepareHeader(m_hMidiOut, lpMidiOutHdr, sizeof(MIDIHDR)));
}

inline MMRESULT CMidiOut::UnprepareHeader(LPMIDIHDR lpMidiOutHdr)
{
	return(midiOutUnprepareHeader(m_hMidiOut, lpMidiOutHdr, sizeof(MIDIHDR)));
}

class CMMTimerPeriod : public WObject {
public:
// Construction
	CMMTimerPeriod();
	~CMMTimerPeriod();

// Attributes
	bool	IsCreated() const;
	UINT	GetPeriod() const;

// Operations
	MMRESULT	Create(UINT uPeriod);
	MMRESULT	Destroy();

protected:
// Data members
	UINT	m_Period;	// timer resolution
};

inline bool CMMTimerPeriod::IsCreated() const
{
	return(m_Period != 0);
}

inline UINT CMMTimerPeriod::GetPeriod() const
{
	return(m_Period);
}

class CMMTimer : public WObject {
public:
// Construction
	CMMTimer();
	~CMMTimer();

// Attributes
	bool	IsCreated() const;
	UINT	GetID() const;

// Operations
	bool	Create(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, W64ULONG dwUser, UINT fuEvent);
	MMRESULT	Destroy();

protected:
// Data members
	UINT	m_ID;		// timer identifier
};

inline bool CMMTimer::IsCreated() const
{
	return(m_ID != 0);
}

inline UINT CMMTimer::GetID() const
{
	return(m_ID);
}

#endif
