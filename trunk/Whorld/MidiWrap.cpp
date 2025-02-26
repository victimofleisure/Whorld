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
		02		27mar18	in GetErrorString, release buffer
		03		17may18	output GetDeviceNames was using input struct size
		04		08jun21	fix get device interface name warnings

		wrap system MIDI and multimedia timer APIs
 
*/

#include "stdafx.h"
#include "MidiWrap.h"

#define DRV_QUERYDEVICEINTERFACE		0x80c	// from mmddk.h
#define DRV_QUERYDEVICEINTERFACESIZE    0x80d

CMidiIn::CMidiIn()
{
	m_hMidiIn = NULL;
}

CMidiIn::~CMidiIn()
{
	VERIFY(MIDI_SUCCEEDED(Close()));
}

MMRESULT CMidiIn::Open(UINT uDeviceID, W64UINT dwCallback, W64UINT dwCallbackInstance, DWORD dwFlags)
{
	VERIFY(MIDI_SUCCEEDED(Close()));
	return(midiInOpen(&m_hMidiIn, uDeviceID, dwCallback, dwCallbackInstance, dwFlags));
}

MMRESULT CMidiIn::Close()
{
	if (!IsOpen())	// if not open
		return(MMSYSERR_NOERROR);	// no error
	Stop();	// stop before closing
	MMRESULT	mr = midiInClose(m_hMidiIn);
	if (MIDI_SUCCEEDED(mr))
		m_hMidiIn = NULL;
	return(mr);
}

MMRESULT CMidiIn::GetDeviceNames(CStringArray& DevList)
{
	MIDIINCAPS	caps;
	int	nDevs = midiInGetNumDevs();
	DevList.RemoveAll();
	DevList.SetSize(nDevs);
	MMRESULT	retc = MMSYSERR_NOERROR;
	for (int iDev = 0; iDev < nDevs; iDev++) {
		MMRESULT	mr = midiInGetDevCaps(iDev, &caps, sizeof(MIDIINCAPS));
		if (MIDI_SUCCEEDED(mr))
			DevList[iDev] = caps.szPname;
		else
			retc = mr;
	}
	return(retc);
}

MMRESULT CMidiIn::GetDeviceInterfaceName(int DevID, CString& Name)
{
	HMIDIIN	hDev = reinterpret_cast<HMIDIIN>(static_cast<W64UINT>(DevID));
	W64ULONG	size = 0;	// receives needed buffer size in bytes
	W64ULONG	pSize = reinterpret_cast<W64ULONG>(&size);
	MMRESULT	mr = midiInMessage(hDev, DRV_QUERYDEVICEINTERFACESIZE, pSize, 0);
	if (MIDI_FAILED(mr))
		return(mr);
	CByteArray	buf;	// receives name as a null-terminated Unicode string
	buf.SetSize(size);
	W64ULONG	pBuf = reinterpret_cast<W64ULONG>(buf.GetData());
	mr = midiInMessage(hDev, DRV_QUERYDEVICEINTERFACE, pBuf, size);
	if (MIDI_FAILED(mr))
		return(mr);
	Name = CString(reinterpret_cast<LPCWSTR>(buf.GetData()));
	return(MMSYSERR_NOERROR);
}

CMidiOut::CMidiOut()
{
	m_hMidiOut = NULL;
}

CMidiOut::~CMidiOut()
{
	VERIFY(MIDI_SUCCEEDED(Close()));
}

MMRESULT CMidiOut::Open(UINT uDeviceID, W64UINT dwCallback, W64UINT dwCallbackInstance, DWORD dwFlags)
{
	VERIFY(MIDI_SUCCEEDED(Close()));
	return(midiOutOpen(&m_hMidiOut, uDeviceID, dwCallback, dwCallbackInstance, dwFlags));
}

MMRESULT CMidiOut::Close()
{
	if (!IsOpen())	// if not open
		return(MMSYSERR_NOERROR);	// no error
	MMRESULT	mr = midiOutClose(m_hMidiOut);
	if (MIDI_SUCCEEDED(mr))
		m_hMidiOut = NULL;
	return(mr);
}

MMRESULT CMidiOut::GetDeviceNames(CStringArray& DevList)
{
	MIDIOUTCAPS	caps;
	int	nDevs = midiOutGetNumDevs();
	DevList.RemoveAll();
	DevList.SetSize(nDevs);
	MMRESULT	retc = MMSYSERR_NOERROR;
	for (int iDev = 0; iDev < nDevs; iDev++) {
		MMRESULT	mr = midiOutGetDevCaps(iDev, &caps, sizeof(MIDIOUTCAPS));
		if (MIDI_SUCCEEDED(mr))
			DevList[iDev] = caps.szPname;
		else
			retc = mr;
	}
	return(retc);
}

