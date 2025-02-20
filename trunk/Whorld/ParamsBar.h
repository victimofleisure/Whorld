// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb25	initial version

*/

#pragma once

#include "MyDockablePane.h"
#include "RowView.h"
#include "WhorldBase.h"

class CParamsBar;
class CParamsRowDlg;
class CWhorldDoc;

class CParamsView : public CRowView, public CWhorldBase {
public:
	DECLARE_DYNCREATE(CParamsView);
	CParamsView();
	virtual ~CParamsView();

// Attributes
	CWhorldDoc* GetDocument() const;
	CParamsRowDlg*	GetRow(int iRow);

// Constants
	static const int	m_arrParamOrder[PARAM_COUNT];	// parameter display order
	static int	m_arrParamRowPos[PARAM_COUNT];	// row position of each parameter

// Public data
	CParamsBar	*m_pParent;

// Overrides
	virtual CRowDlg	*CreateRow(int Idx);
	virtual void UpdateRow(int Idx);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

// Helpers
	static void	CreateRowPositionTable();

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
};

#ifndef _DEBUG  // debug version in .cpp
inline CWhorldDoc* CParamsView::GetDocument() const
{ 
	return reinterpret_cast<CWhorldDoc*>(m_pDocument);
}
#endif

inline CParamsRowDlg* CParamsView::GetRow(int iRow)
{
	return reinterpret_cast<CParamsRowDlg*>(CRowView::GetRow(iRow));
}

class CParamsBar : public CMyDockablePane, public CWhorldBase {
// Construction
public:
	CParamsBar();

// Attributes
public:

// Operations
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

protected:
// Types

// Data members

// Implementation
public:
	virtual ~CParamsBar();

protected:
// Types

// Constants
	enum {	// parameter columns
		#define PARAMCOLDEF(name) COL_##name,
		#include "WhorldDef.h"	// generate enumeration
		COLUMNS
	};
	static const CRowView::COLINFO	m_arrColInfo[COLUMNS];

// Data members
	CParamsView	*m_pView;

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
};
