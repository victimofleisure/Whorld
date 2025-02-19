// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version

*/

#include "stdafx.h"
#include "Resource.h"
#include "WhorldBase.h"
#include "FormatIO.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define PI M_PI	// abbreviation used in some parameter ranges
#define DTR(x) (x * (PI / 180))	// degrees to radians conversion

const CWhorldBase::PARAM_INFO CWhorldBase::m_arrParamInfo[PARAM_COUNT] = {
	#define PARAMDEF(name, minval, maxval, steps, scale, initval) \
		{_T(#name), IDS_PARAM_##name, steps, minval, maxval, scale},
	#include "WhorldDef.h"	// generate array init list
};

const int CWhorldBase::m_arrParamPropInfo[PARAM_PROP_COUNT] = {
	#define PARAMPROPDEF(name, type, prefix, variant) IDS_PROW_##name,
	#include "WhorldDef.h"	// generate array init list
};

const int CWhorldBase::m_arrWaveformInfo[WAVEFORM_COUNT] = {
	#define WAVEFORMDEF(name) IDS_WAVE_##name,
	#include "WhorldDef.h"	// generate array init list
};

const CWhorldBase::PARAM_VALS CWhorldBase::m_paramDefault = {
	#define PARAMDEF(name, minval, maxval, steps, scale, initval) initval,
	#include "WhorldDef.h"	// generate array init list
};

const CWhorldBase::MASTER_PROPS CWhorldBase::m_masterDefault = {
	#define MASTERDEF(name, type, prefix, initval) initval,
	#include "WhorldDef.h"	// generate array init list
};

const DPOINT CWhorldBase::INIT_ORIGIN = {0.5, 0.5};

const CWhorldBase::MAIN_PROPS CWhorldBase::m_mainDefault = {
	#define MAINDEF(name, type, prefix, initval, variant) initval,
	#include "WhorldDef.h"	// generate array init list
};

const CWhorldBase::MASTER_INFO CWhorldBase::m_arrMasterInfo[MASTER_COUNT] = {
	#define MASTERDEF(name, type, prefix, initval) \
		{_T(#name), IDS_MASTER_##name},
	#include "WhorldDef.h"	// generate array init list
};

const CWhorldBase::MAIN_INFO CWhorldBase::m_arrMainInfo[MAIN_COUNT] = {
	#define MAINDEF(name, type, prefix, initval, variant) \
		{_T(#name), IDS_MAIN_##name, FIO_##type, offsetof(MAIN_PROPS, prefix##name)},
	#include "WhorldDef.h"	// generate array init list
};

const LPCTSTR CWhorldBase::m_arrRenderCmdName[RENDER_COMMANDS] = {
	_T("RESIZE"),			// base class commands come first
	_T("FULL_SCREEN"),
	#define PARAMPROPDEF(name, type, prefix, variant) _T("SET_PARAM_") _T(#name),
	#include "WhorldDef.h"	// generate range of parameter commands
	#define RENDERCMDDEF(name) _T(#name),
	#include "WhorldDef.h"	// generate regular commands
};

CString CWhorldBase::m_arrParamName[PARAM_COUNT];
CString CWhorldBase::m_arrParamPropName[PARAM_PROP_COUNT];
CString CWhorldBase::m_arrMasterName[MASTER_COUNT];
CString CWhorldBase::m_arrMainName[MAIN_COUNT];
CString CWhorldBase::m_arrWaveformName[WAVEFORM_COUNT];

void CWhorldBase::SetParamDefaults(PARAM_TABLE& aParam)
{
	static const PARAM_ROW rowDefault = {0, 0, 0, 0, 0.5};
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter
		PARAM_ROW&	row = aParam.row[iParam];
		row = rowDefault;
		row.fVal = m_paramDefault.a[iParam];
	}
}

void CWhorldBase::InitWhorldBase()
{
	ASSERT(m_arrParamName[0].IsEmpty());	// call this method only once
	// initialize strings from their string resource IDs
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter
		m_arrParamName[iParam].LoadString(m_arrParamInfo[iParam].nNameID);
	}
	for (int iProp = 0; iProp < PARAM_PROP_COUNT; iProp++) {	// for each parameter property
		m_arrParamPropName[iProp].LoadString(m_arrParamPropInfo[iProp]);
	}
	for (int iMaster = 0; iMaster < MASTER_COUNT; iMaster++) {	// for each master property
		m_arrMasterName[iMaster].LoadString(m_arrMasterInfo[iMaster].nNameID);
	}
	for (int iMain = 0; iMain < MAIN_COUNT; iMain++) {	// for each main property
		m_arrMainName[iMain].LoadString(m_arrMainInfo[iMain].nNameID);
	}
	for (int iWave = 0; iWave < WAVEFORM_COUNT; iWave++) {	// for each waveform
		m_arrWaveformName[iWave].LoadString(m_arrWaveformInfo[iWave]);
	}
}
