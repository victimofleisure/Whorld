// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version

*/

// WhorldView.h : interface of the CWhorldView class
//

#pragma once

#include "WhorldBase.h"

class CWhorldView : public CView, public CWhorldBase
{
protected: // create from serialization only
	CWhorldView();
	DECLARE_DYNCREATE(CWhorldView)

// Attributes
public:
	CWhorldDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CWhorldView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnDelayedCreate(WPARAM wParam, LPARAM lParam);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	afx_msg void OnWindowPause();
	afx_msg void OnUpdateWindowPause(CCmdUI *pCmdUI);
	afx_msg void OnWindowStep();
	afx_msg void OnUpdateWindowStep(CCmdUI *pCmdUI);
	afx_msg void OnWindowClear();
public:
	afx_msg void OnImageRandomPhase();
};

#ifndef _DEBUG  // debug version in WhorldView.cpp
inline CWhorldDoc* CWhorldView::GetDocument() const
   { return reinterpret_cast<CWhorldDoc*>(m_pDocument); }
#endif

