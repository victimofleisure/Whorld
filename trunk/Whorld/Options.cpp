// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27mar18	initial version
		01		20feb19	rename option info vars
		02		21feb25	customize for Whorld
		03		26feb25	add MIDI input
		
*/

#include "stdafx.h"
#include "Whorld.h"
#include "Options.h"
#include "RegTempl.h"
#include "VariantHelper.h"

const COptions::OPTION_INFO COptions::m_oiGroup[GROUPS] = {
	#define GROUPDEF(name) {_T(#name), IDS_OPT_GROUP_##name},
	#include "OptionsDef.h"
};

const COptions::PROPERTY_INFO COptions::m_Info[PROPERTIES] = {
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		{_T(#group) _T("_") _T(#name), IDS_OPT_NAME_##group##_##name, IDS_OPT_DESC_##group##_##name, \
		offsetof(COptions, m_##group##_##name), sizeof(type), &typeid(type), GROUP_##group, -1, \
		PT_##proptype, items, itemname, minval, maxval},
	#include "OptionsDef.h"
};

COptions::COptions()
{
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		m_##group##_##name = initval;
	#include "OptionsDef.h"
	SetDefaultExportImageFolder();
}

void COptions::SetDefaultExportImageFolder()
{
	theApp.GetSpecialFolderPath(CSIDL_MYPICTURES, m_Export_sImageFolder);
}

int COptions::GetGroupCount() const
{
	return GROUPS;
}

int COptions::GetPropertyCount() const
{
	return PROPERTIES;
}

const COptions::PROPERTY_INFO& COptions::GetPropertyInfo(int iProp) const
{
	ASSERT(IsValidProperty(iProp));
	return m_Info[iProp];
}

void COptions::GetVariants(CVariantArray& Var) const
{
	Var.SetSize(PROPERTIES);
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		Var[PROP_##group##_##name] = CComVariant(m_##group##_##name);
	#include "OptionsDef.h"
}

void COptions::SetVariants(const CVariantArray& Var)
{
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		GetVariant(Var[PROP_##group##_##name], m_##group##_##name);
	#include "OptionsDef.h"
}

CString COptions::GetGroupName(int iGroup) const
{
	ASSERT(IsValidGroup(iGroup));
	return LDS(m_oiGroup[iGroup].nNameID);
}

int COptions::GetOptionCount(int iProp) const
{
	switch (iProp) {
	case PROP_Midi_iInputDevice:
		return theApp.m_midiDevs.GetCount(CMidiDevices::INPUT) + 1;	// add one for none option
	default:
		return CProperties::GetOptionCount(iProp);
	}
}

CString	COptions::GetOptionName(int iProp, int iOption) const
{
	switch (iProp) {
	case PROP_Midi_iInputDevice:
		{
			CString	sName(theApp.m_midiDevs.GetName(CMidiDevices::INPUT, iOption - 1));	// convert to zero-origin
			if (sName.IsEmpty())
				sName.LoadString(IDS_NONE);
			return sName;
		}
		break;
	default:
		return CProperties::GetOptionName(iProp, iOption);
	}
}

void COptions::GetProperty(int iProp, CComVariant& var) const
{
	switch (iProp) {
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
	case PROP_##group##_##name: var = CComVariant(m_##group##_##name); break;
	#include "OptionsDef.h"	// generate code to get members from variant
	default:
		NODEFAULTCASE;
	}
}

void COptions::SetProperty(int iProp, const CComVariant& var)
{
	switch (iProp) {
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		case PROP_##group##_##name: GetVariant(var, m_##group##_##name); break;
	#include "OptionsDef.h"	// generate code to set members from variant
	default:
		NODEFAULTCASE;
	}
}

void COptions::ReadProperties()
{
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		RdReg(_T("Options\\") _T(#group), _T(#name), m_##group##_##name);
	#define OPTS_EXCLUDE_MIDI_DEVICES	// MIDI devices are handled in app
	#include "OptionsDef.h"	// generate code to read properties
}

void COptions::WriteProperties() const
{
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		WrReg(_T("Options\\") _T(#group), _T(#name), m_##group##_##name);
	#define OPTS_EXCLUDE_MIDI_DEVICES	//  MIDI devices are handled in app
	#include "OptionsDef.h"	// generate code to read properties
}

UINT COptions::GetExportFlags() const
{
	UINT	nFlags = 0;
	if (m_Export_bUseViewSize) {
		nFlags |= CWhorldBase::EF_USE_VIEW_SIZE;
	}
	if (m_Export_bScaleToFit) {
		nFlags |= CWhorldBase::EF_SCALE_TO_FIT;
	}
	return nFlags;
}

void COptions::UpdateMidiDevices()
{
	m_Midi_iInputDevice = theApp.m_midiDevs.GetIdx(CMidiDevices::INPUT) + 1;	// add one for none option
}
