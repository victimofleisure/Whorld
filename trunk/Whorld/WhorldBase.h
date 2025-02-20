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

#pragma once

#include "RenderThread.h"

// macro to allocate and assign a variant property for supported types
#define MAKE_VARIANT_PROP(type, val) VARIANT_PROP prop; prop.type = val;

class CWhorldBase : public CRenderThreadBase {
public:
// Construction
	static void InitWhorldBase();	// call in InitInstance

// Constants
	enum {	// parameters
		#define PARAMDEF(name, minval, maxval, steps, scale, initval) PARAM_##name,
		#include "WhorldDef.h"	// generate enumeration
		PARAM_COUNT
	};
	enum {	// master properties
		#define MASTERDEF(name, type, prefix, initval) MASTER_##name,
		#include "WhorldDef.h"	// generate enumeration
		MASTER_COUNT
	};
	enum {	// main properties
		#define MAINDEF(name, type, prefix, initval, variant) MAIN_##name,
		#include "WhorldDef.h"	// generate enumeration
		MAIN_COUNT
	};

// Types
	union PARAM_VALS {		// parameter values
		struct {
			#define PARAMDEF(name, minval, maxval, steps, scale, initval) double f##name;
			#include "WhorldDef.h"	// generate member definitions
		};
		double	a[PARAM_COUNT];	// array of parameters
	};
	struct PARAM_ROW {		// parameter row
		#define PARAMPROPDEF(name, type, prefix, variant) type prefix##name;
		#include "WhorldDef.h"	// generate member definitions
	};
	struct PARAM_TABLE {	// parameter table
		union {
			PARAM_ROW	row[PARAM_COUNT];	// parameter row
		};
		__m128	sseVar;		// enables fast SSE copy by ensuring 16-byte alignment
	};
	struct PARAM_INFO {		// information about parameters
		LPCTSTR	pszName;	// pointer to private name string
		int		nNameID;	// string resource ID of public name
		int		nSteps;		// number of steps for slider
		double	fMinVal;	// minimum value
		double	fMaxVal;	// maximum value
		double	fScale;		// slider control scaling factor
	};
	union MASTER_PROPS {	// master properties
		struct {
			#define MASTERDEF(name, type, prefix, initval) type prefix##name;
			#include "WhorldDef.h"	// generate member definitions
		};
		double	a[MASTER_COUNT];	// array of master properties
	};
	struct MASTER_INFO {	// information about master properties
		LPCTSTR	pszName;	// pointer to private name string
		int		nNameID;	// string resource ID of public name
	};
	struct MAIN_PROPS {		// main properties
		#define MAINDEF(name, type, prefix, initval, variant) type prefix##name;
		#include "WhorldDef.h"	// generate member definitions
	};
	struct MAIN_INFO {		// information about main properties
		LPCTSTR	pszName;	// pointer to private name string
		int		nNameID;	// string resource ID of public name
		int		nFIOType;	// formatted I/O type
		int		nOffset;	// byte offset within MAIN_PROPS
	};
	class CParamHint : public CObject {
	public:
		CParamHint(int iParam, int iProp = 0) : m_iParam(iParam), m_iProp(iProp) {}
		int		m_iParam;	// index of parameter
		int		m_iProp;	// index of property
	};

// Constants
	enum {	// parameter properties
		#define PARAMPROPDEF(name, type, prefix, variant) PARAM_PROP_##name,
		#include "WhorldDef.h"	// generate enumeration
		PARAM_PROP_COUNT
	};
	enum {	// waveforms
		#define WAVEFORMDEF(name) WAVEFORM_##name,
		#include "WhorldDef.h"	// generate enumeration
		WAVEFORM_COUNT
	};
	enum {	// miscellaneous
		MAX_RINGS = 1000,	// maximum number of rings; used in master init
	};
	enum {	// draw modes
		DM_FILL		= 0x01,		// fill area between rings
		DM_OUTLINE	= 0x02,		// outline edges of filled rings
		DM_XRAY		= 0x04		// draw rings with XOR pen
	};
	enum {	// origin motion types
		OM_PARK,	// no motion
		OM_DRAG,	// cursor drag
		OM_RANDOM,	// random jump
		ORIGIN_MOTION_TYPES
	};
	enum {	// Whorld-specific render commands
		RC_START = BASE_RENDER_COMMANDS - 1,	// offset from base render commands
		// set parameter uses a range of commands, one for each parameter property;
		// m_nParam is the parameter index and m_prop is the variant data payload
		#define PARAMPROPDEF(name, type, prefix, variant) RC_SET_PARAM_##name,
		#include "WhorldDef.h"	// generate range of parameter commands
		#define RENDERCMDDEF(name, vartype) RC_##name,
		#include "WhorldDef.h"	// generate regular commands
		RENDER_COMMANDS,
		RC_SET_PARAM_FIRST = RC_START + 1,	// first set parameter command
		RC_SET_PARAM_LAST = RC_SET_PARAM_FIRST + PARAM_PROP_COUNT - 1,
	};
	enum {	// document update hints
		HINT_NONE,			// no hint
		HINT_PARAM,			// update parameter property; pHint is CParamHint
		HINT_MASTER,		// update master property; pHint is CParamHint
		HINT_MAIN,			// update main property; pHint is CParamHint
		HINT_ZOOM,			// update zoom; pHint is unused
		HINT_ORIGIN,		// update origin; pHint is unused
	};
	static const PARAM_VALS m_paramDefault;
	static const MASTER_PROPS m_masterDefault;
	static const MAIN_PROPS m_mainDefault;
	static const DPOINT INIT_ORIGIN;

// Operations
	void SetParamDefaults(PARAM_TABLE& aParam);
	static int	FindParamByName(LPCTSTR pszName);
	static int	FindMasterByName(LPCTSTR pszName);
	static int	FindMainByName(LPCTSTR pszName);

// Attributes
	static bool IsValidParamIdx(int iParam);
	static bool IsValidParamProp(int iProp);
	static bool IsValidMasterProp(int iMaster);
	static bool IsValidMainProp(int iMain);
	static const PARAM_INFO& GetParamInfo(int iParam);
	static const double GetParamDefault(int iParam);
	static const CString GetParamName(int iParam);
	static const CString GetParamPropName(int iProp);
	static const MASTER_INFO& GetMasterInfo(int iMaster);
	static const CString GetMasterName(int iMaster);
	static const MAIN_INFO& GetMainInfo(int iMain);
	static const CString GetMainName(int iMain);
	static const CString GetWaveformName(int iWave);
	static const LPCTSTR GetRenderCmdName(int nCmd);

private:
	// use attribute methods instead, for range-checking
	static const PARAM_INFO m_arrParamInfo[PARAM_COUNT];
	static const int m_arrParamPropInfo[PARAM_PROP_COUNT];
	static const MASTER_INFO m_arrMasterInfo[MASTER_COUNT];
	static const MAIN_INFO m_arrMainInfo[MAIN_COUNT];
	static const int m_arrWaveformInfo[WAVEFORM_COUNT];
	static CString m_arrParamName[PARAM_COUNT];
	static CString m_arrParamPropName[PARAM_PROP_COUNT];
	static CString m_arrMasterName[MASTER_COUNT];
	static CString m_arrMainName[MAIN_COUNT];
	static CString m_arrWaveformName[WAVEFORM_COUNT];
	static const LPCTSTR m_arrRenderCmdName[RENDER_COMMANDS];
};

