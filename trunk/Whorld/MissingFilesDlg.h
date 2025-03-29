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
		03		29mar25	modernize style

		missing files dialog
 
*/

#pragma once

// MissingFilesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMissingFilesDlg dialog

class CMissingFilesDlg : public CDialog
{
	DECLARE_DYNAMIC(CMissingFilesDlg);
public:
// Construction
	CMissingFilesDlg(CStringArrayEx& arrPath, LPCTSTR pszDefaultExt, LPCTSTR pszFilter, CWnd* pParentWnd = NULL);

// Operations
	INT_PTR	Check();

// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
// Dialog data
	enum { IDD = IDD_MISSING_FILES };

// Generated message map functions
	afx_msg void OnProceed();
	afx_msg void OnOpenDlg();
	DECLARE_MESSAGE_MAP()

// Member data
	CStringArrayEx&	m_arrPath;
	LPCTSTR	m_pszFilter;
	LPCTSTR	m_pszDefaultExt;
};
