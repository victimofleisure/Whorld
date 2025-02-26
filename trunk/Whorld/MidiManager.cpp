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
#include "MasterRowDlg.h"	//@@@ bad style

#define CHECK_MIDI(x) { MMRESULT nResult = x; if (MIDI_FAILED(nResult)) { OnMidiError(nResult); return false; } }

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
	if (MIDI_STAT(dwEvent) >= SYSEX)	// if not channel voice message (exclude system status messages)
		return;	// ignore
	int	iChan = MIDI_CHAN(dwEvent);
	if (iChan != 0)
		return;
	UINT	nCmd = MIDI_CMD(dwEvent);
	if (nCmd != CONTROL)
		return;
	int	nCtrl = MIDI_P1(dwEvent);
	int	nVal = MIDI_P2(dwEvent);
	const int	nCtrlStart = 16;
	nCtrl -= nCtrlStart;
	double	fNormVal = nVal / 127.0;
	if (nCtrl >= 0 && nCtrl < MASTER_COUNT) {
		int	iProp = nCtrl;
		double	fVal = CMasterRowDlg::Denorm(iProp, fNormVal);
		if (iProp == MASTER_Zoom) {	// if zoom
			CRenderCmd	cmd(RC_SET_ZOOM, true);	// with damping
			cmd.m_prop.dblVal = fVal;
			theApp.PushRenderCommand(cmd);
		} else {	// generic case
			CRenderCmd	cmd(RC_SET_MASTER, iProp);
			cmd.m_prop.dblVal = fVal;
			theApp.PushRenderCommand(cmd);
		}
		LPARAM	lParam = FloatToLParam(fVal);
		theApp.GetMainFrame()->PostMessage(UWM_MASTER_PROP_CHANGE, iProp, lParam);
	} else {
		nCtrl -= MASTER_COUNT;
		if (nCtrl >= 0 && nCtrl < PARAM_COUNT) {
			int	iRow = nCtrl;
			int	iProp = CParamsView::m_arrParamOrder[iRow];
			const PARAM_INFO&	info = GetParamInfo(iProp);
			double	fVal = fNormVal * (info.fMaxVal - info.fMinVal) + info.fMinVal;
			CRenderCmd	cmd(RC_SET_PARAM_Val, iProp);
			cmd.m_prop.dblVal = fVal;
			theApp.PushRenderCommand(cmd);
			LPARAM	lParam = FloatToLParam(fVal);
			theApp.GetMainFrame()->PostMessage(UWM_PARAM_VAL_CHANGE, iProp, lParam);
		}
	}
}
