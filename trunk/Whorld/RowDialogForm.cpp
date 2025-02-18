// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17may05	initial version
		01		21jul05	make room for header in CalcWindowRect
		02		07sep05	verify dynamic downcast in CalcWindowRect

        form view for use within row dialog
 
*/

// RowDialogForm.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RowDialogForm.h"
#include "RowDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRowDialogForm

IMPLEMENT_DYNCREATE(CRowDialogForm, CFormView)

CRowDialogForm::CRowDialogForm()
	: CFormView(CRowDialogForm::IDD)
{
	//{{AFX_DATA_INIT(CRowDialogForm)
	//}}AFX_DATA_INIT
}

CRowDialogForm::~CRowDialogForm()
{
}

void CRowDialogForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRowDialogForm)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRowDialogForm, CFormView)
	//{{AFX_MSG_MAP(CRowDialogForm)
	ON_WM_MOUSEACTIVATE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRowDialogForm diagnostics

#ifdef _DEBUG
void CRowDialogForm::AssertValid() const
{
	CFormView::AssertValid();
}

void CRowDialogForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRowDialogForm message handlers

int CRowDialogForm::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return MA_ACTIVATE;	// don't call base class, prevents assertion
}

void CRowDialogForm::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	GetParent()->PostMessage(WM_HSCROLL, MAKELONG(nSBCode, nPos), NULL);
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CRowDialogForm::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType) 
{
	CRowDialog	*dp = DYNAMIC_DOWNCAST(CRowDialog, GetParent());
	if (dp != NULL)	// initial parent is dummy frame, not row dialog
		lpClientRect->top += dp->GetTopMargin() + dp->GetHeaderHeight();
	CFormView::CalcWindowRect(lpClientRect, nAdjustType);
}
