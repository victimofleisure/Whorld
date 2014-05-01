// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22jun05	initial version
		01		18apr06	remove general page

        options property sheet
 
*/

#if !defined(AFX_OPTIONSDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_)
#define AFX_OPTIONSDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg

#include "OptsDisplayDlg.h"
#include "OptsInputDlg.h"
#include "OptsMidiDlg.h"
#include "OptsFolderDlg.h"

class COptionsDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionsDlg)
// Construction
public:
	COptionsDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	COptionsDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	COptsDisplayDlg	m_Display;
	COptsInputDlg	m_Input;
	COptsMidiDlg	m_Midi;
	COptsFolderDlg	m_Folder;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionsDlg();

// Generated message map functions
protected:
	//{{AFX_MSG(COptionsDlg)
	afx_msg void OnDestroy();
	afx_msg void OnResetAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CButton	m_ResetAll;	// reset all button
	int		m_CurPage;	// index of current page

// Helpers
	void	Construct();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDLG_H__2EF17119_9713_40D8_8BA0_8D5A182DEA75__INCLUDED_)
