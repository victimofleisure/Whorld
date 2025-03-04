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
	enum {	// global parameters
		#define GLOBALPARAMDEF(name) GLOBAL_##name,
		#include "WhorldDef.h"	// generate enumeration
		GLOBAL_COUNT
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
		LPCTSTR	pszTag;		// pointer to private name string
		int		nNameID;	// string resource ID of public name
		int		nSteps;		// number of steps for slider
		double	fMinVal;	// minimum value
		double	fMaxVal;	// maximum value
		double	fScale;		// slider control scaling factor
	};
	struct PARAM_PROP_INFO {	// information about parameter properties
		LPCTSTR	pszTag;		// pointer to private name string
		int		nNameID;	// string resource ID of public name
	};
	union MASTER_PROPS {	// master properties
		struct {
			#define MASTERDEF(name, type, prefix, initval) type prefix##name;
			#include "WhorldDef.h"	// generate member definitions
		};
		double	a[MASTER_COUNT];	// array of master properties
	};
	struct MASTER_INFO {	// information about master properties
		LPCTSTR	pszTag;		// pointer to private name string
		int		nNameID;	// string resource ID of public name
	};
	struct MAIN_PROPS {		// main properties
		#define MAINDEF(name, type, prefix, initval, variant) type prefix##name;
		#include "WhorldDef.h"	// generate member definitions
	};
	struct MAIN_INFO {		// information about main properties
		LPCTSTR	pszTag;		// pointer to private name string
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
	class CPropHint : public CObject {
	public:
		CPropHint(int iProp = 0) : m_iProp(iProp) {}
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
	enum {	// image export flags
		EF_USE_VIEW_SIZE	= 0x01,	// make image same size as view
		EF_SCALE_TO_FIT		= 0x02,	// scale image to fit within view
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
		HINT_MASTER,		// update master property; pHint is CPropHint
		HINT_MAIN,			// update main property; pHint is CPropHint
		HINT_ZOOM,			// update zoom; pHint is unused
		HINT_ORIGIN,		// update origin; pHint is unused
	};
	static const PARAM_VALS m_paramDefault;
	static const MASTER_PROPS m_masterDefault;
	static const MAIN_PROPS m_mainDefault;
	static const DPOINT INIT_ORIGIN;

// Operations
	void SetParamDefaults(PARAM_TABLE& aParam);
	static int	FindParamByTag(LPCTSTR pszTag);
	static int	FindParamPropByTag(LPCTSTR pszTag);
	static int	FindMasterByTag(LPCTSTR pszTag);
	static int	FindMainByTag(LPCTSTR pszTag);

// Attributes
	static bool IsValidParamIdx(int iParam);
	static bool IsValidParamProp(int iProp);
	static bool IsValidMasterProp(int iMaster);
	static bool IsValidMainProp(int iMain);
	static bool IsValidGlobalParam(int iGlobal);
	static const PARAM_INFO& GetParamInfo(int iParam);
	static const double GetParamDefault(int iParam);
	static const CString GetParamName(int iParam);
	static const PARAM_PROP_INFO& GetParamPropInfo(int iProp);
	static const CString GetParamPropName(int iProp);
	static const MASTER_INFO& GetMasterInfo(int iMaster);
	static const CString GetMasterName(int iMaster);
	static const MAIN_INFO& GetMainInfo(int iMain);
	static const CString GetMainName(int iMain);
	static int MapGlobalToParam(int iGlobal);
	static int MapParamToGlobal(int iParam);
	static const CString GetWaveformName(int iWave);
	static const LPCTSTR GetRenderCmdName(int nCmd);

protected:
// Rendering types
	struct RING {	// per-ring data
		DPOINT	ptShiftDelta;	// additional shear per tick
		double	fRotDelta;		// additional rotation per tick
		double	fHue;			// current hue
		double	fLightness;		// current lightness
		double	fSaturation;	// current saturation
		double	fRot;			// rotation for all vertices, in radians
		double	fRadius;		// radius of even vertices, in pixels
		DPOINT	ptScale;		// anisotropic scaling
		DPOINT	ptShift;		// shear, in pixels
		double	fStarRatio;		// ratio of odd radii to even radii
		short	nSides;			// polygon's number of sides
		short	nDrawMode;		// see draw mode enum
		bool	bDelete;		// true if ring should be deleted
		bool	bSkipFill;		// true if ring should be skipped in fill mode
		D2D1_COLOR_F	clrCur;	// current color
		double	fPinwheel;		// additional rotation for odd vertices, in radians
		double	fLineWidth;		// line width, in pixels
		DPOINT	ptOrigin;		// origin in client coords relative to window center
		double	fEvenCurve;		// even vertex curvature, as multiple of radius
		double	fOddCurve;		// odd vertex curvature, as multiple of radius
		double	fEvenShear;		// even vertex curve point asymmetry ratio
		double	fOddShear;		// odd vertex curve point asymmetry ratio
	};
	struct GLOBRING {	// global ring offsets
		double	fRot;			// rotation for all vertices, in radians
		double	fStarRatio;		// ratio of odd radii to even radii
		double	fPinwheel;		// additional rotation for odd vertices
		DPOINT	ptScale;		// anisotropic scaling
		DPOINT	ptShift;		// shear, in pixels
		double	fEvenCurve;		// even vertex curvature, as multiple of radius
		double	fOddCurve;		// odd vertex curvature, as multiple of radius
		double	fEvenShear;		// even vertex curve point asymmetry ratio
		double	fOddShear;		// odd vertex curve point asymmetry ratio
		double	fLineWidth;		// line width, in pixels
		int		nPolySides;		// number of sides
	};
	static const GLOBRING	m_globalRingDefault;	// default global ring

private:
	// use attribute methods instead, for range-checking
	static const PARAM_INFO m_arrParamInfo[PARAM_COUNT];
	static const PARAM_PROP_INFO m_arrParamPropInfo[PARAM_PROP_COUNT];
	static const MASTER_INFO m_arrMasterInfo[MASTER_COUNT];
	static const MAIN_INFO m_arrMainInfo[MAIN_COUNT];
	static const int m_arrWaveformInfo[WAVEFORM_COUNT];
	static const int m_arrGlobalToParam[GLOBAL_COUNT];
	static int m_arrParamToGlobal[PARAM_COUNT];
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

inline bool CWhorldBase::IsValidGlobalParam(int iGlobal)
{
	return iGlobal >= 0 && iGlobal < GLOBAL_COUNT;
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

inline const CWhorldBase::PARAM_PROP_INFO& CWhorldBase::GetParamPropInfo(int iProp)
{
	ASSERT(IsValidParamProp(iProp));
	return m_arrParamPropInfo[iProp];
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

inline int CWhorldBase::MapGlobalToParam(int iGlobal)
{
	ASSERT(IsValidGlobalParam(iGlobal));
	return m_arrGlobalToParam[iGlobal];
}

inline int CWhorldBase::MapParamToGlobal(int iParam)
{
	ASSERT(IsValidParamIdx(iParam));
	return m_arrParamToGlobal[iParam];
}

inline const LPCTSTR CWhorldBase::GetRenderCmdName(int nCmd)
{
	ASSERT(nCmd >= 0 && nCmd < RENDER_COMMANDS);
	return m_arrRenderCmdName[nCmd];
}

inline int CWhorldBase::FindParamByTag(LPCTSTR pszTag)
{
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter
		if (!_tcscmp(pszTag, m_arrParamInfo[iParam].pszTag))	// if tag found
			return iParam;	// return parameter index
	}
	return -1;	// tag not found
}

inline int CWhorldBase::FindParamPropByTag(LPCTSTR pszTag)
{
	for (int iProp = 0; iProp < PARAM_PROP_COUNT; iProp++) {	// for each parameter property
		if (!_tcscmp(pszTag, m_arrParamPropInfo[iProp].pszTag))	// if tag found
			return iProp;	// return parameter property index
	}
	return -1;	// tag not found
}

inline int CWhorldBase::FindMasterByTag(LPCTSTR pszTag)
{
	for (int iMaster = 0; iMaster < MASTER_COUNT; iMaster++) {	// for each master property
		if (!_tcscmp(pszTag, m_arrMasterInfo[iMaster].pszTag))	// if tag found
			return iMaster;	// return master property index
	}
	return -1;	// tag not found
}

inline int CWhorldBase::FindMainByTag(LPCTSTR pszTag)
{
	for (int iMain = 0; iMain < MAIN_COUNT; iMain++) {	// for each main property
		if (!_tcscmp(pszTag, m_arrMainInfo[iMain].pszTag))	// if tag found
			return iMain;	// return main property index
	}
	return -1;	// tag not found
}
