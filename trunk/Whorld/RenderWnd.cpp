// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version

*/

#include "stdafx.h"
#include "Whorld.h"
#include "RenderWnd.h"
#include "MainFrm.h"
#include "WhorldDoc.h"

IMPLEMENT_DYNAMIC(CRenderWnd, CWnd)

CRenderWnd::CRenderWnd()
{
}

CRenderWnd::~CRenderWnd()
{
}

bool CRenderWnd::CreateWnd(DWORD dwStyle, CRect& rWnd, CWnd *pParentWnd)
{
	LPCTSTR	pszClass = AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW));
	if (!CreateEx(0, pszClass, theApp.m_pszAppName, dwStyle, rWnd, pParentWnd, 0))
		return false;
	return true;
}

BEGIN_MESSAGE_MAP(CRenderWnd, CWnd)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

void CRenderWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (cx > 0 && cy > 0) {
		theApp.m_thrRender.Resize();
	}
}

void CRenderWnd::OnTimer(UINT_PTR nIDEvent)
{
	UNREFERENCED_PARAMETER(nIDEvent);
	// no timers
}

void CRenderWnd::OnPaint()
{
	ValidateRect(NULL);	// end paint cycle
}

BOOL CRenderWnd::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;	// don't erase background
}

void CRenderWnd::OnClose()
{
	theApp.m_pMainWnd->PostMessage(UWM_RENDER_WND_CLOSED, 0, 0);
}

void CRenderWnd::OnKillFocus(CWnd* pNewWnd)
{
	if (theApp.IsSingleMonitor()) {	// if single monitor configuration
		theApp.SetFullScreen(false);	// exit full screen mode
	}
	CWnd::OnKillFocus(pNewWnd);
}

BOOL CRenderWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (theApp.IsFullScreen()) {	// if app is full screen
		SetCursor(NULL);	// hide the cursor by loading a null cursor
		return true;	// skip base class behavior to avoid flicker
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CRenderWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return theApp.GetDocument()->OnMouseWheel(nFlags, zDelta, pt);
}
