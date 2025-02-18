// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30aug05	initial version

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

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressDlg)
	//}}AFX_DATA_INIT
	m_pParentWnd = NULL;
	m_bParentDisabled = FALSE;
	m_Canceled = FALSE;
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
    if (!CDialog::Create(CProgressDlg::IDD, pParent)) {
		ReenableParent();
		return(FALSE);
    }
	return(TRUE);
}

void CProgressDlg::ReenableParent()
{
    if (m_bParentDisabled && m_pParentWnd != NULL)
		m_pParentWnd->EnableWindow(TRUE);
    m_bParentDisabled = FALSE;
}

void CProgressDlg::SetPos(int Pos)
{
	PumpMessages();
	m_Progress.SetPos(Pos);
}

void CProgressDlg::SetRange(int Lower, int Upper)
{
	m_Progress.SetRange32(Lower, Upper);
}

void CProgressDlg::PumpMessages()
{
	ASSERT(m_hWnd != NULL);
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!IsDialogMessage(&msg)) {
	        TranslateMessage(&msg);
		    DispatchMessage(&msg);
		}
	}
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
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
