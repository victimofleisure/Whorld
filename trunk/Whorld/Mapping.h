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

*/

#pragma once

#include "WhorldBase.h"
#include "CritSec.h"
#include "Midi.h"

class CIniFile;

class CMapping : public CWhorldBase {
public:
// Public data
	int		m_nInEvent;			// input event type
	int		m_nOutEvent;		// output event type
	int		m_nInChannel;		// input MIDI channel
	int		m_nInControl;		// input controller number
	int		m_nRangeStart;		// range start
	int		m_nRangeEnd;		// range end

// Constants
	enum {
		#define MAPPINGDEF(name, align, width, member, minval, maxval) PROP_##name,
		#include "MappingDef.h"	// generate member var enumeration
		PROPERTIES
	};
	enum {	// input events
		#define MIDICHANSTATDEF(name) IN_##name,
		#include "MidiCtrlrDef.h"	// enumerate MIDI channel voice messages
		INPUT_EVENTS
	};
	enum {
		#define MASTERDEF(name, type, prefix, initval) OUT_##name,
		#include "WhorldDef.h"	// generate enumeration
		#define PARAMDEF(name, minval, maxval, steps, scale, initval) OUT_##name,
		#include "WhorldDef.h"	// generate enumeration
		OUTPUT_EVENTS
	};

// Attributes
	int		GetProperty(int iProp) const;
	void	SetProperty(int iProp, int nVal);
	DWORD	GetInputMidiMsg() const;
	void	SetInputMidiMsg(DWORD nInMidiMsg);
	int		IsInputMatch(DWORD nInMidiMsg) const;
	static CString	GetInputEventName(int nInEvent);
	static CString	GetOutputEventName(int nOutEvent);
	static LPCTSTR	GetInputEventTag(int nInEvent);
	static LPCTSTR	GetOutputEventTag(int nOutEvent);
	static int		FindInputEventTag(LPCTSTR pszName);
	static int		FindOutputEventTag(LPCTSTR pszName);

// Operations
	static void	Initialize();
	void	SetDefaults();
	void	Read(CIniFile& fIni, LPCTSTR pszSection);
	void	Write(CIniFile& fIni, LPCTSTR pszSection) const;

protected:
// Constants
	static const LPCTSTR m_arrChanStatTag[MIDI_CHANNEL_VOICE_MESSAGES];
	static const LPCTSTR m_arrSysStatTag[MIDI_SYSTEM_STATUS_MESSAGES];
	static const int	m_arrChanStatID[MIDI_CHANNEL_VOICE_MESSAGES];	// channel status message string resource IDs
	static const int	m_arrSysStatID[MIDI_SYSTEM_STATUS_MESSAGES];	// system status message string resource IDs
	static CStringArrayEx	m_arrChanStatName;	// array of channel status message name strings
	static CStringArrayEx	m_arrSysStatName;	// array of system status message name strings
};

inline CString CMapping::GetInputEventName(int nInEvent)
{
	return m_arrChanStatName[nInEvent];
}

inline LPCTSTR CMapping::GetInputEventTag(int nInEvent)
{
	return m_arrChanStatTag[nInEvent];
}

inline int CMapping::FindInputEventTag(LPCTSTR pszName)
{
	return ARRAY_FIND(m_arrChanStatTag, pszName);
}

class CMappingArray : public CArrayEx<CMapping, CMapping&> {
public:
	void	Read(CIniFile& fIni);
	void	Write(CIniFile& fIni) const;
};

class CSeqMapping {
public:
// Attributes
	WCritSec&	GetCritSec();
	int		GetCount() const;
	const CMapping&	GetAt(int iMapping) const;
	int		GetProperty(int iMapping, int iProp) const;
	void	SetProperty(int iMapping, int iProp, int nVal);
	void	GetProperty(const CIntArrayEx& arrSelection, int iProp, CIntArrayEx& arrProp) const;
	void	SetProperty(const CIntArrayEx& arrSelection, int iProp, const CIntArrayEx& arrProp);
	void	SetProperty(const CIntArrayEx& arrSelection, int iProp, int nVal);
	void	GetRange(int iFirst, int nMappings, CMappingArray& arrMapping) const;
	void	GetSelection(const CIntArrayEx& arrSelection, CMappingArray& arrMapping) const;
	void	SetInputMidiMsg(int iMapping, DWORD nInMidiMsg);
	void	GetInputMidiMsg(const CIntArrayEx& arrSelection, CIntArrayEx& arrInMidiMsg) const;
	void	SetInputMidiMsg(const CIntArrayEx& arrSelection, DWORD nInMidiMsg);
	void	SetInputMidiMsg(const CIntArrayEx& arrSelection, const CIntArrayEx& arrInMidiMsg);

// Operations
	void	Insert(int iInsert, CMappingArray& arrMapping);
	void	Insert(const CIntArrayEx& arrSelection, CMappingArray& arrMapping);
	void	Delete(int iMapping, int nCount = 1);
	void	Delete(const CIntArrayEx& arrSelection);
	void	Move(const CIntArrayEx& arrSelection, int iDropPos);
	void	Sort(int iProp, bool bDescending = false);
	static	int		SortCompare(const void *arg1, const void *arg2);
	void	Read(CIniFile& fIni);
	void	Write(CIniFile& fIni) const;

protected:
// Data members
	CMappingArray	m_arrMapping;	// array of mappings
	WCritSec	m_csMapping;		// critical section for serializing access to mappings
	static int	m_iSortProp;		// index of property to sort mappings by
	static bool	m_bSortDescending;	// true if sort should be descending
};

inline WCritSec& CSeqMapping::GetCritSec()
{
	return m_csMapping;
}

inline int CSeqMapping::GetCount() const
{
	return m_arrMapping.GetSize();
}

inline const CMapping& CSeqMapping::GetAt(int iMapping) const
{
	return m_arrMapping[iMapping];
}

inline int CSeqMapping::GetProperty(int iMapping, int iProp) const
{
	return m_arrMapping[iMapping].GetProperty(iProp);
}

inline void CSeqMapping::GetRange(int iFirstMapping, int nMappings, CMappingArray& arrMapping) const
{
	m_arrMapping.GetRange(iFirstMapping, nMappings, arrMapping);
}
