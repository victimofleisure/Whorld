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
		15		01mar25	add misc targets
		16		03mar25	support full resolution pitch bend
		17		19mar25	make mapping range real instead of integer
		18		25mar25	add target types

*/

#include "stdafx.h"
#include "Whorld.h"
#include "Mapping.h"
#include "Midi.h"
#include "IniFile.h"

#define RK_MAPPING_COUNT _T("Count")	// registry keys
#define RK_MAPPING_SECTION _T("Mapping")
#define RK_MAPPING_EVENT _T("Event")
#define RK_MAPPING_TARGET _T("Target")
#define RK_MAPPING_PROPERTY _T("Prop")

const LPCTSTR CMappingBase::m_arrChanStatTag[MIDI_CHANNEL_VOICE_MESSAGES] = {
	#define MIDICHANSTATDEF(name) _T(#name),
	#include "MidiCtrlrDef.h"
};

const LPCTSTR CMappingBase::m_arrSysStatTag[MIDI_SYSTEM_STATUS_MESSAGES] = {
	#define MIDISYSSTATDEF(name) _T(#name),
	#include "MidiCtrlrDef.h"
};

const int CMappingBase::m_arrChanStatID[MIDI_CHANNEL_VOICE_MESSAGES] = {
	#define MIDICHANSTATDEF(name) IDS_CHAN_STAT_##name,
	#include "MidiCtrlrDef.h"
};

#define IDS_MIDI_SYS_STAT_UNDEFINED_1 0
#define IDS_MIDI_SYS_STAT_UNDEFINED_2 0
#define IDS_MIDI_SYS_STAT_UNDEFINED_3 0
#define IDS_MIDI_SYS_STAT_UNDEFINED_4 0
const int CMappingBase::m_arrSysStatID[MIDI_SYSTEM_STATUS_MESSAGES] = {
	#define MIDISYSSTATDEF(name) IDS_MIDI_SYS_STAT_##name,
	#include "MidiCtrlrDef.h"
};

const LPCTSTR CMappingBase::m_arrMiscTargetTag[MISC_TARGETS] = {
	#define MAPPINGDEF_MISC_TARGET(name, type) _T(#name),
	#include "MappingDef.h"	// generate enumeration
};

const int CMappingBase::m_arrMiscTargetID[MISC_TARGETS] = {
	#define MAPPINGDEF_MISC_TARGET(name, type) IDS_MAP_TARGET_##name,
	#include "MappingDef.h"	// generate enumeration
};

const BYTE CMappingBase::m_arrMiscTargetType[MISC_TARGETS] = {
	#define MAPPINGDEF_MISC_TARGET(name, type) TT_##type,
	#include "MappingDef.h"	// generate enumeration
};

CString CMappingBase::m_arrChanStatName[MIDI_CHANNEL_VOICE_MESSAGES];
CString CMappingBase::m_arrSysStatName[MIDI_SYSTEM_STATUS_MESSAGES];
LPCTSTR CMappingBase::m_arrTargetTag[TARGETS];
CString CMappingBase::m_arrTargetName[TARGETS];
BYTE CMappingBase::m_arrTargetType[TARGETS];

#define LOCK_MAPPINGS WCritSec::Lock lock(m_csMapping);

void CMappingBase::Initialize()
{
	// load string resources only once; this can't be done in ctor
	for (int iChSt = 0; iChSt < _countof(m_arrChanStatID); iChSt++)
		m_arrChanStatName[iChSt].LoadString(m_arrChanStatID[iChSt]);
	for (int iSysSt = 0; iSysSt < _countof(m_arrSysStatID); iSysSt++)
		m_arrSysStatName[iSysSt].LoadString(m_arrSysStatID[iSysSt]);
	// order must match order of ranges in mapping target enumeration
	int	iTarget = 0;
	// parameters
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++, iTarget++) {
		m_arrTargetTag[iTarget] = GetParamInfo(iParam).pszTag;
		m_arrTargetName[iTarget] = GetParamName(iParam);
	}
	// master properties
	for (int iMaster = 0; iMaster < MASTER_COUNT; iMaster++, iTarget++) {
		m_arrTargetTag[iTarget] = GetMasterInfo(iMaster).pszTag;
		m_arrTargetName[iTarget] = GetMasterName(iMaster);
	}
	// miscellaneous targets
	for (int iMisc = 0; iMisc < MISC_TARGETS; iMisc++, iTarget++) {
		m_arrTargetTag[iTarget] = m_arrMiscTargetTag[iMisc];
		m_arrTargetName[iTarget].LoadString(m_arrMiscTargetID[iMisc]);
		m_arrTargetType[iTarget] = m_arrMiscTargetType[iMisc];
	}
}

void CMapping::SetDefaults()
{
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		m_##prefix##member = initval;
	#include "MappingDef.h"	// generate member var initialization
}

