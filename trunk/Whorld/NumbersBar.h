// Copyleft 2026 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24oct26	initial version

*/

#pragma once

#include "MyDockablePane.h"
#include "WhorldBase.h"

class CNumbersBar : public CMyDockablePane, public CWhorldBase
{
	DECLARE_DYNAMIC(CNumbersBar)
// Construction
public:
	CNumbersBar();
	virtual ~CNumbersBar();

// Operations
	void	OnGetStatusReply(CStatus* pStatus);

// Implementation
protected:
// Constants
	enum {	// child control identifiers
		ID_CAPTIONS_STATIC = 2010,
		ID_VALUES_STATIC,
	};
	enum {	// misc constants
		STATUS_TIMER_ID = 1865,		// status timer identifier
		STATUS_TIMER_PERIOD = 200,	// status timer period in milliseconds
	};

// Data members
	CStatic	m_stCaptions;		// static control containing captions
	CStatic	m_stValues;			// static control containing values
	CAutoPtr<CStatus>	m_pStatus;	// smart pointer to status; represents ownership
	CStatus*	m_pStatusCopy;	// backup of status address, for reclaiming ownership
	CString	m_sCaptions;		// captions separated by newlines
	int		m_nCaptionWidth;	// maximum caption width, in pixels
	int		m_nSeparatorWidth;	// width of separator between caption and value

// Overrides
	virtual	void	OnShowChanged(bool bShow);
	virtual	void	OnFrameGetMinMaxInfo(HWND hFrameWnd, MINMAXINFO *pMMI);

// Helpers
	void	MeasureCaptions();
	void	MakeCaptions();
	void	DrawValues(const CStatus& status);
	void	DrawCaptions();

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnDelayedCreate(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
