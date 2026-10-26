// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24oct26	initial version

*/

#include "stdafx.h"
#include "Whorld.h"
#include "NumbersBar.h"
#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CNumbersBar, CMyDockablePane)

CNumbersBar::CNumbersBar()
{
	m_pStatusCopy = new CStatus;
	m_pStatus.Attach(m_pStatusCopy);
	m_nCaptionWidth = 0;
	m_nSeparatorWidth = 0;
}

CNumbersBar::~CNumbersBar()
{
}

void CNumbersBar::OnGetStatusReply(CStatus* pStatus)
{
	if (m_pStatus == NULL) {	// if we're expecting a status
		if (pStatus == m_pStatusCopy) {	// if the status is ours
			m_pStatus.Attach(m_pStatusCopy);	// reclaim status by reattaching it
			if (m_stValues.m_hWnd != NULL) {	// if target control exists
				DrawValues(*m_pStatus);	// update values from new status
			}
		}
	}
}

void CNumbersBar::MeasureCaptions()
{
	CClientDC	dc(this);
	dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	m_nCaptionWidth = 0;
	int	iStart = 0;
	CString	sToken;
	while (!(sToken = m_sCaptions.Tokenize(_T("\n"), iStart)).IsEmpty()) {	// while tokens remain
		int	nWidth = dc.GetTextExtent(sToken).cx;	// measure caption width
		if (nWidth > m_nCaptionWidth)	// if width exceeds maximum
			m_nCaptionWidth = nWidth;	// update maximum
	}
	m_nSeparatorWidth = dc.GetTextExtent(_T(" ")).cx;	// measure separator width
}

void CNumbersBar::MakeCaptions()
{
	// maintain this method in parallel with DrawValues
	static const int aTarget[] = {	// list of mapping targets, for their names
		CMappingBase::TARGET_OriginX,
		CMappingBase::TARGET_OriginY,
		CMappingBase::TARGET_Zoom,
	};
	const LPCTSTR pszSuffix = _T(":\n");
	ASSERT(m_sCaptions.IsEmpty());
	// add parameter captions
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter
		m_sCaptions += GetParamName(iParam) + pszSuffix;
	}
	// add captions for selected mapping targets
	for (int iTarg = 0; iTarg < _countof(aTarget); iTarg++) {	// for each target
		m_sCaptions += CMappingBase::GetTargetName(aTarget[iTarg]) + pszSuffix;
	}
	// add miscellaneous captions
	m_sCaptions += LDS(IDS_MASTER_Rings) + pszSuffix;
}

void CNumbersBar::DrawValues(const CStatus& status)
{
	// maintain this method in parallel with MakeCaptions
	CString	sOut, sTmp;
	// add parameter values
	for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter
		sTmp.Format(_T("%.3f\n"), status.params.a[iParam]);
		sOut += sTmp;
	}
	// add remaining values
	sTmp.Format(_T("%.3f\n%.3f\n%.3f\n%lld"), 
		status.dptOrigin.x, 
		status.dptOrigin.y, 
		status.fZoom, 
		status.nRings);
	sOut += sTmp;
	m_stValues.SetWindowText(sOut);	// update values static control
}

void CNumbersBar::DrawCaptions()
{
	m_stCaptions.SetWindowText(m_sCaptions);	// update captions static control
}

void CNumbersBar::OnShowChanged(bool bShow)
{
	if (bShow) {	// if showing pane
		SetTimer(STATUS_TIMER_ID, STATUS_TIMER_PERIOD, NULL);	// start timer
		// request status immediately, else values lag by a timer period
		OnTimer(STATUS_TIMER_ID);
	} else {	// hiding pane
		KillTimer(STATUS_TIMER_ID);	// kill timer
	}
}

void CNumbersBar::OnFrameGetMinMaxInfo(HWND hFrameWnd, MINMAXINFO *pMMI)
{
	CMainFrame::OnFrameGetMinMaxInfo(this, hFrameWnd, pMMI);	// delegate to common handler
}

// CNumbersBar message map

BEGIN_MESSAGE_MAP(CNumbersBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_MESSAGE(UWM_DELAYED_CREATE, OnDelayedCreate)
END_MESSAGE_MAP()

// CNumbersBar message handlers

int CNumbersBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;
	// postpone creation of child controls to avoid
	// repeatedly resizing them; speeds app launch
	PostMessage(UWM_DELAYED_CREATE);
	return 0;
}

LRESULT CNumbersBar::OnDelayedCreate(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	MakeCaptions();	// captions are constant and only need to be made once
	MeasureCaptions();	// measure caption width, which we need for layout
	// create static child control for captions
	CRect	r(0, 0, 0, 0);	// controls will be resized by OnSize
	// SS_NOPREFIX speeds up SetWindowText in DrawValues by about 1.4x
	DWORD	dwCapsStyle = WS_CHILD | WS_VISIBLE | SS_NOPREFIX | SS_RIGHT;
	VERIFY(m_stCaptions.Create(NULL, dwCapsStyle, r, this, ID_CAPTIONS_STATIC));
	m_stCaptions.SendMessage(WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)));
	// create static child control for values
	DWORD	dwValsStyle = WS_CHILD | WS_VISIBLE | SS_NOPREFIX | SS_LEFTNOWORDWRAP;
	VERIFY(m_stValues.Create(NULL, dwValsStyle, r, this, ID_VALUES_STATIC));
	m_stValues.SendMessage(WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)));
	ModifyStyleEx(0, WS_EX_COMPOSITED);	// use compositing to avoid flicker
	DrawCaptions();	// draw captions into static control only once
	CRect	rc;
	GetClientRect(rc);
	OnSize(0, rc.Width(), rc.Height());	// resize controls
	return 0;
}

BOOL CNumbersBar::OnEraseBkgnd(CDC* pDC)
{
	CRect	rc;	// panes aren't erased by default
	GetClientRect(rc);
	pDC->FillSolidRect(rc, GetSysColor(COLOR_WINDOW));
	return TRUE;
}

HBRUSH CNumbersBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_STATIC) {	// if static control
		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));	// set text background color
		return GetSysColorBrush(COLOR_WINDOW);	// set background brush to match
	}
	return CMyDockablePane::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CNumbersBar::OnSize(UINT nType, int cx, int cy)
{
	UNREFERENCED_PARAMETER(nType);
	if (m_stCaptions.m_hWnd) {	// if child controls exist
		m_stCaptions.MoveWindow(0, 0, m_nCaptionWidth, cy, FALSE);	// don't repaint
		int	x = m_nCaptionWidth + m_nSeparatorWidth;
		m_stValues.MoveWindow(x, 0, max(cx - x, 0), cy, FALSE);	// values get remaining width
		Invalidate();	// repaint entire pane
	}
}

void CNumbersBar::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == STATUS_TIMER_ID) {	// if status timer
		if (m_pStatus != NULL) {	// if status is available (not already pending)
			// transfer ownership of status to render thread until we receive its reply
			CStatus	*pStatus = m_pStatus.Detach();	// relinquish ownership by detaching
			if (!theApp.m_thrRender.GetStatus(pStatus)) {	// enqueue get status command
				// enqueue failed, so transfer of ownership must be revoked
				m_pStatus.Attach(pStatus);	// reclaim status by reattaching it
			}
		}
		// don't update UI here; the reply handler does the updating
	} else {	// some other timer
		CMyDockablePane::OnTimer(nIDEvent);
	}
}
