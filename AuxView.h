// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		20jul05	initial version

		generic view for auxiliary frame

*/

#if !defined(AFX_AUXVIEW_H__6B83A2CA_F1FA_4D0C_AEAE_C3A0A16714EC__INCLUDED_)
#define AFX_AUXVIEW_H__6B83A2CA_F1FA_4D0C_AEAE_C3A0A16714EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AuxView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAuxView view

class CAuxView : public CView
{
	DECLARE_DYNCREATE(CAuxView)
protected:
	CAuxView();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAuxView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CAuxView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CAuxView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUXVIEW_H__6B83A2CA_F1FA_4D0C_AEAE_C3A0A16714EC__INCLUDED_)
