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
		06		22dec06	create dialog bar variant from CRowDialog
		07		19jan07	in ReplaceRows, allow zero nRows arg
		08		11jul07	remove OnRowFrameTab
		09		11jul07	in ReplaceRows, replace MoveWindow with SetWindowPos
		10		23nov07	support Unicode
		11		20apr10	refactor
		12		14may11	in CreateCols, fix header column aligment
		13		11nov11	add FixContextMenuPos
		14		11nov11	fix keyboard-triggered context menu
		15		24nov11	add get/set scroll position
		16		21jan12	remove lock window update to fix desktop flicker
		17		21jan12	in CreateRows, scroll before updating row dialogs
		18		31mar12	in MoveRow, make defer pos an argument
		19		06apr12	add column resizing
		20		08feb25	row dialog create must specify parent
		21		10feb25	fix warnings on string ctor from resource ID
		22		10feb25	give child windows unique, non-zero control IDs

        row view
 
*/

// RowView.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RowView.h"
#include "RowDlg.h"
#include "RowForm.h"
#include "Persist.h"

// CRowView dialog

IMPLEMENT_DYNCREATE(CRowView, CView);

const CRect CRowView::m_rMargin(0, 4, 0, 0);

#define RK_COLUMN_WIDTH _T("CW")	// column width registry key suffix

CRowView::CRowView()
{
	m_nCols = 0;
	m_pColInfo = NULL;
	m_pForm = NULL;
	m_nTopMargin = 0;
	m_nHdrHeight = HEADER_HEIGHT;
	m_pNotifyWnd = NULL;
	m_hAccel = NULL;
	m_pAccelWnd = NULL;
	m_bReorderable = false;
	m_bHaveScrollPos = false;
	m_ptScrollPos = CPoint(0, 0);
	m_szRowDlg = CSize(0, 0);
	m_nRowFirstCtrlX = 0;
	m_nRowCtrlCount = 0;
}

BOOL CRowView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	CWinApp	*pApp = AfxGetApp();
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		pApp->LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		pApp->LoadIcon(IDR_MAINFRAME));		// app's icon
	return CView::PreCreateWindow(cs);
}

bool CRowView::ReadColumnWidths(CColWidthArray& aColWidth) const
{
	int	nCols = GetCols();
	aColWidth.SetSize(nCols);
	int	nID = GetDlgCtrlID();
	CString	sEntry(MAKEINTRESOURCE(nID));
	sEntry += RK_COLUMN_WIDTH;
	DWORD	nExpectedSize = nCols * sizeof(int);
	DWORD	nSize = nExpectedSize;
	return CPersist::GetBinary(REG_SETTINGS, sEntry, aColWidth.GetData(), &nSize)
		&& nSize == nExpectedSize;
}

bool CRowView::WriteColumnWidths() const
{
	int	nCols = GetCols();
	CColWidthArray	aColWidth;
	aColWidth.SetSize(nCols);
	for (int iCol = 0; iCol < nCols; iCol++)
		aColWidth[iCol] = m_aColState[iCol].nCurWidth;
	int	nID = GetDlgCtrlID();
	CString	sEntry(MAKEINTRESOURCE(nID));
	sEntry += RK_COLUMN_WIDTH;
	DWORD	nSize = nCols * sizeof(int);
	return CPersist::WriteBinary(REG_SETTINGS, sEntry, aColWidth.GetData(), nSize) != 0;
}

