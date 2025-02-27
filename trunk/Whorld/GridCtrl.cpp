// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23sep13	initial version
		01		03may14	in OnParentNotify, event is low word of message
		02		31may14	add OnItemChange
		03		17nov14	in PreTranslateMessage, let tab edit first subitem
		04		04apr15	use base class column count accessor
		05		19apr18	in OnParentNotify, use subitem rect for CEdit-derived controls
		06		19apr18	if scroll notification from our child control, don't end edit
		07		24apr18	standardize names
		08		25apr18	handle up and down arrow keys only if control key is down
		09		25apr18	in EditSubItem, don't modify selection
		10		27apr18	handle reordered columns
		11		04jun18	give popup edit control non-zero ID
		12		15dec18	only handle tab key if control key is up
		13		17nov20	in OnLButtonDown, set focus before editing subitem
		14		29jan22	ensure item to be edited is horizontally visible
		15		17dec22	in OnParentNotify, get cursor position from current message

		grid control
 
*/

// GridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "GridCtrl.h"
#include "PopupEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl

IMPLEMENT_DYNAMIC(CGridCtrl, CDragVirtualListCtrl);

CGridCtrl::CGridCtrl()
{
	m_pEditCtrl = NULL;
	m_iEditRow = 0;
	m_iEditCol = 0;
}

CGridCtrl::~CGridCtrl()
{
}

bool CGridCtrl::EditSubitem(int iRow, int iCol)
{
	ASSERT(iRow >= 0 && iRow < GetItemCount());	// validate row
	ASSERT(iCol >= 1 && iCol < GetColumnCount());	// subitems only
	EndEdit();	// end previous edit if any
	EnsureVisible(iRow, FALSE);	// make sure specified row is fully visible
	if (AllowEnsureHorizontallyVisible(iCol))
		EnsureHorizontallyVisible(iRow, iCol);
	CRect	rSubitem;
	GetSubItemRect(iRow, iCol, LVIR_BOUNDS, rSubitem);	// get subitem rect
	// clip siblings is mandatory, else edit control overwrites header control
	UINT	style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
	m_iEditRow = iRow;	// update our indices first so create can access them
	m_iEditCol = iCol;
	CString	text(GetItemText(iRow, iCol));	// get subitem text
	m_pEditCtrl = CreateEditCtrl(text, style, rSubitem, this, IDC_POPUP_EDIT);
	if (m_pEditCtrl == NULL)
		return(FALSE);
	ASSERT(IsWindow(m_pEditCtrl->m_hWnd));
	return(TRUE);
}

void CGridCtrl::EndEdit()
{
	if (IsEditing())
		m_pEditCtrl->SendMessage(CPopupEdit::UWM_END_EDIT);
}

void CGridCtrl::CancelEdit()
{
	if (IsEditing())
		m_pEditCtrl->SendMessage(CPopupEdit::UWM_END_EDIT, TRUE);	// cancel edit
}

void CGridCtrl::GotoSubitem(int nDeltaRow, int nDeltaCol)
{
	ASSERT(abs(nDeltaRow) <= 1);	// valid delta range is [-1..1]
	ASSERT(abs(nDeltaCol) <= 1);
	ASSERT(IsEditing());
	int	nCols = GetColumnCount();
	int	iCol = m_iEditCol;
	CIntArrayEx	arrOrder;
	if (GetColumnOrder(arrOrder)) {	// get column order array
		if (arrOrder[iCol] != iCol) {	// if item and column differ
			int	iNewCol = INT64TO32(arrOrder.Find(iCol));	// find column's item
			ASSERT(iNewCol >= 0);	// should always be found
			if (iNewCol >= 0)	// if item found
				iCol = iNewCol;	// convert item to column
		}
	}
	iCol += nDeltaCol;	// offset column
	if (iCol >= nCols) {	// if after last column
		iCol = 1;	// wrap to first column
		nDeltaRow = 1;	// next row
	} else if (iCol < 1) {	// if before first column
		iCol = nCols - 1;	// wrap to last column
		nDeltaRow = -1;	// previous row
	}
	if (arrOrder.GetSize())	// if column order array valid
		iCol = arrOrder[iCol];	// convert column back to item
	int	nRows = GetItemCount();
	int	iRow = m_iEditRow + nDeltaRow;	// offset row
	if (iRow >= nRows) {	// if after last row
		iRow = 0;	// wrap to first row
	} else if (iRow < 0) {	// if before first row
		iRow = nRows - 1;	// wrap to last row
	}
	EditSubitem(iRow, iCol);
}

CWnd *CGridCtrl::CreateEditCtrl(LPCTSTR pszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	UNREFERENCED_PARAMETER(pParentWnd);
	CPopupEdit	*pEdit = new CPopupEdit;
	if (!pEdit->Create(dwStyle, rect, this, nID)) {
		delete pEdit;
		return(NULL);
	}
	pEdit->SetWindowText(pszText);
	pEdit->SetSel(0, -1);	// select entire text
	return(pEdit);
}

