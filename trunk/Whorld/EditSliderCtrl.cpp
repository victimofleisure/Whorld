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
	m_Edit = NULL;
	m_Val = 0;
	m_Scale = 1;
	m_LogBase = 0;
}

CEditSliderCtrl::~CEditSliderCtrl()
{
}

double CEditSliderCtrl::Norm(double x) const
{
	if (m_LogBase)
		x = log(x) / log(m_LogBase);
	return(x * m_Scale);
}

double CEditSliderCtrl::Denorm(double x) const
{
	x /= m_Scale;
	if (m_LogBase)
		x = pow(m_LogBase, x);
	return(x);
}

void CEditSliderCtrl::SetVal(double Val)
{
	m_Val = Val;
	SetPos(Round(Norm(m_Val)));
	if (m_Edit != NULL)
		m_Edit->SetVal(m_Val, CNumEdit::NTF_NONE);	// don't notify anyone
}

void CEditSliderCtrl::SetValNorm(double Val)
{
	int	nMin, nMax;
	GetRange(nMin, nMax);
	SetVal(Denorm(Val * (nMax - nMin) + nMin));
}

double CEditSliderCtrl::GetValNorm() const
{
	int	nMin, nMax;
	GetRange(nMin, nMax);
	return((Norm(m_Val) - nMin) / (nMax - nMin));
}

double CEditSliderCtrl::Norm(const INFO& Info, double Val)
{
	if (Info.LogBase)
		Val = log(Val) / log(double(Info.LogBase));
	return(Val * Info.SliderScale);
}

double CEditSliderCtrl::Denorm(const INFO& Info, double Val)
{
	Val /= Info.SliderScale;
	if (Info.LogBase)
		Val = pow(double(Info.LogBase), Val);
	return(Val);
}

void CEditSliderCtrl::SetEditCtrl(CNumEdit *Edit)
{
	m_Edit = Edit;
	Edit->SetAuxNotify(this);
}

void CEditSliderCtrl::SetInfo(const INFO& Info, CNumEdit *Edit)
{
	SetEditCtrl(Edit);
	SetRange(Info.RangeMin, Info.RangeMax, TRUE);
	SetLogBase(Info.LogBase);
	SetScale(Info.SliderScale);
	SetDefaultPos(Info.DefaultPos);
	SetTicCount(Info.TicCount);
	if (m_Edit != NULL) {
		m_Edit->SetScale(Info.EditScale);
		m_Edit->SetPrecision(Info.EditPrecision);
	}
}

double CEditSliderCtrl::GetDefaultVal() const
{
	return(Denorm(GetDefaultPos()));
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
	if (val != m_Val) {	// if value changed
		m_Val = val;
		if (m_Edit != NULL)
			m_Edit->SetVal(m_Val, CNumEdit::NTF_PARENT);	// notify parent only
	}
}

void CEditSliderCtrl::OnDestroy() 
{
	CClickSliderCtrl::OnDestroy();
	if (m_Edit != NULL) {
		m_Edit->SetAuxNotify(NULL);
		m_Edit = NULL;
	}
}

BOOL CEditSliderCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR	*nmh = (NMHDR *)lParam;
	if (m_Edit != NULL && nmh->hwndFrom == m_Edit->m_hWnd) {
		m_Val = m_Edit->GetVal();
		SetPos(Round(Norm(m_Val)));
	}
	return CClickSliderCtrl::OnNotify(wParam, lParam, pResult);
}
