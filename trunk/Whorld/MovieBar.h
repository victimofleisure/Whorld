// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17mar25	initial version

*/

#pragma once

#include "MyDockablePane.h"
#include "ClickSliderCtrl.h"

class CMovieBar : public CMyDockablePane
{
	DECLARE_DYNAMIC(CMovieBar)
// Construction
public:
	CMovieBar();
	virtual ~CMovieBar();

// Operations
	void	OnPlay(bool bEnable);

// Implementation
protected:
// Constants
	enum {	// child controls
		IDC_MOVIE_SLIDER = 16523,
		IDC_TIME_STATIC,
	};
	enum {
		MOVIE_POS_TIMER_ID = 2026,	// don't change this
		MOVIE_POS_TIMER_PERIOD = 40,	// in milliseconds
		MOVIE_SLIDER_RANGE = 1000,	// trade-off between speed and precision
		BAR_MIN_WIDTH = 100,	// not relevant as we're horizontal
		BAR_MIN_HEIGHT = 20,	// essential, prevents invisible bar
		TIME_WIDTH = 66,		// determined empirically; not too tight
	};

// Data members
	CClickSliderCtrl	m_wndSlider;	// slider for showing movie position and seeking
	CStatic	m_wndTime;			// static control for showing movie position as a time
	int		m_nSliderPos;		// current slider position, for change detection
	CString	m_sTime;			// current time, for change detection

// Helpers
	static int	FrameToSliderPos(LONGLONG iFrame);
	static LONGLONG	SliderPosToFrame(int iPos);
	void	UpdateTime(LONGLONG iFrame);

// Overrides
	virtual BOOL OnBeforeFloat(CRect& rectFloat, AFX_DOCK_METHOD dockMethod);
	virtual BOOL OnShowControlBarMenu(CPoint point);
	virtual BOOL CanAutoHide() const;

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