inline bool CWhorldBase::IsValidParamIdx(int iParam)
{
	return iParam >= 0 && iParam < PARAM_COUNT;
}

inline bool CWhorldBase::IsValidParamProp(int iProp)
{
	return iProp >= 0 && iProp < PARAM_PROP_COUNT;
}

inline bool CWhorldBase::IsValidMasterProp(int iMaster)
{
	return iMaster >= 0 && iMaster < MASTER_COUNT;
}

inline bool CWhorldBase::IsValidMainProp(int iMain)
{
	return iMain >= 0 && iMain < MAIN_COUNT;
}

inline const CWhorldBase::PARAM_INFO& CWhorldBase::GetParamInfo(int iParam)
{
	ASSERT(IsValidParamIdx(iParam));
	return m_arrParamInfo[iParam];
}

inline const double CWhorldBase::GetParamDefault(int iParam)
{
	ASSERT(IsValidParamIdx(iParam));
	return m_paramDefault.a[iParam];
}

inline const CString CWhorldBase::GetParamName(int iParam)
{
	ASSERT(IsValidParamIdx(iParam));
	return m_arrParamName[iParam];
}

inline const CString CWhorldBase::GetParamPropName(int iProp)
{
	ASSERT(IsValidParamProp(iProp));
	return m_arrParamPropName[iProp];
}

inline const CWhorldBase::MASTER_INFO& CWhorldBase::GetMasterInfo(int iMaster)
{
	ASSERT(IsValidMasterProp(iMaster));
	return m_arrMasterInfo[iMaster];
}

inline const CString CWhorldBase::GetMasterName(int iMaster)
{
	ASSERT(IsValidMasterProp(iMaster));
	return m_arrMasterName[iMaster];
}

inline const CWhorldBase::MAIN_INFO& CWhorldBase::GetMainInfo(int iMain)
{
	ASSERT(IsValidMainProp(iMain));
	return m_arrMainInfo[iMain];
}

inline const CString CWhorldBase::GetMainName(int iMain)
{
	ASSERT(IsValidMainProp(iMain));
	return m_arrMainName[iMain];
}

inline const CString CWhorldBase::GetWaveformName(int iWave)
{
	ASSERT(iWave >= 0 && iWave < WAVEFORM_COUNT);
	return m_arrWaveformName[iWave];
}

inline const LPCTSTR CWhorldBase::GetRenderCmdName(int nCmd)
{
	ASSERT(nCmd >= 0 && nCmd < RENDER_COMMANDS);
	return m_arrRenderCmdName[nCmd];
}

inline int CWhorldBase::FindParamByName(LPCTSTR pszName)
{
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter
		if (!_tcscmp(pszName, m_arrParamInfo[iParam].pszName))
			return iParam;
	}
	return -1;
}

inline int CWhorldBase::FindMasterByName(LPCTSTR pszName)
{
	for (int iMast = 0; iMast < MASTER_COUNT; iMast++) {	// for each master property
		if (!_tcscmp(pszName, m_arrMasterInfo[iMast].pszName))
			return iMast;
	}
	return -1;
}

inline int CWhorldBase::FindMainByName(LPCTSTR pszName)
{
	for (int iMain = 0; iMain < MAIN_COUNT; iMain++) {	// for each main property
		if (!_tcscmp(pszName, m_arrMainInfo[iMain].pszName))
			return iMain;
	}
	return -1;
}
