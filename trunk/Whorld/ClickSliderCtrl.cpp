// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28sep04	initial version
		01		22apr05	remove undo handling
		02		31jul05	add SetTicCount

		slider with jump to position and default
 
*/

// ClickSliderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ClickSliderCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClickSliderCtrl

IMPLEMENT_DYNAMIC(CClickSliderCtrl, CSliderCtrl);

CClickSliderCtrl::CClickSliderCtrl()
{
	m_DefPos = 0;
}

CClickSliderCtrl::~CClickSliderCtrl()
{
}

int CClickSliderCtrl::PointToPos(CPoint point)
{
	int	rmin, rmax, rwid;
	GetRange(rmin, rmax);
	rwid = rmax - rmin;
	CRect	cr, tr;
	GetChannelRect(cr);
	GetThumbRect(tr);
	int pos = rmin + rwid * (point.x - cr.left - tr.Width() / 2)
		/ (cr.Width() - tr.Width());
	return(min(max(pos, rmin), rmax));	// clamp to range
}

void CClickSliderCtrl::PostPos()
{
	GetParent()->PostMessage(WM_HSCROLL,
		MAKELONG(SB_THUMBTRACK, GetPos()), long(this->m_hWnd));
}

void CClickSliderCtrl::PostNotification(int Code)
{
	GetParent()->PostMessage(WM_HSCROLL, Code, long(this->m_hWnd));
}

void CClickSliderCtrl::SetTicCount(int Count)
{
	int	lo, hi;
	GetRange(lo, hi);
	SetTicFreq(Count > 0 ? ((hi - lo) / (Count - 1)) : 0);
}

BEGIN_MESSAGE_MAP(CClickSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(CClickSliderCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClickSliderCtrl message handlers

void CClickSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (nFlags & MKU_DEFAULT_POS) {	// restore default position
		SetPos(m_DefPos);
		PostPos();	// post HScroll to parent window
		SetFocus();	// must do this because we're not calling base class
		return;
	}
	CRect	tr;
	GetThumbRect(tr);
	if (!tr.PtInRect(point))	// if click was within thumb, don't jump
		SetPos(PointToPos(point));
	CSliderCtrl::OnLButtonDown(nFlags, point);
	// if click was outside channel, MFC doesn't post HScroll
	CRect	cr;
	GetChannelRect(cr);
	if (!cr.PtInRect(point))
		PostPos();
}

void CClickSliderCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// un-reverse arrow up/down, page up/page down
	switch (nChar) {
	case VK_UP:
		SetPos(GetPos() + GetLineSize());
		PostNotification(TB_LINEUP);
		return;
	case VK_DOWN:
		SetPos(GetPos() - GetLineSize());
		PostNotification(TB_LINEDOWN);
		return;
	case VK_PRIOR:
		SetPos(GetPos() + GetPageSize());
		PostNotification(TB_PAGEUP);
		return;
	case VK_NEXT:
		SetPos(GetPos() - GetPageSize());
		PostNotification(TB_PAGEDOWN);
		return;
	}
	CSliderCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CClickSliderCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// un-reverse wheel
	if (zDelta > 0)
		OnKeyDown(VK_UP, 0, 0);
	else
		OnKeyDown(VK_DOWN, 0, 0);
	return(TRUE);
}