bool CRowView::CreateCols(int nCols, const COLINFO *pColInfo, UINT nRowDlgID)
{
	if (nCols <= 0 || m_nCols)
		return false;
	m_aColState.SetSize(nCols);
	m_nCols = nCols;
	m_pColInfo = pColInfo;
	CRect	rCtrl, rDlg;
	CDialog	dlgRow;
	dlgRow.Create(nRowDlgID, this);
	dlgRow.GetWindowRect(rDlg);	// get row dialog's rectangle
	ScreenToClient(rDlg);
	m_szRowDlg = rDlg.Size();
	m_nRowCtrlCount = nCols;		// one row control per column initially
	CColWidthArray	aRegColWidth;
	bool	bHaveRegColWidths = ReadColumnWidths(aRegColWidth);
	int	x = 0;
	int	nTotalCurWidth = 0;
	// one extra loop because column widths are set in arrears
	for (int iCol = 0; iCol <= nCols; iCol++) {
		if (iCol < nCols) {	// if not last loop
			CWnd	*pRowCtrl = dlgRow.GetDlgItem(pColInfo[iCol].nCtrlID);
			pRowCtrl->GetWindowRect(rCtrl);
			// convert control rectangle to row dialog's coords, not view's,
			// else header columns don't line up with controls in some cases
			dlgRow.ScreenToClient(rCtrl);	// 14may11 added dlgRow
			m_aColState[iCol].nCtrlInitWidth = rCtrl.Width();
			// if control is an edit control
			TCHAR	szClassName[16];
			if (GetClassName(pRowCtrl->m_hWnd, szClassName, 16) 
			&& !_tcsicmp(szClassName, _T("Edit"))) {
				// if next control is a spin control
				CWnd	*pNextCtrl = pRowCtrl->GetNextWindow();
				if (pNextCtrl != NULL
				&& GetClassName(pNextCtrl->m_hWnd, szClassName, 16)
				&& !_tcsicmp(szClassName, _T("msctls_updown32"))) {
					// only right-aligned auto-buddy spin controls are supported
					ASSERT(pNextCtrl->GetStyle() & (UDS_ALIGNRIGHT | UDS_AUTOBUDDY));
					// spin control requires special handling because it
					// doesn't automatically move with its buddy edit control
					m_aColState[iCol].nSpinCtrlID = pNextCtrl->GetDlgCtrlID();
					CRect	rSpin;
					pNextCtrl->GetWindowRect(rSpin);
					dlgRow.ScreenToClient(rSpin);
					m_aColState[iCol].nSpinInitWidth = rSpin.Width();
					m_nRowCtrlCount++;	// bump row control count
				}
			}
		} else {	// last loop
			rCtrl.left = rDlg.Width();	// last column boundary is row dialog width
		}
		if (iCol > 0) {	// if not first column
			int	iPrevCol = iCol - 1;
			int	cx = rCtrl.left - x;	// initial default column width
			m_aColState[iPrevCol].nInitWidth = cx;
			if (bHaveRegColWidths)
				cx = aRegColWidth[iPrevCol];	// restore persistent column width
			m_aColState[iPrevCol].nCurWidth = cx;
			nTotalCurWidth += cx;
			HDITEM	item;
			item.mask = HDI_TEXT | HDI_FORMAT | HDI_WIDTH;
			CString	sText(MAKEINTRESOURCE(pColInfo[iPrevCol].nTitleID));
			item.pszText = sText.GetBuffer(0);
			item.fmt = HDF_CENTER;
			item.cxy = cx;
			m_wndHdr.InsertItem(iPrevCol, &item);
			x = rCtrl.left;
		} else	// first column
			m_nRowFirstCtrlX = rCtrl.left;
	}
	// compensate row dialog width for non-default column widths
	int	nTotalDeltaWidth = nTotalCurWidth - x;
	m_szRowDlg.cx += nTotalDeltaWidth;
	m_pForm->SetScrollSizes(MM_TEXT, CSize(0, 0));	// reset scroll bars
	MoveHeader();	// set header's initial position
	dlgRow.DestroyWindow();
	return true;
}

void CRowView::RemoveAllRows()
{
	if (!m_aRow.GetSize())
		return;	// nothing to do
	for (int iRow = 0; iRow < m_aRow.GetSize(); iRow++) {
		GetRow(iRow)->DestroyWindow();
		delete GetRow(iRow);
	}
	m_aRow.RemoveAll();
	if (m_pForm->m_hWnd) {	// dialog may have been destroyed
		m_pForm->SetScrollSizes(MM_TEXT, CSize(0, 0));	// reset scroll bars
		MoveHeader();	// set header's initial position
	}
}

inline void CRowView::MoveRow(HDWP hDeferPos, CRowDlg& row, int iPos)
{
	CPoint	ptScroll = m_pForm->GetScrollPosition();	// compensate for scroll position
	UINT	nFlags = SWP_NOSIZE | SWP_SHOWWINDOW;
	DeferWindowPos(hDeferPos, row, HWND_BOTTOM, m_rMargin.left - ptScroll.x, 
		m_rMargin.top + iPos * m_szRowDlg.cy - ptScroll.y, 0, 0, nFlags);
}

