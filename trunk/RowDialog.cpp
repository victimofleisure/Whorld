// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15jul05	initial version
		01		21jul05	CreateRows must reposition scroll bars
        02      16aug05	add GetActiveRow, IsTabStop
		03		06sep05	disabled controls can't be tab stops
		04		12sep05	in tab handler, get position from row
		05		17feb06	add ReplaceRows and RemoveAllRows
		06		28jan08	support Unicode

        create a form view of row windows within a dialog
 
*/

// RowDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RowDialog.h"
#include "RowDialogRow.h"
#include "RowDialogForm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRowDialog dialog

IMPLEMENT_DYNAMIC(CRowDialog, CViewDialog);

const CRect CRowDialog::m_Margin(0, 4, -4, 0);

CRowDialog::CRowDialog(UINT nIDTemplate, UINT nIDAccel, LPCTSTR RegKey, CWnd* pParent /*=NULL*/)
	: CViewDialog(nIDTemplate, nIDAccel, RegKey, pParent)
{
	//{{AFX_DATA_INIT(CRowDialog)
	//}}AFX_DATA_INIT
	m_Cols = 0;
	m_ColInfo = NULL;
	m_Form = NULL;
	m_TopMargin = 0;
	m_HdrHeight = HEADER_HEIGHT;
}

bool CRowDialog::CreateCols(int Cols, const COLINFO *ColInfo)
{
	if (Cols <= 0 || ColInfo == NULL || m_Cols)
		return(FALSE);
	m_Cols = Cols;
	m_ColInfo = ColInfo;
	for (int i = 0; i < Cols; i++) {
		HDITEM	item;
		ZeroMemory(&item, sizeof(item));
		item.mask = HDI_TEXT | HDI_FORMAT;
		CString	s((LPCTSTR)ColInfo[i].TitleID);
		item.pszText = s.GetBuffer(0);
		item.fmt = HDF_CENTER;
		m_Hdr.InsertItem(i, &item);
	}
	return(TRUE);
}

void CRowDialog::RemoveAllRows()
{
	if (!m_Row.GetSize())
		return;	// nothing to do
	for (int i = 0; i < m_Row.GetSize(); i++) {
		GetRow(i)->DestroyWindow();
		delete GetRow(i);
	}
	m_Row.RemoveAll();
	if (IsWindow(m_Form->m_hWnd))	// dialog may have been destroyed
		m_Form->SetScrollSizes(MM_TEXT, CSize(0, 0));	// reset scroll bars
}

bool CRowDialog::ReplaceRows(int Rows)
{
	if (Rows <= 0 || m_Form == NULL)	// form must already exist
		return(FALSE);
	RemoveAllRows();
	// create rows
	CRect	r;
	m_Row.SetSize(Rows);
	for (int i = 0; i < Rows; i++) {
		int	Pos = i;	// position defaults to index
		CRowDialogRow	*rp = DYNAMIC_DOWNCAST(CRowDialogRow, CreateRow(i, Pos));
		if (rp == NULL)
			return(FALSE);
		rp->SetNotifyWnd(this, m_Accel);	// request notifications
		rp->SetRowIndex(i);
		rp->SetRowPos(Pos);
		m_Row[i] = rp;
		rp->SetParent(m_Form);	// row scrolls with view
		rp->GetWindowRect(r);
		ScreenToClient(r);
		rp->MoveWindow(m_Margin.left, m_Margin.top + Pos * r.Height(), 
			r.Width() + m_Margin.right, r.Height());
		rp->ShowWindow(SW_SHOW);
	}
	// set view's scrollable area and update scroll bars
	CSize	ViewArea(m_Margin.left + r.Width() + m_Margin.right,
		m_Margin.top + r.Height() * Rows + m_Margin.bottom);
	m_Form->SetScrollSizes(MM_TEXT, ViewArea);
	RepositionBars(0, 0, AFX_IDW_PANE_FIRST, CWnd::reposDefault);
	// set window's maximum size
	CRect	wr, cr;
	GetWindowRect(wr);
	ScreenToClient(wr);
	GetClientRect(cr);
	m_MaxSize = wr.Size() - cr.Size() + ViewArea +
		CSize(SCROLLBAR_WIDTH, m_TopMargin + m_HdrHeight + SCROLLBAR_WIDTH);
	return(TRUE);
}

bool CRowDialog::CreateRows(int Rows, int TopMargin)
{
	if (Rows <= 0 || !m_Cols || m_Form != NULL)
		return(FALSE);
	m_TopMargin = TopMargin;
	// create view
	CCreateContext	ct;
	ct.m_pNewViewClass = RUNTIME_CLASS(CRowDialogForm);
	if (CreateView(&ct) == NULL)
		return(FALSE);
	m_Form = DYNAMIC_DOWNCAST(CRowDialogForm, m_View);
	if (m_Form == NULL)
		return(FALSE);
	if (!ReplaceRows(Rows))
		return(FALSE);
	// set header column widths
	int	x = 0;
	CRect	cr, lrr;
	GetRow(Rows - 1)->GetWindowRect(lrr);	// get last row's rectangle
	ScreenToClient(lrr);
	for (int i = 0; i < GetCols(); i++) {
		HDITEM	item;
		ZeroMemory(&item, sizeof(item));
		item.mask = HDI_WIDTH;
		if (i < GetCols() - 1) {
			CWnd	*wp = GetRow(0)->GetDlgItem(m_ColInfo[i + 1].CtrlID);
			wp->GetWindowRect(cr);
			ScreenToClient(cr);
			item.cxy = cr.left - x;
			x = cr.left;
		} else
			item.cxy = lrr.right - x;
		m_Hdr.SetItem(i, &item);
	}
	MoveHeader();	// set header's initial position
	GetRow(0)->SetFocus();	// move to first control
	return(TRUE);
}