void CGridCtrl::OnItemChange(LPCTSTR pszText)
{
	SetItemText(m_iEditRow, m_iEditCol, pszText);
}

bool CGridCtrl::AllowEnsureHorizontallyVisible(int iCol)
{
	UNREFERENCED_PARAMETER(iCol);
	return true;
}

BEGIN_MESSAGE_MAP(CGridCtrl, CDragVirtualListCtrl)
	//{{AFX_MSG_MAP(CGridCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_PARENTNOTIFY()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(CPopupEdit::UWM_TEXT_CHANGE, OnTextChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl message handlers

void CGridCtrl::OnDestroy() 
{
	EndEdit();	
	CDragVirtualListCtrl::OnDestroy();
}

void CGridCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	LVHITTESTINFO	info;
	info.pt = point;
	info.flags = UINT_MAX;
	int	item = SubItemHitTest(&info);
	if (item >= 0 && info.iSubItem > 0) {	// if clicked on a subitem
		SetFocus();	// required because base class isn't called; avoids crash or exception
		// SetFocus can fail, for example if window that's losing focus resets focus
		if (::GetFocus() == m_hWnd)	// if we actually have focus
			EditSubitem(item, info.iSubItem);	// edit subitem
	} else	// not on a subitem
		CDragVirtualListCtrl::OnLButtonDown(nFlags, point);	// delegate to base class
}

BOOL CGridCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (IsEditing()) {
		if (pMsg->message == WM_KEYDOWN) {
			switch (pMsg->wParam) {
			case VK_UP:
				if (GetKeyState(VK_CONTROL) & GKS_DOWN) {	// if control key down
					GotoSubitem(-1, 0);
					return(TRUE);
				}
				break;
			case VK_DOWN:
				if (GetKeyState(VK_CONTROL) & GKS_DOWN) {	// if control key down
					GotoSubitem(1, 0);
					return(TRUE);
				}
				break;
			case VK_TAB:
				if (!(GetKeyState(VK_CONTROL) & GKS_DOWN)) {	// if control key up
					int	nDeltaCol = (GetKeyState(VK_SHIFT) & GKS_DOWN) ? -1 : 1;
					GotoSubitem(0, nDeltaCol);
				}
				return(TRUE);
			case VK_RETURN:
				EndEdit();
				return(TRUE);
			case VK_ESCAPE:
				CancelEdit();
				return(TRUE);
			}
		}
	} else {	// not editing
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB
		&& !(GetKeyState(VK_SHIFT) & GKS_DOWN)	// if unshifted tab key pressed
		&& !(GetKeyState(VK_CONTROL) & GKS_DOWN)	// and control key up
		&& GetColumnCount() > 1) {	// and at least one subitem
			int	iRow = GetSelectionMark();
			if (iRow >= 0) {	// if current row valid
				int	iCol = 1;
				CIntArrayEx	arrOrder;
				if (GetColumnOrder(arrOrder))
					iCol = arrOrder[iCol];
				EditSubitem(iRow, iCol);	// edit row's first subitem
				return(TRUE);
			}
		}
	}
	return CDragVirtualListCtrl::PreTranslateMessage(pMsg);
}

void CGridCtrl::OnParentNotify(UINT message, LPARAM lParam) 
{
	CDragVirtualListCtrl::OnParentNotify(message, lParam);
	// the following ensures left-clicking in header control ends edit
	if (IsEditing()) {
		switch (LOWORD(message)) {	// high word may contain child window ID
		case WM_LBUTTONDOWN:
			{
				CRect	rEdit;
				if (m_pEditCtrl->IsKindOf(RUNTIME_CLASS(CEdit))) {	// if editing control is CEdit-derived
					// use subitem rect because it includes spin button control if any
					GetSubItemRect(m_iEditRow, m_iEditCol, LVIR_BOUNDS, rEdit);
					ClientToScreen(rEdit);	// for testing cursor position in screen coords
				} else {	// not an edit control; control may be bigger than subitem, e.g. combo box
					m_pEditCtrl->GetWindowRect(rEdit);	// use child control's rect
				}
				// don't rely on the lParam position, because we can't be sure which parent
				// window it's relative to; for a combo, it could be relative to either the
				// combo's parent (us), or the combo itself if its edit control was clicked
				if (!rEdit.PtInRect(GetCurrentMessage()->pt))	// if clicked outside of edit control
					EndEdit();
			}
			break;
		case WM_DESTROY:
			m_pEditCtrl = NULL;
			break;
		}
	}
}

void CGridCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (!IsChild(pScrollBar))	// if notifier isn't our child control (such as spin button)
		EndEdit();
	CDragVirtualListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGridCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (!IsChild(pScrollBar))	// if notifier isn't our child control (such as spin button)
		EndEdit();
	CDragVirtualListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

LRESULT CGridCtrl::OnTextChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	OnItemChange(LPCTSTR(wParam));
	return(0);
}
