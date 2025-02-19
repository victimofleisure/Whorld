// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17may05	initial version
		01		22dec06	create dialog bar variant from CRowDialogForm
		02		16jul07	relay keyboard messages to parent
		03		20apr10	refactor
		04		12may11	add OnSetFocus

        row form
 
*/

#if !defined(AFX_ROWFORM_H__F9AB8865_A0D5_49C5_97A9_9A1E2F74BEB1__INCLUDED_)
#define AFX_ROWFORM_H__F9AB8865_A0D5_49C5_97A9_9A1E2F74BEB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RowFormView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRowForm form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CRowForm : public CFormView
{
	DECLARE_DYNCREATE(CRowForm)
// Construction
public:
	CRowForm();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRowForm)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CRowForm();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
	//{{AFX_MSG(CRowForm)
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROWFORM_H__F9AB8865_A0D5_49C5_97A9_9A1E2F74BEB1__INCLUDED_)
