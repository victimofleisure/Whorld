// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10feb25	initial version

*/

#pragma once

#include "MyDockablePane.h"
#include "RowView.h"

class CMasterBar;
class CMasterRowDlg;
class CWhorldDoc;

class CMasterView : public CRowView, public CWhorldBase {
public:
	DECLARE_DYNCREATE(CMasterView);
	CMasterView();
	virtual ~CMasterView();

// Attributes
	CWhorldDoc* GetDocument() const;
	CMasterRowDlg*	GetRow(int iRow);

// Public data
	CMasterBar	*m_pParent;

// Overrides
	virtual CRowDlg	*CreateRow(int Idx);
	virtual void UpdateRow(int Idx);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
};

#ifndef _DEBUG  // debug version in .cpp
inline CWhorldDoc* CMasterView::GetDocument() const
{ 
	return reinterpret_cast<CWhorldDoc*>(m_pDocument);
}
#endif

inline CMasterRowDlg* CMasterView::GetRow(int iRow)
{
	return reinterpret_cast<CMasterRowDlg*>(CRowView::GetRow(iRow));
}

class CMasterBar : public CMyDockablePane, public CWhorldBase {
// Construction
public:
	CMasterBar();

// Attributes
public:

// Operations
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

protected:
// Types

// Data members

// Implementation
public:
	virtual ~CMasterBar();

protected:
// Types

// Constants
	enum {	// master columns
		#define MASTERCOLDEF(name) COL_##name,
		#include "WhorldDef.h"	// generate enumeration
		COLUMNS
	};
	static const CRowView::COLINFO	m_arrColInfo[COLUMNS];

// Data members
	CMasterView	*m_pView;

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
};
