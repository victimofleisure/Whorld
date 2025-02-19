// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov13	initial version
		01		13feb14	add SetSelected
		02		15jun14	add tool tip support
		03		15jul14	fix OnToolHitTest return type
		04		23mar15	add RedrawSubItem
		05		24mar15	add column order methods
		06		04apr15	add GetInsertPos
		07		24apr18	standardize names
		08		17mar20	add method to delete all columns
		09		01apr20	fix context menu top left if no selection
		10		29jan22	add method to ensure item is horizontally visible

		extended selection list control
 
*/

// ListCtrlExSel.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ListCtrlExSel.h"
#include "Persist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListCtrlExSel

IMPLEMENT_DYNCREATE(CListCtrlExSel, CListCtrl)

CListCtrlExSel::CListCtrlExSel()
{
}

CListCtrlExSel::~CListCtrlExSel()
{
}

void CListCtrlExSel::CreateColumns(const COL_INFO *pColInfo, int nColumns)
{
	for (int iCol = 0; iCol < nColumns; iCol++) {	// for each column
		const COL_INFO&	info = pColInfo[iCol];
		InsertColumn(iCol, LDS(info.nTitleID), info.nAlign, info.nWidth);
	}
}

void CListCtrlExSel::DeleteAllColumns()
{
	int nCols = GetColumnCount();
	for (int iCol = 0; iCol < nCols; iCol++)
		DeleteColumn(0);
}

void CListCtrlExSel::SetColumnName(int iCol, LPCTSTR pszName)
{
	LVCOLUMN	lvcol;
	lvcol.mask = LVCF_TEXT;	// only column name is valid
	lvcol.pszText = const_cast<LPTSTR>(pszName);
	SetColumn(iCol, &lvcol);
}

int CListCtrlExSel::GetSelection() const
{
	POSITION	pos = GetFirstSelectedItemPosition();
	if (pos == NULL)
		return(-1);
	return(GetNextSelectedItem(pos));
}

void CListCtrlExSel::GetSelection(CIntArrayEx& arrSel) const
{
	int	nSels = GetSelectedCount();
	arrSel.SetSize(nSels);
	POSITION	pos = GetFirstSelectedItemPosition();
	for (int iSel = 0; iSel < nSels; iSel++)
		arrSel[iSel] = GetNextSelectedItem(pos);
}

void CListCtrlExSel::SetSelection(const CIntArrayEx& arrSel)
{
	Deselect();
	int	nSels = arrSel.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++)
		Select(arrSel[iSel]);
}

void CListCtrlExSel::SetSelected(int iItem, bool bEnable)
{
	SetItemState(iItem, bEnable ? LVIS_SELECTED : 0, LVIS_SELECTED);
}

void CListCtrlExSel::Select(int iItem)
{
	int	StateMask = LVIS_FOCUSED | LVIS_SELECTED;
	SetItemState(iItem, StateMask, StateMask);
	SetSelectionMark(iItem);
}

void CListCtrlExSel::SelectOnly(int iItem)
{
	Deselect();
	Select(iItem);
}

void CListCtrlExSel::SelectRange(int iFirstItem, int nItems)
{
	Deselect();
	int	iEnd = iFirstItem + nItems;
	for (int iItem = iFirstItem; iItem < iEnd; iItem++)
		Select(iItem);
}

void CListCtrlExSel::SelectAll()
{
	int	nItems = GetItemCount();
	int	StateMask = LVIS_FOCUSED | LVIS_SELECTED;
	for (int iItem = 0; iItem < nItems; iItem++)
		SetItemState(iItem, StateMask, StateMask);
}

void CListCtrlExSel::Deselect()
{
	int	StateMask = LVIS_FOCUSED | LVIS_SELECTED;
	POSITION	pos = GetFirstSelectedItemPosition();
	while (pos != NULL)	// deselect all items
		SetItemState(GetNextSelectedItem(pos), 0, StateMask);
}

int CListCtrlExSel::GetInsertPos() const
{
	int	iItem = GetSelection();
	if (iItem >= 0)
		return(iItem);
	return(GetItemCount());
}

void CListCtrlExSel::GetColumnWidths(CIntArrayEx& arrWidth)
{
	int	nCols = GetColumnCount();
	if (nCols < 0)	// if error
		return;
	arrWidth.SetSize(nCols);
	for (int iCol = 0; iCol < nCols; iCol++)
		arrWidth[iCol] = GetColumnWidth(iCol);
}

void CListCtrlExSel::SetColumnWidths(const CIntArrayEx& arrWidth)
{
	int	nCols = min(GetColumnCount(), arrWidth.GetSize());
	for (int iCol = 0; iCol < nCols; iCol++)
		SetColumnWidth(iCol, arrWidth[iCol]);
}

