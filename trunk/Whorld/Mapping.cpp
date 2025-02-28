// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20mar20	initial version
		01		27mar20	match control parameter only if event uses it
		02		29mar20	add get/set input message for selected mappings
		03		05apr20	add track step mapping
		04		07sep20	add preset and part mapping
		05		15feb21	add mapping targets for transport commands
		06		07jun21	rename rounding functions
		07		30jun21	move step mapping range checks into critical section
		08		25oct21	add optional sort direction
		09		21jan22	add tempo mapping target
		10		05feb22	add tie mapping target
		11		19feb22	use INI file class directly instead of via profile
		12		23dec23	add mapping target for note overlap method
		13		01sep24	add mapping target for duplicate note method
		14		26feb25	adapt for Whorld

*/

#include "stdafx.h"
#include "Whorld.h"
#include "Mapping.h"
#include "Midi.h"
#include "IniFile.h"

#define RK_MAPPING_COUNT _T("Count")	// registry keys
#define RK_MAPPING_SECTION _T("Mapping")
#define RK_MAPPING_IN_EVENT _T("InEvent")
#define RK_MAPPING_OUT_EVENT _T("OutEvent")

const LPCTSTR CMapping::m_arrChanStatTag[MIDI_CHANNEL_VOICE_MESSAGES] = {
	#define MIDICHANSTATDEF(name) _T(#name),
	#include "MidiCtrlrDef.h"
};

const LPCTSTR CMapping::m_arrSysStatTag[MIDI_SYSTEM_STATUS_MESSAGES] = {
	#define MIDISYSSTATDEF(name) _T(#name),
	#include "MidiCtrlrDef.h"
};

const int CMapping::m_arrChanStatID[MIDI_CHANNEL_VOICE_MESSAGES] = {
	#define MIDICHANSTATDEF(name) IDS_CHAN_STAT_##name,
	#include "MidiCtrlrDef.h"
};

#define IDS_MIDI_SYS_STAT_UNDEFINED_1 0
#define IDS_MIDI_SYS_STAT_UNDEFINED_2 0
#define IDS_MIDI_SYS_STAT_UNDEFINED_3 0
#define IDS_MIDI_SYS_STAT_UNDEFINED_4 0
const int CMapping::m_arrSysStatID[MIDI_SYSTEM_STATUS_MESSAGES] = {
	#define MIDISYSSTATDEF(name) IDS_MIDI_SYS_STAT_##name,
	#include "MidiCtrlrDef.h"
};

CStringArrayEx CMapping::m_arrChanStatName;
CStringArrayEx CMapping::m_arrSysStatName;

#define LOCK_MAPPINGS WCritSec::Lock lock(m_csMapping);

void CMapping::Initialize()
{
	m_arrChanStatName.SetSize(_countof(m_arrChanStatID));
	for (int iChSt = 0; iChSt < _countof(m_arrChanStatID); iChSt++)
		m_arrChanStatName[iChSt].LoadString(m_arrChanStatID[iChSt]);
	m_arrSysStatName.SetSize(_countof(m_arrSysStatID));
	for (int iSysSt = 0; iSysSt < _countof(m_arrSysStatID); iSysSt++)
		m_arrSysStatName[iSysSt].LoadString(m_arrSysStatID[iSysSt]);
}

void CMapping::SetDefaults()
{
	m_nInEvent = MIDI_CVM_CONTROL;
	m_nInChannel = 0;
	m_nInControl = 1;
	m_nOutEvent = 0;
	m_nRangeStart = 0;
	m_nRangeEnd = 127;
}

int CMapping::GetProperty(int iProp) const
{
	ASSERT(iProp >= 0 && iProp < PROPERTIES);
	switch (iProp) {
	#define MAPPINGDEF(name, align, width, member, minval, maxval) case PROP_##name: return m_n##member;
	#include "MappingDef.h"	// generate cases for each member var
	}
	return 0;	// error
}

void CMapping::SetProperty(int iProp, int nVal)
{
	ASSERT(iProp >= 0 && iProp < PROPERTIES);
	switch (iProp) {
	#define MAPPINGDEF(name, align, width, member, minval, maxval) case PROP_##name: m_n##member = nVal; break;
	#include "MappingDef.h"	// generate cases for each member
	}
}

CString CMapping::GetOutputEventName(int nOutEvent)
{
	if (nOutEvent < MASTER_COUNT) {
		return GetMasterName(nOutEvent);
	}
	nOutEvent -= MASTER_COUNT;
	return GetParamName(nOutEvent);
}

