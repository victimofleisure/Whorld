// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jul05	initial version
        01      02dec06	in HScroll, ignore button up
		02		04mar09	in set/get val norm, use min, not only max
		03		04mar09	add static norm/denorm functions
		04		12may11	in HScroll, notify only if value changed
		05		30may11	in HScroll, handle all scroll-bar codes
		06		09feb25	rename round function
		07		19feb25	remove range shift from static norm/denorm
		08		03mar25	modernize style

		slider with buddy numeric edit control
 
*/

// EditSliderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "EditSliderCtrl.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditSliderCtrl

IMPLEMENT_DYNAMIC(CEditSliderCtrl, CClickSliderCtrl);

CEditSliderCtrl::CEditSliderCtrl()
{
	m_pEdit = NULL;
	m_fVal = 0;
	m_fScale = 1;
	m_fLogBase = 0;
}

CEditSliderCtrl::~CEditSliderCtrl()
{
}

double CEditSliderCtrl::Norm(double x) const
{
	if (m_fLogBase)
		x = log(x) / log(m_fLogBase);
	return x * m_fScale;
}

double CEditSliderCtrl::Denorm(double x) const
{
	x /= m_fScale;
	if (m_fLogBase)
		x = pow(m_fLogBase, x);
	return x;
}

void CEditSliderCtrl::SetVal(double fVal)
{
	m_fVal = fVal;
	SetPos(Round(Norm(m_fVal)));
	if (m_pEdit != NULL)
		m_pEdit->SetVal(m_fVal, CNumEdit::NTF_NONE);	// don't notify anyone
}

void CEditSliderCtrl::SetValNorm(double fVal)
{
	int	nMin, nMax;
	GetRange(nMin, nMax);
	SetVal(Denorm(fVal * (nMax - nMin) + nMin));
}

double CEditSliderCtrl::GetValNorm() const
{
	int	nMin, nMax;
	GetRange(nMin, nMax);
	return (Norm(m_fVal) - nMin) / (nMax - nMin);
}

double CEditSliderCtrl::Norm(const INFO& info, double fVal)
{
	if (info.fLogBase)
		fVal = log(fVal) / log(double(info.fLogBase));
	return fVal * info.fSliderScale;
}

double CEditSliderCtrl::Denorm(const INFO& info, double fVal)
{
	fVal /= info.fSliderScale;
	if (info.fLogBase)
		fVal = pow(double(info.fLogBase), fVal);
	return fVal;
}

void CEditSliderCtrl::SetEditCtrl(CNumEdit *Edit)
{
	m_pEdit = Edit;
	Edit->SetAuxNotify(this);
}

void CEditSliderCtrl::SetInfo(const INFO& info, CNumEdit *Edit)
{
	SetEditCtrl(Edit);
	SetRange(info.nRangeMin, info.nRangeMax, TRUE);
	SetfLogBase(info.fLogBase);
	SetScale(info.fSliderScale);
	SetDefaultPos(info.nDefaultPos);
	SetTicCount(info.nTicCount);
	if (m_pEdit != NULL) {
		m_pEdit->SetScale(info.fEditScale);
		m_pEdit->SetPrecision(info.nEditPrecision);
	}
}

double CEditSliderCtrl::GetDefaultVal() const
{
	return Denorm(GetDefaultPos());
}

BEGIN_MESSAGE_MAP(CEditSliderCtrl, CClickSliderCtrl)
	//{{AFX_MSG_MAP(CEditSliderCtrl)
	ON_WM_HSCROLL_REFLECT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditSliderCtrl message handlers

void CEditSliderCtrl::HScroll(UINT nSBCode, UINT nPos)
{
	UNREFERENCED_PARAMETER(nSBCode);
	UNREFERENCED_PARAMETER(nPos);
	int	pos = GetPos();
	double	val = Denorm(pos);
	if (val != m_fVal) {	// if value changed
		m_fVal = val;
		if (m_pEdit != NULL)
			m_pEdit->SetVal(m_fVal, CNumEdit::NTF_PARENT);	// notify parent only
	}
}

void CEditSliderCtrl::OnDestroy() 
{
	CClickSliderCtrl::OnDestroy();
	if (m_pEdit != NULL) {
		m_pEdit->SetAuxNotify(NULL);
		m_pEdit = NULL;
	}
}

BOOL CEditSliderCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR	*nmh = (NMHDR *)lParam;
	if (m_pEdit != NULL && nmh->hwndFrom == m_pEdit->m_hWnd) {
		m_fVal = m_pEdit->GetVal();
		SetPos(Round(Norm(m_fVal)));
	}
	return CClickSliderCtrl::OnNotify(wParam, lParam, pResult);
}
