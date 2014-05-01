// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jul05	initial version
		01		21jul05	if app is deactivated, cancel show
		02		31aug05	change window size, not visible style
		03		21dec07	rename GetMainFrame to GetThis

		snapshot slide show dialog
 
*/

// SlideShowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SlideShowDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSlideShowDlg dialog

IMPLEMENT_DYNAMIC(CSlideShowDlg, CDialog);

CSlideShowDlg::CSlideShowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSlideShowDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSlideShowDlg)
	m_Seconds = 5;
	m_FullScreen = FALSE;
	m_Loop = FALSE;
	//}}AFX_DATA_INIT
	m_Frm = NULL;
	m_Slides = 0;
	m_CurPos = 0;
	m_Timer = 0;
	m_WasFullScreen = FALSE;
	m_Hidden = FALSE;
	m_InitRect.SetRectEmpty();
}

void CSlideShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSlideShowDlg)
	DDX_Control(pDX, IDC_SS_SECS_SPIN, m_SecsSpin);
	DDX_Text(pDX, IDC_SS_SECONDS, m_Seconds);
	DDV_MinMaxInt(pDX, m_Seconds, 0, 60);
	DDX_Check(pDX, IDC_SS_FULL_SCREEN, m_FullScreen);
	DDX_Check(pDX, IDC_SS_LOOP, m_Loop);
	//}}AFX_DATA_MAP
}

void CSlideShowDlg::Hide(bool Enable)
{
	if (Enable == m_Hidden)
		return;	// nothing to do
	CSize sz = Enable ? CSize(0, 0) : m_InitRect.Size();
	// topmost window of zero size is invisible but retains input focus
	SetWindowPos(Enable ? &CWnd::wndTopMost : &CWnd::wndNoTopMost,
		0, 0, sz.cx, sz.cy, SWP_NOMOVE);
	if (Enable)
		SetCapture();
	else
		ReleaseCapture();
	m_Hidden = Enable;
}

BEGIN_MESSAGE_MAP(CSlideShowDlg, CDialog)
	//{{AFX_MSG_MAP(CSlideShowDlg)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_ACTIVATEAPP()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSlideShowDlg message handlers

BOOL CSlideShowDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Frm = CMainFrame::GetThis();
	ASSERT(m_Frm != NULL);
	m_WasFullScreen = m_Frm->IsFullScreen();
	m_SecsSpin.SetRange(0, 60);
	m_SecsSpin.SetPos(m_Seconds);
	m_Hidden = FALSE;
	GetWindowRect(m_InitRect);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CSlideShowDlg::OnTimer(UINT nIDEvent) 
{
	if (m_CurPos >= m_Slides && !m_Loop)
		EndDialog(IDOK);	// show is over
	else {
		if (m_CurPos >= m_Slides)
			m_CurPos = 0;
		m_Frm->ShowSnapshot(m_CurPos);
		m_CurPos++;
		CString	s;
		s.Format(IDS_SS_SLIDE_NUMBER, m_CurPos);
		m_Frm->SetMessageText(s);
	}
	CDialog::OnTimer(nIDEvent);
}

void CSlideShowDlg::OnOK()
{
	if (!m_Timer) {	// if show hasn't started
		if (UpdateData(TRUE)) {	// if data is valid
			m_Timer = SetTimer(TIMER_ID, m_Seconds * 1000, 0);
			if (m_Timer) {
				m_Frm->FullScreen(m_FullScreen != 0);
				m_Slides = m_Frm->SnapCount();
				m_CurPos = 0;
				OnTimer(0);	// show the first slide
				Hide(TRUE);
			} else
				OnCancel();
		}
	} else	// show is canceled
		OnCancel();
}

void CSlideShowDlg::OnDestroy() 
{
	if (m_Timer) {
		m_Frm->FullScreen(m_WasFullScreen);
		KillTimer(m_Timer);
		m_Timer = 0;
		m_Frm->SetMessageText(AFX_IDS_IDLEMESSAGE);
	}
	CDialog::OnDestroy();
}

void CSlideShowDlg::OnActivateApp(BOOL bActive, HTASK hTask)
{
	if (!bActive)	// if app is being deactivated
		OnCancel();		// else we might not regain focus
}

void CSlideShowDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	OnCancel();
	CDialog::OnLButtonDown(nFlags, point);
}

BOOL CSlideShowDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && m_Hidden) {
		OnCancel();
		return(TRUE);
	}
	return CDialog::PreTranslateMessage(pMsg);
}
