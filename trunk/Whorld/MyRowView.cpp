// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03mar25	initial version
 
*/

#include "stdafx.h"
#include "resource.h"
#include "MyRowView.h"

IMPLEMENT_DYNCREATE(CMyRowView, CRowView);

CMyRowView::CMyRowView()
{
}

BEGIN_MESSAGE_MAP(CMyRowView, CRowView)
	ON_WM_MOUSEACTIVATE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_LIST_COL_HDR_RESET, OnListColHdrReset)
END_MESSAGE_MAP()

// CMyRowView message handlers

int CMyRowView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	UNREFERENCED_PARAMETER(pDesktopWnd);
	UNREFERENCED_PARAMETER(nHitTest);
	UNREFERENCED_PARAMETER(message);
	return MA_NOACTIVATE;	// prevents assertion
}

void CMyRowView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	UNREFERENCED_PARAMETER(pWnd);
	if (point.x == -1 && point.y == -1)	// if menu triggered via keyboard
		return;
	CHeaderCtrl&	wndHdr = GetHeader();
	CPoint	ptClient(point);
	wndHdr.ScreenToClient(&ptClient);
	HDHITTESTINFO	hti = {ptClient};
	wndHdr.HitTest(&hti);
	// if click on column header, display column header context menu
	if (hti.flags & (HHT_ONHEADER | HHT_NOWHERE | HHT_ONDIVIDER)) {
		CMenu	menu;
		VERIFY(menu.LoadMenu(IDR_LIST_COL_HDR));
		menu.GetSubMenu(0)->TrackPopupMenu(0, point.x, point.y, this);
	}
}

void CMyRowView::OnListColHdrReset()
{
	ResetColumnWidths();
}
