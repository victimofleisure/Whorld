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
		03		22dec06	create dialog bar variant from CRowDialogForm
		04		16jul07	relay keyboard messages to parent
		05		20apr10	refactor
		06		12may11	avoid focus, pass it to parent row view

        row form
 
*/

// RowForm.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RowForm.h"

// CRowForm

IMPLEMENT_DYNCREATE(CRowForm, CFormView)

CRowForm::CRowForm() :
	CFormView(IDD_ROW_FORM)
{
}

CRowForm::~CRowForm()
{
}

BOOL CRowForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BOOL CRowForm::PreCreateWindow(CREATESTRUCT& cs) 
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	CWinApp	*pApp = AfxGetApp();
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		pApp->LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		pApp->LoadIcon(IDR_MAINFRAME));		// app's icon
	return CFormView::PreCreateWindow(cs);
}

void CRowForm::OnDraw(CDC* pDC)
{
	CRect	cb;
	pDC->GetClipBox(cb);
	pDC->FillSolidRect(cb, GetSysColor(COLOR_3DFACE));
}

BEGIN_MESSAGE_MAP(CRowForm, CFormView)
	ON_WM_MOUSEACTIVATE()
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

// CRowForm diagnostics

#ifdef _DEBUG
void CRowForm::AssertValid() const
{
	CFormView::AssertValid();
}

void CRowForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// CRowForm message handlers

int CRowForm::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	SetScrollSizes(MM_TEXT, CSize(0, 0));
	
	return 0;
}

int CRowForm::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	UNREFERENCED_PARAMETER(pDesktopWnd);
	UNREFERENCED_PARAMETER(nHitTest);
	UNREFERENCED_PARAMETER(message);
	return MA_ACTIVATE;	// don't call base class, prevents assertion
}

void CRowForm::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	GetParent()->PostMessage(WM_HSCROLL, MAKELONG(nSBCode, nPos), NULL);
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CRowForm::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
		GetParent()->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
	return CFormView::PreTranslateMessage(pMsg);
}

void CRowForm::OnSetFocus(CWnd* pOldWnd)
{
	UNREFERENCED_PARAMETER(pOldWnd);
	// if one of our child row dialogs is deleted while one of its controls
	// has focus, we get focus, which causes UI bugs, e.g. opening a recent
	// file causes an extra file menu popup; pass focus to parent row view
	CWnd	*pParent = GetParent();
	if (pParent != NULL)
		GetParent()->SetFocus();
}
