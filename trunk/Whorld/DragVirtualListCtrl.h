// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jun05	initial version
        01      23feb06	remove item margin
		02		02aug07	convert for virtual list control
		03		30jan08	add IsDragging
		04		06jan10	W64: make OnTimer 64-bit compatible
		05		04oct13	add drop position tracking
		06		21nov13	derive from extended selection list
		07		12jun14	add drag enable
		08		04apr15	add GetCompensatedDropPos
		09		24apr18	standardize names
		10		16jun18	make CompensateDropPos static

        virtual list control with drag reordering
 
*/

#pragma once

// DragVirtualListCtrl.h : header file
//

#include "ListCtrlExSel.h"

/////////////////////////////////////////////////////////////////////////////
// CDragVirtualListCtrl window

#define ULVN_REORDER	(LVN_LAST - 100U)	// user list view notification

class CDragVirtualListCtrl : public CListCtrlExSel
{
	DECLARE_DYNAMIC(CDragVirtualListCtrl);
// Construction
public:
	CDragVirtualListCtrl();

// Attributes
public:
	void	SetDragEnable(bool bEnable);
	bool	GetDragEnable() const;
	int		GetDropPos() const;
	int		GetCompensatedDropPos() const;
	bool	IsDragging() const;
	void	TrackDropPos(bool bEnable);

// Operations
public:
	void	CancelDrag();
	static	bool	CompensateDropPos(CIntArrayEx& arrSel, int& DropPos);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragVirtualListCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDragVirtualListCtrl();

protected:
// Generated message map functions
	//{{AFX_MSG(CDragVirtualListCtrl)
	afx_msg BOOL OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		TIMER_ID = 1000,
		SCROLL_DELAY = 50	// milliseconds
	};

// Member data
	bool	m_bDragEnable;		// true if drag is enabled
	bool	m_bDragging;		// true if items are being dragged
	bool	m_bTrackDropPos;	// true if tracking drop position
	int		m_nScrollDelta;		// scroll by this amount per timer tick
	W64UINT	m_nScrollTimer;		// if non-zero, timer instance for scrolling
	int		m_iDropPos;			// position at which items were dropped

// Overrideables
	virtual	void	UpdateCursor(CPoint point);

// Helpers
	void	AutoScroll(const CPoint& point);
};

inline void CDragVirtualListCtrl::SetDragEnable(bool bEnable)
{
	m_bDragEnable = bEnable;
}

inline bool CDragVirtualListCtrl::GetDragEnable() const
{
	return(m_bDragEnable);
}

inline int CDragVirtualListCtrl::GetDropPos() const
{
	return(m_iDropPos);
}

inline bool CDragVirtualListCtrl::IsDragging() const
{
	return(m_bDragging);
}

inline void CDragVirtualListCtrl::TrackDropPos(bool bEnable)
{
	m_bTrackDropPos = bEnable;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
