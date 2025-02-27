// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		23sep13	initial version
		01		16mar15	add ending edit flag to avoid reentrance

		popup edit control

*/

#pragma once

// PopupEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit window

class CPopupEdit : public CEdit
{
	DECLARE_DYNAMIC(CPopupEdit);
// Construction
public:
	CPopupEdit();

// Constants
	enum {
		UWM_END_EDIT = WM_USER + 400,	// wParam: true if canceling, lParam: none
		UWM_TEXT_CHANGE,				// wParam: pointer to text, lParam: none
	};

// Operations
public:
	bool	Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0);
	void	EndEdit();
	void	CancelEdit();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPopupEdit();

// Generated message map functions
protected:
	//{{AFX_MSG(CPopupEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	afx_msg LRESULT OnEndEdit(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Data members
	bool	m_bEndingEdit;	// true if ending edit
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
