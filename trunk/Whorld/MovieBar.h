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
		IDC_MOVIE_SLIDER = 5013,
		IDC_TIME_STATIC,
	};
	enum {
		MOVIE_POS_TIMER_ID = 2026,
		MOVIE_POS_TIMER_PERIOD = 40,
		MOVIE_SLIDER_RANGE = 1000,
		BAR_MIN_WIDTH = 100,
		BAR_MIN_HEIGHT = 20,
		TIME_WIDTH = 80,
	};

// Data members
	CClickSliderCtrl	m_wndSlider;
	CStatic	m_wndTime;
	int		m_nSliderPos;
	CString	m_sTime;

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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
