// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		23nov07	support Unicode

        dialog that saves and restores its position
 
*/

#if !defined(AFX_PERSISTDLG_H__583BF356_FB65_4584_B723_AD9F32A0E27A__INCLUDED_)
#define AFX_PERSISTDLG_H__583BF356_FB65_4584_B723_AD9F32A0E27A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PersistDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPersistDlg dialog

class CPersistDlg : public CDialog
{
	DECLARE_DYNAMIC(CPersistDlg);
// Construction
public:
	CPersistDlg(UINT nIDTemplate, UINT nIDAccel, LPCTSTR RegKey, CWnd *pParent);

// Operations
	static	void	UpdateMenu(CWnd *pWnd, CMenu *pMenu);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPersistDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPersistDlg)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPersistDlg)
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	bool	m_WasShown;		// true if dialog was shown at least once
	LPCTSTR	m_RegKey;		// registry key prefix for window position
	HACCEL	m_Accel;		// accelerator handle for dialog shortcut keys
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERSISTDLG_H__583BF356_FB65_4584_B723_AD9F32A0E27A__INCLUDED_)