inline LPCTSTR CMapping::GetOutputEventTag(int nOutEvent)
{
	if (nOutEvent < MASTER_COUNT) {
		return GetMasterInfo(nOutEvent).pszName;
	}
	nOutEvent -= MASTER_COUNT;
	return GetParamInfo(nOutEvent).pszName;
}

inline int CMapping::FindOutputEventTag(LPCTSTR pszName)
{
	int	iPos = FindMasterByName(pszName);
	if (iPos < 0) {
		iPos = FindParamByName(pszName);
		if (iPos >= 0) {
			iPos += MASTER_COUNT;
		}
	}
	return iPos;
}

void CMapping::Read(CIniFile& fIn, LPCTSTR pszSection)
{
	CString	sName;
	sName = fIn.GetString(pszSection, RK_MAPPING_IN_EVENT);
	m_nInEvent = FindInputEventTag(sName);
	ASSERT(m_nInEvent >= 0);	// check for unknown input event name
	if (m_nInEvent < 0)	// if unknown input event name
		m_nInEvent = 0;		// avoid range errors downstream
	sName = fIn.GetString(pszSection, RK_MAPPING_OUT_EVENT);
	m_nOutEvent = FindOutputEventTag(sName);
	ASSERT(m_nOutEvent >= 0);	// check for unknown output event name
	if (m_nOutEvent < 0)	// if unknown output event name
		m_nOutEvent = 0;		// avoid range errors downstream
	// conditional to exclude events is optimized away in release build
	#define MAPPINGDEF(name, align, width, member, minval, maxval) \
		if (PROP_##name != PROP_IN_EVENT && PROP_##name != PROP_OUT_EVENT) \
			m_n##member = fIn.GetInt(pszSection, _T(#member), 0);
	#include "MappingDef.h"	// generate profile read for each member, excluding events
}

void CMapping::Write(CIniFile& fOut, LPCTSTR pszSection) const
{
	fOut.WriteString(pszSection, RK_MAPPING_IN_EVENT, GetInputEventTag(m_nInEvent));
	fOut.WriteString(pszSection, RK_MAPPING_OUT_EVENT, GetOutputEventTag(m_nOutEvent));
	// conditional to exclude events is optimized away in release build
	#define MAPPINGDEF(name, align, width, member, minval, maxval) \
		if (PROP_##name != PROP_IN_EVENT && PROP_##name != PROP_OUT_EVENT) \
			fOut.WriteInt(pszSection, _T(#member), m_n##member);
	#include "MappingDef.h"	// generate profile write for each member, excluding events
}

void CMappingArray::Read(CIniFile& fIn)
{
	CString	sSectionIdx;
	int	nItems = fIn.GetInt(RK_MAPPING_SECTION, _T("Count"), 0);
	SetSize(nItems);
	for (int iItem = 0; iItem < nItems; iItem++) {	// for each mapping
		sSectionIdx.Format(_T("%d"), iItem);
		GetAt(iItem).Read(fIn, RK_MAPPING_SECTION _T("\\") + sSectionIdx);
	}
}

DWORD CMapping::GetInputMidiMsg() const
{
	return MakeMidiMsg((m_nInEvent + 8) << 4, m_nInChannel, m_nInControl, 0);
}

void CMapping::SetInputMidiMsg(DWORD nInMidiMsg)
{
	// channel voice messages only; caller is responsible for ensuring this
	int	iEvent = MIDI_CMD_IDX(nInMidiMsg);	// convert MIDI status to event index
	ASSERT(iEvent >= 0 && iEvent < INPUT_EVENTS);	// check event index range
	m_nInEvent = iEvent;
	m_nInChannel = MIDI_CHAN(nInMidiMsg);
	m_nInControl = MIDI_P1(nInMidiMsg);
}

int CMapping::IsInputMatch(DWORD nInMidiMsg) const
{
	int	iInChan = MIDI_CHAN(nInMidiMsg);	// channel index
	if (iInChan != m_nInChannel) {	// if channel doesn't match
		return -1;	// fail
	}
	int	iInCmd = MIDI_CMD_IDX(nInMidiMsg);	// command index
	if (iInCmd != m_nInEvent) {	// if event doesn't match
		return -1;	// fail
	}
	int	iInP1 = MIDI_P1(nInMidiMsg);	// parameter 1
	if (iInCmd <= MIDI_CVM_CONTROL)	{	// if command has a note/controller
		if (iInP1 != m_nInControl) {	// if control doesn't match
			return -1;	// fail
		}
		return MIDI_P2(nInMidiMsg);	// success: parameter 2 is data value
	} else {	// command doesn't have a note/controller
		return iInP1;	// success: parameter 1 is data value
	}
}

void CMappingArray::Write(CIniFile& fOut) const
{
	CString	sSectionIdx;
	int	nItems = GetSize();
	fOut.WriteInt(RK_MAPPING_SECTION, _T("Count"), nItems);
	for (int iItem = 0; iItem < nItems; iItem++) {	// for each mapping
		sSectionIdx.Format(_T("%d"), iItem);
		GetAt(iItem).Write(fOut, RK_MAPPING_SECTION _T("\\") + sSectionIdx);
	}
}

void CSeqMapping::RemoveAll()
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.RemoveAll();
}

void CSeqMapping::SetArray(const CMappingArray& arrMapping)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping = arrMapping;
}

void CSeqMapping::SetProperty(int iMapping, int iProp, int nVal)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping[iMapping].SetProperty(iProp, nVal);
}

void CSeqMapping::GetSelection(const CIntArrayEx& arrSelection, CMappingArray& arrMapping) const
{
	m_arrMapping.GetSelection(arrSelection, arrMapping);
}

void CSeqMapping::SetInputMidiMsg(int iMapping, DWORD nInMidiMsg)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping[iMapping].SetInputMidiMsg(nInMidiMsg);
}

