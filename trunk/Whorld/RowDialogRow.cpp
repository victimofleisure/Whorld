// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
		01		12sep05	add row position

        generic row form
 
*/

// RowDialogRow.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RowDialogRow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRowDialogRow dialog

IMPLEMENT_DYNAMIC(CRowDialogRow, CDialog);

CRowDialogRow::CRowDialogRow(UINT Template, CWnd* pParent /*=NULL*/)
	: CDialog(Template, pParent)
{
	//{{AFX_DATA_INIT(CRowDialogRow)
	//}}AFX_DATA_INIT
	m_NotifyWnd = NULL;
	m_RowIdx = 0;
	m_RowPos = 0;
	m_Accel = NULL;
}

void CRowDialogRow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRowDialogRow)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRowDialogRow, CDialog)
	//{{AFX_MSG_MAP(CRowDialogRow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRowDialogRow message handlers

void CRowDialogRow::OnOK()
{
}

void CRowDialogRow::OnCancel()
{
	m_NotifyWnd->PostMessage(WM_CLOSE);
}

BOOL CRowDialogRow::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		if (m_Accel != NULL && TranslateAccelerator(m_NotifyWnd->m_hWnd, m_Accel, pMsg))
			return(TRUE);
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
			if (m_NotifyWnd->SendMessage(UWM_ROWDIALOGTAB, m_RowIdx))
				return(TRUE);
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
