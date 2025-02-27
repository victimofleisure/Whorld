// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jun05	initial version
		01		02aug05	add image support
		02		05aug05	add autoscroll
		03		23feb06	fix sloppy autoscroll boundary tests
		04		02aug07	convert for virtual list control
		05		29jan08	comment out unused wizard-generated locals
		06		30jan08	add CancelDrag
		07		06jan10	W64: make OnTimer 64-bit compatible
		08		04oct13	add drop position tracking
		09		21nov13	derive from extended selection list
		10		22nov13	in PreTranslateMessage, do base class if not dragging
		11		12jun14	add drag enable
		12		04apr15	add GetCompensatedDropPos
		13		24apr18	standardize names
		14		02jun18	in OnLButtonUp, fix x64 crash due to casting pointer to long
		15		16jun18	make CompensateDropPos static
		16		01nov18	set focus when drag begins
		17		01nov18	cancel drag if window loses focus
		18		30apr20	in cancel drag, kill scroll timer

        virtual list control with drag reordering
 
*/

// DragVirtualListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "DragVirtualListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDragVirtualListCtrl

IMPLEMENT_DYNAMIC(CDragVirtualListCtrl, CListCtrlExSel);

CDragVirtualListCtrl::CDragVirtualListCtrl()
{
	m_bDragEnable = TRUE;
	m_bDragging = FALSE;
	m_bTrackDropPos = FALSE;
	m_nScrollDelta = 0;
	m_nScrollTimer = 0;
	m_iDropPos = 0;
}

CDragVirtualListCtrl::~CDragVirtualListCtrl()
{
}

void CDragVirtualListCtrl::AutoScroll(const CPoint& point)
{
	if (GetItemCount() > GetCountPerPage()) {	// if view is scrollable
		CRect	rc, rItem, rHdr;
		GetClientRect(rc);
		GetHeaderCtrl()->GetClientRect(rHdr);
		rc.top += rHdr.Height() - 1;	// top scroll boundary is bottom of header
		GetItemRect(0, rItem, LVIR_BOUNDS);
		int	Offset = rItem.Height() / 2;	// vertical scrolling is quantized to lines
		if (point.y < rc.top)	// if cursor is above top boundary
			m_nScrollDelta = point.y - rc.top - Offset;	// start scrolling up
		else if (point.y >= rc.bottom)	// if cursor is below bottom boundary
			m_nScrollDelta = point.y - rc.bottom + Offset;	// start scrolling down
		else
			m_nScrollDelta = 0;	// stop scrolling
	} else
		m_nScrollDelta = 0;
	if (m_nScrollDelta && !m_nScrollTimer)
		m_nScrollTimer = SetTimer(TIMER_ID, SCROLL_DELAY, NULL);
}

void CDragVirtualListCtrl::UpdateCursor(CPoint point)
{
	if (ChildWindowFromPoint(point) == this) {
		int	nCursorID;
		if (GetSelectedCount() > 1)
			nCursorID = IDC_CURSOR_DRAG_MULTI;
		else
			nCursorID = IDC_CURSOR_DRAG_SINGLE;
		SetCursor(AfxGetApp()->LoadCursor(nCursorID));
	} else
 		SetCursor(LoadCursor(NULL, IDC_NO));
}

void CDragVirtualListCtrl::CancelDrag()
{
	if (m_bDragging) {
		m_bDragging = FALSE;
		if (m_bTrackDropPos) {	// if tracking drop position
			if (m_iDropPos >= 0)	// if previous drop position valid
				SetItemState(m_iDropPos, 0, LVIS_FOCUSED);	// clear focused style
			int	iItem = GetSelectionMark();
			if (iItem >= 0)	// move focus rectangle back to selection mark
				SetItemState(iItem, LVIS_FOCUSED, LVIS_FOCUSED);	// set focused style
		}
		ReleaseCapture();
		KillTimer(m_nScrollTimer);
		m_nScrollTimer = 0;
	}
}

