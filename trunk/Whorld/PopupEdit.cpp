// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		23sep13	initial version
		01		16mar15	send end edit message instead of posting it

		popup edit control

*/

// PopupEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PopupEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit

IMPLEMENT_DYNAMIC(CPopupEdit, CEdit);

CPopupEdit::CPopupEdit()
{
	m_bEndingEdit = FALSE;
}

CPopupEdit::~CPopupEdit()
{
}

bool CPopupEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= WS_BORDER | ES_AUTOHSCROLL;
	if (!CEdit::Create(dwStyle, rect, pParentWnd, nID))	// create control
		return(FALSE);	// control creation failed
	SetFont(pParentWnd->GetFont());	// set font same as parent
	SetFocus();	// give control focus
	return(TRUE);
}

void CPopupEdit::EndEdit()
{
	m_bEndingEdit = TRUE;	// avoid reentrance if we lose focus
	if (GetModify()) {	// if text was modified
		CString	text;
		GetWindowText(text);
		LPCTSTR	pText = text;
		GetParent()->SendMessage(UWM_TEXT_CHANGE, LPARAM(pText));
	}
	delete this;
}

void CPopupEdit::CancelEdit()
{
	SetModify(FALSE);
	EndEdit();
}

BEGIN_MESSAGE_MAP(CPopupEdit, CEdit)
	//{{AFX_MSG_MAP(CPopupEdit)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_END_EDIT, OnEndEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit message handlers

void CPopupEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	if (!m_bEndingEdit)	// if not ending edit already
		SendMessage(UWM_END_EDIT);
}

BOOL CPopupEdit::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_RETURN:
		case VK_ESCAPE:
			SendMessage(UWM_END_EDIT, pMsg->wParam == VK_ESCAPE);	// cancel if escape
			return TRUE;	// no further processing; our instance is deleted
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}

LRESULT CPopupEdit::OnEndEdit(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if (wParam)	// if canceling
		CancelEdit();
	else
		EndEdit();
	return(0);
}
