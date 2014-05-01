// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		13aug05	initial version

		wrap windows MIDI support

*/

#ifndef CMIDIIO_INCLUDED
#define CMIDIIO_INCLUDED

#include "mmsystem.h"

class CMidiIO : public WObject {
public:
// Constants
	enum {
		NO_DEVICE = -1
	};

// Types
	typedef union tagMSG {
		DWORD	dw;
		struct {
			BYTE	cmd;
			BYTE	p1;
			BYTE	p2;
		} s;
	} MSG;
	typedef void (CALLBACK *INPUT_CALLBACK)(HMIDIIN hMidiIn, UINT wMsg, 
		DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	typedef void (CALLBACK *OUTPUT_CALLBACK)(HMIDIOUT hMidiOut, UINT wMsg, 
		DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

// Construction
	CMidiIO();
	~CMidiIO();

// Attributes
	bool	IsInputEnabled() const;
	bool	IsOutputEnabled() const;

// Operations
	void	SetInputCallback(INPUT_CALLBACK Callback, PVOID Instance);
	void	SetOutputCallback(OUTPUT_CALLBACK Callback, PVOID Instance);
	bool	EnableInput(bool Enable, int Device = -1);
	bool	EnableOutput(bool Enable, int Device = -1);
	static	int	GetInputDeviceNames(CStringArray& DevList);
	static	int	GetOutputDeviceNames(CStringArray& DevList);
	bool	Send(DWORD Msg);
	MMRESULT	GetLastError() const;
	bool	GetLastErrorString(CString& Str) const;

private:
// Member data
	INPUT_CALLBACK	m_InputCallback;	// pointer to input callback
	PVOID		m_InputInstance;		// pointer to input instance
	OUTPUT_CALLBACK	m_OutputCallback;	// pointer to output callback
	PVOID		m_OutputInstance;		// pointer to output callback
	HMIDIIN		m_MidiIn;	// input handle
	HMIDIOUT	m_MidiOut;	// output handle
	MMRESULT	m_mr;		// most recent multimedia error code
};

inline bool CMidiIO::IsInputEnabled() const
{
	return(m_MidiIn != NULL);
}

inline bool CMidiIO::IsOutputEnabled() const
{
	return(m_MidiOut != NULL);
}

inline bool CMidiIO::Send(DWORD Msg)
{
	return((m_mr = midiOutShortMsg(m_MidiOut, Msg)) == MMSYSERR_NOERROR);
}

inline MMRESULT CMidiIO::GetLastError() const
{
	return(m_mr);
}

#endif
