// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20mar20	initial version
		01		29mar20	add get/set input message for selected mappings
		02		05apr20	add track step mapping
		03		07sep20	add preset and part mapping
		04		13feb21	add string ID to special target macro
		05		25oct21	add optional sort direction
		06		19feb22	use INI file class directly instead of via profile
		07		26feb25	adapt for Whorld
		08		01mar25	add misc targets
		09		19mar25	make mapping range real instead of integer
		10		25mar25	add target types

*/

#pragma once

#include "WhorldBase.h"
#include "CritSec.h"
#include "Midi.h"

class CIniFile;

class CMappingBase : public CWhorldBase {
public:
// Constants
	enum {	// properties, corresponding to mapping columns
		#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) PROP_##name,
		#include "MappingDef.h"	// generate member var enumeration
		PROPERTIES
	};
	enum {	// events, equivalent to MIDI channel voice messages
		#define MIDICHANSTATDEF(name) EVENT_##name,
		#include "MidiCtrlrDef.h"	// generate enumeration
		EVENTS
	};
	enum {	// mapping targets, in a series of ranges
		#define PARAMDEF(name, minval, maxval, steps, scale, initval) TARGET_##name,
		#include "WhorldDef.h"	// generate parameter range
		#define MASTERDEF(name, type, prefix, initval) TARGET_##name,
		#include "WhorldDef.h"	// generate master property range
		#define MAPPINGDEF_MISC_TARGET(name, type) TARGET_##name,
		#include "MappingDef.h"	// generate miscellaneous target range
		TARGETS
	};
	enum {	// miscellaneous targets
		#define MAPPINGDEF_MISC_TARGET(name, type) MT_##name,
		#include "MappingDef.h"	// generate enumeration
		MISC_TARGETS
	};
	enum {	// undo codes
		#define MAPPINGUNDODEF(name) UCODE_##name,
		#include "MappingDef.h"	// generate enum
		MAPPING_UNDO_CODES
	};
	enum {	// target types
		TT_RANGE,	// continuous or multiple-choice property
		TT_BOOL,	// on/off or true/false binary property
		TT_ACTION,	// triggered or one-shot command (bang)
		TARGET_TYPES
	};

// Attributes
	static bool		IsValidEvent(int iEvent);
	static bool		IsValidTarget(int iTarget);
	static bool		TargetHasProperty(int iTarget);
	static LPCTSTR	GetChannelStatusTag(int iChanStat);
	static CString	GetChannelStatusName(int iChanStat);
	static LPCTSTR	GetSystemStatusTag(int iSysStat);
	static CString	GetSystemStatusName(int iSysStat);
	static LPCTSTR	GetEventTag(int iEvent);
	static CString	GetEventName(int iEvent);
	static LPCTSTR	GetTargetTag(int iTarget);
	static CString	GetTargetName(int iTarget);
	static int		GetTargetType(int iTarget);
	static int		FindEventTag(LPCTSTR pszName);
	static int		FindTargetTag(LPCTSTR pszName);
	static bool		IsIntegerProperty(int iProp);

// Operations
	static void	Initialize();

protected:
// Constants
	static const LPCTSTR m_arrChanStatTag[MIDI_CHANNEL_VOICE_MESSAGES];
	static const LPCTSTR m_arrSysStatTag[MIDI_SYSTEM_STATUS_MESSAGES];
	static const int m_arrChanStatID[MIDI_CHANNEL_VOICE_MESSAGES];
	static const int m_arrSysStatID[MIDI_SYSTEM_STATUS_MESSAGES];
	static CString m_arrChanStatName[MIDI_CHANNEL_VOICE_MESSAGES];
	static CString m_arrSysStatName[MIDI_SYSTEM_STATUS_MESSAGES];
	static LPCTSTR m_arrTargetTag[TARGETS];
	static CString m_arrTargetName[TARGETS];
	static const LPCTSTR m_arrMiscTargetTag[MISC_TARGETS];
	static const int m_arrMiscTargetID[MISC_TARGETS];
	static const BYTE m_arrMiscTargetType[MISC_TARGETS];
	static BYTE m_arrTargetType[TARGETS];
};

inline bool CMappingBase::IsValidEvent(int iEvent)
{
	return iEvent >= 0 && iEvent < EVENTS;
}

inline bool CMappingBase::IsValidTarget(int iTarget)
{
	return iTarget >= 0 && iTarget < TARGETS;
}

inline bool CMappingBase::TargetHasProperty(int iTarget)
{
	// assume parameters are first in target enumeration
	return iTarget < PARAM_COUNT;
}

inline LPCTSTR CMappingBase::GetChannelStatusTag(int iChanStat)
{
	ASSERT(iChanStat >= 0 && iChanStat < MIDI_CHANNEL_VOICE_MESSAGES);
	return m_arrChanStatTag[iChanStat];
}

inline CString CMappingBase::GetChannelStatusName(int iChanStat)
{
	ASSERT(iChanStat >= 0 && iChanStat < MIDI_CHANNEL_VOICE_MESSAGES);
	return m_arrChanStatName[iChanStat];
}

inline LPCTSTR CMappingBase::GetSystemStatusTag(int iSysStat)
{
	ASSERT(iSysStat >= 0 && iSysStat < MIDI_SYSTEM_STATUS_MESSAGES);
	return m_arrSysStatTag[iSysStat];
}

inline CString CMappingBase::GetSystemStatusName(int iSysStat)
{
	ASSERT(iSysStat >= 0 && iSysStat < MIDI_SYSTEM_STATUS_MESSAGES);
	return m_arrSysStatName[iSysStat];
}

inline LPCTSTR CMappingBase::GetEventTag(int iEvent)
{
	return GetChannelStatusTag(iEvent);
}

