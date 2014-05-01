// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		20jul05	initial version
		01		06nov05	in OnKeyDown, don't relay modifier keys
		02		21dec07	replace AfxGetMainWnd with GetThis 
		03		29jan08	in OnDraw, remove unused local var

		generic view for auxiliary frame

*/

// AuxView.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "AuxView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAuxView

IMPLEMENT_DYNCREATE(CAuxView, CView)

CAuxView::CAuxView()
{
}

CAuxView::~CAuxView()
{
}


BEGIN_MESSAGE_MAP(CAuxView, CView)
	//{{AFX_MSG_MAP(CAuxView)
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAuxView drawing

void CAuxView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CAuxView diagnostics

#ifdef _DEBUG
void CAuxView::AssertValid() const
{
	CView::AssertValid();
}

void CAuxView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAuxView message handlers

void CAuxView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CMainFrame::GetThis()->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
	CView::OnLButtonDown(nFlags, point);
}

void CAuxView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CMainFrame::GetThis()->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
	CView::OnRButtonDown(nFlags, point);
}

void CAuxView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	CMainFrame::GetThis()->SendMessage(WM_MBUTTONDOWN, nFlags, MAKELONG(point.x, point.y));
	CView::OnMButtonDown(nFlags, point);
}

void CAuxView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// relay to main frame, except auto-repeats and modifier keys
	if (!(nFlags & KF_REPEAT) && nChar != VK_SHIFT && nChar != VK_CONTROL)
		CMainFrame::GetThis()->SendMessage(WM_KEYDOWN, nChar, MAKELONG(nRepCnt, nFlags));
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
