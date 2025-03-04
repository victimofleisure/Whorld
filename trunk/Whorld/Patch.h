// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17apr06	initial version
		01		18apr06	add HueLoopLength and CanvasScale
		02		18apr06	bump file version to 2
		03		21jun06	add tag to MasterDef macro
		04		28jan08	support Unicode
		05		09feb25	refactor
		06		02mar25	implement global parameters

		patch container
 
*/

#pragma once

#include "WhorldBase.h"

// DPoint to string conversion; needed for Origin in main properties
inline STD_OSTREAM& operator<<(STD_OSTREAM& os, const DPoint& pt)
{
    os << _T("(") << pt.x << _T(",") << pt.y << _T(")");
    return os;
}

class CPatch : public CWhorldBase {
public:
// Construction
	CPatch() {}	// use SetDefaults to initialize this object
	~CPatch() {}

// Public data
	PARAM_TABLE		m_aParam;	// table of parameter rows
	MASTER_PROPS	m_master;	// master properties
	MAIN_PROPS		m_main;		// main properties

// Attributes
	void	SetDefaults();
	const PARAM_ROW&	GetParamRow(int iParam) const;
	PARAM_ROW&	GetParamRow(int iParam);
	void	GetParam(int iParam, int iProp, CComVariant& prop) const;
	void	SetParam(int iParam, int iProp, const CComVariant& prop);
	void	GetParamRC(int iParam, int iProp, VARIANT_PROP& prop) const;
	void	SetParamRC(int iParam, int iProp, const VARIANT_PROP& prop);
	double	GetMasterProp(int iProp) const;
	void	SetMasterProp(int iProp, double fVal);
	void	GetMainProp(int iProp, VARIANT_PROP& prop) const;
	void	SetMainProp(int iProp, const VARIANT_PROP& prop);
	double	GetGlobalParam(int iParam) const;
	void	SetGlobalParam(int iParam, double fVal);

// Operations
	static CString	ParamToString(int iProp, const VARIANT_PROP& prop);
	static CString	MasterToString(int iProp, const VARIANT_PROP& prop);
	static CString	MainToString(int iProp, const VARIANT_PROP& prop);
	bool	Read(LPCTSTR pszPath);
	bool	Write(LPCTSTR pszPath) const;

protected:
// Constants
	enum {
		FILE_VERSION = 3	// file format version number
	};
	static const LPCTSTR FILE_ID;		// file format signature

// Helpers
	bool	ParseLine(CString sLine);
};

inline const CPatch::PARAM_ROW& CPatch::GetParamRow(int iParam) const
{
	ASSERT(IsValidParamIdx(iParam));
	return m_aParam.row[iParam];
}

inline CPatch::PARAM_ROW& CPatch::GetParamRow(int iParam)
{
	ASSERT(IsValidParamIdx(iParam));
	return m_aParam.row[iParam];
}

inline double CPatch::GetMasterProp(int iProp) const
{
	ASSERT(IsValidMasterProp(iProp));
	return m_master.a[iProp];
}

inline void CPatch::SetMasterProp(int iProp, double fVal)
{
	ASSERT(IsValidMasterProp(iProp));
	m_master.a[iProp] = fVal;
}

inline double CPatch::GetGlobalParam(int iParam) const
{
	ASSERT(IsValidParamIdx(iParam));
	return m_aParam.row[iParam].fGlobal;
}

inline void CPatch::SetGlobalParam(int iParam, double fVal)
{
	ASSERT(IsValidParamIdx(iParam));
	m_aParam.row[iParam].fGlobal = fVal;
}
