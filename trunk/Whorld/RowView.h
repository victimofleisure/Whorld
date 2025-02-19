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

#if !defined(AFX_ROWVIEW_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
#define AFX_ROWVIEW_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RowView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
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
		int		CtrlID;
		int		TitleID;
	} COLINFO; 

// Constants
	enum {	// set column width flags
		SCW_RESIZE_HEADER	= 0x01,
		SCW_ALL_COLUMNS		= 0x02,
	};

// Attributes
	int		GetRows() const;
	int		GetCols() const;
	CRowDlg	*GetRow(int Idx) const;
	CHeaderCtrl&	GetHeader();
	int		GetHeaderHeight() const;
	int		GetTopMargin() const;
	void	SetTopMargin(int Margin);
	int		GetActiveRow() const;
	CWnd	*GetNotifyWnd() const;
	void	SetNotifyWnd(CWnd *Wnd);
	HACCEL	GetAccel(CWnd*& AccelWnd) const;
	void	SetAccel(HACCEL Accel, CWnd *AccelWnd);
	bool	GetReorderable() const;
	void	SetReorderable(bool Enable);
	CPoint	GetScrollPos() const;
	int		GetColumnWidth(int ColIdx) const;
	void	SetColumnWidth(int ColIdx, int Width, UINT Flags = SCW_RESIZE_HEADER);

// Operations
	bool	CreateCols(int Cols, const COLINFO *ColInfo, UINT RowDlgID);
	bool	CreateRows(int Rows);
	bool	CreateRows(int Rows, CPoint ScrollPos);
	void	RemoveAllRows();
	bool	FixContextMenuPos(CPoint& point) const;
	void	ResetColumnWidths();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRowView)
	public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CRowView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg void OnHdrItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHdrDividerDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

// Types
	struct COL_STATE {
		int		InitWidth;		// column's initial width
		int		CurWidth;		// column's current width
		int		CtrlInitWidth;	// initial width of column's control
		int		SpinCtrlID;		// ID of attached spin control, if any
		int		SpinInitWidth;	// initial width of attached spin control
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
	static const CRect	m_Margin;	// extra margins around view
	enum {
		HEADER_ID = 0x1234,		// non-zero to distinguish from CListCtrl header
		FORM_ID = 0x1235,		// form dialog's control ID
		ROW_BASE_ID = 0x10,		// base control ID for row dialogs
		HEADER_HEIGHT = 18,	// height of header control, in logical coords
	};

// Member data
	int		m_Cols;				// number of columns in list
	const	COLINFO	*m_ColInfo;	// array of information about each column
	CRowForm	*m_Form;		// pointer to form view, parent of rows
	CRowDlgArray	m_Row;		// array of pointers to row dialogs
	CHeaderCtrl	m_Hdr;			// list's header control
	int		m_TopMargin;		// extra top margin for derived controls
	int		m_HdrHeight;		// height of header control
	CWnd	*m_NotifyWnd;		// notifications are sent to this window
	HACCEL	m_Accel;			// if non-null, row's keyboard accelerators
	CWnd	*m_AccelWnd;		// accelerators are translated by this window
	bool	m_Reorderable;		// true if rows can be reordered
	bool	m_HaveScrollPos;	// true if target scroll position is valid
	CPoint	m_ScrollPos;		// target scroll position if any
	CSize	m_RowDlgSize;		// size of row dialog in client coords
	CColStateArray	m_ColState;	// array of column states
	int		m_RowFirstCtrlX;	// x-coord of row's first control
	int		m_RowCtrlCount;		// total number of controls in row

// Overridables
	virtual	CRowDlg	*CreateRow(int Idx);
	virtual	void	UpdateRow(int Idx);

// Overrides

// Helpers
	void	MoveHeader();
	void	MoveRow(HDWP DeferPos, CRowDlg& Row, int Pos);
	void	ResizeColumn(int ColIdx, int Width, int FirstRow = 0);
	bool	ReadColumnWidths(CColWidthArray& ColWidth) const;
	bool	WriteColumnWidths() const;
};

inline int CRowView::GetRows() const
{
	return(m_Row.GetSize());
}

inline int CRowView::GetCols() const
{
	return(m_Cols);
}

inline CRowDlg *CRowView::GetRow(int Idx) const
{
	return((CRowDlg *)m_Row[Idx]);
}

inline CHeaderCtrl& CRowView::GetHeader()
{
	return(m_Hdr);
}

inline int CRowView::GetHeaderHeight() const
{
	return(m_HdrHeight);
}

inline void CRowView::SetTopMargin(int Margin)
{
	m_TopMargin = Margin;
}

inline int CRowView::GetTopMargin() const
{
	return(m_TopMargin);
}

inline CWnd *CRowView::GetNotifyWnd() const
{
	return(m_NotifyWnd);
}

inline void CRowView::SetNotifyWnd(CWnd *Wnd)
{
	m_NotifyWnd = Wnd;
}

inline HACCEL CRowView::GetAccel(CWnd*& AccelWnd) const
{
	AccelWnd = m_AccelWnd;
	return(m_Accel);
}

inline void CRowView::SetAccel(HACCEL Accel, CWnd *AccelWnd)
{
	m_Accel = Accel;
	m_AccelWnd = AccelWnd;
}

inline bool CRowView::GetReorderable() const
{
	return(m_Reorderable);
}

inline void CRowView::SetReorderable(bool Enable)
{
	m_Reorderable = Enable;
}

inline int CRowView::GetColumnWidth(int ColIdx) const
{
	return(m_ColState[ColIdx].CurWidth);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROWVIEW_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
