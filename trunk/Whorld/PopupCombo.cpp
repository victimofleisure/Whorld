// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		23sep13	initial version
		01		31may14	in Create, add vertical scroll to default style
		02		16mar15	send end edit message instead of posting it
		03		16dec22	add support for drop down with edit control

		popup combo box control

*/

// PopupCombo.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PopupCombo.h"
#include "PopupEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopupCombo

IMPLEMENT_DYNAMIC(CPopupCombo, CComboBox);

CPopupCombo::CPopupCombo()
{
	m_bEndingEdit = FALSE;
}

CPopupCombo::~CPopupCombo()
{
}

BOOL CPopupCombo::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!dwStyle)	// if style not specified, use default style
		dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST;
	if (!CComboBox::Create(dwStyle, rect, pParentWnd, nID))	// create control
		return(FALSE);	// control creation failed
	SetFont(pParentWnd->GetFont());	// set font same as parent
	SetFocus();	// give control focus
	return(TRUE);
}

CPopupCombo *CPopupCombo::Factory(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, int DropHeight)
{
	CPopupCombo	*pCombo = new CPopupCombo;	// allocate instance
	CRect	r(rect);
	r.bottom += DropHeight;
	if (!pCombo->Create(dwStyle, r, pParentWnd, nID)) {	// if create fails
		delete pCombo;	// destroy instance
		pCombo = NULL;
	}
	return(pCombo);
}

void CPopupCombo::EndEdit()
{
	m_bEndingEdit = TRUE;	// avoid reentrance if we lose focus
	CString	text;
	GetWindowText(text);
	LPCTSTR	pText = text;
	GetParent()->SendMessage(CPopupEdit::UWM_TEXT_CHANGE, LPARAM(pText));
	delete this;
}

void CPopupCombo::CancelEdit()
{
	delete this;
}

BEGIN_MESSAGE_MAP(CPopupCombo, CComboBox)
	//{{AFX_MSG_MAP(CPopupCombo)
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseup)
	ON_CONTROL_REFLECT(CBN_KILLFOCUS, OnReflectKillFocus)
	ON_MESSAGE(CPopupEdit::UWM_END_EDIT, OnEndEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupCombo message handlers

void CPopupCombo::OnKillFocus(CWnd* pNewWnd) 
{
	CComboBox::OnKillFocus(pNewWnd);
	if (!IsChild(pNewWnd)) {	// if window gaining focus isn't our child
		if (!m_bEndingEdit)	// if not ending edit already
			SendMessage(CPopupEdit::UWM_END_EDIT);
	}
}

void CPopupCombo::OnReflectKillFocus()
{
	// this message is only relevant if we have a child edit control
	if ((GetStyle() & CBS_DROPDOWNLIST) != CBS_DROPDOWNLIST)	// if not drop list style
		EndEdit();
}

BOOL CPopupCombo::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_RETURN:
		case VK_ESCAPE:
			SendMessage(CPopupEdit::UWM_END_EDIT, pMsg->wParam == VK_ESCAPE);	// cancel if escape
			return TRUE;	// no further processing; our instance is deleted
		}
	}
	return CComboBox::PreTranslateMessage(pMsg);
}

LRESULT CPopupCombo::OnEndEdit(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if (wParam)	// if canceling
		CancelEdit();
	else
		EndEdit();
	return(0);
}

void CPopupCombo::OnCloseup() 
{
	EndEdit();
}
