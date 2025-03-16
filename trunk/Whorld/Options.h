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
        04      09mar25	add export scale to fit
		05      11mar25	add legacy snapshot sizes
		
*/

#pragma once

#include "Properties.h"

class COptions : public CProperties {
public:
// Construction
	COptions();

// Types

// Constants
	enum {	// groups
		#define GROUPDEF(name) GROUP_##name,
		#include "OptionsDef.h"
		GROUPS
	};
	enum {	// properties
		#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) PROP_##group##_##name,
		#include "OptionsDef.h"
		PROPERTIES
	};
	enum {	// scale to fit types
		#define SCALETOFITDEF(x) SCALE_TO_FIT_##x,
		#include "OptionsDef.h"
		SCALE_TO_FIT_TYPES
	};
	enum {	// legacy sizes
		#define LEGACYSIZEDEF(width, height, name) LEGACY_SIZE_##name,
		#include "OptionsDef.h"
		LEGACY_SIZES
	};
	static const OPTION_INFO	m_oiGroup[GROUPS];	// group names
	static const PROPERTY_INFO	m_Info[PROPERTIES];	// fixed info for each property
	static const OPTION_INFO	m_oiScaleToFit[SCALE_TO_FIT_TYPES];	// scale to fit options
	static const OPTION_INFO	m_oiLegacySize[LEGACY_SIZES];	// legacy size options
	static const SIZE	m_aLegacySize[LEGACY_SIZES];	// legacy sizes

// Attributes
	CSize	GetExportImageSize() const;
	UINT	GetExportFlags() const;
	void	SetDefaultExportImageFolder();
	CSize	GetLegacySnapshotSize() const;

// Operations
	void	ReadProperties();
	void	WriteProperties() const;
	void	UpdateMidiDevices();

// Public data
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) type m_##group##_##name;
	#include "OptionsDef.h"

// Overrides
	virtual	int		GetGroupCount() const;
	virtual	int		GetPropertyCount() const;
	virtual	const PROPERTY_INFO&	GetPropertyInfo(int iProp) const;
	virtual	void	GetVariants(CVariantArray& Var) const;
	virtual	void	SetVariants(const CVariantArray& Var);
	virtual	CString	GetGroupName(int iGroup) const;
	virtual	int		GetOptionCount(int iProp) const;
	virtual	CString	GetOptionName(int iProp, int iOption) const;
	virtual	void	GetProperty(int iProp, CComVariant& var) const;
	virtual	void	SetProperty(int iProp, const CComVariant& var);
};

inline CSize COptions::GetExportImageSize() const
{
	return CSize(m_Export_nImageWidth, m_Export_nImageHeight);
}

inline CSize COptions::GetLegacySnapshotSize() const
{
	ASSERT(m_Snapshot_iLegacySize >= 0 && m_Snapshot_iLegacySize < LEGACY_SIZES);
	return CSize(m_aLegacySize[m_Snapshot_iLegacySize]);
}
