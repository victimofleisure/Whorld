// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26dec04	initial version
		01		23nov07	support Unicode

        simple hyperlink control
 
*/

#if !defined(AFX_HYPERLINK_H__B003156A_9003_475B_B121_68760E26E8C6__INCLUDED_)
#define AFX_HYPERLINK_H__B003156A_9003_475B_B121_68760E26E8C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Hyperlink.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHyperlink window

class CHyperlink : public CStatic
{
	DECLARE_DYNAMIC(CHyperlink);
// Construction
public:
	CHyperlink();
	virtual ~CHyperlink();

// Attributes
public:
	void	SetUrl(LPCTSTR Url);
	void	FitWindowToUrl();
	void	SetUnderline(bool Enable);
	void	SetColors(COLORREF Link, COLORREF Visited, COLORREF Hover);
	static	bool	GotoUrl(LPCTSTR Url);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHyperlink)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CHyperlink)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Member data
	CString	m_Url;			// URL to go to when link is selected
	HCURSOR	m_Cursor;		// cursor to display while over link
	bool	m_Visited;		// true if link has been visited
	bool	m_Hovering;		// true while cursor is over link
	CFont	m_Font;			// link text is displayed in this font
	COLORREF	m_ColorLink;	// color to use for normal link
	COLORREF	m_ColorVisited;	// color to use for visited link
	COLORREF	m_ColorHover;	// color to use when cursor is over link
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HYPERLINK_H__B003156A_9003_475B_B121_68760E26E8C6__INCLUDED_)
