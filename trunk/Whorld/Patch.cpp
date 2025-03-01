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
		02		21jun06	add tag to MasterDef macro
		03		24jun06	add Copies and Spread
		04		28jan08	support Unicode
		05		09feb25	refactor
		06		01mar25	implement global parameters

		patch container
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "Patch.h"
#include "FormatIO.h"

const LPCTSTR CPatch::FILE_ID = _T("WHORLD%d");	// placeholder for version number

void CPatch::SetDefaults()
{
	SetParamDefaults(m_aParam);
	m_master = m_masterDefault;
	m_main = m_mainDefault;
}

void CPatch::GetParam(int iParam, int iProp, CComVariant& prop) const
{
	// assign parameter row property to caller's variant
	ASSERT(IsValidParamIdx(iParam));	// check parameter index range
	ASSERT(IsValidParamProp(iProp));	// check parameter property index range
	const PARAM_ROW&	row = m_aParam.row[iParam];	// reference specified parameter row
	switch (iProp) {
	#define PARAMPROPDEF(name, type, prefix, variant) \
	case PARAM_PROP_##name: prop.variant = row.prefix##name; break;
	#include "WhorldDef.h"	// generate cases for each property type
	}
}

void CPatch::SetParam(int iParam, int iProp, const CComVariant& prop)
{
	// assign caller's variant to parameter row property
	ASSERT(IsValidParamIdx(iParam));	// check parameter index range
	ASSERT(IsValidParamProp(iProp));	// check parameter property index range
	PARAM_ROW&	row = m_aParam.row[iParam];	// reference specified parameter row
	switch (iProp) {
	#define PARAMPROPDEF(name, type, prefix, variant) \
	case PARAM_PROP_##name: row.prefix##name = prop.variant; break;
	#include "WhorldDef.h"	// generate cases for each property type
	}
}

void CPatch::GetParamRC(int iParam, int iProp, VARIANT_PROP& prop) const
{
	// assign parameter row property to caller's variant
	ASSERT(IsValidParamIdx(iParam));	// check parameter index range
	ASSERT(IsValidParamProp(iProp));	// check parameter property index range
	const PARAM_ROW&	row = m_aParam.row[iParam];	// reference specified parameter row
	switch (iProp) {
	#define PARAMPROPDEF(name, type, prefix, variant) \
	case PARAM_PROP_##name: prop.##variant = row.prefix##name; break;
	#include "WhorldDef.h"	// generate cases for each property type
	}
}

void CPatch::SetParamRC(int iParam, int iProp, const VARIANT_PROP& prop)
{
	// assign parameter row property to caller's variant
	ASSERT(IsValidParamIdx(iParam));	// check parameter index range
	ASSERT(IsValidParamProp(iProp));	// check parameter property index range
	PARAM_ROW&	row = m_aParam.row[iParam];	// reference specified parameter row
	switch (iProp) {
	#define PARAMPROPDEF(name, type, prefix, variant) \
	case PARAM_PROP_##name: row.prefix##name = prop.##variant; break;
	#include "WhorldDef.h"	// generate cases for each property type
	}
}

void CPatch::GetMainProp(int iProp, VARIANT_PROP& prop) const
{
	ASSERT(IsValidMainProp(iProp));
	switch (iProp) {
	#define MAINDEF(name, type, prefix, initval, variant) \
	case MAIN_##name: prop.variant = m_main.##prefix##name; break;
	#include "WhorldDef.h"	// generate cases for each property type
	}
}

void CPatch::SetMainProp(int iProp, const VARIANT_PROP& prop)
{
	ASSERT(IsValidMainProp(iProp));
	switch (iProp) {
	#define MAINDEF(name, type, prefix, initval, variant) \
	case MAIN_##name: m_main.##prefix##name = prop.variant; break;
	#include "WhorldDef.h"	// generate cases for each property type
	}
}

