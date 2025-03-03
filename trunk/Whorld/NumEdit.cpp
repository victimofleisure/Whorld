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
		06		19sep13	add spin control format
		07		17oct13	in PreTranslateMessage, don't change focus
		08		18feb14	add OnEnable to invalidate spin control
		09		19apr18	move spin control creation to helper
		10		24apr18	standardize names
		11		02jun18	in Notify, fix x64 crash due to casting pointer to long
		12		14dec22	add fraction format
		13		10feb25	in AddSpin, if clamped value is unchanged, don't notify

        numeric edit control
 
*/

// NumEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "NumEdit.h"
#include "NumSpin.h"
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
	m_fVal = 0;
	m_fScale = 1;
	m_fLogBase = 0;
	m_nPrecision = -1;
	m_pAuxNotify = NULL;
	m_fMinVal = 0;
	m_fMaxVal = 0;
	m_bHaveRange = FALSE;
	m_nFormat = DF_REAL;
	m_nFracScale = 0;
	m_pSpin = NULL;
}

CNumEdit::~CNumEdit()
{
	delete m_pSpin;
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

void CNumEdit::SetVal(double fVal)
{
	m_fVal = fVal;
	SetText();
	Notify(NTF_AUX);	// don't notify parent, to avoid feedback
}

void CNumEdit::SetVal(double fVal, int nNotifyMask)
{
	m_fVal = fVal;
	SetText();
	Notify(nNotifyMask);	// caller determines who gets notified
}

void CNumEdit::AddSpin(double fDelta)
{
	GetText();	// freshen value before incrementing it
	double	fNewVal = m_fVal + fDelta;
	if (m_bHaveRange) {	// if range was specified
		fNewVal = CLAMP(fNewVal, m_fMinVal, m_fMaxVal);
		if (fNewVal == m_fVal)	// if clamped value is unchanged
			return;	// don't notify
	}
	m_fVal = fNewVal;
	SetText();
	Notify();
}

void CNumEdit::StrToVal(LPCTSTR Str)
{
	double	r;
	int	nNumerator, nDenominator;
	if ((m_nFormat & DF_FRACTION)	// if fractions are enabled
	&& _stscanf_s(Str, _T("%d/%d"), &nNumerator, &nDenominator) == 2	// and both values scanned
	&& nDenominator != 0)	// and denominator is non-zero
		r = nNumerator / double(nDenominator) * m_nFracScale;
	else
		r = _tstof(Str) * m_fScale;
	if (m_fLogBase)
		r = log(r) / log(m_fLogBase);
	m_fVal = r;
}

void CNumEdit::ValToStr(CString& Str)
{
	double	r = m_fVal;
	if (m_fLogBase)
		r = pow(m_fLogBase, r);
	r /= m_fScale;
	if (m_nPrecision < 0)
		Str.Format(_T("%g"), r);
	else
		Str.Format(_T("%.*f"), m_nPrecision, r);
}

bool CNumEdit::IsValidChar(int nChar)
{
	switch (nChar) {
	case '-':
	case VK_BACK:
		break;
	case '.':
	case 'e':
		if (m_nFormat & DF_INT)
			return FALSE;
		break;
	case '/':
		return (m_nFormat & DF_FRACTION) != 0;
	default:
		if (!(isdigit(nChar) || iscntrl(nChar)))
			return FALSE;
	}
	return TRUE;
}

void CNumEdit::Notify(int nNotifyMask)
{
	NMHDR	nmh;
	nmh.hwndFrom = m_hWnd;
	nmh.idFrom = GetDlgCtrlID();
	nmh.code = NEN_CHANGED;
	// notify aux before parent; else if parent send its own notification via
	// SendMessage, and recipient reads value from aux, value will be stale
	if (m_pAuxNotify != NULL && (nNotifyMask & NTF_AUX))	// notify aux first
		m_pAuxNotify->SendMessage(WM_NOTIFY, nmh.idFrom, reinterpret_cast<LPARAM>(&nmh));
	if (nNotifyMask & NTF_PARENT)
		GetParent()->SendMessage(WM_NOTIFY, nmh.idFrom, reinterpret_cast<LPARAM>(&nmh));
}

void CNumEdit::SetRange(double fMinVal, double fMaxVal)
{
	m_fMinVal = fMinVal;
	m_fMaxVal = fMaxVal;
	m_bHaveRange = TRUE;
}

void CNumEdit::CreateSpinCtrl()
{
	ASSERT(m_pSpin == NULL);
	m_pSpin = new CNumSpin;
	UINT	style = WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS;
	UINT	IDC_SPIN_OFFSET = 0x4000;
	UINT	nID = GetDlgCtrlID() + IDC_SPIN_OFFSET;
	if (!m_pSpin->Create(style, CRect(0, 0, 0, 0), GetParent(), nID))
		AfxThrowResourceException();
	m_pSpin->SetWindowPos(this, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	// set Z order
	m_pSpin->SetBuddy(this);
}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
	//{{AFX_MSG_MAP(CNumEdit)
	ON_CONTROL_REFLECT_EX(EN_KILLFOCUS, OnKillfocus)
	ON_WM_CHAR()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumEdit message handlers

BOOL CNumEdit::OnKillfocus() 
{
	if (GetModify()) {
		GetText();
		if (m_bHaveRange) {
			m_fVal = CLAMP(m_fVal, m_fMinVal, m_fMaxVal);
			SetText();
		}
		Notify();
	}
	return FALSE;	// let parent handle notification too
}

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (IsValidChar(nChar))
		CEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CNumEdit::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		OnKillfocus();
		SetSel(0, -1);	// select entire text
	}
	return CEdit::PreTranslateMessage(pMsg);
}

void CNumEdit::PreSubclassWindow() 
{
	SetVal(m_fVal);
	CEdit::PreSubclassWindow();
	if (m_nFormat & DF_SPIN)	// if spin control requested
		CreateSpinCtrl();
}

void CNumEdit::OnEnable(BOOL bEnable) 
{
	CEdit::OnEnable(bEnable);
	if (m_pSpin != NULL)	// if spin control attached
		m_pSpin->Invalidate();	// make sure it gets repainted
}
