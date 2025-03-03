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
		03		18feb09	in OnLButtonDown, temporarily disable paging
		04		10feb25	fix warnings on casting window handle
		05		03mar25	modernize style

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
	m_nDefPos = 0;
}

CClickSliderCtrl::~CClickSliderCtrl()
{
}

int CClickSliderCtrl::PointToPos(CPoint point)
{
	int	nMin, nMax, nWidth;
	GetRange(nMin, nMax);
	nWidth = nMax - nMin;
	CRect	rChan, rThumb;
	GetChannelRect(rChan);
	GetThumbRect(rThumb);
	int nPos = nMin + nWidth * (point.x - rChan.left - rThumb.Width() / 2)
		/ (rChan.Width() - rThumb.Width());
	return(min(max(nPos, nMin), nMax));	// clamp to range
}

void CClickSliderCtrl::PostPos()
{
	GetParent()->PostMessage(WM_HSCROLL,
		MAKELONG(SB_THUMBTRACK, GetPos()), reinterpret_cast<LPARAM>(this->m_hWnd));
}

void CClickSliderCtrl::PostNotification(int nCode)
{
	GetParent()->PostMessage(WM_HSCROLL, nCode, reinterpret_cast<LPARAM>(this->m_hWnd));
}

void CClickSliderCtrl::SetTicCount(int nCount)
{
	int	nStart, nEnd;
	GetRange(nStart, nEnd);
	SetTicFreq(nCount > 0 ? ((nEnd - nStart) / (nCount - 1)) : 0);
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
		SetPos(m_nDefPos);
		PostPos();	// post HScroll to parent window
		SetFocus();	// must do this because we're not calling base class
		return;
	}
	CRect	rThumb;
	GetThumbRect(rThumb);
	if (!rThumb.PtInRect(point))	// if click was within thumb, don't jump
		SetPos(PointToPos(point));
	// We need to call the base class for capture and mouse move handling, but
	// the base class also sets the current position, potentially altering the
	// position we just set. The base class jumps to the nearest page boundary,
	// so its position could differ significantly from ours, particularly for
	// small ranges. We fix this by temporarily setting the page size to one.
	int nPageSize = GetPageSize();	// save page size
	SetPageSize(1);	// disable paging so base class doesn't change position
	CSliderCtrl::OnLButtonDown(nFlags, point);	// do base class behavior
	SetPageSize(nPageSize);	// restore page size
	// if click was outside channel, MFC doesn't post HScroll
	CRect	rChan;
	GetChannelRect(rChan);
	if (!rChan.PtInRect(point))
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
	UNREFERENCED_PARAMETER(pt);
	UNREFERENCED_PARAMETER(nFlags);
	// un-reverse wheel
	if (zDelta > 0)
		OnKeyDown(VK_UP, 0, 0);
	else
		OnKeyDown(VK_DOWN, 0, 0);
	return TRUE;
}
