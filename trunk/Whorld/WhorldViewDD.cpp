// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20may05	initial version
        01		05jun05	add mirror
		02		10jun05	create back buffer in OnCreate
		03		22jun05	get video memory setting from registry
		04		07jul05	if display changes, recreate surface
		05		20jul05	handle create surface errors
        06      14oct05	add exclusive mode
        07      24oct05	SetExclusive is no longer an override
		08		26oct05	don't disable DirectDraw in OnCreateSurfaceError
		09		22dec05	add repaint flag to Mirror
		10		11jan06 if fatal error, offer GDI option
		11		28mar06	repaint flag replaced by pause
		12		06may06	add video overlay

        Whorld view using DirectDraw
 
*/

// WhorldViewDD.cpp : implementation of the CWhorldViewDD class
//

#include "stdafx.h"
#include "Resource.h"
#include "WhorldViewDD.h"
#include "MainFrm.h"
#include "Persist.h"
  
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWhorldViewDD

CWhorldViewDD::CWhorldViewDD()
{
}

CWhorldViewDD::~CWhorldViewDD()
{
}

IMPLEMENT_DYNCREATE(CWhorldViewDD, CWhorldView)

BEGIN_MESSAGE_MAP(CWhorldViewDD, CWhorldView)
	//{{AFX_MSG_MAP(CWhorldViewDD)
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
END_MESSAGE_MAP()

void CWhorldViewDD::SetOptions(int Options)
{
	if (Options != m_Options) {
		m_dd.SetOptions(HIWORD(Options));	// low word is base class options
		if (!m_dd.CreateSurface(m_Size.cx, m_Size.cy))
			OnCreateSurfaceError(m_Size.cx, m_Size.cy);
	}
	CWhorldView::SetOptions(Options);
}

void  CWhorldViewDD::Mirror(bool Enable)
{
	m_dd.SetMirror(Enable);
	CWhorldView::Mirror(Enable);
}

void CWhorldViewDD::SetExclusive(HWND hWnd, bool Enable)
{
	if (!m_dd.SetExclusive(hWnd, m_hWnd, Enable))
		OnCreateSurfaceError(m_Size.cx, m_Size.cy);
	if (Enable)
		SetWndSize(m_dd.GetSize());	// make view full-screen
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldViewDD drawing

void CWhorldViewDD::OnDraw(CDC* pDC)
{
	HDC	dc;
	m_dd.GetDC(&dc);
	Draw(dc);
	m_dd.ReleaseDC(dc);
	m_dd.Blt();
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldViewDD message handlers

int CWhorldViewDD::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)	// skip a level to avoid GDI
		return -1;
	
	if (!m_dd.Create(NULL, m_hWnd) || !m_dd.CreateSurface(1, 1)) {
		CString	s;
		AfxFormatString1(s, IDS_DD_CANT_CREATE, m_dd.GetLastErrorString());
		AfxMessageBox(s);
		OnFatalError();
	}
	if (m_MainView == NULL)	// if first instance, save 'this' for frame
		m_MainView = this;
	if (!m_VideoList.CreateSurfaces(m_dd)) {
		CString	s;
		s.Format(IDS_DD_CANT_CREATE_SURFACE, 1, 1, m_VideoList.GetLastErrorString());
		AfxMessageBox(s);
	}
	return 0;
}

void CWhorldViewDD::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);	// so we don't resize base class back buffer
	SetWndSize(CSize(cx, cy));
	if (!m_dd.IsExclusive()) {	// surface can't change size in exclusive mode
		if (!m_dd.CreateSurface(cx, cy))
			OnCreateSurfaceError(cx, cy);
	}
}

LRESULT CWhorldViewDD::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	if (!m_dd.CreateSurface(m_Size.cx, m_Size.cy))
		OnCreateSurfaceError(m_Size.cx, m_Size.cy);
	Invalidate();
	return Default();
}

void CWhorldViewDD::OnCreateSurfaceError(int cx, int cy)
{
	ShowWindow(SW_HIDE);	// disable painting, else OnDraw would crash us
	CString	s;
	s.Format(IDS_DD_CANT_CREATE_SURFACE, cx, cy, m_dd.GetLastErrorString());
	AfxMessageBox(s);
	OnFatalError();
}

void CWhorldViewDD::OnFatalError()
{
	if (AfxMessageBox(IDS_DD_TRY_GDI, MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		CPersist::WriteInt(REG_SETTINGS, REG_USE_DIR_DRAW, 0);
		AfxMessageBox(IDS_OP_MUST_RESTART);
	}
	exit(0);	// last resort
}