bool CDragVirtualListCtrl::CompensateDropPos(CIntArrayEx& arrSel, int& iDropPos)
{
	int	iPos = iDropPos;
	int	nSels = arrSel.GetSize();
	if (!(nSels > 1 || (nSels == 1 && iPos != arrSel[0])))
		return(FALSE);	// nothing changed
	// reverse iterate for stability
	for (int iSel = nSels - 1; iSel >= 0; iSel--) {	// for each selected item
		if (arrSel[iSel] < iPos)	// if below drop position
			iPos--;	// compensate drop position
	}
	iDropPos = max(iPos, 0);	// keep it positive
	return(TRUE);
}

int CDragVirtualListCtrl::GetCompensatedDropPos() const
{
	CIntArrayEx	arrSel;
	GetSelection(arrSel);
	int	iDropPos = GetDropPos();
	if (!CompensateDropPos(arrSel, iDropPos))
		return(-1);	// nothing changed
	return(iDropPos);
}

BEGIN_MESSAGE_MAP(CDragVirtualListCtrl, CListCtrlExSel)
	//{{AFX_MSG_MAP(CDragVirtualListCtrl)
	ON_NOTIFY_REFLECT_EX(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragVirtualListCtrl message handlers

BOOL CDragVirtualListCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (m_bDragEnable) {	// if drag enabled
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		m_bDragging = TRUE;
		SetCapture();
		UpdateCursor(pNMListView->ptAction);
		m_iDropPos = -1;
		SetFocus();
	}
	*pResult = 0;
	return(FALSE);	// let parent handle notification too
}

void CDragVirtualListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CListCtrlExSel::OnLButtonUp(nFlags, point);
	// NOTE that only report view is supported
	if (m_bDragging) {
		m_bDragging = FALSE;
		ReleaseCapture();
		UINT	nHitFlags;
		int	iItem = HitTest(point, &nHitFlags);
		if (iItem < 0) {
			if (nHitFlags & LVHT_ABOVE)
				iItem = 0;
			else	// assume end of list
				iItem = GetItemCount();	// this works, amazingly
		}
		m_iDropPos = iItem;	// update drop position member
		// notify the parent window
		NMLISTVIEW	lvh;
		ZeroMemory(&lvh, sizeof(lvh));
		lvh.hdr.hwndFrom = m_hWnd;
		lvh.hdr.idFrom = GetDlgCtrlID();
		lvh.hdr.code = ULVN_REORDER;
		GetParent()->SendMessage(WM_NOTIFY, lvh.hdr.idFrom, reinterpret_cast<LPARAM>(&lvh));
		KillTimer(m_nScrollTimer);
		m_nScrollTimer = 0;
		EnsureVisible(min(iItem, GetItemCount() - 1), FALSE);
	}
}

void CDragVirtualListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bDragging) {
		UpdateCursor(point);
		AutoScroll(point);
		if (m_bTrackDropPos) {	// if tracking drop position
			int	iItem = HitTest(point);
			if (iItem != m_iDropPos) {	// if hit item changed
				if (m_iDropPos >= 0)	// if previous drop position valid
					SetItemState(m_iDropPos, 0, LVIS_FOCUSED);	// clear focused style
				if (iItem >= 0)	// show focus rectangle around hit item
					SetItemState(iItem, LVIS_FOCUSED, LVIS_FOCUSED);	// set focused style
				m_iDropPos = iItem;
			}
		}
	}
	CListCtrlExSel::OnMouseMove(nFlags, point);
}

void CDragVirtualListCtrl::OnTimer(W64UINT nIDEvent) 
{
	if (nIDEvent == TIMER_ID) {
		if (m_nScrollDelta)
			Scroll(CSize(0, m_nScrollDelta));
	} else
		CListCtrlExSel::OnTimer(nIDEvent);
}

BOOL CDragVirtualListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		if (m_bDragging) {
			CancelDrag();
			return(TRUE);
		}
	}
	return CListCtrlExSel::PreTranslateMessage(pMsg);
}

void CDragVirtualListCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CancelDrag();	// release capture before losing focus
	CListCtrlExSel::OnKillFocus(pNewWnd);
}