bool CListCtrlExSel::SaveColumnWidths(LPCTSTR pszSection, LPCTSTR pszKey)
{
	CIntArrayEx	arrWidth;
	GetColumnWidths(arrWidth);
	DWORD	nSize = arrWidth.GetSize() * sizeof(int);
	return(CPersist::WriteBinary(pszSection, pszKey, arrWidth.GetData(), nSize) != 0);
}

bool CListCtrlExSel::LoadColumnWidths(LPCTSTR pszSection, LPCTSTR pszKey)
{
	int	nCols = GetColumnCount();
	if (nCols < 0)	// if error
		return(FALSE);
	CIntArrayEx	arrWidth;
	if (!LoadArray(pszSection, pszKey, arrWidth, nCols))
		return(FALSE);
	SetColumnWidths(arrWidth);
	return(TRUE);
}

void CListCtrlExSel::ResetColumnWidths(const COL_INFO *pColInfo, int nColumns)
{
	ASSERT(nColumns == GetColumnCount());
	CIntArrayEx	arrWidth;
	arrWidth.SetSize(nColumns);
	for (int iCol = 0; iCol < nColumns; iCol++)
		arrWidth[iCol] = pColInfo[iCol].nWidth;
	SetColumnWidths(arrWidth);
}

bool CListCtrlExSel::GetColumnOrder(CIntArrayEx& arrOrder)
{
	int	nCols = GetColumnCount();
	if (nCols < 0)	// if error
		return(FALSE);
	arrOrder.SetSize(nCols);
	return(GetColumnOrderArray(arrOrder.GetData(), nCols) != 0);
}

bool CListCtrlExSel::SetColumnOrder(const CIntArrayEx& arrOrder)
{
	int	nCols = min(GetColumnCount(), arrOrder.GetSize());
	return(SetColumnOrderArray(nCols, const_cast<int *>(arrOrder.GetData())) != 0);
}

bool CListCtrlExSel::SaveColumnOrder(LPCTSTR pszSection, LPCTSTR pszKey)
{
	CIntArrayEx	arrOrder;
	GetColumnOrder(arrOrder);
	DWORD	nSize = arrOrder.GetSize() * sizeof(int);
	return(CPersist::WriteBinary(pszSection, pszKey, arrOrder.GetData(), nSize) != 0);
}

bool CListCtrlExSel::LoadColumnOrder(LPCTSTR pszSection, LPCTSTR pszKey)
{
	int	nCols = GetColumnCount();
	if (nCols < 0)	// if error
		return(FALSE);
	CIntArrayEx	arrOrder;
	if (!LoadArray(pszSection, pszKey, arrOrder, nCols))
		return(FALSE);
	return(SetColumnOrder(arrOrder));
}

bool CListCtrlExSel::ResetColumnOrder()
{
	int	nCols = GetColumnCount();
	if (nCols < 0)	// if error
		return(FALSE);
	CIntArrayEx	arrOrder;
	arrOrder.SetSize(nCols);
	for (int iCol = 0; iCol < nCols; iCol++)
		arrOrder[iCol] = iCol;
	return(SetColumnOrder(arrOrder));
}

void CListCtrlExSel::ResetColumnHeader(const COL_INFO *pColInfo, int nColumns)
{
	SetRedraw(false);	// disable drawing to reduce flicker
	ResetColumnWidths(pColInfo, nColumns);	// reset column widths
	if (GetExtendedStyle() & LVS_EX_HEADERDRAGDROP)	// if column reordering enabled
		VERIFY(ResetColumnOrder());	// reset column order too
	SetRedraw();	// reenable drawing
	Invalidate();
}

bool CListCtrlExSel::LoadArray(LPCTSTR pszSection, LPCTSTR pszKey, CIntArrayEx& Array, int nElems)
{
	Array.SetSize(nElems);
	DWORD	nExpectedSize = nElems * sizeof(int);
	DWORD	nSize = nExpectedSize;
	if (!CPersist::GetBinary(pszSection, pszKey, Array.GetData(), &nSize))
		return(FALSE);
	if (nSize != nExpectedSize)	// if unexpected nSize
		return(FALSE);
	return(TRUE);
}

void CListCtrlExSel::FixContextMenuPoint(CPoint& point)
{
	if (point.x == -1 && point.y == -1) {	// if menu triggered via keyboard
		CRect	rc;
		GetWindowRect(rc);
		int	iItem = GetSelectionMark();
		if (iItem >= 0) {	// if selection exists
			CRect	rItem;
			GetItemRect(iItem, rItem, LVIR_BOUNDS);
			ClientToScreen(rItem);
			CPoint	ptCtrItem(rItem.CenterPoint());
			CPoint	ptCtrClient(rc.CenterPoint());
			// if item center x within window
			if (ptCtrItem.x >= rc.left && ptCtrItem.x < rc.right)
				point.x = ptCtrItem.x;
			else	// use client center x instead
				point.x = ptCtrClient.x;
			// if item center y within window
			if (ptCtrItem.y >= rc.top && ptCtrItem.y < rc.bottom)
				point.y = ptCtrItem.y;
			else	// use client center y instead
				point.y = ptCtrClient.y;
		} else {	// no selection
			point = rc.TopLeft();
		}
	}
}

