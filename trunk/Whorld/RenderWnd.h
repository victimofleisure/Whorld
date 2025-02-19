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

#pragma once

#include "WhorldBase.h"

class CStatistics;

class CRenderWnd : public CWnd, public CWhorldBase {
public:
	DECLARE_DYNAMIC(CRenderWnd)

// Construction
	CRenderWnd();
	virtual ~CRenderWnd();
	bool	CreateWnd(DWORD dwStyle, CRect& rWnd, CWnd *pParentWnd);

// Operations

// Overrides

protected:
// Constants

// Helpers

// Message handlers
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};
