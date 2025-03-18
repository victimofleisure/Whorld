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

#include "stdafx.h"
#include "Whorld.h"
#include "MovieBar.h"
#include "MainFrm.h"
#include "MovieExportDlg.h"	// only for time conversion methods

IMPLEMENT_DYNAMIC(CMovieBar, CMyDockablePane)

CMovieBar::CMovieBar()
{
	m_nSliderPos = 0;
}

CMovieBar::~CMovieBar()
{
}

int CMovieBar::FrameToSliderPos(LONGLONG iFrame)
{
	LONGLONG	nFrames = theApp.m_thrRender.GetMovieFrameCount();
	return Round(static_cast<double>(iFrame) / (nFrames - 1) * 1000);
}

LONGLONG CMovieBar::SliderPosToFrame(int iPos)
{
	LONGLONG	nFrames = theApp.m_thrRender.GetMovieFrameCount();
	return Round(static_cast<double>(iPos) / 1000 * (nFrames - 1));
}

void CMovieBar::OnPlay(bool bEnable)
{
	if (bEnable) {	// if starting playback
		SetTimer(MOVIE_POS_TIMER_ID, MOVIE_POS_TIMER_PERIOD, NULL);
	} else {	// ending playback
		KillTimer(MOVIE_POS_TIMER_ID);
		m_wndSlider.SetPos(0);
		m_nSliderPos = 0;
	}
	ShowPane(bEnable, 0, 0);	// show pane during playback only
	m_wndSlider.EnableWindow(bEnable);	// enable slider during playback only
}

BOOL CMovieBar::OnBeforeFloat(CRect& rectFloat, AFX_DOCK_METHOD dockMethod)
{ 
	UNREFERENCED_PARAMETER(rectFloat);
	UNREFERENCED_PARAMETER(dockMethod);
	return false;	// prevent floating in some cases
}

BOOL CMovieBar::OnShowControlBarMenu(CPoint point)
{
	UNREFERENCED_PARAMETER(point);
	return false;	// prevent another means of floating
}

BOOL CMovieBar::CanAutoHide() const
{
	return false;
}

// CMovieBar message map

BEGIN_MESSAGE_MAP(CMovieBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CMovieBar message handlers

int CMovieBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

	m_bHasGripper = false;	// prevent caption bar
    // create slider
    CRect	rDummy(0, 0, 0, 0); // placeholder size
	DWORD	dwSliderStyle = WS_CHILD | WS_VISIBLE | WS_DISABLED | TBS_BOTH | TBS_NOTICKS;
    if (!m_wndSlider.Create(dwSliderStyle, rDummy, this, IDC_MOVIE_SLIDER)) {
		return -1;
	}
	DWORD	dwStaticStyle = WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE;
	if (!m_wndTime.Create(NULL, dwStaticStyle, rDummy, this, IDC_TIME_STATIC)) {
		return -1;
	}
	m_wndSlider.SetRange(0, MOVIE_SLIDER_RANGE);
	m_wndTime.SendMessage(WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)));
	SetMinSize(CSize(BAR_MIN_WIDTH, BAR_MIN_HEIGHT));

    return 0;
}

void CMovieBar::OnSize(UINT nType, int cx, int cy)
{
	CMyDockablePane::OnSize(nType, cx, cy);
	m_wndSlider.MoveWindow(0, 0, cx - TIME_WIDTH, cy);
	m_wndTime.MoveWindow(cx - TIME_WIDTH, 0, cx, cy);
}

void CMovieBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CMyDockablePane::OnHScroll(nSBCode, nPos, pScrollBar);
	switch (nSBCode) {
	case SB_ENDSCROLL:
	case SB_THUMBPOSITION:
		break;
	default:
		int	nSliderPos = m_wndSlider.GetPos();
		if (nSliderPos != m_nSliderPos) {
			m_nSliderPos = nSliderPos;
			if (theApp.GetMainFrame()->IsMoviePlaying()) {
				LONGLONG	iFrame = SliderPosToFrame(nSliderPos);
				theApp.m_thrRender.MovieSeek(iFrame);
				UpdateTime(iFrame);
			}
		}
	}
}

void CMovieBar::UpdateTime(LONGLONG iFrame)
{
	float	fFrameRate = theApp.m_thrRender.GetMovieFrameRate();
	CString	sTime(CMovieExportDlg::FrameToTimeString(static_cast<int>(iFrame), fFrameRate));
	if (sTime != m_sTime) {
		m_wndTime.SetWindowText(_T("   ") + sTime);
		m_sTime = sTime;
	}
}

void CMovieBar::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case MOVIE_POS_TIMER_ID:
		if (!theApp.GetMainFrame()->IsPaused()) {
			LONGLONG	iFrame = theApp.m_thrRender.GetMoviePlaybackPos();
			int	m_nSliderPos = FrameToSliderPos(iFrame);
			m_wndSlider.SetPos(m_nSliderPos);
			UpdateTime(iFrame);
		}
		return;
	}
	CMyDockablePane::OnTimer(nIDEvent);
}