bool CRowView::CreateRows(int nRows)
{
	if (nRows < 0 || m_pForm == NULL)	// form must already exist
		return false;
	// scroll BEFORE updating row dialogs to avoid painting them prematurely
	if (nRows) {
		// set view's scrollable area and update scroll bars
		CSize	szView(m_rMargin.left + m_szRowDlg.cx + m_rMargin.right,
			m_rMargin.top + m_szRowDlg.cy * nRows + m_rMargin.bottom);
		m_pForm->SetScrollSizes(MM_TEXT, szView);
		RepositionBars(0, 0, AFX_IDW_PANE_FIRST, CWnd::reposDefault);
		// set window's maximum size
		CRect	rCtrl;
		GetClientRect(rCtrl);
		if (m_bHaveScrollPos) {	// if target scroll position is valid
			// compensate scroll position for view area and client rect
			m_pForm->GetClientRect(rCtrl);
			CSize	MaxScrollPos(
				max(szView.cx - rCtrl.Width(), 0), 
				max(szView.cy - rCtrl.Height(), 0));
			m_ptScrollPos.x = CLAMP(m_ptScrollPos.x, 0, MaxScrollPos.cx);
			m_ptScrollPos.y = CLAMP(m_ptScrollPos.y, 0, MaxScrollPos.cy);
			m_pForm->ScrollToPosition(m_ptScrollPos);
			m_bHaveScrollPos = false;
			MoveHeader();
		}
	} else
		m_pForm->SetScrollSizes(MM_TEXT, CSize(0, 0));
	// update and add/delete row dialogs
	int	nPrevRows = GetRows();
	int	nUpdates = min(nRows, nPrevRows);
	HDWP	hDeferPos = BeginDeferWindowPos(nRows);	// defer positions to reduce flicker
	for (int iRow = 0; iRow < nUpdates; iRow++) {	// update existing rows
		CRowDlg	*pRow = GetRow(iRow);
		int	nPrevPos = pRow->GetRowPos();	// save row position
		UpdateRow(iRow);
		int	nPos = m_bReorderable ? pRow->GetRowPos() : iRow;
		if (nPos != nPrevPos)	// if row position changed
			MoveRow(hDeferPos, *pRow, nPos);
	}
	if (nRows > nPrevRows) {	// if adding rows
		m_aRow.SetSize(nRows);
		for (int iRow = nPrevRows; iRow < nRows; iRow++) {
			CRowDlg	*pRow = CreateRow(iRow);
			if (pRow == NULL)
				return false;
			m_aRow[iRow] = pRow;
			pRow->SetParent(m_pForm);	// row scrolls with view
			pRow->SetRowIndex(iRow);
			pRow->SetDlgCtrlID(ROW_BASE_ID + iRow);
			int	nPos = m_bReorderable ? pRow->GetRowPos() : iRow;
			MoveRow(hDeferPos, *pRow, nPos);
		}
		// find first column with non-default width, if any
		int iCol;
		int	nCols = GetCols();
		for (iCol = 0; iCol < nCols; iCol++) {
			if (m_aColState[iCol].nCurWidth != m_aColState[iCol].nInitWidth)
				break;
		}
		if (iCol < nCols) {	// if column resizing needed
			int	nWidth = m_aColState[iCol].nCurWidth;
			m_aColState[iCol].nCurWidth = -1;	// spoof no-op test
			ResizeColumn(iCol, nWidth, nPrevRows);	// resize column
		}
	} else {	// deleting rows
		for (int iRow = nRows; iRow < nPrevRows; iRow++) {
			GetRow(iRow)->DestroyWindow();
			delete GetRow(iRow);
		}
		m_aRow.SetSize(nRows);
	}
	EndDeferWindowPos(hDeferPos);
	m_bHaveScrollPos = false;
	return true;
}

bool CRowView::CreateRows(int nRows, CPoint ptScrollPos)
{
	m_bHaveScrollPos = true;
	m_ptScrollPos = ptScrollPos;
	return CreateRows(nRows);
}

CPoint CRowView::GetScrollPos() const
{
	return m_pForm->GetScrollPosition();
}

CRowDlg *CRowView::CreateRow(int iRow)
{
	UNREFERENCED_PARAMETER(iRow);
	return NULL;
}

void CRowView::UpdateRow(int iRow)
{
	UNREFERENCED_PARAMETER(iRow);
}

void CRowView::MoveHeader()
{
	CRect	rForm, rView;
	GetClientRect(rForm);
	m_pForm->GetWindowRect(rView);
	ScreenToClient(rView);
	rForm.left -= m_pForm->GetScrollPosition().x;
	rForm.bottom = rView.top;
	rForm.top = rView.top - m_nHdrHeight;
	m_wndHdr.MoveWindow(rForm);
}

