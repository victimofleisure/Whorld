// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30aug05	initial version
		01		10aug07	add template resource ID to ctor
		02		27dec09	add ShowPercent
		03		07jun21	rename rounding functions
		04		12dec22	add marquee mode; make pump messages public
		05		03mar25	modernize style
		06		16mar25 do idle processing in pump messages to fix invisible dialog

        progress dialog
 
*/

// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ProgressDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog);

CProgressDlg::CProgressDlg(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	m_nIDTemplate = nIDTemplate;
	m_pParentWnd = NULL;
	m_bParentDisabled = FALSE;
	m_bCanceled = FALSE;
	m_bShowPercent = FALSE;
	m_nPrevPercent = 0;
}

CProgressDlg::~CProgressDlg()
{
    if (m_hWnd != NULL)
		DestroyWindow();
}

bool CProgressDlg::Create(CWnd* pParent)
{
	// NOTE: dialog resource must be WS_OVERLAPPED and WS_VISIBLE
	m_pParentWnd = GetSafeOwner(pParent);
    if (m_pParentWnd != NULL && m_pParentWnd->IsWindowEnabled()) {
		m_pParentWnd->EnableWindow(FALSE);
		m_bParentDisabled = TRUE;
    }
    if (!CDialog::Create(m_nIDTemplate, pParent)) {
		ReenableParent();
		return FALSE;
    }
	return TRUE;
}

void CProgressDlg::ReenableParent()
{
    if (m_bParentDisabled && m_pParentWnd != NULL)
		m_pParentWnd->EnableWindow(TRUE);
    m_bParentDisabled = FALSE;
}

void CProgressDlg::SetPos(int nPos)
{
	PumpMessages();
	m_wndProgress.SetPos(nPos);
	if (m_bShowPercent) {
		int	nLower, nUpper;
		m_wndProgress.GetRange(nLower, nUpper);
		int	nRange = abs(nUpper - nLower);
		if (nRange) {	// avoid potential divide by zero
			int	nPercent = Round((nPos - nLower) * 100.0 / nRange);
			if (nPercent != m_nPrevPercent) {	// if percentage changed
				m_nPrevPercent = nPercent;
				CString	s;
				s.Format(_T("%d%%"), nPercent);
				m_wndPercent.SetWindowText(s);
			}
		}
	}
}

void CProgressDlg::SetRange(int nLower, int nUpper)
{
	m_wndProgress.SetRange32(nLower, nUpper);
}

void CProgressDlg::PumpMessages(HWND hWnd)
{
	// We do a mini "main loop" that also runs MFC idle
	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		// Let MFC do its standard PreTranslate for in-menu/toolbar/idle logic
		if (!AfxGetApp()->PreTranslateMessage(&msg)) {
			// Then pass to dialog or default
			if (!::IsDialogMessage(hWnd, &msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}
	// Now do idle processing to update toolbars, etc.
	AfxGetApp()->OnIdle(0);
}

void CProgressDlg::SetMarquee(bool bEnable, int nInterval)
{
	m_wndProgress.SetMarquee(bEnable, nInterval);	// requires UNICODE
	DWORD	dwRemove, dwAdd;
	if (bEnable) {
		dwRemove = 0;
		dwAdd = PBS_MARQUEE;
	} else {
		dwRemove = PBS_MARQUEE;
		dwAdd = 0;
	}
	m_wndProgress.ModifyStyle(dwRemove, dwAdd);
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_PROGRESS_PERCENT, m_wndPercent);
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL CProgressDlg::DestroyWindow() 
{
    ReenableParent();
	return CDialog::DestroyWindow();
}

void CProgressDlg::OnCancel() 
{
	m_bCanceled = TRUE;
	CDialog::OnCancel();
}
