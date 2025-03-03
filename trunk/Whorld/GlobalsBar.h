// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02mar25	initial version

*/

#pragma once

#include "MyDockablePane.h"
#include "MyRowView.h"
#include "WhorldBase.h"

class CGlobalsBar;
class CGlobalsRowDlg;
class CWhorldDoc;

class CGlobalsView : public CMyRowView, public CWhorldBase {
public:
	DECLARE_DYNCREATE(CGlobalsView);
	CGlobalsView();
	virtual ~CGlobalsView();

// Attributes
	CGlobalsRowDlg*	GetRow(int iRow);

// Public data
	CGlobalsBar	*m_pParent;

// Overrides
	virtual CRowDlg	*CreateRow(int Idx);
	virtual void UpdateRow(int Idx);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

inline CGlobalsRowDlg* CGlobalsView::GetRow(int iRow)
{
	return reinterpret_cast<CGlobalsRowDlg*>(CRowView::GetRow(iRow));
}

class CGlobalsBar : public CMyDockablePane, public CWhorldBase {
// Construction
public:
	CGlobalsBar();

// Attributes
public:

// Operations
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

// Overrides
	virtual void	OnFrameGetMinMaxInfo(HWND hFrameWnd, MINMAXINFO *pMMI);

protected:
// Types

// Data members

// Implementation
public:
	virtual ~CGlobalsBar();

protected:
// Types

// Constants
	enum {	// parameter columns
		#define GLOBALCOLDEF(name) COL_##name,
		#include "WhorldDef.h"	// generate enumeration
		COLUMNS
	};
	static const CRowView::COLINFO	m_arrColInfo[COLUMNS];

// Data members
	CGlobalsView	*m_pView;

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
};
