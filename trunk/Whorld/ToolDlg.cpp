// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		23nov07	support Unicode
		02		22jan08	add FastIsVisible
		03		31jan08	add non-client handlers to fix caption bar freeze

        base class for toolbar-like modeless dialog 
 
*/

// ToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ToolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolDlg dialog

IMPLEMENT_DYNAMIC(CToolDlg, CPersistDlg);

CToolDlg::CToolDlg(UINT nIDTemplate, UINT nIDAccel, LPCTSTR RegKey, CWnd *pParent)
	: CPersistDlg(nIDTemplate, nIDAccel, RegKey, pParent)
{
	//{{AFX_DATA_INIT(CToolDlg)
	//}}AFX_DATA_INIT
	m_IsVisible = FALSE;
	m_IsNCMoving = FALSE;
}

void CToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToolDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CToolDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CToolDlg)
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_NCRBUTTONUP()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolDlg message handlers

void CToolDlg::OnOK()
{
	ShowWindow(SW_HIDE);	// don't end dialog, preserve focused control
}

void CToolDlg::OnCancel()
{
	ShowWindow(SW_HIDE);	// don't end dialog, preserve focused control
}

void CToolDlg::OnClose() 
{
	ShowWindow(SW_HIDE);	// don't end dialog, preserve focused control
}

void CToolDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	// this is the correct way to catch show/hide; OnShowWindow is unreliable
	CPersistDlg::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		m_IsVisible = TRUE;
	else if (lpwndpos->flags & SWP_HIDEWINDOW)
		m_IsVisible = FALSE;
}

void CToolDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// If you left-click in the caption bar, the message loop is blocked until
	// you release the mouse button, move the mouse, or one second elapses. If
	// you left-click the close (X) button, the message loop is blocked until 
	// you release the mouse button. In an app that drives refresh with timer
	// messages, these default behaviors effectively pause the display, so we
	// must override them, emulating the standard UI as closely as possible.
	switch (nHitTest) {
	case HTCAPTION:
		// SC_MOVE makes cursor jump to middle of caption bar; workaround is
		// store current cursor position, and restore it in OnEnterSizeMove.
		m_NCLBDownPos = point;
		m_IsNCMoving = TRUE;
		ShowCursor(FALSE);	// hide cursor, else jump is briefly visible
		SetActiveWindow();	// emulate CWnd behavior
		SendMessage(WM_SYSCOMMAND, SC_MOVE);
		break;
	case HTCLOSE:
		// we could do nothing here and close in OnNcLButtonUp, but then button
		// doesn't depress when clicked, which looks like a bug, so instead we
		// close on button down; it's non-standard but it beats being blocked
		SendMessage(WM_SYSCOMMAND, SC_CLOSE);
		break;
	default:
		CPersistDlg::OnNcLButtonDown(nHitTest, point);
	}
}

void CToolDlg::OnNcRButtonDown(UINT nHitTest, CPoint point)
{
	// If you right-click in the caption bar, the message loop is blocked until
	// you release the mouse button. This is unacceptable in a timer-driven app.
	switch (nHitTest) {
	case HTCAPTION:
		break;	// do nothing, and display context menu in OnNcRButtonUp
	default:
		CPersistDlg::OnNcRButtonDown(nHitTest, point);
	}
}

void CToolDlg::OnNcRButtonUp(UINT nHitTest, CPoint point)
{
	switch (nHitTest) {
	case HTCAPTION:
		SendMessage(WM_CONTEXTMENU, (LONG)m_hWnd, MAKELONG(point.x, point.y));
		break;
	default:
		CPersistDlg::OnNcRButtonUp(nHitTest, point);
	}
}

LRESULT CToolDlg::OnEnterSizeMove(WPARAM wParam, LPARAM lParam)
{
	if (m_IsNCMoving) {	// if move was initiated by left-click in caption bar
		SetCursorPos(m_NCLBDownPos.x, m_NCLBDownPos.y);	// from OnNcLButtonDown
		ShowCursor(TRUE);
		m_IsNCMoving = FALSE;
	}
	return(0);
}