void CSeqMapping::GetProperty(const CIntArrayEx& arrSelection, int iProp, CIntArrayEx& arrProp) const
{
	int	nSels = arrSelection.GetSize();
	arrProp.SetSize(nSels);
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		arrProp[iSel] = m_arrMapping[iMapping].GetProperty(iProp);
	}
}

void CSeqMapping::SetProperty(const CIntArrayEx& arrSelection, int iProp, const CIntArrayEx& arrProp)
{
	LOCK_MAPPINGS; // exclusive write
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		m_arrMapping[iMapping].SetProperty(iProp, arrProp[iSel]);
	}
}

void CSeqMapping::SetProperty(const CIntArrayEx& arrSelection, int iProp, int nVal)
{
	LOCK_MAPPINGS; // exclusive write
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		m_arrMapping[iMapping].SetProperty(iProp, nVal);
	}
}

void CSeqMapping::Insert(int iInsert, CMappingArray& arrMapping)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.InsertAt(iInsert, &arrMapping);
}

void CSeqMapping::Insert(const CIntArrayEx& arrSelection, CMappingArray& arrMapping)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.InsertSelection(arrSelection, arrMapping);
}

void CSeqMapping::Delete(int iMapping, int nCount)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.RemoveAt(iMapping, nCount);
}

void CSeqMapping::Delete(const CIntArrayEx& arrSelection)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.DeleteSelection(arrSelection);
}

void CSeqMapping::Move(const CIntArrayEx& arrSelection, int iDropPos)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.MoveSelection(arrSelection, iDropPos);
}

void CSeqMapping::Sort(int iProp, bool bDescending)
{
	LOCK_MAPPINGS; // exclusive write
	m_iSortProp = iProp;
	m_bSortDescending = bDescending;
	qsort(m_arrMapping.GetData(), m_arrMapping.GetSize(), sizeof(CMapping), SortCompare);
}

int CSeqMapping::m_iSortProp;	// index of property to sort mappings by
bool CSeqMapping::m_bSortDescending;	// true if sort should be descending

int CSeqMapping::SortCompare(const void *arg1, const void *arg2)
{
	const CMapping*	pMap1 = (CMapping *)arg1;
	const CMapping*	pMap2 = (CMapping *)arg2;
	return SortCompareTpl(pMap1->GetProperty(m_iSortProp), pMap2->GetProperty(m_iSortProp));
}

void CSeqMapping::GetInputMidiMsg(const CIntArrayEx& arrSelection, CIntArrayEx& arrInMidiMsg) const
{
	int	nSels = arrSelection.GetSize();
	arrInMidiMsg.SetSize(nSels);
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		arrInMidiMsg[iSel] = m_arrMapping[iMapping].GetInputMidiMsg();
	}
}

void CSeqMapping::SetInputMidiMsg(const CIntArrayEx& arrSelection, DWORD nInMidiMsg)
{
	LOCK_MAPPINGS; // exclusive write
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		m_arrMapping[iMapping].SetInputMidiMsg(nInMidiMsg);	// set input MIDI message
	}
}

void CSeqMapping::SetInputMidiMsg(const CIntArrayEx& arrSelection, const CIntArrayEx& arrInMidiMsg)
{
	LOCK_MAPPINGS; // exclusive write
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		m_arrMapping[iMapping].SetInputMidiMsg(arrInMidiMsg[iSel]);
	}
}

void CSeqMapping::Read(CIniFile& fIn)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.Read(fIn);
}

void CSeqMapping::Write(CIniFile& fOut) const
{
	m_arrMapping.Write(fOut);
}
