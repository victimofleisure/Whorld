// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		28jan08	support Unicode

        about dialog
 
*/

#if !defined(AFX_ABOUTDLG_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_)
#define AFX_ABOUTDLG_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AboutDlg.h : header file
//

#include "Hyperlink.h"

class CAboutDlg : public CDialog
{
// Construction
public:
	CAboutDlg();

// Constants
	static const LPCTSTR HOME_PAGE_URL;

// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CEdit	m_Credits;
	CEdit	m_License;
	CHyperlink	m_AboutUrl;
	CStatic	m_AboutText;
	//}}AFX_DATA
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUTDLG_H__810797E4_C040_4E3D_98F7_C75B7C55507D__INCLUDED_)