CMapping::VARIANT_PROP CMapping::GetProperty(int iProp) const
{
	ASSERT(iProp >= 0 && iProp < PROPERTIES);
	VARIANT_PROP	prop = {0};	// clear entire property
	switch (iProp) {
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		case PROP_##name: prop.type##Val = m_##prefix##member; break;
	#include "MappingDef.h"	// generate cases for each member var
	}
	return prop;
}

void CMapping::SetProperty(int iProp, VARIANT_PROP prop)
{
	ASSERT(iProp >= 0 && iProp < PROPERTIES);
	switch (iProp) {
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		case PROP_##name: m_##prefix##member = prop.type##Val; break;
	#include "MappingDef.h"	// generate cases for each member
	}
}

int	CMapping::Compare(int iProp, VARIANT_PROP prop1, VARIANT_PROP prop2)
{
	ASSERT(iProp >= 0 && iProp < PROPERTIES);
	switch (iProp) {
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		case PROP_##name: return SortCompareTpl(prop1.type##Val, prop2.type##Val);
	#include "MappingDef.h"	// generate cases for each member
	}
	return 0;
}

#define STAY_POSITIVE(x) x = max(x, 0)

void CMapping::Read(CIniFile& fIn, LPCTSTR pszSection)
{
	SetDefaults();	// init members to default values
	CString	sTag;
	// read event tag
	sTag = fIn.GetString(pszSection, RK_MAPPING_EVENT);
	m_iEvent = FindEventTag(sTag);
	ASSERT(IsValidEvent(m_iEvent));	// validate event index
	STAY_POSITIVE(m_iEvent);	// avoid downstream range error
	// read target tag
	sTag = fIn.GetString(pszSection, RK_MAPPING_TARGET);
	m_iTarget = FindTargetTag(sTag);
	ASSERT(IsValidTarget(m_iTarget));	// validate target index
	STAY_POSITIVE(m_iTarget);	// avoid downstream range error
	if (TargetHasProperty()) {	// if target has a property
		// read property tag
		sTag = fIn.GetString(pszSection, RK_MAPPING_PROPERTY);
		m_iProp = FindParamPropByTag(sTag);
		ASSERT(IsValidParamProp(m_iProp));	// validate parameter index
		STAY_POSITIVE(m_iProp);	// avoid downstream range error
	} else {	// target doesn't have a property
		m_iProp = PARAM_PROP_Val;	// set property to reasonable default
	}
	#define MAPPINGDEF_EXCLUDE_TAGS	// tags were already read above
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		fIn.Get(pszSection, _T(#member), m_##prefix##member);
	#include "MappingDef.h"	// generate profile reads for remaining members
}

void CMapping::Write(CIniFile& fOut, LPCTSTR pszSection) const
{
	// write event tag
	fOut.WriteString(pszSection, RK_MAPPING_EVENT, GetEventTag(m_iEvent));
	// write target tag
	fOut.WriteString(pszSection, RK_MAPPING_TARGET, GetTargetTag(m_iTarget));
	if (TargetHasProperty()) {	// if target has a property
		// write property tag
		fOut.WriteString(pszSection, RK_MAPPING_PROPERTY, GetParamPropInfo(m_iProp).pszTag);
	}
	#define MAPPINGDEF_EXCLUDE_TAGS	// tags were already written above
	// mandatory members are always written, regardless of their value
	#define MAPPINGDEF_OPTIONAL 0	// include mandatory members, exclude optional ones
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		fOut.Put(pszSection, _T(#member), m_##prefix##member);
	#include "MappingDef.h"	// generate profile writes for mandatory members
	#define MAPPINGDEF_EXCLUDE_TAGS	// tags were already written above
	// optional members are only written if they differ from their initial value
	#define MAPPINGDEF_OPTIONAL 1	// include optional members, exclude mandatory ones
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		if (m_##prefix##member != initval) \
			fOut.Put(pszSection, _T(#member), m_##prefix##member);
	#include "MappingDef.h"	// generate profile writes for optional members
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
	return MakeMidiMsg(MIDI_IDX_CMD(m_iEvent), m_iChannel, m_iControl, 0);
}

void CMapping::SetInputMidiMsg(DWORD nInMidiMsg)
{
	// channel voice messages only; caller is responsible for ensuring this
	int	iCmd = MIDI_CMD_IDX(nInMidiMsg);	// convert MIDI status to event index
	ASSERT(iCmd >= 0 && iCmd < EVENTS);	// check event index range
	m_iEvent = iCmd;	// set event to channel voice command index
	m_iChannel = MIDI_CHAN(nInMidiMsg);	// set channel index
	m_iControl = MIDI_P1(nInMidiMsg);	// set note/controller number
}

int CMapping::IsInputMatch(DWORD nInMidiMsg) const
{
	int	iChan = MIDI_CHAN(nInMidiMsg);	// channel index
	if (iChan != m_iChannel) {	// if channel doesn't match
		return -1;	// fail
	}
	int	iCmd = MIDI_CMD_IDX(nInMidiMsg);	// command index
	if (iCmd != m_iEvent) {	// if event doesn't match
		return -1;	// fail
	}
	int	nP1 = MIDI_P1(nInMidiMsg);	// parameter 1
	int	nP2 = MIDI_P2(nInMidiMsg);	// parameter 2
	if (iCmd <= MIDI_CVM_CONTROL) {	// if command has a note/controller
		if (nP1 != m_iControl) {	// if control doesn't match
			return -1;	// fail
		}
		return nP2;	// data is P2
	} else {	// command doesn't have a note/controller
		if (iCmd == MIDI_CVM_WHEEL) {
			return nP1 | (nP2 << 7);	// data is P1 and P2
		} else {
			return nP1;	// data is P1
		}
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

void CSafeMapping::RemoveAll()
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.RemoveAll();
}

void CSafeMapping::SetAt(int iMapping, const CMapping& mapping)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping[iMapping] = mapping;
}

void CSafeMapping::SetArray(const CMappingArray& arrMapping)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping = arrMapping;
}

void CSafeMapping::SetProperty(int iMapping, int iProp, VARIANT_PROP prop)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping[iMapping].SetProperty(iProp, prop);
}

void CSafeMapping::GetSelection(const CIntArrayEx& arrSelection, CMappingArray& arrMapping) const
{
	m_arrMapping.GetSelection(arrSelection, arrMapping);
}

void CSafeMapping::SetInputMidiMsg(int iMapping, DWORD nInMidiMsg)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping[iMapping].SetInputMidiMsg(nInMidiMsg);
}

void CSafeMapping::GetProperty(const CIntArrayEx& arrSelection, int iProp, CVariantPropArray& arrProp) const
{
	int	nSels = arrSelection.GetSize();
	arrProp.SetSize(nSels);
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		arrProp[iSel] = m_arrMapping[iMapping].GetProperty(iProp);
	}
}

void CSafeMapping::SetProperty(const CIntArrayEx& arrSelection, int iProp, const CVariantPropArray& arrProp)
{
	LOCK_MAPPINGS; // exclusive write
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		m_arrMapping[iMapping].SetProperty(iProp, arrProp[iSel]);
	}
}

void CSafeMapping::SetProperty(const CIntArrayEx& arrSelection, int iProp, VARIANT_PROP prop)
{
	LOCK_MAPPINGS; // exclusive write
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		m_arrMapping[iMapping].SetProperty(iProp, prop);
	}
}

void CSafeMapping::Insert(int iInsert, CMappingArray& arrMapping)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.InsertAt(iInsert, &arrMapping);
}

