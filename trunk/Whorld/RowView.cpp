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
		07		19jan07	in ReplaceRows, allow zero Rows arg
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRowView dialog

IMPLEMENT_DYNCREATE(CRowView, CView);

const CRect CRowView::m_Margin(0, 4, 0, 0);

#define RK_COLUMN_WIDTH _T("CW")	// column width registry key suffix

CRowView::CRowView()
{
	//{{AFX_DATA_INIT(CRowView)
	//}}AFX_DATA_INIT
	m_Cols = 0;
	m_ColInfo = NULL;
	m_Form = NULL;
	m_TopMargin = 0;
	m_HdrHeight = HEADER_HEIGHT;
	m_NotifyWnd = NULL;
	m_Accel = NULL;
	m_AccelWnd = NULL;
	m_Reorderable = FALSE;
	m_HaveScrollPos = FALSE;
	m_ScrollPos = CPoint(0, 0);
	m_RowDlgSize = CSize(0, 0);
	m_RowFirstCtrlX = 0;
	m_RowCtrlCount = 0;
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

bool CRowView::ReadColumnWidths(CColWidthArray& ColWidth) const
{
	int	cols = GetCols();
	ColWidth.SetSize(cols);
	int	nID = GetDlgCtrlID();
	CString	s(MAKEINTRESOURCE(nID));
	s += RK_COLUMN_WIDTH;
	DWORD	ExpectedSize = cols * sizeof(int);
	DWORD	size = ExpectedSize;
	return(CPersist::GetBinary(REG_SETTINGS, s, ColWidth.GetData(), &size)
		&& size == ExpectedSize);
}

bool CRowView::WriteColumnWidths() const
{
	int	cols = GetCols();
	CColWidthArray	ColWidth;
	ColWidth.SetSize(cols);
	for (int iCol = 0; iCol < cols; iCol++)
		ColWidth[iCol] = m_ColState[iCol].CurWidth;
	int	nID = GetDlgCtrlID();
	CString	s(MAKEINTRESOURCE(nID));
	s += RK_COLUMN_WIDTH;
	DWORD	size = cols * sizeof(int);
	return(CPersist::WriteBinary(REG_SETTINGS, s, ColWidth.GetData(), size) != 0);
}

bool CRowView::CreateCols(int Cols, const COLINFO *ColInfo, UINT RowDlgID)
{
	if (Cols <= 0 || m_Cols)
		return(FALSE);
	m_ColState.SetSize(Cols);
	m_Cols = Cols;
	m_ColInfo = ColInfo;
	CRect	cr, dr;
	CDialog	RowDlg;
	RowDlg.Create(RowDlgID, this);
	RowDlg.GetWindowRect(dr);	// get row dialog's rectangle
	ScreenToClient(dr);
	m_RowDlgSize = dr.Size();
	m_RowCtrlCount = Cols;		// one row control per column initially
	CColWidthArray	RegColWidth;
	bool	HaveRegColWidths = ReadColumnWidths(RegColWidth);
	int	x = 0;
	int	TotalCurWidth = 0;
	// one extra loop because column widths are set in arrears
	for (int iCol = 0; iCol <= Cols; iCol++) {
		if (iCol < Cols) {	// if not last loop
			CWnd	*RowCtrl = RowDlg.GetDlgItem(ColInfo[iCol].CtrlID);
			RowCtrl->GetWindowRect(cr);
			// convert control rectangle to row dialog's coords, not view's,
			// else header columns don't line up with controls in some cases
			RowDlg.ScreenToClient(cr);	// 14may11 added RowDlg
			m_ColState[iCol].CtrlInitWidth = cr.Width();
			// if control is an edit control
			TCHAR	szClassName[16];
			if (GetClassName(RowCtrl->m_hWnd, szClassName, 16) 
			&& !_tcsicmp(szClassName, _T("Edit"))) {
				// if next control is a spin control
				CWnd	*NextCtrl = RowCtrl->GetNextWindow();
				if (NextCtrl != NULL
				&& GetClassName(NextCtrl->m_hWnd, szClassName, 16)
				&& !_tcsicmp(szClassName, _T("msctls_updown32"))) {
					// only right-aligned auto-buddy spin controls are supported
					ASSERT(NextCtrl->GetStyle() & (UDS_ALIGNRIGHT | UDS_AUTOBUDDY));
					// spin control requires special handling because it
					// doesn't automatically move with its buddy edit control
					m_ColState[iCol].SpinCtrlID = NextCtrl->GetDlgCtrlID();
					CRect	sr;
					NextCtrl->GetWindowRect(sr);
					RowDlg.ScreenToClient(sr);
					m_ColState[iCol].SpinInitWidth = sr.Width();
					m_RowCtrlCount++;	// bump row control count
				}
			}
		} else	// last loop
			cr.left = dr.Width();	// last column boundary is row dialog width
		if (iCol > 0) {	// if not first column
			int	iPrevCol = iCol - 1;
			int	cx = cr.left - x;	// initial default column width
			m_ColState[iPrevCol].InitWidth = cx;
			if (HaveRegColWidths)
				cx = RegColWidth[iPrevCol];	// restore persistent column width
			m_ColState[iPrevCol].CurWidth = cx;
			TotalCurWidth += cx;
			HDITEM	item;
			item.mask = HDI_TEXT | HDI_FORMAT | HDI_WIDTH;
			CString	s(MAKEINTRESOURCE(ColInfo[iPrevCol].TitleID));
			item.pszText = s.GetBuffer(0);
			item.fmt = HDF_CENTER;
			item.cxy = cx;
			m_Hdr.InsertItem(iPrevCol, &item);
			x = cr.left;
		} else	// first column
			m_RowFirstCtrlX = cr.left;
	}
	// compensate row dialog width for non-default column widths
	int	TotalDeltaWidth = TotalCurWidth - x;
	m_RowDlgSize.cx += TotalDeltaWidth;
	m_Form->SetScrollSizes(MM_TEXT, CSize(0, 0));	// reset scroll bars
	MoveHeader();	// set header's initial position
	RowDlg.DestroyWindow();
	return(TRUE);
}

void CRowView::RemoveAllRows()
{
	if (!m_Row.GetSize())
		return;	// nothing to do
	for (int iRow = 0; iRow < m_Row.GetSize(); iRow++) {
		GetRow(iRow)->DestroyWindow();
		delete GetRow(iRow);
	}
	m_Row.RemoveAll();
	if (m_Form->m_hWnd) {	// dialog may have been destroyed
		m_Form->SetScrollSizes(MM_TEXT, CSize(0, 0));	// reset scroll bars
		MoveHeader();	// set header's initial position
	}
}

inline void CRowView::MoveRow(HDWP DeferPos, CRowDlg& Row, int Pos)
{
	CPoint	sp = m_Form->GetScrollPosition();	// compensate for scroll position
	UINT	flags = SWP_NOSIZE | SWP_SHOWWINDOW;
	DeferWindowPos(DeferPos, Row, HWND_BOTTOM, m_Margin.left - sp.x, 
		m_Margin.top + Pos * m_RowDlgSize.cy - sp.y, 0, 0, flags);
}

bool CRowView::CreateRows(int Rows)
{
	if (Rows < 0 || m_Form == NULL)	// form must already exist
		return(FALSE);
	// scroll BEFORE updating row dialogs to avoid painting them prematurely
	if (Rows) {
		// set view's scrollable area and update scroll bars
		CSize	ViewArea(m_Margin.left + m_RowDlgSize.cx + m_Margin.right,
			m_Margin.top + m_RowDlgSize.cy * Rows + m_Margin.bottom);
		m_Form->SetScrollSizes(MM_TEXT, ViewArea);
		RepositionBars(0, 0, AFX_IDW_PANE_FIRST, CWnd::reposDefault);
		// set window's maximum size
		CRect	cr;
		GetClientRect(cr);
		if (m_HaveScrollPos) {	// if target scroll position is valid
			// compensate scroll position for view area and client rect
			m_Form->GetClientRect(cr);
			CSize	MaxScrollPos(
				max(ViewArea.cx - cr.Width(), 0), 
				max(ViewArea.cy - cr.Height(), 0));
			m_ScrollPos.x = CLAMP(m_ScrollPos.x, 0, MaxScrollPos.cx);
			m_ScrollPos.y = CLAMP(m_ScrollPos.y, 0, MaxScrollPos.cy);
			m_Form->ScrollToPosition(m_ScrollPos);
			m_HaveScrollPos = FALSE;
			MoveHeader();
		}
	} else
		m_Form->SetScrollSizes(MM_TEXT, CSize(0, 0));
	// update and add/delete row dialogs
	int	PrevRows = GetRows();
	int	Updates = min(Rows, PrevRows);
	HDWP	DeferPos = BeginDeferWindowPos(Rows);	// defer positions to reduce flicker
	for (int iRow = 0; iRow < Updates; iRow++) {	// update existing rows
		CRowDlg	*rp = GetRow(iRow);
		int	PrevPos = rp->GetRowPos();	// save row position
		UpdateRow(iRow);
		int	pos = m_Reorderable ? rp->GetRowPos() : iRow;
		if (pos != PrevPos)	// if row position changed
			MoveRow(DeferPos, *rp, pos);
	}
	if (Rows > PrevRows) {	// if adding rows
		m_Row.SetSize(Rows);
		for (int iRow = PrevRows; iRow < Rows; iRow++) {
			CRowDlg	*rp = CreateRow(iRow);
			if (rp == NULL)
				return(FALSE);
			m_Row[iRow] = rp;
			rp->SetParent(m_Form);	// row scrolls with view
			rp->SetRowIndex(iRow);
			rp->SetDlgCtrlID(ROW_BASE_ID + iRow);
			int	pos = m_Reorderable ? rp->GetRowPos() : iRow;
			MoveRow(DeferPos, *rp, pos);
		}
		// find first column with non-default width, if any
		int iCol;
		int	nCols = GetCols();
		for (iCol = 0; iCol < nCols; iCol++) {
			if (m_ColState[iCol].CurWidth != m_ColState[iCol].InitWidth)
				break;
		}
		if (iCol < nCols) {	// if column resizing needed
			int	width = m_ColState[iCol].CurWidth;
			m_ColState[iCol].CurWidth = -1;	// spoof no-op test
			ResizeColumn(iCol, width, PrevRows);	// resize column
		}
	} else {	// deleting rows
		for (int iRow = Rows; iRow < PrevRows; iRow++) {
			GetRow(iRow)->DestroyWindow();
			delete GetRow(iRow);
		}
		m_Row.SetSize(Rows);
	}
	EndDeferWindowPos(DeferPos);
	m_HaveScrollPos = FALSE;
	return(TRUE);
}

bool CRowView::CreateRows(int Rows, CPoint ScrollPos)
{
	m_HaveScrollPos = TRUE;
	m_ScrollPos = ScrollPos;
	return(CreateRows(Rows));
}

CPoint CRowView::GetScrollPos() const
{
	return(m_Form->GetScrollPosition());
}

CRowDlg *CRowView::CreateRow(int Idx)
{
	UNREFERENCED_PARAMETER(Idx);
	return(NULL);
}

void CRowView::UpdateRow(int Idx)
{
	UNREFERENCED_PARAMETER(Idx);
}

void CRowView::MoveHeader()
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

int CRowView::GetActiveRow() const
{
	CWnd	*wp = GetFocus();
	if (wp != NULL) {
		int	rows = GetRows();
		for (int iRow = 0; iRow < rows; iRow++) {
			if (GetRow(iRow)->IsChild(wp))
				return(iRow);
		}
	}
	return(-1);
}

void CRowView::ResizeColumn(int ColIdx, int Width, int FirstRow)
{
	int	rows = GetRows();
	int	cols = GetCols();
	if (Width == m_ColState[ColIdx].CurWidth)	// if column width unchanged
		return;
	// build control span array
	int	x = 0;
	CCtrlSpanArray	CtrlSpan;
	CtrlSpan.SetSize(m_RowCtrlCount);	// count includes spin controls
	int	nCtrls = 0;
	int	iResizedCtrl = 0;
	for (int iCol = 0; iCol < cols; iCol++) {
		int	cx;
		COL_STATE&	state = m_ColState[iCol];
		if (iCol == ColIdx) {	// if column is resizing
			cx = Width;
			state.CurWidth = cx;	// set column's current width
			iResizedCtrl = nCtrls;	// save index of resized control
		} else	// column isn't resizing
			cx = state.CurWidth;	// use column's current width
		CTRL_SPAN	span;
		if (!iCol)	// if first column
			span.left = m_RowFirstCtrlX;
		else	// not first column
			span.left = x;
		int	gutter = state.InitWidth - state.CtrlInitWidth;
		span.nID = m_ColInfo[iCol].CtrlID;
		span.right = span.left + cx - gutter;
		span.right = max(span.right, x);	// avoid negative width
		CtrlSpan[nCtrls] = span;
		nCtrls++;
		if (state.SpinCtrlID) {	// if control has attached spin control
			CTRL_SPAN	ss;	// add spin control's span to span array
			const int SPIN_BUDDY_OVERLAP = 2;
			ss.nID = state.SpinCtrlID;
			ss.left = span.left + cx - gutter - SPIN_BUDDY_OVERLAP;
			ss.right = ss.left + m_ColState[iCol].SpinInitWidth;
			ss.left = max(ss.left, span.left);	// don't extend beyond buddy
			CtrlSpan[nCtrls] = ss;
			nCtrls++;
		}
		x += cx;
	}
	m_RowDlgSize.cx = x;	// update row dialog width
	// resize row dialogs and their controls
	CRect	r;
	m_Form->ShowWindow(SW_HIDE);	// somewhat reduces flicker
	HDWP	RowDeferPos = BeginDeferWindowPos(rows);
	for (int iRow = FirstRow; iRow < rows; iRow++) {
		CWnd	*pRow = GetRow(iRow);
		HDWP	DeferPos = BeginDeferWindowPos(nCtrls - iResizedCtrl);
		for (int iCtrl = iResizedCtrl; iCtrl < nCtrls; iCtrl++) {
			const CTRL_SPAN& span = CtrlSpan[iCtrl];
			CWnd	*pCtrl = pRow->GetDlgItem(span.nID);
			pCtrl->GetWindowRect(r);
			pRow->ScreenToClient(r);
			r.left = span.left;
			r.right = span.right;
			UINT	flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
			DeferPos = DeferWindowPos(DeferPos, pCtrl->m_hWnd, 
				NULL, r.left, r.top, r.Width(), r.Height(), flags);
//printf("%d %d %d %d\n", r.left, r.top, r.bottom, r.right);
		}
		EndDeferWindowPos(DeferPos);
		UINT	flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
		RowDeferPos = DeferWindowPos(RowDeferPos, pRow->m_hWnd, 
			NULL, 0, 0, m_RowDlgSize.cx, m_RowDlgSize.cy, flags);
	}
	EndDeferWindowPos(RowDeferPos);
	m_Form->ShowWindow(SW_SHOW);
}

void CRowView::SetColumnWidth(int ColIdx, int Width, UINT Flags)
{
	if (Width < 0)	// if width is LVSCW_AUTOSIZE
		Width = m_ColState[ColIdx].InitWidth;	// restore initial width
	ResizeColumn(ColIdx, Width);
	CSize	sz = m_Form->GetTotalSize();
	sz.cx = m_Margin.left + m_RowDlgSize.cx + m_Margin.right;
	m_Form->SetScrollSizes(MM_TEXT, sz);
	m_Form->UpdateWindow();	// prevents sloppy painting for large row counts
	if (Flags & SCW_RESIZE_HEADER) {	// if resizing header
		int	iFirstCol, iLastCol;
		if (Flags & SCW_ALL_COLUMNS) {	// if resizing all columns
			iFirstCol = 0; 
			iLastCol = GetCols();
		} else {	// resize specifed column only
			iFirstCol = ColIdx; 
			iLastCol = ColIdx + 1;
		}
		for (int iCol = iFirstCol; iCol < iLastCol; iCol++) {
			HDITEM	item;
			item.mask = HDI_WIDTH;
			item.cxy = m_ColState[iCol].CurWidth;
			m_Hdr.SetItem(iCol, &item);	// resize header item to match column
		}
	}
	MoveHeader();	// looks weird but standard behavior, same as list control
}

void CRowView::ResetColumnWidths()
{
	int	cols = GetCols();
	for (int iCol = 1; iCol < cols; iCol++)
		m_ColState[iCol].CurWidth = m_ColState[iCol].InitWidth;
	m_ColState[0].CurWidth = -1;	// spoof no-op test
	SetColumnWidth(0, LVSCW_AUTOSIZE, SCW_RESIZE_HEADER | SCW_ALL_COLUMNS);
}

void CRowView::OnDraw(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
}

BEGIN_MESSAGE_MAP(CRowView, CView)
	//{{AFX_MSG_MAP(CRowView)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	ON_NOTIFY(HDN_ITEMCHANGING, HEADER_ID, OnHdrItemChanging)
	ON_NOTIFY(HDN_DIVIDERDBLCLICK, HEADER_ID, OnHdrDividerDblClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRowView message handlers

int CRowView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// create form
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CRowForm);
	m_Form = DYNAMIC_DOWNCAST(CRowForm, pFactory->CreateObject());
	if (m_Form == NULL)
		return -1;
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
    CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_Form->Create(NULL, NULL, dwStyle, r, this, FORM_ID, NULL))
		return -1;

	// create header control
	GetClientRect(r);
	r.bottom = m_HdrHeight;
	m_Hdr.Create(HDS_HORZ | HDS_FULLDRAG, r, this, HEADER_ID);
	m_Hdr.SetFont(GetFont());
	m_Hdr.ShowWindow(SW_SHOW);
	m_Hdr.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	
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
	if (m_Form != NULL) {
		CRect	r;
		GetClientRect(r);
		r.top += m_HdrHeight + m_TopMargin;	// leave room for header
		m_Form->MoveWindow(r);
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
		CRect	r;
		CWnd	*pWnd = GetFocus();
		int	rows = GetRows();
		int	i;
		for (i = 0; i < rows; i++) {
			if (GetRow(i)->IsChild(pWnd)) {	// if one of our children has focus
				pWnd->GetWindowRect(r);	// position menu over child window
				break;
			}
		}
		if (i >= rows)	// if focused window wasn't one of our children
			GetWindowRect(r);	// position menu in top left corner of view
		point = r.TopLeft() + CSize(10, 10);	// offset looks nicer
		return(TRUE);
	}
	return(FALSE);
}

void CRowView::OnHdrItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER	phdr = (LPNMHEADER)pNMHDR;
	if (phdr->pitem->mask & HDI_WIDTH)	// if width changed
		SetColumnWidth(phdr->iItem, phdr->pitem->cxy, 0);	// don't resize header
	*pResult = 0;
}

void CRowView::OnHdrDividerDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER	phdr = (LPNMHEADER)pNMHDR;
	SetColumnWidth(phdr->iItem, LVSCW_AUTOSIZE);
	*pResult = 0;
}

void CRowView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ADD && (GetKeyState(VK_CONTROL) & GKS_DOWN))
		ResetColumnWidths();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
