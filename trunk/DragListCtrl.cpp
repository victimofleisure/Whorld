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
		04		29jan08	in OnBegindrag, remove unused local var
		05		30jan08	update cursor type on mouse move
		06		30jan08	add CancelDrag

        list control with drag reordering
 
*/

// DragListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "DragListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDragListCtrl

IMPLEMENT_DYNAMIC(CDragListCtrl, CListCtrl);

CDragListCtrl::CDragListCtrl()
{
	m_Dragging = FALSE;
	m_ScrollDelta = 0;
	m_ScrollTimer = 0;
}

CDragListCtrl::~CDragListCtrl()
{
}

void CDragListCtrl::AutoScroll(const CPoint& Cursor)
{
	if (GetItemCount() > GetCountPerPage()) {	// if view is scrollable
		CRect	cr, ir, hr;
		GetClientRect(cr);
		GetHeaderCtrl()->GetClientRect(hr);
		cr.top += hr.Height() - 1;	// top scroll boundary is bottom of header
		GetItemRect(0, ir, LVIR_BOUNDS);
		int	Offset = ir.Height() / 2;	// vertical scrolling is quantized to lines
		if (Cursor.y < cr.top)	// if cursor is above top boundary
			m_ScrollDelta = Cursor.y - cr.top - Offset;	// start scrolling up
		else if (Cursor.y >= cr.bottom)	// if cursor is below bottom boundary
			m_ScrollDelta = Cursor.y - cr.bottom + Offset;	// start scrolling down
		else
			m_ScrollDelta = 0;	// stop scrolling
	} else
		m_ScrollDelta = 0;
	if (m_ScrollDelta && !m_ScrollTimer)
		m_ScrollTimer = SetTimer(TIMER_ID, SCROLL_DELAY, NULL);
}

void CDragListCtrl::UpdateCursor(CPoint point)
{
	if (ChildWindowFromPoint(point) == this)
		SetCursor(AfxGetApp()->LoadCursor(IDC_DRAG_SINGLE));
	else
 		SetCursor(LoadCursor(NULL, IDC_NO));
}

void CDragListCtrl::CancelDrag()
{
	if (m_Dragging) {
		m_Dragging = FALSE;
		ReleaseCapture();
	}
}

BEGIN_MESSAGE_MAP(CDragListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CDragListCtrl)
	ON_NOTIFY_REFLECT_EX(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragListCtrl message handlers

BOOL CDragListCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_Dragging = TRUE;
	SetCapture();
	int	ResID = GetSelectedCount() > 1 ? IDC_DRAG_MULTI : IDC_DRAG_SINGLE;
	SetCursor(AfxGetApp()->LoadCursor(ResID));
	*pResult = 0;
	return(FALSE);	// let parent handle notification too
}

void CDragListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// NOTE the following restrictions:
	// 1. only report view is supported
	// 2. the list must not be sorted or owner draw
	// 3. all item and subitem strings must be owned by the control (not callbacks)
	if (m_Dragging) {
		m_Dragging = FALSE;
		ReleaseCapture();
		CStringArray ItemText;
		CDWordArray	ItemIdx;
		CDWordArray	ItemData;
		CDWordArray	ItemImage;
		int	cols = GetHeaderCtrl()->GetItemCount();
		int	i, j;
		// save the selected items
	    POSITION pos = GetFirstSelectedItemPosition();
        while (pos != NULL) {
			i = GetNextSelectedItem(pos);
			for (j = 0; j < cols; j++)
				ItemText.Add(GetItemText(i, j));
			ItemIdx.Add(i);
			ItemData.Add(GetItemData(i));
			LVITEM	item;
			ZeroMemory(&item, sizeof(item));
			item.mask = LVIF_IMAGE;
			item.iItem = i;
			GetItem(&item);
			ItemImage.Add(item.iImage);
		}
		// delete the selected items in reverse order
		for (i = ItemIdx.GetSize() - 1; i >= 0; i--)
			DeleteItem(ItemIdx[i]);
		// determine the insert position
		int	k = 0;
		UINT	flags;
		int	InsertPos = HitTest(point, &flags);
		if (InsertPos < 0) {
			if (flags & LVHT_ABOVE)
				InsertPos = 0;
			else	// assume end of list
				InsertPos = GetItemCount();	// this works, amazingly
		}
		// insert the saved items at the new position
		for (i = 0; i < ItemIdx.GetSize(); i++) {
			int	ActualPos = InsertItem(InsertPos + i, ItemText[k++], ItemImage[i]);
			for (j = 1; j < cols; j++)
				SetItemText(ActualPos, j, ItemText[k++]);
			SetItemData(ActualPos, ItemData[i]);
		}
		EnsureVisible(InsertPos, FALSE);
		// notify the parent window
		NMLISTVIEW	lvh;
		ZeroMemory(&lvh, sizeof(lvh));
		lvh.hdr.hwndFrom = m_hWnd;
		lvh.hdr.idFrom = GetDlgCtrlID();
		lvh.hdr.code = ULVN_REORDER;
		GetParent()->SendMessage(WM_NOTIFY, lvh.hdr.idFrom, long(&lvh));
		KillTimer(m_ScrollTimer);
		m_ScrollTimer = 0;
	}
	CListCtrl::OnLButtonUp(nFlags, point);
}

void CDragListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		UpdateCursor(point);
		AutoScroll(point);
	}
	CListCtrl::OnMouseMove(nFlags, point);
}

void CDragListCtrl::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == TIMER_ID) {
		if (m_ScrollDelta)
			Scroll(CSize(0, m_ScrollDelta));
	} else
		CListCtrl::OnTimer(nIDEvent);
}
