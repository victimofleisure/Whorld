// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		02mar25	implement global parameters

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

const CWhorldBase::PARAM_PROP_INFO CWhorldBase::m_arrParamPropInfo[PARAM_PROP_COUNT] = {
	#define PARAMPROPDEF(name, type, prefix, variant) {_T(#name), IDS_PROW_##name},
	#include "WhorldDef.h"	// generate array init list
};

const int CWhorldBase::m_arrWaveformInfo[WAVEFORM_COUNT] = {
	#define WAVEFORMDEF(name) IDS_WAVE_##name,
	#include "WhorldDef.h"	// generate array init list
};

const int CWhorldBase::m_arrGlobalToParam[GLOBAL_COUNT] = {
	#define GLOBALPARAMDEF(name) PARAM_##name,
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
	#define RENDERCMDDEF(name, vartype) _T(#name),
	#include "WhorldDef.h"	// generate regular commands
};

CString CWhorldBase::m_arrParamName[PARAM_COUNT];
CString CWhorldBase::m_arrParamPropName[PARAM_PROP_COUNT];
CString CWhorldBase::m_arrMasterName[MASTER_COUNT];
CString CWhorldBase::m_arrMainName[MAIN_COUNT];
CString CWhorldBase::m_arrWaveformName[WAVEFORM_COUNT];
int CWhorldBase::m_arrParamToGlobal[PARAM_COUNT];

const CWhorldBase::GLOB_RING CWhorldBase::m_globalRingDefault = {
	0,	// fRot
	1,	// fStarRatio
	0,	// fPinwheel
	{1, 1},	// ptScale
	{0, 0},	// ptShift
	0,	// fEvenCurve
	0,	// fOddCurve
	1,	// fEvenShear
	1,	// fOddShear
	0,	// fLineWidth
	0,	// nPolySides
};

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
		m_arrParamPropName[iProp].LoadString(m_arrParamPropInfo[iProp].nNameID);
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
	// any parameter that lacks a global will have an index of -1
	memset(m_arrParamToGlobal, -1, sizeof(m_arrParamToGlobal));	// due to this init
	// initialize reverse lookup table for mapping parameter index to global index
	for (int iGlobal = 0; iGlobal < GLOBAL_COUNT; iGlobal++) {	// for each global parameter
		int	iParam = m_arrGlobalToParam[iGlobal];	// map from global to its parameter
		m_arrParamToGlobal[iParam] = iGlobal;	// store global's index in parameter's element
	}
}

CWhorldBase::CMovieExportParams::CMovieExportParams()
{
	m_szFrame = CSize(0, 0);
	m_nExportFlags = 0;
	m_nStartFrame = 0;
	m_nEndFrame = 0;
}
