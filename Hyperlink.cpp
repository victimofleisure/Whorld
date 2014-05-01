// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26dec04	initial version
		01		23nov07	support Unicode

        simple hyperlink control
 
*/

// Hyperlink.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "Hyperlink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHyperlink

IMPLEMENT_DYNAMIC(CHyperlink, CStatic);

CHyperlink::CHyperlink()
{
	m_Cursor = AfxGetApp()->LoadCursor(IDC_HAND);
	m_Visited = FALSE;
	m_Hovering = FALSE;
	m_ColorLink		= RGB(0, 0, 255);	// blue
	m_ColorVisited	= RGB(128, 0, 128);	// purple
	m_ColorHover	= RGB(255, 0, 0);	// red
}

CHyperlink::~CHyperlink()
{
}

void CHyperlink::SetUrl(LPCTSTR Url)
{
	m_Url = Url;
}

void CHyperlink::FitWindowToUrl()
{
	// adjust window's right and bottom edges to fit URL text
    CRect	r;
    CWnd	*pParent = GetParent();
	if (pParent != NULL) {
	    GetWindowRect(r);
        pParent->ScreenToClient(r);
	} else
        GetClientRect(r);
	CString	s;
	GetWindowText(s);
    CDC	*pDC = GetDC();
	ASSERT(pDC != NULL);
    CFont	*pOldFont = (CFont *)pDC->SelectObject(GetFont());
    CSize	Extent = pDC->GetTextExtent(s);
    pDC->SelectObject(pOldFont);
    ReleaseDC(pDC);
	r.right = r.left + Extent.cx;
	r.bottom = r.top + Extent.cy;
	MoveWindow(r);
}

void CHyperlink::SetUnderline(bool Enable)
{
	LOGFONT	lf;
	if (GetFont()->GetLogFont(&lf)) {
		lf.lfUnderline = TRUE;
		m_Font.Detach();
		m_Font.CreateFontIndirect(&lf);
		SetFont(&m_Font);
		FitWindowToUrl();
	}
}

void CHyperlink::SetColors(COLORREF Link, COLORREF Visited, COLORREF Hover)
{
	m_ColorLink		= Link;
	m_ColorVisited	= Visited;
	m_ColorHover	= Hover;
	Invalidate();
}

bool CHyperlink::GotoUrl(LPCTSTR Url)
{
	int	retc = (int)ShellExecute(NULL, NULL, Url, NULL, NULL, SW_SHOWNORMAL);
	return(retc > HINSTANCE_ERROR);
}

BEGIN_MESSAGE_MAP(CHyperlink, CStatic)
	//{{AFX_MSG_MAP(CHyperlink)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHyperlink message handlers

void CHyperlink::PreSubclassWindow() 
{
	FitWindowToUrl();
	CStatic::PreSubclassWindow();
}

HBRUSH CHyperlink::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	pDC->SetTextColor(m_Hovering ? m_ColorHover :
		(m_Visited ? m_ColorVisited : m_ColorLink));
	pDC->SetBkMode(TRANSPARENT);	// transparent background
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

void CHyperlink::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (GotoUrl(m_Url))
		m_Visited = TRUE;
	else
		AfxMessageBox(IDS_HLINK_CANT_LAUNCH);
	CStatic::OnLButtonDown(nFlags, point);
}


BOOL CHyperlink::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_Cursor != NULL) {
		::SetCursor(m_Cursor);
		return FALSE;
	}
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

void CHyperlink::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_Hovering) {
		TRACKMOUSEEVENT	tme;
		memset(&tme, 0, sizeof(tme));
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		_TrackMouseEvent(&tme);	// request WM_MOUSELEAVE notification
		m_Hovering = TRUE;
		Invalidate();
	}
	CStatic::OnMouseMove(nFlags, point);
}

LRESULT CHyperlink::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_Hovering = FALSE;
	Invalidate();
	return(0);
}
