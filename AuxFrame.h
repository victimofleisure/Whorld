// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		20jul05	initial version
		01		21dec07	add OnSetCursor

		auxiliary frame for detached view

*/

#if !defined(AFX_AUXFRAME_H__522EEDAE_820F_4763_900C_2ECA0D8DA18D__INCLUDED_)
#define AFX_AUXFRAME_H__522EEDAE_820F_4763_900C_2ECA0D8DA18D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AuxFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAuxFrame frame

class CAuxFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CAuxFrame)
public:
	CAuxFrame();
	virtual ~CAuxFrame();

// Attributes
public:
	CView	*GetView() const;
	bool	IsFullScreen() const;

// Operations
public:
	void	FullScreen(bool Enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAuxFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CAuxFrame)
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	CRect	m_ScreenRect;		// window rect to restore when full-screen ends
	CView	*m_View;			// pointer to the view
	bool	m_WasShown;			// true if dialog was shown at least once
	bool	m_IsFullScreen;		// true if we're in full-screen mode
};

inline CView *CAuxFrame::GetView() const
{
	return(m_View);
}

inline bool CAuxFrame::IsFullScreen() const
{
	return(m_IsFullScreen);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUXFRAME_H__522EEDAE_820F_4763_900C_2ECA0D8DA18D__INCLUDED_)
