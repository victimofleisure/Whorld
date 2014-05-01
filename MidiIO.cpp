// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		13aug05	initial version
		01		23nov07	support Unicode

		wrap windows MIDI support

*/

#include "stdafx.h"
#include "MidiIO.h"

CMidiIO::CMidiIO()
{
	m_InputCallback = NULL;
	m_InputInstance = NULL;
	m_OutputCallback = NULL;
	m_OutputInstance = NULL;
	m_MidiIn = NULL;
	m_MidiOut = NULL;
	m_mr = 0;
}

CMidiIO::~CMidiIO()
{
	EnableInput(FALSE);
	EnableOutput(FALSE);
}

int	CMidiIO::GetInputDeviceNames(CStringArray& DevList)
{
	MIDIINCAPS	caps;
	int	Devs = midiInGetNumDevs();
	DevList.RemoveAll();
	for (int i = 0; i < Devs; i++) {
		if (!midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS)))
			DevList.Add(caps.szPname);
	}
	return(Devs);
}

int	CMidiIO::GetOutputDeviceNames(CStringArray& DevList)
{
	MIDIOUTCAPS	caps;
	int	Devs = midiOutGetNumDevs();
	DevList.RemoveAll();
	for (int i = 0; i < Devs; i++) {
		if (!midiOutGetDevCaps(i, &caps, sizeof(MIDIOUTCAPS)))
			DevList.Add(caps.szPname);
	}
	return(Devs);
}

void CMidiIO::SetInputCallback(INPUT_CALLBACK Callback, PVOID Instance)
{
	m_InputCallback = Callback;
	m_InputInstance = Instance;
}

void CMidiIO::SetOutputCallback(OUTPUT_CALLBACK Callback, PVOID Instance)
{
	m_OutputCallback = Callback;
	m_OutputInstance = Instance;
}

bool CMidiIO::EnableInput(bool Enable, int Device)
{
	if (Enable == IsInputEnabled())	// if we're already there, bail
		return(TRUE);
	if (Enable) {
		int	Flags = m_InputCallback != NULL ? CALLBACK_FUNCTION : CALLBACK_NULL;
		m_mr = midiInOpen(&m_MidiIn, Device, (DWORD)m_InputCallback,
			(DWORD)m_InputInstance, Flags);
		if (m_mr != MMSYSERR_NOERROR)
			return(FALSE);
		m_mr = midiInStart(m_MidiIn);
		if (m_mr != MMSYSERR_NOERROR)
			return(FALSE);
	} else {
		m_mr = midiInReset(m_MidiIn);
		if (m_mr != MMSYSERR_NOERROR)
			return(FALSE);
		m_mr = midiInClose(m_MidiIn);
		if (m_mr != MMSYSERR_NOERROR)
			return(FALSE);
		m_MidiIn =  NULL;
	}
	return(TRUE);
}

bool CMidiIO::EnableOutput(bool Enable, int Device)
{
	if (Enable == IsOutputEnabled())	// if we're already there, bail
		return(TRUE);
	if (Enable) {
		int	Flags = m_OutputCallback != NULL ? CALLBACK_FUNCTION : CALLBACK_NULL;
		m_mr = midiOutOpen(&m_MidiOut, Device, (DWORD)m_OutputCallback,
			(DWORD)m_OutputInstance, Flags);
		if (m_mr != MMSYSERR_NOERROR)
			return(FALSE);
	} else {
		m_mr = midiOutReset(m_MidiOut);
		if (m_mr != MMSYSERR_NOERROR)
			return(FALSE);
		m_mr = midiOutClose(m_MidiOut);
		if (m_mr != MMSYSERR_NOERROR)
			return(FALSE);
		m_MidiOut =  NULL;
	}
	return(TRUE);
}

bool CMidiIO::GetLastErrorString(CString& Str) const
{
	LPTSTR	p = Str.GetBuffer(MAXERRORLENGTH);
	MMRESULT	retc = midiInGetErrorText(m_mr, p, MAXERRORLENGTH);
	Str.ReleaseBuffer();
	return(retc == MMSYSERR_NOERROR);
}