MMRESULT CMidiOut::GetDeviceInterfaceName(int DevID, CString& Name)
{
	HMIDIOUT	hDev = reinterpret_cast<HMIDIOUT>(static_cast<W64UINT>(DevID));
	W64ULONG	size = 0;	// receives needed buffer size in bytes
	W64ULONG	pSize = reinterpret_cast<W64ULONG>(&size);
	MMRESULT	mr = midiOutMessage(hDev, DRV_QUERYDEVICEINTERFACESIZE, pSize, 0);
	if (MIDI_FAILED(mr))
		return(mr);
	CByteArray	buf;	// receives name as a null-terminated Unicode string
	buf.SetSize(size);
	W64ULONG	pBuf = reinterpret_cast<W64ULONG>(buf.GetData());
	mr = midiOutMessage(hDev, DRV_QUERYDEVICEINTERFACE, pBuf, size);
	if (MIDI_FAILED(mr))
		return(mr);
	Name = CString(reinterpret_cast<LPCWSTR>(buf.GetData()));
	return(MMSYSERR_NOERROR);
}

CString	CMidiOut::GetErrorString(MMRESULT mmrError)
{
	CString	s;
	LPTSTR	pBuf = s.GetBuffer(MAXERRORLENGTH);
	MMRESULT	mr = midiOutGetErrorText(mmrError, pBuf, MAXERRORLENGTH);
	s.ReleaseBuffer();
	if (MIDI_FAILED(mr))
		s.Format(_T("midiOutGetErrorText error %d"), mr);
	return(s);
}

MMRESULT CMidiOut::OutLongMsg(LPSTR lpData, DWORD dwLength)
{
	MIDIHDR	hdr;
	hdr.lpData = lpData;
	hdr.dwBufferLength = dwLength;
	hdr.dwFlags = 0;
	MMRESULT	mr = PrepareHeader(&hdr);
	if (MIDI_FAILED(mr))
		return(mr);
	while ((mr = OutLongMsg(&hdr)) == MIDIERR_STILLPLAYING) {
		// loop while buffer is still playing
	}
	if (MIDI_FAILED(mr)) {
		UnprepareHeader(&hdr);
		return(mr);
	}
	return(UnprepareHeader(&hdr));
}

MMRESULT CMidiOut::OutLongMsg(const BYTE *lpData, DWORD dwLength)
{
	// data must reside in writable memory, so copy it to temporary buffer
	CByteArray	buf;
	buf.SetSize(dwLength);
	CopyMemory(buf.GetData(), lpData, dwLength);
	LPSTR	pBuf = reinterpret_cast<LPSTR>(buf.GetData());
	return(OutLongMsg(pBuf, dwLength));
}

CMMTimerPeriod::CMMTimerPeriod()
{
	m_Period = 0;
}

CMMTimerPeriod::~CMMTimerPeriod()
{
	VERIFY(TIMER_SUCCEEDED(Destroy()));
}

MMRESULT CMMTimerPeriod::Create(UINT uPeriod)
{
	VERIFY(TIMER_SUCCEEDED(Destroy()));
	MMRESULT	mr = timeBeginPeriod(uPeriod);
	if (TIMER_SUCCEEDED(mr))
		m_Period = uPeriod;
	return(mr);
}

MMRESULT CMMTimerPeriod::Destroy()
{
	if (!IsCreated())	// if not created
		return(TIMERR_NOERROR);	// no error
	MMRESULT	mr = timeEndPeriod(m_Period);
	if (TIMER_SUCCEEDED(mr))
		m_Period = 0;
	return(mr);
}

CMMTimer::CMMTimer()
{
	m_ID = 0;
}

CMMTimer::~CMMTimer()
{
	VERIFY(TIMER_SUCCEEDED(Destroy()));
}

bool CMMTimer::Create(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, W64ULONG dwUser, UINT fuEvent)
{
	VERIFY(TIMER_SUCCEEDED(Destroy()));
	m_ID = timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
	return(IsCreated());
}

MMRESULT CMMTimer::Destroy()
{
	if (!IsCreated())	// if not created
		return(TIMERR_NOERROR);	// no error
	MMRESULT	mr = timeKillEvent(m_ID);
	if (TIMER_SUCCEEDED(mr))
		m_ID = 0;
	return(mr);
}
