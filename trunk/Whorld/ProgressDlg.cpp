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

        progress dialog
 
*/

// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog);

CProgressDlg::CProgressDlg(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	//{{AFX_DATA_INIT(CProgressDlg)
	//}}AFX_DATA_INIT
	m_IDTemplate = nIDTemplate;
	m_ParentWnd = NULL;
	m_ParentDisabled = FALSE;
	m_Canceled = FALSE;
	m_ShowPercent = FALSE;
	m_Lower = 0;
	m_Upper = 100;
	m_PrevPercent = 0;
}

CProgressDlg::~CProgressDlg()
{
    if (m_hWnd != NULL)
		DestroyWindow();
}

bool CProgressDlg::Create(CWnd* pParent)
{
	// NOTE: dialog resource must be WS_OVERLAPPED and WS_VISIBLE
	m_ParentWnd = GetSafeOwner(pParent);
    if (m_ParentWnd != NULL && m_ParentWnd->IsWindowEnabled()) {
		m_ParentWnd->EnableWindow(FALSE);
		m_ParentDisabled = TRUE;
    }
    if (!CDialog::Create(m_IDTemplate, pParent)) {
		ReenableParent();
		return(FALSE);
    }
	return(TRUE);
}

void CProgressDlg::ReenableParent()
{
    if (m_ParentDisabled && m_ParentWnd != NULL)
		m_ParentWnd->EnableWindow(TRUE);
    m_ParentDisabled = FALSE;
}

void CProgressDlg::SetPos(int Pos)
{
	PumpMessages();
	m_Progress.SetPos(Pos);
	if (m_ShowPercent) {
		int	Lower, Upper;
		m_Progress.GetRange(Lower, Upper);
		int	Range = abs(Upper - Lower);
		if (Range) {	// avoid potential divide by zero
			int	percent = Round((Pos - Lower) * 100.0 / Range);
			if (percent != m_PrevPercent) {	// if percentage changed
				m_PrevPercent = percent;
				CString	s;
				s.Format(_T("%d%%"), percent);
				m_Percent.SetWindowText(s);
			}
		}
	}
}

void CProgressDlg::SetRange(int Lower, int Upper)
{
	m_Progress.SetRange32(Lower, Upper);
	m_Lower = Lower;
	m_Upper = Upper;
}

void CProgressDlg::PumpMessages(HWND hWnd)
{
	ASSERT(hWnd != NULL);
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!::IsDialogMessage(hWnd, &msg)) {
	        TranslateMessage(&msg);
		    DispatchMessage(&msg);
		}
	}
}

void CProgressDlg::SetMarquee(bool bEnable, int nInterval)
{
	m_Progress.SetMarquee(bEnable, nInterval);	// requires UNICODE
	DWORD	dwRemove, dwAdd;
	if (bEnable) {
		dwRemove = 0;
		dwAdd = PBS_MARQUEE;
	} else {
		dwRemove = PBS_MARQUEE;
		dwAdd = 0;
	}
	m_Progress.ModifyStyle(dwRemove, dwAdd);
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_PROGRESS_PERCENT, m_Percent);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CProgressDlg)
	//}}AFX_MSG_MAP
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
	m_Canceled = TRUE;
	CDialog::OnCancel();
}
