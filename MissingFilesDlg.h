// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov03 initial version
		01		05aug05	pass default extension to ctor
		02		23nov07	support Unicode

		missing files dialog
 
*/

#if !defined(AFX_MISSINGFILESDLG_H__91DA9BFE_D10D_4D1C_89BE_67FF19A108B7__INCLUDED_)
#define AFX_MISSINGFILESDLG_H__91DA9BFE_D10D_4D1C_89BE_67FF19A108B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MissingFilesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMissingFilesDlg dialog

class CMissingFilesDlg : public CDialog
{
	DECLARE_DYNAMIC(CMissingFilesDlg);
public:
// Construction
	CMissingFilesDlg(CStringArray& Path, LPCTSTR DefaultExt, LPCTSTR Filter, CWnd* pParent = NULL);

// Operations
	int	Check();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissingFilesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog data
	//{{AFX_DATA(CMissingFilesDlg)
	enum { IDD = IDD_MISSING_FILES_DLG };
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMissingFilesDlg)
	afx_msg void OnProceed();
	afx_msg void OnOpendlg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CStringArray&	m_Path;
	LPCTSTR	m_Filter;
	LPCTSTR	m_DefaultExt;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISSINGFILESDLG_H__91DA9BFE_D10D_4D1C_89BE_67FF19A108B7__INCLUDED_)
