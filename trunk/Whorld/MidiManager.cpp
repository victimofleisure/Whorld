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
		04		03mar25	support full resolution pitch bend

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
	// This callback function runs in a worker thread context; 
	// data shared with other threads may require serialization.
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

inline void CMidiManager::PostMainMsg(int nMsg, WPARAM wParam, LPARAM lParam)
{
	theApp.GetMainFrame()->PostMessage(nMsg, wParam, lParam);	// saves some typing
}

void CMidiManager::PushMasterProperty(int iProp, double fNormVal)
{
	// This method runs in the MIDI callback's worker thread context;
	// only push render commands or post messages to the main thread.
	double	fVal = CMasterRowDlg::Denorm(iProp, fNormVal);
	if (iProp == MASTER_Zoom) {	// if zoom
		theApp.m_thrRender.SetZoom(fVal, true);	// with damping
	} else {	// generic case
		theApp.m_thrRender.SetMasterProp(iProp, fVal);
	}
	PostMainMsg(UWM_MASTER_PROP_CHANGE, iProp, FloatToLParam(fVal));
}

void CMidiManager::PushParameter(int iParam, int iProp, double fNormVal)
{
	// This method runs in the MIDI callback's worker thread context;
	// only push render commands or post messages to the main thread.
	const PARAM_INFO&	info = GetParamInfo(iParam);
	LPARAM	lParam;
	switch (iProp) {
	case PARAM_PROP_Wave:
		{
			int	iWaveform = Trunc(fNormVal * WAVEFORM_COUNT);
			iWaveform = CLAMP(iWaveform, 0, WAVEFORM_COUNT - 1);
			VARIANT_PROP	prop;
			prop.intVal = iWaveform;
			theApp.m_thrRender.SetParam(iParam, PARAM_PROP_Wave, prop);
			lParam = iWaveform;	// waveform is passed as integer
		}
		break;
	case PARAM_PROP_Global:
		{
			int	iGlobal = MapParamToGlobal(iParam);
			if (iGlobal < 0) {	// if parameter lacks a global
				return;	// avoid posting a useless update
			}
			double	fVal = (fNormVal - 0.5) * info.fMaxVal * 2;
			theApp.m_thrRender.SetDampedGlobal(iParam, fVal);
			lParam = FloatToLParam(fVal);	// globals is passed as float
		}
		break;
	default:
		double	fVal;
		switch (iProp) {
		case PARAM_PROP_Val:
		case PARAM_PROP_Amp:
			fVal = fNormVal * (info.fMaxVal - info.fMinVal) + info.fMinVal;
			break;
		default:
			fVal = fNormVal;
		}
		lParam = FloatToLParam(fVal);	// all other properties are passed as float
		VARIANT_PROP	prop;
		prop.dblVal = fVal;
		theApp.m_thrRender.SetParam(iParam, iProp, prop);
	}
	// using MAKELONG limits us to 64K parameters and 64K properties, that's fine
	PostMainMsg(UWM_PARAM_CHANGE, MAKELONG(iParam, iProp), lParam);
}

inline UINT CMidiManager::SetOrClear(UINT nDest, UINT nMask, bool bIsSet)
{
	return bIsSet ? (nDest | nMask) : (nDest & ~nMask);
}

void CMidiManager::PushMiscTarget(int iMiscTarget, double fNormVal)
{
	// This method runs in the MIDI callback's worker thread context;
	// only push render commands or post messages to the main thread.
	switch (iMiscTarget) {
	case MT_OriginX:
		theApp.m_thrRender.SetOriginX(fNormVal, true);	// with damping
		break;
	case MT_OriginY:
		theApp.m_thrRender.SetOriginY(fNormVal, true);	// with damping
		break;
	case MT_Fill:
		theApp.m_thrRender.SetDrawMode(DM_FILL, fNormVal != 0 ? DM_FILL : 0);
		break;
	case MT_Outline:
		theApp.m_thrRender.SetDrawMode(DM_OUTLINE, fNormVal != 0 ? DM_OUTLINE : 0);
		break;
	case MT_OriginDrag:
		PushOriginMotion(fNormVal != 0 ? OM_DRAG : OM_PARK);
		break;
	case MT_OriginRandom:
		PushOriginMotion(fNormVal != 0 ? OM_RANDOM : OM_PARK);
		break;
	case MT_Reverse:
		PushMainBool(MAIN_Reverse, fNormVal != 0);
		break;
	case MT_Convex:
		PushMainBool(MAIN_Convex, fNormVal != 0);
		break;
	case MT_LoopHue:
		PushMainBool(MAIN_LoopHue, fNormVal != 0);
		break;
	case MT_RandomPhase:
		theApp.m_thrRender.RandomPhase();
		break;
	case MT_Clear:
		theApp.m_thrRender.SetEmpty();
		break;
	case MT_Pause:
		// let main thread handle pause as its pause state is canonical
		PostMainMsg(WM_COMMAND, ID_WINDOW_PAUSE, 0);
		break;
	default:
		NODEFAULTCASE;	// missing case
	}
}

void CMidiManager::PushMainBool(int iProp, bool bVal)
{
	VARIANT_PROP	prop;
	prop.boolVal = bVal;
	theApp.m_thrRender.SetMainProp(iProp, prop);
	PostMainMsg(UWM_MAIN_PROP_CHANGE, iProp, bVal);
}

void CMidiManager::PushOriginMotion(int nOrgMotion)
{
	VARIANT_PROP	prop;
	prop.intVal = nOrgMotion;
	theApp.m_thrRender.SetMainProp(MAIN_OrgMotion, prop);
	PostMainMsg(UWM_MAIN_PROP_CHANGE, MAIN_OrgMotion, prop.intVal);
}

void CMidiManager::OnMidiEvent(DWORD dwEvent)
{
	// This method runs in the MIDI callback's thread context;
	// data shared with other threads may require serialization.
	if (!MIDI_IS_SHORT_MSG(dwEvent))	// if event is a short MIDI message
		return;	// ignore
	// exclude system status messages
	if (MIDI_STAT(dwEvent) >= SYSEX)	// if not channel voice message
		return;	// ignore
	if (m_bLearnMode) {	// if learning mappings
		// post MIDI message to mapping bar; do NOT access bar's members directly
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
			int	nMapRange = map.m_nEnd - map.m_nStart;	// can be negative if start > end
			int	nMidiMax;
			double	fCenterEpsilon;
			if (map.m_iEvent == MIDI_CVM_WHEEL) {	// if pitch bend
				nMidiMax = MIDI_PITCH_BEND_MAX;
				fCenterEpsilon = 0.00005;	// 100 times more resolution
			} else {	// not pitch bend
				nMidiMax = MIDI_NOTE_MAX;
				fCenterEpsilon = 0.005;	// determined empirically
			}
			// convert from MIDI to range coordinates and offset by start of range
			double	fMidiVal = double(nMidiVal) / nMidiMax * nMapRange + map.m_nStart;
			double	fNormVal = fMidiVal / MIDI_NOTE_MAX;	// normalized target value
			// many targets are zero-centered, so we must avoid infinitesimal at center
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
