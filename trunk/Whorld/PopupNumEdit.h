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
		02		04jun18	define user messages in PopupEdit.h only
		03		26feb25	remove note edit

		popup edit control

*/

#pragma once

// PopupNumEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupNumEdit window

#include "NumEdit.h"

class CPopupNumEdit : public CNumEdit
{
	DECLARE_DYNAMIC(CPopupNumEdit);
// Construction
public:
	CPopupNumEdit();

// Constants

// Operations
public:
	bool	Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0);
	void	EndEdit();
	void	CancelEdit();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupNumEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL
	virtual	void	AddSpin(double fDelta);

// Implementation
public:
	virtual ~CPopupNumEdit();

// Generated message map functions
protected:
	//{{AFX_MSG(CPopupNumEdit)
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
