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
		05		10mar25	fix learn mode; post to mapping bar, not main frame
		06		11mar25	remove output device from device change handler
		07		12mar25	fix draw mode change not updating UI
		08		19mar25	make mapping range real instead of integer
		09		25mar25	add random origin target
		10		29mar25	add patch target
		11      11apr25	add antialiasing target

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
		CString	sErrorMsg;
		int	nIDHelp;
		sErrorMsg.Format(IDS_SEQ_MIDI_ERROR, nResult);
		sErrorMsg += '\n' + CMidiOut::GetErrorString(nResult);
		theApp.WriteLogEntry(sErrorMsg);
		nIDHelp = -1;
		AfxMessageBox(sErrorMsg, MB_OK, nIDHelp);
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

bool CMidiManager::CloseInputDevice()
{
	return MIDI_SUCCEEDED(m_midiIn.Close());
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
		m_midiDevs.OnDeviceChange(nChangeMask);	// handle MIDI device change
		if (nChangeMask & CMidiDevices::CM_INPUT) {	// if MIDI input device changed
			OpenInputDevice();
		}
		if (nChangeMask & CMidiDevices::CM_CHANGE) {	// if MIDI device state changed
			CMainFrame	*pMainFrm = theApp.GetMainFrame();
			if (pMainFrm != NULL) {
				CWnd	*pPopupWnd = pMainFrm->GetLastActivePopup();
				COptionsDlg	*pOptionsDlg = DYNAMIC_DOWNCAST(COptionsDlg, pPopupWnd);
				if (pOptionsDlg != NULL)	// if options dialog is active
					pOptionsDlg->UpdateMidiDevices();	// update dialog's MIDI device combos
			}
		}
	}
}

void CALLBACK CMidiManager::MidiInProc(HMIDIIN hMidiIn, UINT wMsg, W64UINT dwInstance, W64UINT dwParam1, W64UINT dwParam2)
{
	// This callback function runs in a worker thread context; 
	// data shared with other threads may require serialization.
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

inline BOOL CMidiManager::PostMsgToMainWnd(int nMsg, WPARAM wParam, LPARAM lParam)
{
	CMainFrame	*pMainFrame = theApp.GetMainFrame();
	if (pMainFrame != NULL) {	// if main window exists
		return pMainFrame->PostMessage(nMsg, wParam, lParam);
	}
	return false;
}

inline BOOL CMidiManager::PostMsgToMappingBar(int nMsg, WPARAM wParam, LPARAM lParam)
{
	CMainFrame	*pMainFrame = theApp.GetMainFrame();
	if (pMainFrame != NULL) {	// if main window exists
		return pMainFrame->m_wndMappingBar.PostMessage(nMsg, wParam, lParam);
	}
	return false;
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
	PostMsgToMainWnd(UWM_MASTER_PROP_CHANGE, iProp, FloatToLParam(fVal));
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
			VARIANT_PROP	prop = {iWaveform};
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
	PostMsgToMainWnd(UWM_PARAM_CHANGE, MAKELONG(iParam, iProp), lParam);
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
		PushDrawMode(DM_FILL, fNormVal != 0);
		break;
	case MT_Outline:
		PushDrawMode(DM_OUTLINE, fNormVal != 0);
		break;
	case MT_Mirror:
		// not implemented yet
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
	case MT_ZoomCenter:
		PushMainBool(MAIN_ZoomCenter, fNormVal != 0);
		break;
	case MT_Antialiasing:
		PushMainBool(MAIN_Antialiasing, fNormVal != 0);
		break;
	case MT_RandomPhase:
		theApp.m_thrRender.RandomPhase();
		break;
	case MT_RandomOrigin:
		theApp.m_thrRender.RandomOrigin();
		break;
	case MT_Clear:
		theApp.m_thrRender.SetEmpty();
		break;
	case MT_Pause:
		// let main thread handle pause as its pause state is canonical
		PostMsgToMainWnd(WM_COMMAND, ID_WINDOW_PAUSE, 0);
		break;
	case MT_Patch:
		// let main thread handle patch as read can throw file exceptions
		PostMsgToMainWnd(UWM_PLAY_PATCH, Round(fNormVal * MIDI_NOTE_MAX));
		break;
	default:
		NODEFAULTCASE;	// missing case
	}
}

void CMidiManager::PushMainBool(int iProp, bool bVal)
{
	VARIANT_PROP	prop = {bVal};
	theApp.m_thrRender.SetMainProp(iProp, prop);
	PostMsgToMainWnd(UWM_MAIN_PROP_CHANGE, iProp, bVal);
}

void CMidiManager::PushOriginMotion(int nOrgMotion)
{
	VARIANT_PROP	prop = {nOrgMotion};
	theApp.m_thrRender.SetMainProp(MAIN_OrgMotion, prop);
	PostMsgToMainWnd(UWM_MAIN_PROP_CHANGE, MAIN_OrgMotion, prop.intVal);
}

void CMidiManager::PushDrawMode(UINT nMask, bool bEnable)
{
	UINT	nValue = bEnable ? nMask : 0;
	theApp.m_thrRender.SetDrawMode(nMask, nValue);
	PostMsgToMainWnd(UWM_SET_DRAW_MODE, nMask, nValue);
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
		PostMsgToMappingBar(UWM_MIDI_EVENT, dwEvent);
	}
	// lock the mapping array during iteration
	WCritSec::Lock lock(m_midiMaps.GetCritSec());
	int	nMaps = m_midiMaps.GetCount();
	for (int iMap = 0; iMap < nMaps; iMap++) {	// for each mapping
		const CMapping&	map = m_midiMaps.GetAt(iMap);
		int	nMidiVal = map.IsInputMatch(dwEvent);	// try to map MIDI message
		if (nMidiVal >= 0) {	// if message was mapped
			// mapping result is a MIDI data value; account for range and then normalize it
			double	fNormVal;
			if (map.m_iEvent != MIDI_CVM_WHEEL) {	// if input MIDI value is 7-bit
				// Split normalization into two ranges, to compensate for the fact that an
				// integer range with an even number of values has a fractional midpoint.
				if (nMidiVal <= MIDI_NOTES / 2) {	// if at or below MIDI center
					fNormVal = static_cast<double>(nMidiVal) / MIDI_NOTES;
				} else {	// above MIDI center; scale differently to minimize error
					fNormVal = static_cast<double>(nMidiVal - 1) / (MIDI_NOTE_MAX - 1);
				}
			} else {	// pitch bend event; input MIDI value is 14-bit
				// same as 7-bit case except it substitutes 14-bit constants
				if (nMidiVal <= MIDI_PITCH_BEND_STEPS / 2) {	// if at or below center
					fNormVal = static_cast<double>(nMidiVal) / MIDI_PITCH_BEND_STEPS;
				} else {	// above MIDI center; scale differently to minimize error
					fNormVal = static_cast<double>(nMidiVal - 1) / (MIDI_PITCH_BEND_MAX - 1);
				}
			}
			double	fStart = map.m_fStart / 100;	// range start and end are both percentages
			double	fEnd = map.m_fEnd / 100;	// and must be converted to normalized coords
			double	fRange = fEnd - fStart;	// range can be negative if start > end
			fNormVal = fNormVal * fRange + fStart;	// apply range to normalized value
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
