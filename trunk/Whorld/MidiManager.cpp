// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26feb25	initial version

*/

#include "stdafx.h"
#include "Whorld.h"
#include "MidiManager.h"
#include "Midi.h"
#include "MainFrm.h"
#include "OptionsDlg.h"
#include "SaveObj.h"
#include "IniFile.h"
#include "MasterRowDlg.h"	//@@@ bad style

#define CHECK_MIDI(x) { MMRESULT nResult = x; if (MIDI_FAILED(nResult)) { OnMidiError(nResult); return false; } }

void CMidiManager::Initialize()
{
	CMapping::Initialize();
}

void CMidiManager::ReadMappings(LPCTSTR pszPath)
{
	CIniFile	fIn(pszPath);
	fIn.Read();
	m_midiMaps.Read(fIn);
}

void CMidiManager::WriteMappings(LPCTSTR pszPath) const
{
	CIniFile	fOut(pszPath, true);
	m_midiMaps.Write(fOut);
	fOut.Write();
}

void CMidiManager::OnMidiError(MMRESULT nResult)
{
	if (!m_bInMsgBox) {	// if not already displaying message box
		CSaveObj<bool>	save(m_bInMsgBox, true);	// save and set reentry guard
		CString	sError;
		int	nIDHelp;
		sError.Format(LDS(IDS_SEQ_MIDI_ERROR), nResult);
		sError += '\n' + CMidiOut::GetErrorString(nResult);
		nIDHelp = -1;
		AfxMessageBox(sError, MB_OK, nIDHelp);
	}
}

bool CMidiManager::OpenInputDevice(bool bEnable)
{
	bool	bIsOpen = IsInputDeviceOpen();
	if (bEnable == bIsOpen)	// if already in requested state
		return true;	// nothing to do
	if (bEnable) {	// if opening device
		int	iMidiInDev = GetDeviceIdx(CMidiDevices::INPUT);
		if (iMidiInDev < 0)
			return false;
		CHECK_MIDI(m_midiIn.Open(iMidiInDev, reinterpret_cast<W64UINT>(MidiInProc), reinterpret_cast<W64UINT>(this), CALLBACK_FUNCTION));
		CHECK_MIDI(m_midiIn.Start());
	} else {	// closing device
		CHECK_MIDI(m_midiIn.Close());
	}
	return true;
}

bool CMidiManager::OpenInputDevice()
{
	return OpenInputDevice(GetDeviceIdx(CMidiDevices::INPUT) >= 0);
}

void CMidiManager::CloseInputDevice()
{
	m_midiIn.Close();
}

bool CMidiManager::ReopenInputDevice()
{
	CloseInputDevice();
	return OpenInputDevice();
}

void CMidiManager::OnDeviceChange()
{
	if (!m_bInMsgBox) {	// if not already displaying message box
		CSaveObj<bool>	save(m_bInMsgBox, true);	// save and set reentry guard
		UINT	nChangeMask;
		bool	bResult = m_midiDevs.OnDeviceChange(nChangeMask);	// handle MIDI device change
		if (nChangeMask & CMidiDevices::CM_INPUT) {	// if MIDI input device changed
			OpenInputDevice();
		}
		if (nChangeMask & CMidiDevices::CM_OUTPUT) {	// if MIDI output device changed
			if (!bResult && GetDeviceCount(CMidiDevices::OUTPUT)) {	// if user canceled and an output device is available
				SetDeviceIdx(CMidiDevices::OUTPUT, 0);	// select first output device
			}
		}
		if (nChangeMask & CMidiDevices::CM_CHANGE) {	// if MIDI device state changed
			CWnd	*pPopupWnd = theApp.GetMainFrame()->GetLastActivePopup();
			COptionsDlg	*pOptionsDlg = DYNAMIC_DOWNCAST(COptionsDlg, pPopupWnd);
			if (pOptionsDlg != NULL)	// if options dialog is active
				pOptionsDlg->UpdateMidiDevices();	// update dialog's MIDI device combos
		}
	}
}

void CALLBACK CMidiManager::MidiInProc(HMIDIIN hMidiIn, UINT wMsg, W64UINT dwInstance, W64UINT dwParam1, W64UINT dwParam2)
{
	// this callback function runs in a worker thread context; 
	// data shared with main thread may require serialization
	static CDWordArrayEx	arrMappedEvent;
	UNREFERENCED_PARAMETER(hMidiIn);
	UNREFERENCED_PARAMETER(dwInstance);
	UNREFERENCED_PARAMETER(dwParam2);
//	_tprintf(_T("MidiInProc %d %d\n"), GetCurrentThreadId(), ::GetThreadPriority(GetCurrentThread()));
	switch (wMsg) {
	case MIM_DATA:
		{
//			_tprintf(_T("%x %d\n"), dwParam1, dwParam2);
			theApp.m_midiMgr.OnMidiEvent(static_cast<DWORD>(dwParam1));
		}
		break;
	}
}

void CMidiManager::OnMidiEvent(DWORD dwEvent)
{
	if (!MIDI_IS_SHORT_MSG(dwEvent))	// if event is a short MIDI message
		return;	// ignore
	// exclude system status messages
	if (MIDI_STAT(dwEvent) >= SYSEX)	// if not channel voice message
		return;	// ignore
	// lock the mapping array during iteration
	WCritSec::Lock lock(m_midiMaps.GetCritSec());
	int	nMaps = m_midiMaps.GetCount();
	for (int iMap = 0; iMap < nMaps; iMap++) {	// for each mapping
		const CMapping&	map = m_midiMaps.GetAt(iMap);
		int	nTargVal = map.IsInputMatch(dwEvent);	// try to map MIDI message
		if (nTargVal >= 0) {	// if message was mapped, we have a target value
			int	nDeltaRange = map.m_nRangeEnd - map.m_nRangeStart;	// can be negative if start > end
			double	fTargVal = nTargVal / 127.0 * nDeltaRange + map.m_nRangeStart;	// offset by start of range
			double	fNormTargVal = fTargVal / 127.0;	// normalized target value
			int	iProp = map.m_nOutEvent;
			if (iProp < MASTER_COUNT) {	// if target is master property
				double	fVal = CMasterRowDlg::Denorm(iProp, fNormTargVal);
				if (iProp == MASTER_Zoom) {	// if zoom
					MAKE_RENDER_CMD(cmd, RC_SET_ZOOM, true, dblVal, fVal);	// zoom with damping
					theApp.PushRenderCommand(cmd);
				} else {	// generic case
					MAKE_RENDER_CMD(cmd, RC_SET_MASTER, iProp, dblVal, fVal);
					theApp.PushRenderCommand(cmd);
				}
				LPARAM	lParam = FloatToLParam(fVal);
				theApp.GetMainFrame()->PostMessage(UWM_MASTER_PROP_CHANGE, iProp, lParam);
			} else {	// target isn't master property
				iProp -= MASTER_COUNT;
				if (iProp < PARAM_COUNT) {	// if target is parameter
					const PARAM_INFO&	info = GetParamInfo(iProp);
					double	fVal = fNormTargVal * (info.fMaxVal - info.fMinVal) + info.fMinVal;
					MAKE_RENDER_CMD(cmd, RC_SET_PARAM_Val, iProp, dblVal, fVal);
					theApp.PushRenderCommand(cmd);
					LPARAM	lParam = FloatToLParam(fVal);
					theApp.GetMainFrame()->PostMessage(UWM_PARAM_VAL_CHANGE, iProp, lParam);
				}
			}
		}
	}
}