void CSafeMapping::Insert(const CIntArrayEx& arrSelection, CMappingArray& arrMapping)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.InsertSelection(arrSelection, arrMapping);
}

void CSafeMapping::Delete(int iMapping, int nCount)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.RemoveAt(iMapping, nCount);
}

void CSafeMapping::Delete(const CIntArrayEx& arrSelection)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.DeleteSelection(arrSelection);
}

void CSafeMapping::Move(const CIntArrayEx& arrSelection, int iDropPos)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.MoveSelection(arrSelection, iDropPos);
}

void CSafeMapping::Sort(int iProp, bool bDescending)
{
	LOCK_MAPPINGS; // exclusive write
	m_iSortProp = iProp;
	m_bSortDescending = bDescending;
	qsort(m_arrMapping.GetData(), m_arrMapping.GetSize(), sizeof(CMapping), SortCompare);
}

int CSafeMapping::m_iSortProp;	// index of property to sort mappings by
bool CSafeMapping::m_bSortDescending;	// true if sort should be descending

int CSafeMapping::SortCompare(const void *arg1, const void *arg2)
{
	const CMapping*	pMap1 = (CMapping *)arg1;
	const CMapping*	pMap2 = (CMapping *)arg2;
	return pMap1->Compare(m_iSortProp, *pMap2);
}

void CSafeMapping::GetInputMidiMsg(const CIntArrayEx& arrSelection, CIntArrayEx& arrInMidiMsg) const
{
	int	nSels = arrSelection.GetSize();
	arrInMidiMsg.SetSize(nSels);
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		arrInMidiMsg[iSel] = m_arrMapping[iMapping].GetInputMidiMsg();
	}
}

void CSafeMapping::SetInputMidiMsg(const CIntArrayEx& arrSelection, DWORD nInMidiMsg)
{
	LOCK_MAPPINGS; // exclusive write
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		m_arrMapping[iMapping].SetInputMidiMsg(nInMidiMsg);	// set input MIDI message
	}
}

void CSafeMapping::SetInputMidiMsg(const CIntArrayEx& arrSelection, const CIntArrayEx& arrInMidiMsg)
{
	LOCK_MAPPINGS; // exclusive write
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {	// for each selected mapping
		int	iMapping = arrSelection[iSel];
		m_arrMapping[iMapping].SetInputMidiMsg(arrInMidiMsg[iSel]);
	}
}

void CSafeMapping::Read(CIniFile& fIn)
{
	LOCK_MAPPINGS; // exclusive write
	m_arrMapping.Read(fIn);
}

void CSafeMapping::Write(CIniFile& fOut) const
{
	m_arrMapping.Write(fOut);
}
