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
		02		22jan08	add FastIsVisible
		03		31jan08	add non-client handlers to fix caption bar freeze

        base class for toolbar-like modeless dialog 
 
*/

#if !defined(AFX_TOOLDLG_H__C9AAC500_6366_4932_A0A0_3DE944078654__INCLUDED_)
#define AFX_TOOLDLG_H__C9AAC500_6366_4932_A0A0_3DE944078654__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolDlg.h : header file
//

#include "PersistDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CToolDlg dialog

class CToolDlg : public CPersistDlg
{
	DECLARE_DYNAMIC(CToolDlg);
// Construction
public:
	CToolDlg(UINT nIDTemplate, UINT nIDAccel, LPCTSTR RegKey, CWnd *pParent);

// Attributes
	bool	FastIsVisible() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CToolDlg)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CToolDlg)
	afx_msg void OnClose();
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);
	//}}AFX_MSG
	LRESULT OnEnterSizeMove(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Member data
	bool	m_IsVisible;		// if true, we're visible
	bool	m_IsNCMoving;		// if true, we're being moved by non-client drag
	CPoint	m_NCLBDownPos;		// cursor position at non-client left button down
};

inline bool CToolDlg::FastIsVisible() const
{
	return(m_IsVisible);	// much faster than CWnd::IsWindowVisible()
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLDLG_H__C9AAC500_6366_4932_A0A0_3DE944078654__INCLUDED_)