inline CString CMappingBase::GetEventName(int iEvent)
{
	return GetChannelStatusName(iEvent);
}

inline int CMappingBase::FindEventTag(LPCTSTR pszName)
{
	return ARRAY_FIND(m_arrChanStatTag, pszName);
}

inline LPCTSTR CMappingBase::GetTargetTag(int iTarget)
{
	ASSERT(IsValidTarget(iTarget));
	return m_arrTargetTag[iTarget];
}

inline CString CMappingBase::GetTargetName(int iTarget)
{
	ASSERT(IsValidTarget(iTarget));
	return m_arrTargetName[iTarget];
}

inline int CMappingBase::GetTargetType(int iTarget)
{
	ASSERT(IsValidTarget(iTarget));
	return m_arrTargetType[iTarget];
}

inline int CMappingBase::FindTargetTag(LPCTSTR pszName)
{
	return ARRAY_FIND(m_arrTargetTag, pszName);
}

inline bool CMappingBase::IsIntegerProperty(int iProp)
{
	switch (iProp) {
	case PROP_START:	// list non-integer properties here
	case PROP_END:
		return false;
	default:
		return true;
	}
}

class CMapping : public CMappingBase {
public:
// Public data
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		type m_##prefix##member;
	#include "MappingDef.h"	// generate member var definitions

// Attributes
	VARIANT_PROP	GetProperty(int iProp) const;
	void	SetProperty(int iProp, VARIANT_PROP prop);
	DWORD	GetInputMidiMsg() const;
	void	SetInputMidiMsg(DWORD nInMidiMsg);
	int		IsInputMatch(DWORD nInMidiMsg) const;
	bool	TargetHasProperty() const;

// Operations
	void	SetDefaults();
	void	Read(CIniFile& fIn, LPCTSTR pszSection);
	void	Write(CIniFile& fOut, LPCTSTR pszSection) const;
	int		Compare(int iProp, const CMapping& mapping) const;
	static int	Compare(int iProp, VARIANT_PROP prop1, VARIANT_PROP prop2);
};

inline bool CMapping::TargetHasProperty() const
{
	return CMappingBase::TargetHasProperty(m_iTarget);
}

inline int CMapping::Compare(int iProp, const CMapping& mapping) const
{
	return Compare(iProp, GetProperty(iProp), mapping.GetProperty(iProp));
}

class CMappingArray : public CArrayEx<CMapping, CMapping&> {
public:
	void	Read(CIniFile& fIn);
	void	Write(CIniFile& fOut) const;
};

class CSafeMapping : public CWhorldBase {
public:
// Attributes
	WCritSec&	GetCritSec();
	int		GetCount() const;
	const CMapping&	GetAt(int iMapping) const;
	void	SetAt(int iMapping, const CMapping& mapping);
	const CMappingArray&	GetArray() const;
	void	SetArray(const CMappingArray& arrMapping);
	VARIANT_PROP	GetProperty(int iMapping, int iProp) const;
	void	SetProperty(int iMapping, int iProp, VARIANT_PROP prop);
	void	GetProperty(const CIntArrayEx& arrSelection, int iProp, CVariantPropArray& arrProp) const;
	void	SetProperty(const CIntArrayEx& arrSelection, int iProp, const CVariantPropArray& arrProp);
	void	SetProperty(const CIntArrayEx& arrSelection, int iProp, VARIANT_PROP prop);
	void	GetRange(int iFirst, int nMappings, CMappingArray& arrMapping) const;
	void	GetSelection(const CIntArrayEx& arrSelection, CMappingArray& arrMapping) const;
	void	SetInputMidiMsg(int iMapping, DWORD nInMidiMsg);
	void	GetInputMidiMsg(const CIntArrayEx& arrSelection, CIntArrayEx& arrInMidiMsg) const;
	void	SetInputMidiMsg(const CIntArrayEx& arrSelection, DWORD nInMidiMsg);
	void	SetInputMidiMsg(const CIntArrayEx& arrSelection, const CIntArrayEx& arrInMidiMsg);

// Operations
	void	RemoveAll();
	void	Insert(int iInsert, CMappingArray& arrMapping);
	void	Insert(const CIntArrayEx& arrSelection, CMappingArray& arrMapping);
	void	Delete(int iMapping, int nCount = 1);
	void	Delete(const CIntArrayEx& arrSelection);
	void	Move(const CIntArrayEx& arrSelection, int iDropPos);
	void	Sort(int iProp, bool bDescending = false);
	static	int		SortCompare(const void *arg1, const void *arg2);
	void	Read(CIniFile& fIn);
	void	Write(CIniFile& fOut) const;

protected:
// Data members
	CMappingArray	m_arrMapping;	// array of mappings
	WCritSec	m_csMapping;		// critical section for serializing access to mappings
	static int	m_iSortProp;		// index of property to sort mappings by
	static bool	m_bSortDescending;	// true if sort should be descending
};

inline WCritSec& CSafeMapping::GetCritSec()
{
	return m_csMapping;
}

inline int CSafeMapping::GetCount() const
{
	return m_arrMapping.GetSize();
}

inline const CMapping& CSafeMapping::GetAt(int iMapping) const
{
	return m_arrMapping[iMapping];
}

inline const CMappingArray& CSafeMapping::GetArray() const
{
	return m_arrMapping;
}

inline CSafeMapping::VARIANT_PROP CSafeMapping::GetProperty(int iMapping, int iProp) const
{
	return m_arrMapping[iMapping].GetProperty(iProp);
}

inline void CSafeMapping::GetRange(int iFirstMapping, int nMappings, CMappingArray& arrMapping) const
{
	m_arrMapping.GetRange(iFirstMapping, nMappings, arrMapping);
}