int CRowView::GetActiveRow() const
{
	CWnd	*pWnd = GetFocus();
	if (pWnd != NULL) {
		int	nRows = GetRows();
		for (int iRow = 0; iRow < nRows; iRow++) {
			if (GetRow(iRow)->IsChild(pWnd))
				return iRow;
		}
	}
	return -1;
}

void CRowView::ResizeColumn(int iColumn, int nWidth, int iFirstRow)
{
	int	nRows = GetRows();
	int	nCols = GetCols();
	if (nWidth == m_aColState[iColumn].nCurWidth)	// if column width unchanged
		return;
	// build control span array
	int	x = 0;
	CCtrlSpanArray	CtrlSpan;
	CtrlSpan.SetSize(m_nRowCtrlCount);	// count includes spin controls
	int	nCtrls = 0;
	int	iResizedCtrl = 0;
	for (int iCol = 0; iCol < nCols; iCol++) {
		int	cx;
		COL_STATE&	state = m_aColState[iCol];
		if (iCol == iColumn) {	// if column is resizing
			cx = nWidth;
			state.nCurWidth = cx;	// set column's current width
			iResizedCtrl = nCtrls;	// save index of resized control
		} else	// column isn't resizing
			cx = state.nCurWidth;	// use column's current width
		CTRL_SPAN	span;
		if (!iCol)	// if first column
			span.left = m_nRowFirstCtrlX;
		else	// not first column
			span.left = x;
		int	gutter = state.nInitWidth - state.nCtrlInitWidth;
		span.nID = m_pColInfo[iCol].nCtrlID;
		span.right = span.left + cx - gutter;
		span.right = max(span.right, x);	// avoid negative width
		CtrlSpan[nCtrls] = span;
		nCtrls++;
		if (state.nSpinCtrlID) {	// if control has attached spin control
			CTRL_SPAN	ss;	// add spin control's span to span array
			const int SPIN_BUDDY_OVERLAP = 2;
			ss.nID = state.nSpinCtrlID;
			ss.left = span.left + cx - gutter - SPIN_BUDDY_OVERLAP;
			ss.right = ss.left + m_aColState[iCol].nSpinInitWidth;
			ss.left = max(ss.left, span.left);	// don't extend beyond buddy
			CtrlSpan[nCtrls] = ss;
			nCtrls++;
		}
		x += cx;
	}
	m_szRowDlg.cx = x;	// update row dialog width
	// resize row dialogs and their controls
	CRect	r;
	m_pForm->ShowWindow(SW_HIDE);	// somewhat reduces flicker
	HDWP	RowDeferPos = BeginDeferWindowPos(nRows);
	for (int iRow = iFirstRow; iRow < nRows; iRow++) {
		CWnd	*pRow = GetRow(iRow);
		HDWP	hDeferPos = BeginDeferWindowPos(nCtrls - iResizedCtrl);
		for (int iCtrl = iResizedCtrl; iCtrl < nCtrls; iCtrl++) {
			const CTRL_SPAN& span = CtrlSpan[iCtrl];
			CWnd	*pCtrl = pRow->GetDlgItem(span.nID);
			pCtrl->GetWindowRect(r);
			pRow->ScreenToClient(r);
			r.left = span.left;
			r.right = span.right;
			UINT	nFlags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
			hDeferPos = DeferWindowPos(hDeferPos, pCtrl->m_hWnd, 
				NULL, r.left, r.top, r.Width(), r.Height(), nFlags);
//printf("%d %d %d %d\n", r.left, r.top, r.bottom, r.right);
		}
		EndDeferWindowPos(hDeferPos);
		UINT	nFlags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
		RowDeferPos = DeferWindowPos(RowDeferPos, pRow->m_hWnd, 
			NULL, 0, 0, m_szRowDlg.cx, m_szRowDlg.cy, nFlags);
	}
	EndDeferWindowPos(RowDeferPos);
	m_pForm->ShowWindow(SW_SHOW);
}

