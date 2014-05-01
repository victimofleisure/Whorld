// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28sep04	initial version
		01		22apr05	remove undo handling, use doubles
		02		27apr05	remove log base, add scale
		03		24jul05	add log base and aux notify
		04		13aug06	notify aux before parent
		05		28jan08	support Unicode

        numeric edit control
 
*/

// NumEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "NumEdit.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumEdit

IMPLEMENT_DYNAMIC(CNumEdit, CEdit);

CNumEdit::CNumEdit()
{
	m_Val = 0;
	m_Scale = 1;
	m_LogBase = 0;
	m_Precision = -1;
	m_AuxNotify = NULL;
	m_MinVal = 0;
	m_MaxVal = 0;
	m_HaveRange = FALSE;
	m_Format = DF_REAL;
}

CNumEdit::~CNumEdit()
{
}

void CNumEdit::SetText()
{
	CString	s;
	ValToStr(s);
	SetWindowText(s);
}

void CNumEdit::GetText()
{
	CString	s;
	GetWindowText(s);
	StrToVal(s);
}

void CNumEdit::SetVal(double Val)
{
	m_Val = Val;
	SetText();
	Notify(NTF_AUX);	// don't notify parent, to avoid feedback
}

void CNumEdit::SetVal(double Val, int NotifyMask)
{
	m_Val = Val;
	SetText();
	Notify(NotifyMask);	// caller determines who gets notified
}

void CNumEdit::AddSpin(double Delta)
{
	GetText();	// freshen value before incrementing it
	m_Val += Delta;
	if (m_HaveRange)
		m_Val = CLAMP(m_Val, m_MinVal, m_MaxVal);
	SetText();
	Notify();
}

void CNumEdit::StrToVal(LPCTSTR Str)
{
	double	r = _tstof(Str) * m_Scale;
	if (m_LogBase)
		r = log(r) / log(m_LogBase);
	m_Val = r;
}

void CNumEdit::ValToStr(CString& Str)
{
	double	r = m_Val;
	if (m_LogBase)
		r = pow(m_LogBase, r);
	r /= m_Scale;
	if (m_Precision < 0)
		Str.Format(_T("%g"), r);
	else
		Str.Format(_T("%.*f"), m_Precision, r);
}

bool CNumEdit::IsValidChar(int Char)
{
	switch (Char) {
	case '-':
	case VK_BACK:
		break;
	case '.':
	case 'e':
		if (m_Format != DF_REAL)
			return(FALSE);
		break;
	default:
		if (!(isdigit(Char) || iscntrl(Char)))
			return(FALSE);
	}
	return(TRUE);
}

void CNumEdit::Notify(int NotifyMask)
{
	NMHDR	nmh;
	nmh.hwndFrom = m_hWnd;
	nmh.idFrom = GetDlgCtrlID();
	nmh.code = NEN_CHANGED;
	// notify aux before parent; else if parent send its own notification via
	// SendMessage, and recipient reads value from aux, value will be stale
	if (m_AuxNotify != NULL && (NotifyMask & NTF_AUX))	// notify aux first
		m_AuxNotify->SendMessage(WM_NOTIFY, nmh.idFrom, long(&nmh));
	if (NotifyMask & NTF_PARENT)
		GetParent()->SendMessage(WM_NOTIFY, nmh.idFrom, long(&nmh));
}

void CNumEdit::SetRange(double MinVal, double MaxVal)
{
	m_MinVal = MinVal;
	m_MaxVal = MaxVal;
	m_HaveRange = TRUE;
}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
	//{{AFX_MSG_MAP(CNumEdit)
	ON_CONTROL_REFLECT_EX(EN_KILLFOCUS, OnKillfocus)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumEdit message handlers

BOOL CNumEdit::OnKillfocus() 
{
	if (GetModify()) {
		GetText();
		if (m_HaveRange) {
			m_Val = CLAMP(m_Val, m_MinVal, m_MaxVal);
			SetText();
		}
		Notify();
	}
	return(FALSE);	// let parent handle notification too
}

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (IsValidChar(nChar))
		CEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CNumEdit::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		GetParent()->SetFocus();	// return ends edit
	return CEdit::PreTranslateMessage(pMsg);
}

void CNumEdit::PreSubclassWindow() 
{
	SetVal(m_Val);
	CEdit::PreSubclassWindow();
}