CWnd *CRowDialog::CreateRow(int Idx, int& Pos)
{
	return(NULL);
}

void CRowDialog::DoDataExchange(CDataExchange* pDX)
{
	CViewDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRowDialog)
	//}}AFX_DATA_MAP
}

void CRowDialog::MoveHeader()
{
	CRect	fr, vr;
	GetClientRect(fr);
	m_Form->GetWindowRect(vr);
	ScreenToClient(vr);
	fr.left -= m_Form->GetScrollPosition().x;
	fr.bottom = vr.top;
	fr.top = vr.top - m_HdrHeight;
	m_Hdr.MoveWindow(fr);
}

bool CRowDialog::IsTabStop(int Col)
{
	ASSERT(GetRows() > 0);
	CDialog	*dp = DYNAMIC_DOWNCAST(CDialog, GetRow(0));	// row must be a dialog
	if (dp != NULL) {
		CWnd	*Ctrl = dp->GetDlgItem(m_ColInfo[Col].CtrlID);
		DWORD	Style;
		return ((Style = Ctrl->GetStyle()) != NULL &&
			(Style & WS_TABSTOP) && !(Style & WS_DISABLED));
	}
	return(FALSE);
}

int CRowDialog::GetActiveRow() const
{
	CWnd	*wp = GetFocus();
	if (wp != NULL) {
		for (int i = 0; i < GetRows(); i++) {
			if (GetRow(i)->IsChild(wp))
				return(i);
		}
	}
	return(-1);
}

BEGIN_MESSAGE_MAP(CRowDialog, CViewDialog)
	//{{AFX_MSG_MAP(CRowDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_ROWDIALOGTAB, OnRowDialogTab)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRowDialog message handlers

BOOL CRowDialog::OnInitDialog() 
{
	CViewDialog::OnInitDialog();
	
	// create header control
	CRect	r;
	GetClientRect(r);
	r.bottom = m_HdrHeight;
	m_Hdr.Create(HDS_HORZ, r, this, 0);
	m_Hdr.SetFont(GetFont());
	m_Hdr.ShowWindow(SW_SHOW);
	m_Hdr.EnableWindow(FALSE);	// prevent divider dragging

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CRowDialog::OnDestroy() 
{
	CViewDialog::OnDestroy();
	RemoveAllRows();
}

void CRowDialog::OnSize(UINT nType, int cx, int cy) 
{
	CPersistDlg::OnSize(nType, cx, cy);	// skip a level in class hierarchy
	if (m_Form != NULL) {
		CRect	r;
		GetClientRect(r);
		r.top += m_HdrHeight + m_TopMargin;	// leave room for header
		m_Form->MoveWindow(r);
		MoveHeader();
	}
}

void CRowDialog::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (m_Form != NULL) {
		lpMMI->ptMaxTrackSize.x = m_MaxSize.cx;
		lpMMI->ptMaxTrackSize.y = m_MaxSize.cy;
	}
}

void CRowDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	MoveHeader();
	CViewDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

LRESULT CRowDialog::OnRowDialogTab(WPARAM wParam, LPARAM lParam)
{
	int	Idx = wParam;
	int	Rows = GetRows();
	ASSERT(Idx >= 0 && Idx <= Rows);
	int	EndCtrl[2] = {0, 0};
	int	i;
	for (i = 0; i < GetCols(); i++) {	// find first tab stop
		if (IsTabStop(i)) {
			EndCtrl[0] = m_ColInfo[i].CtrlID;
			break;
		}
	}
	for (i = GetCols() - 1; i >= 0; i--) {	// find last tab stop
		if (IsTabStop(i)) {
			EndCtrl[1] = m_ColInfo[i].CtrlID;
			break;
		}
	}
	bool	BackTab = (GetAsyncKeyState(VK_SHIFT) & GKS_DOWN) != 0;
	CWnd	*wp = GetFocus();
	if (wp != NULL && wp == GetRow(Idx)->GetDlgItem(EndCtrl[!BackTab])) {
		CRowDialogRow	*rp = DYNAMIC_DOWNCAST(CRowDialogRow, GetRow(Idx));
		if (rp != NULL) {	// make sure it's a valid row
			int Row = rp->GetRowPos();	// row storage and display orders may differ
			Row += BackTab ? -1 : 1;	// next or prev row
			if (Row < 0)				// wraparound
				Row = Rows - 1;
			else if (Row >= Rows)
				Row = 0;
			for (i = 0; i < Rows; i++) {	// search for a row at the new position
				rp = DYNAMIC_DOWNCAST(CRowDialogRow, GetRow(i));
				if (rp != NULL && rp->GetRowPos() == Row)
					break;
			}
			if (i < Rows) {	// if we found the next row
				wp = rp->GetDlgItem(EndCtrl[BackTab]);
				if (wp != NULL) {
					rp->GotoDlgCtrl(wp);
					return(TRUE);	// tell row we handled it
				}
			}
		}
	}
	return(FALSE);	// defer to row
}
