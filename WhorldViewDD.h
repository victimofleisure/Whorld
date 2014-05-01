// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20may05	initial version
		01		28mar06	repaint flag replaced by pause

        Whorld view using DirectDraw
 
*/

// WhorldViewDD.h : interface of the CWhorldViewDD class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WHORLDVIEWDD_H__4E7A57CC_64AF_47DE_98D1_C3DF8A34C524__INCLUDED_)
#define AFX_WHORLDVIEWDD_H__4E7A57CC_64AF_47DE_98D1_C3DF8A34C524__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CWhorldViewDD window

#include "WhorldView.h"
#include "BackBufDD.h"

class CWhorldViewDD : public CWhorldView
{
	DECLARE_DYNCREATE(CWhorldViewDD)
// Construction
public:
	CWhorldViewDD();

// Constants
	enum {	// options: low-order word is reserved for base class
		OPT_AUTO_MEMORY		= MAKELONG(0, CBackBufDD::OPT_AUTO_MEMORY),
		OPT_USE_VIDEO_MEM	= MAKELONG(0, CBackBufDD::OPT_USE_VIDEO_MEM),
		OPT_MIRROR_PRECISE	= MAKELONG(0, CBackBufDD::OPT_MIRROR_PRECISE)
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhorldViewDD)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	//}}AFX_VIRTUAL
	virtual	void	SetOptions(int Options);
	virtual	void	Mirror(bool Enable);

// Attributes
	void	SetExclusive(HWND hWnd, bool Enable);
	bool	IsExclusive() const;

// Implementation
public:
	virtual ~CWhorldViewDD();

// Generated message map functions
protected:
	//{{AFX_MSG(CWhorldViewDD)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Member data
	CBackBufDD	m_dd;		// DirectDraw back buffer

// Helpers
	void	OnCreateSurfaceError(int cx, int cy);
	void	OnFatalError();
};

inline bool CWhorldViewDD::IsExclusive() const
{
	return(m_dd.IsExclusive());
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHORLDVIEWDD_H__4E7A57CC_64AF_47DE_98D1_C3DF8A34C524__INCLUDED_)