CString	CPatch::ParamToString(int iProp, const VARIANT_PROP& prop)
{
	ASSERT(IsValidParamProp(iProp));	// check parameter property index range
	CString	sOut;
	switch (iProp) {
	#define PARAMPROPDEF(name, type, prefix, variant) \
	case PARAM_PROP_##name: sOut = ValToString(prop.variant); break;
	#include "WhorldDef.h"	// generate cases for each property type
	}
	return sOut;
}

CString	CPatch::MasterToString(int iProp, const VARIANT_PROP& prop)
{
	UNREFERENCED_PARAMETER(iProp);
	return ValToString(prop.dblVal);
}

CString	CPatch::MainToString(int iProp, const VARIANT_PROP& prop)
{
	ASSERT(IsValidMainProp(iProp));	// check parameter property index range
	CString	sOut;
	switch (iProp) {
	#define MAINDEF(name, type, prefix, initval, variant) \
	case MAIN_##name: sOut = ValToString(prop.variant); break;
	#include "WhorldDef.h"	// generate cases for each property type
	}
	return sOut;
}

bool CPatch::ParseLine(CString sLine)
{
	sLine.TrimLeft();
	CString	sName = sLine.SpanExcluding(_T(" \t"));
	CString	sArg = sLine.Mid(sName.GetLength());
	int	iParam = FindParamByName(sName);
	if (iParam >= 0) {	// if parameter found
		PARAM_ROW&	row = m_aParam.row[iParam];
		_stscanf_s(sArg, _T("%lf %d %lf %lf %lf %lf"),
			&row.fVal, &row.iWave, &row.fAmp, &row.fFreq, &row.fPW, &row.fGlobal);
		return true;
	}
	int	iMaster = FindMasterByName(sName);
	if (iMaster >= 0) {	// if master property found
		_stscanf_s(sArg, _T("%lf"), &m_master.a[iMaster]);
		return true;
	}
	int	iMain = FindMainByName(sName);
	if (iMain >= 0) {	// if main property found
		const MAIN_INFO& info = GetMainInfo(iMain);
		CFormatIO::StrToVal(info.nFIOType, sArg, ((BYTE *)&m_main) + info.nOffset);
		return true;
	}
	return false;	// unknown symbol; parse failed
}

bool CPatch::Read(LPCTSTR pszPath)
{
	CStdioFile	fIn(pszPath, CFile::modeRead);	// may throw
	CString	sLine;
	int	nVersion;
	if (!fIn.ReadString(sLine) || _stscanf_s(sLine, FILE_ID, &nVersion) != 1) {
		AfxMessageBox(IDS_DOC_BAD_FORMAT);
		return(false);
	}
	SetDefaults();	// in case some lines are missing
	while (fIn.ReadString(sLine)) {
		ParseLine(sLine);
	}
	return(true);
}

bool CPatch::Write(LPCTSTR pszPath) const
{
	CStdioFile	fOut(pszPath, CFile::modeCreate | CFile::modeWrite);	// may throw
	CString	sLine;
	sLine.Format(FILE_ID, FILE_VERSION);
	fOut.WriteString(sLine + "\n");
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter
		const PARAM_ROW& row = m_aParam.row[iParam];
		const PARAM_INFO& info = GetParamInfo(iParam); 
		sLine.Format(_T("%s\t%g\t%d\t%g\t%g\t%g\t%g\n"), info.pszName,
			row.fVal, row.iWave, row.fAmp, row.fFreq, row.fPW, row.fGlobal);
		fOut.WriteString(sLine);
	}
	for (int iMaster = 0; iMaster < MASTER_COUNT; iMaster++) {	// for each master property
		const MASTER_INFO& info = GetMasterInfo(iMaster);
		sLine.Format(_T("%s\t%g\n"), info.pszName, m_master.a[iMaster]);
		fOut.WriteString(sLine);
	}
	for (int iMain = 0; iMain < MAIN_COUNT; iMain++) {	// for each main property
		const MAIN_INFO& info = GetMainInfo(iMain);
		CFormatIO::ValToStr(info.nFIOType, ((BYTE *)&m_main) + info.nOffset, sLine);
		fOut.WriteString(CString(info.pszName) + '\t' + sLine + '\n');
	}
	return(true);
}
