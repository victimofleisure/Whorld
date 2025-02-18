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

// ViewDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ViewDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewDialog dialog

IMPLEMENT_DYNAMIC(CViewDialog, CToolDlg);

CViewDialog::CViewDialog(UINT nIDTemplate, UINT nIDAccel, LPCTSTR RegKey, CWnd* pParent /*=NULL*/)
	: CToolDlg(nIDTemplate, nIDAccel, RegKey, pParent)
{
	//{{AFX_DATA_INIT(CViewDialog)
	//}}AFX_DATA_INIT
	m_DummyFrame = NULL;
	m_View = NULL;
}

bool CViewDialog::CreateView(CCreateContext *ct)
{
	m_DummyFrame = new CFrameWnd;
	if (!m_DummyFrame->Create(NULL, NULL))	// create placeholder frame for view
		return(FALSE);
	//
	// NOTE: The created view MUST override OnMouseActivate, otherwise a mouse
	// click within the view causes a debug assertion, and activation problems
	// occur in release.  The OnMouseActivate override must NOT call the base
	// class, it should simply return MA_ACTIVATE.  Thanks to Naama Goraly for
	// discovering this.  Also note that the derived dialog's resource should
	// have the "Clip children" property set, to avoid dueling-paint flicker.
	//
	m_View = DYNAMIC_DOWNCAST(CView, m_DummyFrame->CreateView(ct));	// create view
	if (m_View == NULL)
		return(FALSE);
	m_View->SetParent(this);	// adopt view from frame
	return(TRUE);
}

void CViewDialog::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewDialog)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CViewDialog, CToolDlg)
	//{{AFX_MSG_MAP(CViewDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewDialog message handlers

void CViewDialog::OnDestroy() 
{
	CToolDlg::OnDestroy();
	if (m_View != NULL) {
		m_View->SetParent(m_DummyFrame);	// return view to frame
		m_DummyFrame->DestroyWindow();	// frame deletes itself and view
	}
}

void CViewDialog::OnSize(UINT nType, int cx, int cy) 
{
	CToolDlg::OnSize(nType, cx, cy);
	if (m_View != NULL) {
		CRect	r;
		GetClientRect(r);
		m_View->MoveWindow(r);
	}
}
