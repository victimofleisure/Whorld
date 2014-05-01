// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		24jul06	add command-line flag to start in VJ mode
		02		27jan08	add GetAppDataFolder

        Whorld application
 
*/

// Whorld.h : main header file for the WHORLD application
//

#if !defined(AFX_WHORLD_H__5AE44A19_CE24_4C34_A2F6_99C6A6BB0383__INCLUDED_)
#define AFX_WHORLD_H__5AE44A19_CE24_4C34_A2F6_99C6A6BB0383__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "Resource.h"       // main symbols
#include "NoAccessKeys.h"

/////////////////////////////////////////////////////////////////////////////
// CWhorldApp:
// See Whorld.cpp for the implementation of this class
//

class CWhorldApp : public CWinApp
{
public:
	CWhorldApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhorldApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL IsIdleMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Operations
	static	bool	GetAppDataFolder(CString& Folder);

protected:
// Implementation
	//{{AFX_MSG(CWhorldApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppHomePage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	class CMyCommandLineInfo : public CCommandLineInfo {
	public:
		CMyCommandLineInfo();
		void	ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
		bool	m_StartVJ;
	};
	CMyCommandLineInfo	m_cmdInfo;
	CNoAccessKeys	m_NoAccess;

// Helpers
	static LONG __stdcall CrashHandler(EXCEPTION_POINTERS *ExceptionInfo);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHORLD_H__5AE44A19_CE24_4C34_A2F6_99C6A6BB0383__INCLUDED_)
