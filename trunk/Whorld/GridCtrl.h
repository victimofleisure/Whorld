// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23sep13	initial version
		01		31may14	add OnItemChange
		02		12may15	remove unused update pos message
		03		24apr18	standardize names
		04		04jun18	give popup edit control non-zero ID

		grid control
 
*/

#pragma once

// GridCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl window

#include "DragVirtualListCtrl.h"

class CGridCtrl : public CDragVirtualListCtrl
{
	DECLARE_DYNAMIC(CGridCtrl);
// Construction
public:
	CGridCtrl();

// Attributes
public:
	bool	IsEditing() const;

// Operations
public:
	bool	EditSubitem(int iRow, int iCol);
	void	EndEdit();
	void	CancelEdit();

// Constants
	enum {
		IDC_POPUP_EDIT = 2001,
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGridCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGridCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnTextChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Data members
	CWnd	*m_pEditCtrl;		// popup control for subitem editing
	int		m_iEditRow;			// row index of subitem being edited
	int		m_iEditCol;			// column index of subitem being edited

// Overrideables
	virtual	CWnd	*CreateEditCtrl(LPCTSTR pszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual	void	OnItemChange(LPCTSTR pszText);
	virtual bool	AllowEnsureHorizontallyVisible(int iCol);

// Helpers
	void	GotoSubitem(int nDeltaRow, int nDeltaCol);
};

inline bool CGridCtrl::IsEditing() const
{
	return(m_pEditCtrl != NULL);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
