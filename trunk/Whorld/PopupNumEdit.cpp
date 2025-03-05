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
		02		26feb25	remove note edit

		popup edit control

*/

// PopupNumEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PopupNumEdit.h"
#include "PopupEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CPopupNumEdit

IMPLEMENT_DYNAMIC(CPopupNumEdit, CNumEdit);

CPopupNumEdit::CPopupNumEdit()
{
	m_bEndingEdit = FALSE;
}

CPopupNumEdit::~CPopupNumEdit()
{
}

bool CPopupNumEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= WS_BORDER | ES_AUTOHSCROLL;
	if (!CNumEdit::Create(dwStyle, rect, pParentWnd, nID))	// create control
		return(FALSE);	// control creation failed
	if (m_nFormat & DF_SPIN)	// if spin control requested
		CreateSpinCtrl();
	SetFont(pParentWnd->GetFont());	// set font same as parent
	SetFocus();	// give control focus
	return(TRUE);
}

void CPopupNumEdit::EndEdit()
{
	m_bEndingEdit = TRUE;	// avoid reentrance if we lose focus
	if (GetModify()) {	// if text was modified
		GetText();
		if (m_bHaveRange) {
			m_fVal = CLAMP(m_fVal, m_fMinVal, m_fMaxVal);
			SetText();
		}
		GetParent()->SendMessage(CPopupEdit::UWM_TEXT_CHANGE, NULL);
	}
	delete this;
}

void CPopupNumEdit::CancelEdit()
{
	SetModify(FALSE);
	EndEdit();
}

void CPopupNumEdit::AddSpin(double fDelta)
{
	CNumEdit::AddSpin(fDelta);
	SetModify();	// so EndEdit handles text change
}

BEGIN_MESSAGE_MAP(CPopupNumEdit, CNumEdit)
	ON_WM_KILLFOCUS()
	ON_MESSAGE(CPopupEdit::UWM_END_EDIT, OnEndEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupNumEdit message handlers

void CPopupNumEdit::OnKillFocus(CWnd* pNewWnd) 
{
	BOOL	bModified = GetModify();	// save modified flag
	CNumEdit::OnKillFocus(pNewWnd);	// base class may reset modified flag
	SetModify(bModified);	// restore modified flag
	if (!m_bEndingEdit)	// if not ending edit already
		SendMessage(CPopupEdit::UWM_END_EDIT);
}

BOOL CPopupNumEdit::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_RETURN:
		case VK_ESCAPE:
			SendMessage(CPopupEdit::UWM_END_EDIT, pMsg->wParam == VK_ESCAPE);	// cancel if escape
			return TRUE;	// no further processing; our instance is deleted
		}
	}
	return CNumEdit::PreTranslateMessage(pMsg);
}

void CPopupNumEdit::PreSubclassWindow() 
{
	// skip CNumEdit override to avoid crash; caller must set initial value and create spin control
	CEdit::PreSubclassWindow();
}

LRESULT CPopupNumEdit::OnEndEdit(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if (wParam)	// if canceling
		CancelEdit();
	else
		EndEdit();
	return(0);
}
