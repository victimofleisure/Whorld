// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		15sep05	initial version
		01		28jan08	support Unicode
		02		28jan08	support Unicode

        folder options property page
 
*/

#if !defined(AFX_OPTSFOLDERDLG_H__AE207B8B_DB4B_4259_910C_0D9B6DB4996C__INCLUDED_)
#define AFX_OPTSFOLDERDLG_H__AE207B8B_DB4B_4259_910C_0D9B6DB4996C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsFolderDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsFolderDlg dialog

#include "PersistVal.h"

class COptsFolderDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(COptsFolderDlg)

// Construction
public:
	COptsFolderDlg();
	~COptsFolderDlg();

// Attributes
	LPCTSTR	GetSnapshotFolder() const;
	bool	SnapFolderAlways() const;

// Operations
	void	SetDefaults();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptsFolderDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(COptsFolderDlg)
	enum { IDD = IDD_OPTS_FOLDER };
	CEdit	m_SnapFolderEdit;
	int		m_SnapFolderAlways;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(COptsFolderDlg)
	afx_msg void OnSnapFolderBrowse();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagSTATE {
		bool	SnapFolderAlways;	// if true, snapshots, movies, and mixes always
									// go to default folder; else only in VJ mode
	} STATE;

// Constants
	static	const STATE	m_Default;

// Member data
	STATE	m_st;
	PersistVal<bool>	m_RegSnapFolderAlways;
	CString	m_SnapshotFolder;

// Helpers
	static	bool	GetMyDocs(CString& str);
};

inline LPCTSTR COptsFolderDlg::GetSnapshotFolder() const
{
	return(m_SnapshotFolder);
}

inline bool COptsFolderDlg::SnapFolderAlways() const
{
	return(m_st.SnapFolderAlways);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSFOLDERDLG_H__AE207B8B_DB4B_4259_910C_0D9B6DB4996C__INCLUDED_)