void CRowView::SetColumnWidth(int iColumn, int nWidth, UINT nFlags)
{
	if (nWidth < 0)	// if width is LVSCW_AUTOSIZE
		nWidth = m_aColState[iColumn].nInitWidth;	// restore initial width
	ResizeColumn(iColumn, nWidth);
	CSize	sz = m_pForm->GetTotalSize();
	sz.cx = m_rMargin.left + m_szRowDlg.cx + m_rMargin.right;
	m_pForm->SetScrollSizes(MM_TEXT, sz);
	m_pForm->UpdateWindow();	// prevents sloppy painting for large row counts
	if (nFlags & SCW_RESIZE_HEADER) {	// if resizing header
		int	iFirstCol, iLastCol;
		if (nFlags & SCW_ALL_COLUMNS) {	// if resizing all columns
			iFirstCol = 0; 
			iLastCol = GetCols();
		} else {	// resize specifed column only
			iFirstCol = iColumn; 
			iLastCol = iColumn + 1;
		}
		for (int iCol = iFirstCol; iCol < iLastCol; iCol++) {
			HDITEM	item;
			item.mask = HDI_WIDTH;
			item.cxy = m_aColState[iCol].nCurWidth;
			m_wndHdr.SetItem(iCol, &item);	// resize header item to match column
		}
	}
	MoveHeader();	// looks weird but standard behavior, same as list control
}

void CRowView::ResetColumnWidths()
{
	int	nCols = GetCols();
	for (int iCol = 1; iCol < nCols; iCol++)
		m_aColState[iCol].nCurWidth = m_aColState[iCol].nInitWidth;
	m_aColState[0].nCurWidth = -1;	// spoof no-op test
	SetColumnWidth(0, LVSCW_AUTOSIZE, SCW_RESIZE_HEADER | SCW_ALL_COLUMNS);
}

void CRowView::OnDraw(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
}

BEGIN_MESSAGE_MAP(CRowView, CView)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_NOTIFY(HDN_ITEMCHANGING, HEADER_ID, OnHdrItemChanging)
	ON_NOTIFY(HDN_DIVIDERDBLCLICK, HEADER_ID, OnHdrDividerDblClick)
END_MESSAGE_MAP()

// CRowView message handlers

int CRowView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// create form
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CRowForm);
	m_pForm = DYNAMIC_DOWNCAST(CRowForm, pFactory->CreateObject());
	if (m_pForm == NULL)
		return -1;
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
    CRect rect(0, 0, 0, 0);	// arbitrary initial size
    if (!m_pForm->Create(NULL, NULL, dwStyle, rect, this, FORM_ID, NULL))
		return -1;

	// create header control
	GetClientRect(rect);
	rect.bottom = m_nHdrHeight;
	m_wndHdr.Create(HDS_HORZ | HDS_FULLDRAG, rect, this, HEADER_ID);
	m_wndHdr.SetFont(GetFont());
	m_wndHdr.ShowWindow(SW_SHOW);
	m_wndHdr.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	
	return 0;
}

void CRowView::OnDestroy() 
{
	WriteColumnWidths();
	RemoveAllRows();
	CView::OnDestroy();
}

void CRowView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	if (m_pForm != NULL) {
		CRect	rc;
		GetClientRect(rc);
		rc.top += m_nHdrHeight + m_nTopMargin;	// leave room for header
		m_pForm->MoveWindow(rc);
		MoveHeader();
	}
}

void CRowView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	MoveHeader();
	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

bool CRowView::FixContextMenuPos(CPoint& point) const
{
	if (point.x == -1 && point.y == -1) {	// if menu triggered via keyboard
		CRect	rWnd;
		CWnd	*pWnd = GetFocus();
		int	nRows = GetRows();
		int	i;
		for (i = 0; i < nRows; i++) {
			if (GetRow(i)->IsChild(pWnd)) {	// if one of our children has focus
				pWnd->GetWindowRect(rWnd);	// position menu over child window
				break;
			}
		}
		if (i >= nRows)	// if focused window wasn't one of our children
			GetWindowRect(rWnd);	// position menu in top left corner of view
		point = rWnd.TopLeft() + CSize(10, 10);	// offset looks nicer
		return true;
	}
	return false;
}

void CRowView::OnHdrItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER	pHdr = (LPNMHEADER)pNMHDR;
	if (pHdr->pitem->mask & HDI_WIDTH)	// if width changed
		SetColumnWidth(pHdr->iItem, pHdr->pitem->cxy, 0);	// don't resize header
	*pResult = 0;
}

void CRowView::OnHdrDividerDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER	pHdr = (LPNMHEADER)pNMHDR;
	SetColumnWidth(pHdr->iItem, LVSCW_AUTOSIZE);
	*pResult = 0;
}

void CRowView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ADD && (GetKeyState(VK_CONTROL) & GKS_DOWN))
		ResetColumnWidths();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
