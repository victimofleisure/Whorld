// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26feb25	initial version
		01		28feb25	add center epsilon
		02		01mar25	add misc targets
		03		01mar25	add learn mode

*/

#include "stdafx.h"
#include "Whorld.h"
#include "MidiManager.h"
#include "Midi.h"
#include "MainFrm.h"
#include "OptionsDlg.h"
#include "SaveObj.h"
#include "IniFile.h"
#include "MasterRowDlg.h"	// for norm and denorm

#define CHECK_MIDI(x) { MMRESULT nResult = x; if (MIDI_FAILED(nResult)) { OnMidiError(nResult); return false; } }

// macro to construct and push a render command object with a variant property value
#define PUSH_RENDER_CMD(cmd, cmdidx, param, type, val) \
	CRenderCmd cmd(cmdidx, param); \
	cmd.m_prop.type = val; \
	theApp.PushRenderCommand(cmd);

CMidiManager::CMidiManager()
{
	m_bInMsgBox = false;
	m_bLearnMode = false;
}

void CMidiManager::Initialize()
{
	CMapping::Initialize();	// initialize base class
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

inline void CMidiManager::UpdateUI(int nMsg, WPARAM wParam, LPARAM lParam)
{
	theApp.GetMainFrame()->PostMessage(nMsg, wParam, lParam);	// saves some typing
}

void CMidiManager::PushMasterProperty(int iProp, double fNormVal)
{
	double	fVal = CMasterRowDlg::Denorm(iProp, fNormVal);
	if (iProp == MASTER_Zoom) {	// if zoom
		PUSH_RENDER_CMD(cmd, RC_SET_ZOOM, true, dblVal, fVal);	// zoom with damping
	} else {	// generic case
		PUSH_RENDER_CMD(cmd, RC_SET_MASTER, iProp, dblVal, fVal);
	}
	UpdateUI(UWM_MASTER_PROP_CHANGE, iProp, FloatToLParam(fVal));
}

void CMidiManager::PushParameter(int iParam, int iProp, double fNormVal)
{
	const PARAM_INFO&	info = GetParamInfo(iParam);
	LPARAM	lParam;
	switch (iProp) {
	case PARAM_PROP_Wave:
		{
			int	iWaveform = Trunc(fNormVal * WAVEFORM_COUNT);
			iWaveform = CLAMP(iWaveform, 0, WAVEFORM_COUNT - 1);
			lParam = iWaveform;	// waveform is passed as integer
			PUSH_RENDER_CMD(cmd, RC_SET_PARAM_Wave, iParam, intVal, iWaveform);
		}
		break;
	default:
		int	iCmd = RC_SET_PARAM_Val + iProp;
		double	fVal;
		switch (iProp) {
		case PARAM_PROP_Val:
		case PARAM_PROP_Amp:
			fVal = fNormVal * (info.fMaxVal - info.fMinVal) + info.fMinVal;
			break;
		case PARAM_PROP_Global:
			{
				int	iGlobal = MapParamToGlobal(iParam);
				if (iGlobal < 0) {	// if parameter lacks a global
					return;	// avoid posting a useless update
				}
				fVal = (fNormVal - 0.5) * info.fMaxVal * 2;
				iCmd = RC_SET_DAMPED_GLOBAL;
			}
			break;
		default:
			fVal = fNormVal;
		}
		lParam = FloatToLParam(fVal);	// all other properties are passed as float
		PUSH_RENDER_CMD(cmd, iCmd, iParam, dblVal, fVal);
	}
	// using MAKELONG limits us to 64K parameters and 64K properties
	UpdateUI(UWM_PARAM_CHANGE, MAKELONG(iParam, iProp), lParam);
}

inline UINT CMidiManager::SetOrClear(UINT nDest, UINT nMask, bool bIsSet)
{
	return bIsSet ? (nDest | nMask) : (nDest & ~nMask);
}

void CMidiManager::PushMiscTarget(int iMiscTarget, double fNormVal)
{
	switch (iMiscTarget) {
	case MT_OriginX:
	case MT_OriginY:
		{
			int	nCmdIdx = (iMiscTarget == MT_OriginX ? RC_SET_ORIGIN_X : RC_SET_ORIGIN_Y);
			PUSH_RENDER_CMD(cmd, nCmdIdx, true, dblVal, fNormVal);	// origin motion with damping
			// don't update UI as our knowledge of origin is incomplete
		}
		break;
	case MT_Fill:
		{
			UINT nDrawMode = SetOrClear(theApp.GetDocument()->m_main.nDrawMode, DM_FILL, fNormVal != 0);
			PUSH_RENDER_CMD(cmd, RC_SET_MAIN, MAIN_DrawMode, uintVal, nDrawMode);
			UpdateUI(UWM_MAIN_PROP_CHANGE, cmd.m_nParam, cmd.m_prop.uintVal);
		}
		break;
	case MT_Outline:
		{
			UINT nDrawMode = SetOrClear(theApp.GetDocument()->m_main.nDrawMode, DM_OUTLINE, fNormVal != 0);
			PUSH_RENDER_CMD(cmd, RC_SET_MAIN, MAIN_DrawMode, uintVal, nDrawMode);
			UpdateUI(UWM_MAIN_PROP_CHANGE, cmd.m_nParam, cmd.m_prop.uintVal);
		}
		break;
	case MT_OriginDrag:
		{
			PUSH_RENDER_CMD(cmd, RC_SET_MAIN, MAIN_OrgMotion, intVal, fNormVal != 0 ? OM_DRAG : 0);
			UpdateUI(UWM_MAIN_PROP_CHANGE, cmd.m_nParam, cmd.m_prop.intVal);
		}
		break;
	case MT_OriginRandom:
		{
			PUSH_RENDER_CMD(cmd, RC_SET_MAIN, MAIN_OrgMotion, intVal, fNormVal != 0 ? OM_RANDOM : 0);
			UpdateUI(UWM_MAIN_PROP_CHANGE, cmd.m_nParam, cmd.m_prop.intVal);
		}
		break;
	case MT_Reverse:
		{
			PUSH_RENDER_CMD(cmd, RC_SET_MAIN, MAIN_Reverse, boolVal, fNormVal != 0);
			UpdateUI(UWM_MAIN_PROP_CHANGE, cmd.m_nParam, cmd.m_prop.boolVal);
		}
		break;
	case MT_Convex:
		{
			PUSH_RENDER_CMD(cmd, RC_SET_MAIN, MAIN_Convex, boolVal, fNormVal != 0);
			UpdateUI(UWM_MAIN_PROP_CHANGE, cmd.m_nParam, cmd.m_prop.boolVal);
		}
		break;
	case MT_LoopHue:
		{
			PUSH_RENDER_CMD(cmd, RC_SET_MAIN, MAIN_LoopHue, boolVal, fNormVal != 0);
			UpdateUI(UWM_MAIN_PROP_CHANGE, cmd.m_nParam, cmd.m_prop.boolVal);
		}
		break;
	case MT_RandomPhase:
		{
			CRenderCmd	cmd(RC_RANDOM_PHASE);
			theApp.PushRenderCommand(cmd);
		}
		break;
	case MT_Clear:
		{
			CRenderCmd	cmd(RC_SET_EMPTY);
			theApp.PushRenderCommand(cmd);
		}
		break;
	default:
		NODEFAULTCASE;	// missing case
	}
}

void CMidiManager::OnMidiEvent(DWORD dwEvent)
{
	if (!MIDI_IS_SHORT_MSG(dwEvent))	// if event is a short MIDI message
		return;	// ignore
	// exclude system status messages
	if (MIDI_STAT(dwEvent) >= SYSEX)	// if not channel voice message
		return;	// ignore
	if (m_bLearnMode) {	// if learning mappings
		// post MIDI message to mapping bar
		theApp.GetMainFrame()->m_wndMappingBar.PostMessage(UWM_MIDI_EVENT, dwEvent);
	}
	// lock the mapping array during iteration
	WCritSec::Lock lock(m_midiMaps.GetCritSec());
	int	nMaps = m_midiMaps.GetCount();
	for (int iMap = 0; iMap < nMaps; iMap++) {	// for each mapping
		const CMapping&	map = m_midiMaps.GetAt(iMap);
		int	nMidiVal = map.IsInputMatch(dwEvent);	// try to map MIDI message
		if (nMidiVal >= 0) {	// if message was mapped
			// mapping result is a MIDI data value; account for range and then normalize it
			int	nRange = map.m_nEnd - map.m_nStart;	// can be negative if start > end
			double	fMidiVal = nMidiVal / 127.0 * nRange + map.m_nStart;	// offset by start of range
			double	fNormVal = fMidiVal / 127.0;	// normalized target value
			const double	fCenterEpsilon = 0.005;	// half a percent, determined empirically
			if (fabs(fNormVal - 0.5) < fCenterEpsilon) {	// if within epsilon of center
				fNormVal = 0.5;	// call it center
			}
			// order must match order of ranges in mapping target enumeration
			int	iTarget = map.m_iTarget;
			if (iTarget < PARAM_COUNT) {	// if target is parameter
				PushParameter(iTarget, map.m_iProp, fNormVal);
			} else {	// target isn't parameter
				iTarget -= PARAM_COUNT;
				if (iTarget < MASTER_COUNT) {	// if target is master property
					PushMasterProperty(iTarget, fNormVal);
				} else {	// target isn't master property
					iTarget -= MASTER_COUNT;
					if (iTarget < MISC_TARGETS) {	// if miscellanous target
						PushMiscTarget(iTarget, fNormVal);
					}
				}
			}
		}
	}
}
