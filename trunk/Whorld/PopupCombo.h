// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		23sep13	initial version
		01		31may14	add class factory
		02		16mar15	add ending edit flag to avoid reentrance
		03		04jun18	define user messages in PopupEdit.h only
		04		16dec22	add support for drop down with edit control

		popup combo box control

*/

#pragma once

// PopupCombo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupCombo window

class CPopupCombo : public CComboBox
{
	DECLARE_DYNAMIC(CPopupCombo);
// Construction
public:
	CPopupCombo();

// Constants

// Operations
public:
	BOOL	Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0);
	static	CPopupCombo	*Factory(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, int DropHeight);
	void	EndEdit();
	void	CancelEdit();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupCombo)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPopupCombo();

// Generated message map functions
protected:
	//{{AFX_MSG(CPopupCombo)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnCloseup();
	afx_msg void OnReflectKillFocus();
	//}}AFX_MSG
	afx_msg LRESULT OnEndEdit(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Data members
	bool	m_bEndingEdit;	// true if ending edit
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
