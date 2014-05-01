// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		12may06	add atomic bank/patch select
		02		22jan08	remove obsolete MIDI messages
		03		28jan08	support Unicode
		04		29jan08	disable a few warnings

        standard include files
 
*/

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3FA27C74_530D_4F92_ABBE_DF086B75AD6C__INCLUDED_)
#define AFX_STDAFX_H__3FA27C74_530D_4F92_ABBE_DF086B75AD6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _WIN32_WINNT 0x0500	// for GDI functions SetDCPenColor, SetDCBrushColor
#define WINVER	0x0500		// for monitor API; may cause NT 5.0 beta warning

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// key status bits for GetAsyncKeyState
#define GKS_TOGGLED			0x00000001
#define GKS_DOWN			0x80000000

// registry strings
#define REG_SETTINGS		_T("Settings")
#define	REG_USE_DIR_DRAW	_T("UseDirDraw")

// document extensions
#define EXT_PATCH			_T(".whp")
#define EXT_SNAPSHOT		_T(".whs")
#define EXT_PLAYLIST		_T(".whl")
#define EXT_MOVIE			_T(".whm")
#define EXT_BITMAP			_T(".bmp")
#define EXT_AVI				_T(".avi")

// user window messages
enum {
	UWM_PARMROWEDIT	= WM_APP + 1,	// wParam: row index, lParam: control ID
	UWM_ROWDIALOGTAB,	// wParam: row index
	UWM_NEWDOCUMENT,	// wParam: non-zero for save, else open
	UWM_MASTEREDIT,		// wParam: CMasterDlg control mask
	UWM_MULTIFILESEL,	// wParam: CMultiFileDlg* of instance
	UWM_VIEWPRINT,		// wParam: notification, lParam: CView*
	UWM_SHELLOPEN,		// wParam: path of file to open, lParam: startup flags
	UWM_TEMPOTAP,		// wParam: float elapsed time in seconds
	UWM_AUXFRAMECLOSE,	// wParam: not used
	UWM_MIDIROWEDIT,	// wParam: row index, lParam: control ID
	UWM_MIDIROWSEL,		// wParam: row index, lParam: control ID
	UWM_MIDIIN,			// wParam: incoming MIDI message
};

// trig macros
#define PI 3.141592653589793
#define DTR(x) (x * PI / 180)	// degrees to radians
#define RTD(x) (x * 180 / PI)	// radians to degrees

// clamp a value to a range
#define CLAMP(x, lo, hi) (min(max((x), (lo)), (hi)))

// load string from resource via temporary object
#define LDS(x) CString((LPCTSTR)x)

// optimized FPU rounding
inline int round(double x)
{
	int		temp;
	__asm	fld		x		// load real
	__asm	fistp	temp	// store integer and pop stack
	return(temp);
}

// double-precision coordinate
#include "DPoint.h"

// minimal base for non-CObject classes
#include "WObject.h"

// wrapper for shell function SHMessageBoxCheck
#include "MessageBoxCheck.h"

// atof's generic-text wrapper is missing in MFC 6
#ifndef _tstof
#ifdef UNICODE
#define _tstof(x) _tcstod(x, NULL)
#else
#define _tstof(x) atof(x)
#endif
#endif

// disable a few warnings
#pragma warning(disable : 4201)	// nameless struct/union
#pragma warning(disable : 4100)	// unreferenced formal parameter
#pragma warning(disable : 4706)	// assignment within conditional expression

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__3FA27C74_530D_4F92_ABBE_DF086B75AD6C__INCLUDED_)
