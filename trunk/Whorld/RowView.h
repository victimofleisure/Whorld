// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15jul05	initial version
		01		17feb06	add ReplaceRows and RemoveAllRows
		02		22dec06	create dialog bar variant from CRowDialog
		03		11jul07	remove OnRowFrameTab
		04		23nov07	support Unicode
		05		20apr10	refactor
		06		11nov11	add FixContextMenuPos
		07		24nov11	add get/set scroll position
		08		21jan12	add row dialog size
		09		31mar12	in MoveRow, make defer pos an argument
		10		06apr12	add column resizing
		11		10feb25	give child windows unique, non-zero control IDs

        row view
 
*/

#pragma once

// RowView.h : header file
//

// CRowView dialog

#include "ArrayEx.h"

class CRowForm;
class CRowDlg;

class CRowView : public CView
{
	DECLARE_DYNCREATE(CRowView);
// Construction
public:
	CRowView();

// Types
	typedef struct tagCOLINFO {
		int		nCtrlID;
		int		nTitleID;
	} COLINFO; 

// Constants
	enum {	// set column width flags
		SCW_RESIZE_HEADER	= 0x01,
		SCW_ALL_COLUMNS		= 0x02,
	};

// Attributes
	int		GetRows() const;
	int		GetCols() const;
	CRowDlg	*GetRow(int iRow) const;
	CHeaderCtrl&	GetHeader();
	int		GetHeaderHeight() const;
	int		GetTopMargin() const;
	void	SetTopMargin(int nMargin);
	int		GetActiveRow() const;
	CWnd	*GetNotifyWnd() const;
	void	SetNotifyWnd(CWnd *pWnd);
	HACCEL	GetAccel(CWnd*& pAccelWnd) const;
	void	SetAccel(HACCEL hAccel, CWnd *pAccelWnd);
	bool	GetReorderable() const;
	void	SetReorderable(bool bEnable);
	CPoint	GetScrollPos() const;
	int		GetColumnWidth(int iColumn) const;
	void	SetColumnWidth(int iColumn, int nWidth, UINT nFlags = SCW_RESIZE_HEADER);

// Operations
	bool	CreateCols(int nCols, const COLINFO *pColInfo, UINT nRowDlgID);
	bool	CreateRows(int nRows);
	bool	CreateRows(int nRows, CPoint ptScrollPos);
	void	RemoveAllRows();
	bool	FixContextMenuPos(CPoint& point) const;
	void	ResetColumnWidths();

// Overrides
	// ClassWizard generated virtual function overrides
	public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
protected:
// Generated message map functions
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHdrItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdrDividerDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

// Types
	struct COL_STATE {
		int		nInitWidth;		// column's initial width
		int		nCurWidth;		// column's current width
		int		nCtrlInitWidth;	// initial width of column's control
		int		nSpinCtrlID;	// ID of attached spin control, if any
		int		nSpinInitWidth;	// initial width of attached spin control
	};
	struct CTRL_SPAN {
		int		nID;			// control's resource ID
		int		left;			// span's left coordinate
		int		right;			// span's right coordinate
	};
	typedef CArrayEx<CRowDlg *, CRowDlg *> CRowDlgArray;
	typedef CArrayEx<COL_STATE, COL_STATE&> CColStateArray;
	typedef CArrayEx<CTRL_SPAN, CTRL_SPAN&> CCtrlSpanArray;
	typedef CArrayEx<int, int> CColWidthArray;

// Constants
	static const CRect	m_rMargin;	// extra margins around view
	enum {
		HEADER_ID = 0x1234,		// non-zero to distinguish from CListCtrl header
		FORM_ID = 0x1235,		// form dialog's control ID
		ROW_BASE_ID = 0x10,		// base control ID for row dialogs
		HEADER_HEIGHT = 18,		// height of header control, in logical coords
	};

// Member data
	int		m_nCols;			// number of columns in list
	const	COLINFO	*m_pColInfo;	// array of information about each column
	CRowForm	*m_pForm;		// pointer to form view, parent of rows
	CRowDlgArray	m_aRow;		// array of pointers to row dialogs
	CHeaderCtrl	m_wndHdr;		// list's header control
	int		m_nTopMargin;		// extra top margin for derived controls
	int		m_nHdrHeight;		// height of header control
	CWnd	*m_pNotifyWnd;		// notifications are sent to this window
	HACCEL	m_hAccel;			// if non-null, row's keyboard accelerators
	CWnd	*m_pAccelWnd;		// accelerators are translated by this window
	bool	m_bReorderable;		// true if rows can be reordered
	bool	m_bHaveScrollPos;	// true if target scroll position is valid
	CPoint	m_ptScrollPos;		// target scroll position if any
	CSize	m_szRowDlg;			// size of row dialog in client coords
	CColStateArray	m_aColState;	// array of column states
	int		m_nRowFirstCtrlX;	// x-coord of row's first control
	int		m_nRowCtrlCount;	// total number of controls in row

// Overridables
	virtual	CRowDlg	*CreateRow(int iRow);
	virtual	void	UpdateRow(int iRow);

// Overrides

// Helpers
	void	MoveHeader();
	void	MoveRow(HDWP DeferPos, CRowDlg& row, int iPos);
	void	ResizeColumn(int iColumn, int nWidth, int iFirstRow = 0);
	bool	ReadColumnWidths(CColWidthArray& aColWidth) const;
	bool	WriteColumnWidths() const;
};

inline int CRowView::GetRows() const
{
	return m_aRow.GetSize();
}

inline int CRowView::GetCols() const
{
	return m_nCols;
}

inline CRowDlg *CRowView::GetRow(int iRow) const
{
	return (CRowDlg *)m_aRow[iRow];
}

inline CHeaderCtrl& CRowView::GetHeader()
{
	return m_wndHdr;
}

inline int CRowView::GetHeaderHeight() const
{
	return m_nHdrHeight;
}

inline void CRowView::SetTopMargin(int nMargin)
{
	m_nTopMargin = nMargin;
}

inline int CRowView::GetTopMargin() const
{
	return m_nTopMargin;
}

inline CWnd *CRowView::GetNotifyWnd() const
{
	return m_pNotifyWnd;
}

inline void CRowView::SetNotifyWnd(CWnd *pWnd)
{
	m_pNotifyWnd = pWnd;
}

inline HACCEL CRowView::GetAccel(CWnd*& pAccelWnd) const
{
	pAccelWnd = m_pAccelWnd;
	return m_hAccel;
}

inline void CRowView::SetAccel(HACCEL hAccel, CWnd *pAccelWnd)
{
	m_hAccel = hAccel;
	m_pAccelWnd = pAccelWnd;
}

inline bool CRowView::GetReorderable() const
{
	return m_bReorderable;
}

inline void CRowView::SetReorderable(bool bEnable)
{
	m_bReorderable = bEnable;
}

inline int CRowView::GetColumnWidth(int iColumn) const
{
	return m_aColState[iColumn].nCurWidth;
}
