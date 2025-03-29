// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov03 initial version
		01		05aug05	make all strings resources
		02		05aug05	pass default extension to ctor
		03		23nov07	support Unicode
		04		29mar25	modernize style

        replace files dialog
 
*/

#pragma once

// ReplaceFilesDlg.h : header file
//

#include <afxtempl.h>
#include "FileSearchDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CReplaceFilesDlg dialog

class CReplaceFilesDlg : public CDialog
{
	DECLARE_DYNAMIC(CReplaceFilesDlg);
public:
// Construction
	CReplaceFilesDlg(CStringArrayEx& arrPath, LPCTSTR pszDefaultExt, LPCTSTR pszFilter, CWnd* pParentWnd = NULL);

// Operations
	bool	Search();

// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// Implementation
protected:
// Dialog data
	enum { IDD = IDD_REPLACE_FILES };
	CListCtrl	m_wndList;

// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	afx_msg void OnSearchAll();
	afx_msg void OnSearchFolder();
	DECLARE_MESSAGE_MAP()

// Nested classes
	class CMissing {
	public:
		CMissing(LPCTSTR FileName, int iItem, int iPath);
		CMissing();
		CMissing(const CMissing& Missing);
		CString	m_sFileName;
		int		m_iItem;
		int		m_iPath;
	};
	class CMyFileSearchDlg : public CFileSearchDlg {
	public:
		CMyFileSearchDlg(LPCTSTR pszFolder, CReplaceFilesDlg *pRFD);
		bool	OnFile(LPCTSTR pszPath);
	private:
		CReplaceFilesDlg *m_pRFD;
	};

// Member data
	CArrayEx<CMissing, CMissing&>	m_arrMissing;
	CStringArrayEx& m_arrPath;
	CString	m_sFolder;
	LPCTSTR	m_pszFilter;
	LPCTSTR	m_pszDefaultExt;
	CString	m_sMissing;
	CString	m_sReplaced;

// Helpers
	bool	Search(LPCTSTR pszFolder);
	void	InitMissing();
	bool	OnFile(LPCTSTR pszPath);

// Friends
	friend	CMyFileSearchDlg;
};
