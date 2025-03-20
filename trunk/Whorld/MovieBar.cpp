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

LONGLONG CMovieBar::SliderPosToFrame(int nPos)
{
	LONGLONG	nFrames = theApp.m_thrRender.GetMovieFrameCount();
	return Round(static_cast<double>(nPos) / 1000 * (nFrames - 1));
}

void CMovieBar::OnSliderScroll(int nPos)
{
	if (nPos != m_nSliderPos) {	// if slider position changed
		m_nSliderPos = nPos;	// updated cached copy
		if (theApp.GetMainFrame()->IsMoviePlaying()) {
			LONGLONG	iFrame = SliderPosToFrame(nPos);
			theApp.m_thrRender.MovieSeek(iFrame);
			UpdateTime(iFrame);
		}
	}
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
	return false;	// disable auto-hide
}

BOOL CMovieBar::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {	// if key down message
		// we should only get messages if slider has focus but check anyway
		HWND	hWnd = ::GetFocus();
		if (hWnd == m_wndSlider.m_hWnd) {	// if slider has input focus
			// The following keys may be used as main frame accelerators,
			// but the slider should respond to them when it has focus,
			// so intercept these keys and relay them to the slider.
			switch (pMsg->wParam) {
			case VK_PRIOR:
			case VK_NEXT:
			case VK_END:
			case VK_HOME:
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
				// relay key down message to slider
				m_wndSlider.SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
				break;
			}
		}
	}
	return CMyDockablePane::PreTranslateMessage(pMsg);
}

// CMovieBar message map

BEGIN_MESSAGE_MAP(CMovieBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CMovieBar message handlers

int CMovieBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

	// The gripper (AKA caption bar) would look absurd and must be avoided.
	// In addition to clearing this flag, we must never allow the bar to float.
	m_bHasGripper = false;	// prevent caption bar
    // create slider
    CRect	rDummy(0, 0, 0, 0); // placeholder size
	// a rectangular slider thumb with no ticks looks best
	DWORD	dwSliderStyle = WS_CHILD | WS_VISIBLE | WS_DISABLED | TBS_BOTH | TBS_NOTICKS;
    if (!m_wndSlider.Create(dwSliderStyle, rDummy, this, IDC_MOVIE_SLIDER)) {
		return -1;
	}
	// center text vertically within the control
	DWORD	dwStaticStyle = WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE;
	if (!m_wndTime.Create(NULL, dwStaticStyle, rDummy, this, IDC_TIME_STATIC)) {
		return -1;
	}
	m_wndSlider.SetRange(0, MOVIE_SLIDER_RANGE);
	// tell the control to use the default GUI font; cryptic but easy
	m_wndTime.SendMessage(WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)));
	// specify a minimum height; there is no easy way to specify a maximum
	SetMinSize(CSize(BAR_MIN_WIDTH, BAR_MIN_HEIGHT));

    return 0;
}

BOOL CMovieBar::OnEraseBkgnd(CDC* pDC)
{
	// By default, the background is unpainted because CBasePane overrides
	// OnEraseBkgnd to do nothing. That's fine if the child controls cover
	// the entire client area, but it's unhelpful in this case, because we
	// have a margin between our child controls. In this scenario the pane
	// should clip children, which it does; see CreateDockingWindows.
    DoPaint(pDC); // fill background with the standard theme color
    return true;  // no further erasing is required
}

void CMovieBar::OnSize(UINT nType, int cx, int cy)
{
	CMyDockablePane::OnSize(nType, cx, cy);
	int	nMargin = GetSystemMetrics(SM_CXPADDEDBORDER);
	m_wndSlider.MoveWindow(0, 0, cx - TIME_WIDTH - nMargin, cy);
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
		OnSliderScroll(m_wndSlider.GetPos());
	}
}

void CMovieBar::UpdateTime(LONGLONG iFrame)
{
	// This method is called by OnTimer, so its overhead must be considered.
	// It takes 10 microseconds if it doesn't update the control, but as much
	// as half a millisecond if it does, hence change detection is warranted.
	float	fFrameRate = theApp.m_thrRender.GetMovieFrameRate();
	CString	sTime;
	CMovieExportDlg::FrameToTimeString(static_cast<int>(iFrame), sTime, fFrameRate);
	if (sTime != m_sTime) {	// if time changed
		m_wndTime.SetWindowText(sTime);	// updating window is relatively slow
		m_sTime = sTime;	// update cached copy
	}
}

void CMovieBar::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case MOVIE_POS_TIMER_ID:
		if (!theApp.GetMainFrame()->IsPaused()) {
			LONGLONG	iFrame = theApp.m_thrRender.GetMoviePlaybackPos();
			int	nSliderPos = FrameToSliderPos(iFrame);
			m_wndSlider.SetPos(nSliderPos);
			m_nSliderPos = nSliderPos;
			UpdateTime(iFrame);
		}
		return;
	}
	CMyDockablePane::OnTimer(nIDEvent);
}
