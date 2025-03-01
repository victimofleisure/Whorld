// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26feb25	initial version
		01		01mar25	add misc targets

*/

#pragma once

#include "WhorldBase.h"
#include "MidiWrap.h"
#include "MidiDevices.h"
#include "Mapping.h"

class COptions;

class CMidiManager : public CMappingBase {
public:
// Attributes
	int		GetDeviceCount(int iType) const;
	int		GetDeviceIdx(int iType) const;
	void	SetDeviceIdx(int iType, int iDev);
	CString	GetDeviceName(int iType, int iDev) const;
	bool	IsInputDeviceOpen() const;
	bool	IsLearnMode() const;
	void	SetLearnMode(bool bEnable);

// Operations
	void	Initialize();
	void	ReadDevices();
	void	WriteDevices();
	void	RemoveAllMappings();
	void	ReadMappings(CIniFile& fIn);
	void	WriteMappings(CIniFile& fOut) const;
	void	OnMidiError(MMRESULT nResult);
	bool	OpenInputDevice(bool bEnable);
	bool	OpenInputDevice();
	void	CloseInputDevice();
	bool	ReopenInputDevice();
	void	OnDeviceChange();
	void	OnMidiEvent(DWORD dwEvent);
	void	PushMasterProperty(int iProp, double fNormVal);
	void	PushParameter(int iParam, double fNormVal);
	void	PushMiscTarget(int iMiscTarget, double fNormVal);
	void	UpdateUI(int nMsg, WPARAM wParam, LPARAM lParam);
	static LPARAM	FloatToLParam(double fVal);
	static double	LParamToFloat(LPARAM lParam);
	static UINT	SetOrClear(UINT nDest, UINT nMask, bool bIsSet);

// Public data
	CSafeMapping	m_midiMaps;		// MIDI mappings

protected:
// Data members
	CMidiDevices	m_midiDevs;	// MIDI device info
	CMidiIn	m_midiIn;			// MIDI input device
	bool	m_bInMsgBox;		// true if displaying message box
	bool	m_bLearnMode;		// true if we're learning mappings

// Helpers
	static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, W64UINT dwInstance, W64UINT dwParam1, W64UINT dwParam2);
};

inline bool CMidiManager::IsInputDeviceOpen() const
{
	return m_midiIn.IsOpen();
}

inline int CMidiManager::GetDeviceCount(int iType) const
{
	return m_midiDevs.GetCount(iType);
}

inline int CMidiManager::GetDeviceIdx(int iType) const
{
	return m_midiDevs.GetIdx(iType);
}

inline void CMidiManager::SetDeviceIdx(int iType, int iDev)
{
	m_midiDevs.SetIdx(iType, iDev);
}

inline CString CMidiManager::GetDeviceName(int iType, int iDev) const
{
	return m_midiDevs.GetName(iType, iDev);
}

inline void CMidiManager::ReadDevices()
{
	m_midiDevs.Read();
}

inline void CMidiManager::WriteDevices()
{
	m_midiDevs.Write();
}

inline void CMidiManager::RemoveAllMappings()
{
	m_midiMaps.RemoveAll();
}

inline void CMidiManager::ReadMappings(CIniFile& fIn)
{
	m_midiMaps.Read(fIn);
}

inline void CMidiManager::WriteMappings(CIniFile& fOut) const
{
	m_midiMaps.Write(fOut);
}

inline bool CMidiManager::IsLearnMode() const
{
	return m_bLearnMode;
}

inline void CMidiManager::SetLearnMode(bool bEnable)
{
	m_bLearnMode = bEnable;
}

inline LPARAM CMidiManager::FloatToLParam(double fVal)
{
#if _WIN64	// if LPARAM is 64 bits, double fits
	return *reinterpret_cast<LPARAM*>(&fVal);	// cast double to LPARAM
#else	// LPARAM is 32 bits, so use float instead
	float	fSPVal = static_cast<float>(fVal);	// cast double to float
	return *reinterpret_cast<LPARAM*>(&fSPVal);	// cast float to LPARAM
#endif
}

inline double CMidiManager::LParamToFloat(LPARAM lParam)
{
#if _WIN64	// if LPARAM is 64 bits, double fits
	return *reinterpret_cast<double*>(&lParam);	// cast LPARAM to double
#else	// LPARAM is 32 bits, so use float instead
	return *reinterpret_cast<float*>(&lParam);	// cast LPARAM to float
#endif
}
