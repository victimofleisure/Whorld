// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		20jul05	initial version
		01		17oct05	relay mouse button downs to main frame
		02		24oct05	add DirectDraw exclusive mode
		03		21dec07	show/hide caption bar in exclusive mode
		04		21dec07	hide cursor in exclusive mode
		05		21dec07	replace AfxGetMainWnd with GetThis 
		06		28jan08	support Unicode

		auxiliary frame for detached view

*/

// AuxFrame.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "AuxFrame.h"
#include "AuxView.h"
#include "Persist.h"
#include "MainFrm.h"
#include "WhorldViewDD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAuxFrame

#define REG_AUX_FRAME	_T("AuxFrame")

IMPLEMENT_DYNCREATE(CAuxFrame, CFrameWnd)

CAuxFrame::CAuxFrame()
{
	m_ScreenRect.SetRectEmpty();
	m_View = NULL;
	m_WasShown = FALSE;
	m_IsFullScreen = FALSE;
}

CAuxFrame::~CAuxFrame()
{
}

void CAuxFrame::FullScreen(bool Enable)
{
	if (Enable == m_IsFullScreen)
		return;	// nothing to do
	CMainFrame	*frm = CMainFrame::GetThis();
	if (frm->AllowExclusive()) {
		if (Enable)
			ModifyStyle(WS_CAPTION | WS_THICKFRAME, 0);	// hide caption and frame
		else
			ModifyStyle(0, WS_CAPTION | WS_THICKFRAME);	// restore caption and frame
		CWhorldViewDD	*ViewDD = DYNAMIC_DOWNCAST(CWhorldViewDD, frm->GetView());
		if (ViewDD != NULL)
			ViewDD->SetExclusive(m_hWnd, Enable);
	} else {
		CRect	rc;
		if (Enable) {
			GetWindowRect(m_ScreenRect);
			CBackBufDD::GetFullScreenRect(*this, rc);
			rc.InflateRect(2, 2);	// fudge thin frame
			ModifyStyle(WS_CAPTION | WS_THICKFRAME, 0);
		} else {
			rc = m_ScreenRect;
			ModifyStyle(0, WS_CAPTION | WS_THICKFRAME);
		}
		SetWindowPos(NULL, rc.left, rc.top,
			rc.Width(), rc.Height(), SWP_NOZORDER);
	}
	m_IsFullScreen = Enable;
}

BEGIN_MESSAGE_MAP(CAuxFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CAuxFrame)
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_PARENTNOTIFY()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAuxFrame message handlers

BOOL CAuxFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		NULL,							// no cursor (use default)
		NULL,							// no background brush
		AfxGetApp()->LoadIcon(IDR_MAINFRAME));	// app icon
    ASSERT(cs.lpszClass);
	
	return CFrameWnd::PreCreateWindow(cs);
}

BOOL CAuxFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	CCreateContext	ct;
	ct.m_pNewViewClass = RUNTIME_CLASS(CAuxView);
	m_View = DYNAMIC_DOWNCAST(CView, CreateView(&ct));
	if (m_View == NULL)
		return(FALSE);
	SetActiveView(m_View);
	
	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CAuxFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFrameWnd::OnShowWindow(bShow, nStatus);
	if (!m_WasShown && !IsWindowVisible()) {
		m_WasShown = TRUE;
		CPersist::LoadWnd(REG_SETTINGS, this, REG_AUX_FRAME, CPersist::NO_MINIMIZE);
	}
}

void CAuxFrame::OnClose() 
{
	FullScreen(FALSE);
	CMainFrame	*frm = CMainFrame::GetThis();
	if (frm != NULL)	// main frame could already be destroyed
		frm->SendMessage(UWM_AUXFRAMECLOSE);
	CFrameWnd::OnClose();
}

void CAuxFrame::OnDestroy() 
{
	CPersist::SaveWnd(REG_SETTINGS, this, REG_AUX_FRAME);
	CFrameWnd::OnDestroy();
}

BOOL CAuxFrame::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	CMainFrame::GetThis()->SendMessage(WM_MOUSEWHEEL,	// relay to main frame
		MAKELONG(nFlags, zDelta), MAKELONG(pt.x, pt.y));
	return CFrameWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CAuxFrame::OnParentNotify(UINT message, LPARAM lParam)
{
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		CMainFrame::GetThis()->SendMessage(message, 0, lParam);
		break;
	}
	CFrameWnd::OnParentNotify(message, lParam);
}

BOOL CAuxFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_IsFullScreen && CMainFrame::GetThis()->IsViewDetached()) {
		SetCursor(NULL);	// hide cursor
		return(TRUE);
	}
	return CFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}
