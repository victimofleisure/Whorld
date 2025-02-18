// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17may05	initial version

        form view for use within row dialog
 
*/

#if !defined(AFX_ROWDIALOGFORM_H__F9AB8865_A0D5_49C5_97A9_9A1E2F74BEB1__INCLUDED_)
#define AFX_ROWDIALOGFORM_H__F9AB8865_A0D5_49C5_97A9_9A1E2F74BEB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RowDialogForm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRowDialogForm form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CRowDialogForm : public CFormView
{
	DECLARE_DYNCREATE(CRowDialogForm)
protected:
	CRowDialogForm();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRowDialogForm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CRowDialogForm();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
	//{{AFX_MSG(CRowDialogForm)
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Form Data
	//{{AFX_DATA(CRowDialogForm)
	enum { IDD = IDD_ROW_DIALOG_FORM };
	//}}AFX_DATA
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROWDIALOGFORM_H__F9AB8865_A0D5_49C5_97A9_9A1E2F74BEB1__INCLUDED_)