W64INT CListCtrlExSel::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	LVHITTESTINFO	hti;
	hti.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hti);	// use macro to avoid const issue
	CRect	rClient;
    GetClientRect(rClient);
	pTI->hwnd = m_hWnd;
	// return unique ID for each item, causing each item to get its own tool tip;
	// clear ID's most significant bit so ID range is valid even if subitem is -1
	pTI->uId = MAKELONG(hti.iItem, hti.iSubItem) & INT_MAX;
	pTI->lpszText = LPSTR_TEXTCALLBACK;	// request need text notification
	pTI->rect = rClient;
    return pTI->uId;
}

void CListCtrlExSel::EnableToolTips(BOOL bEnable)
{
	SendMessage(LVM_SETTOOLTIPS, 0, 0);	// disable list's tooltip control
	CListCtrl::EnableToolTips(bEnable);
}

int CListCtrlExSel::GetToolTipText(const LVHITTESTINFO* pHTI, CString& Text)
{
	UNREFERENCED_PARAMETER(pHTI);
	UNREFERENCED_PARAMETER(Text);
	return(0);
}

void CListCtrlExSel::RedrawSubItem(int iItem, int iSubItem)
{
	CRect	rItem;
	GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, rItem);
	InvalidateRect(rItem);
}

void CListCtrlExSel::EnsureHorizontallyVisible(int iItem, int iSubItem)
{
	CRect	rClient, rItem;
	GetClientRect(rClient);
	GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, rItem);
	if (iSubItem == 1) {	// if second column
		CRect	rFirstItem;
		GetSubItemRect(iItem, 0, LVIR_BOUNDS, rFirstItem);
		if (rItem.right - rFirstItem.left <= rClient.Width())	// if first two columns fit
			rItem.left = rFirstItem.left;	// also ensure first column is visible
	}
	int	nScrollDeltaX = rItem.left - rClient.left;	// compute difference between left edges
	// align item's left edge with client's left edge, unless nScrollDeltaX is changed below
	if (nScrollDeltaX >= 0) {	// if item's left edge is not left of client's left edge
		int	nRightDeltaX = rItem.right - rClient.right;	// compute difference between right edges
		if (nRightDeltaX <= 0) {	// if item's right edge is not right of client's right edge
			nScrollDeltaX = 0;	// item is within client horizontally, so scrolling isn't needed
		} else {	// item's right edge is right of client's right edge
			if (rClient.Width() > rItem.Width())	// if client is wider than item
				nScrollDeltaX = nRightDeltaX;	// align item's right edge with client's right edge
		}
	}
	if (nScrollDeltaX) {	// if scrolling needed
		Scroll(CSize(nScrollDeltaX, 0));	// scroll window horizontally by specified amount
	}	
}

/////////////////////////////////////////////////////////////////////////////
// CListCtrlExSel message map

BEGIN_MESSAGE_MAP(CListCtrlExSel, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlExSel)
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX(TTN_NEEDTEXTW, 0, OnToolTipNeedText)	// Unicode handler
	ON_NOTIFY_EX(TTN_NEEDTEXTA, 0, OnToolTipNeedText)	// ANSI handler
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlExSel message handlers

BOOL CListCtrlExSel::OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(id);
	UNREFERENCED_PARAMETER(pResult);
	CPoint	pt(GetMessagePos());
    ScreenToClient(&pt);
	LVHITTESTINFO	hti;
	hti.pt = pt;
	ListView_SubItemHitTest(m_hWnd, &hti);
	CString	sText;
	int	nID = GetToolTipText(&hti, sText);	// get resource ID or string from derived class
	if (!nID && sText.IsEmpty())	// if neither resource ID nor string provided
		return(FALSE);	// no tip
	USES_CONVERSION;
	if (pNMHDR->code == TTN_NEEDTEXTA) {	// if ANSI notification
		TOOLTIPTEXTA *pTTT = (TOOLTIPTEXTA *)pNMHDR;
		if (nID) {	// if resource ID provided, use it
			pTTT->lpszText = LPSTR(MAKEINTRESOURCE(nID));
			pTTT->hinst = AfxGetResourceHandle();
		} else	// use string
			strncpy_s(pTTT->szText, T2CA(sText), _countof(pTTT->szText));
	} else {	// Unicode notification
		TOOLTIPTEXTW *pTTT = (TOOLTIPTEXTW *)pNMHDR;
		if (nID) {	// if resource ID provided, use it
			pTTT->lpszText = LPWSTR(MAKEINTRESOURCE(nID));
			pTTT->hinst = AfxGetResourceHandle();
		} else	// use string
			wcsncpy_s(pTTT->szText, T2CW(sText), _countof(pTTT->szText));
	}
	return(TRUE);
}
