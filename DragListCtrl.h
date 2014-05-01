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
		02		30jan08	update cursor type on mouse move
		03		30jan08	add IsDragging

        list control with drag reordering
 
*/

#if !defined(AFX_DRAGLISTCTRL_H__99B4EEB6_9380_4505_8BFB_AC4E2E67FD23__INCLUDED_)
#define AFX_DRAGLISTCTRL_H__99B4EEB6_9380_4505_8BFB_AC4E2E67FD23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDragListCtrl window

#define ULVN_REORDER	(LVN_LAST - 100U)	// user list view notification

class CDragListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CDragListCtrl);
// Construction
public:
	CDragListCtrl();

// Attributes
public:
	bool	IsDragging() const;

// Operations
public:
	void	CancelDrag();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDragListCtrl();

protected:
// Generated message map functions
	//{{AFX_MSG(CDragListCtrl)
	afx_msg BOOL OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		TIMER_ID = 1000,
		SCROLL_DELAY = 50	// milliseconds
	};

// Member data
	bool	m_Dragging;			// true if items are being dragged
	int		m_ScrollDelta;		// scroll by this amount per timer tick
	DWORD	m_ScrollTimer;		// if non-zero, timer instance for scrolling

// Helpers
	void	AutoScroll(const CPoint& Cursor);
	void	UpdateCursor(CPoint point);
};

inline bool CDragListCtrl::IsDragging() const
{
	return(m_Dragging);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGLISTCTRL_H__99B4EEB6_9380_4505_8BFB_AC4E2E67FD23__INCLUDED_)
