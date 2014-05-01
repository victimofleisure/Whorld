// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15jul05	initial version
		01		28jan08	support Unicode

		create a view within a dialog
 
*/

#if !defined(AFX_VIEWDIALOG_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
#define AFX_VIEWDIALOG_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewDialog dialog

#include "ToolDlg.h"

class CViewDialog : public CToolDlg
{
	DECLARE_DYNAMIC(CViewDialog);
// Construction
public:
	CViewDialog(UINT nIDTemplate, UINT nIDAccel, LPCTSTR RegKey, CWnd* pParent = NULL);

// Operations
	bool	CreateView(CCreateContext *ct);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CViewDialog)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CViewDialog)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnViewDialogTab(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Member data
	CFrameWnd	*m_DummyFrame;
	CView	*m_View;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWDIALOG_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
