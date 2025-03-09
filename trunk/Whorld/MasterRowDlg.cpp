// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10feb25	initial version
		01		25feb25	add edit precision for zoom, damping, and trail
		02		09mar25	set slider page size

*/

#include "stdafx.h"
#include "Whorld.h"
#include "MasterBar.h"
#include "MasterRowDlg.h"
#include "WhorldDoc.h"
#include "WhorldView.h"
#include <math.h>

// CMasterRowDlg

const CEditSliderCtrl::INFO CMasterRowDlg::m_arrSliderInfo[MASTER_COUNT] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{-100,	100,	20,		100,	0,		0,		.01f,	0},	// Speed
	{-100,	100,	10,		100,	0,		0,		.01f,	2},	// Zoom
	{0,		100,	20,		101,	50,		0,		100,	2},	// Damping
	{0,		100,	10,		100,	0,		0,		.01f,	2},	// Trail
	{0,		100,	30,		100,	100,	0,		1,		0},	// Rings
	{0,		250,	0,		1,		100,	0,		1,		2},	// Tempo
	{0,		360,	0,		1,		30,		0,		1,		0},	// Hue Loop
	{100,	300,	0,		100,	140,	0,		.01f,	0},	// Canvas
	{1,		20,		0,		1,		0,		0,		1,		0},	// Copies
	{0,		600,	0,		1,		0,		0,		1,		0},	// Spread
};

const CMasterRowDlg::DBL_RANGE CMasterRowDlg::m_arrEditRange[MASTER_COUNT] = {
	{0,		1e2},	// Speed
	{1e-3,	1e2},	// Zoom
	{1e-3,	1},		// Damping
	{0,		1},		// Trail
	{0,		CPatch::MAX_RINGS},	// Rings
};

CMasterRowDlg::CMasterRowDlg() : CRowDlg(IDD_MASTER_ROW)
{
}

CMasterRowDlg::~CMasterRowDlg()
{
}

inline CMasterView* CMasterRowDlg::GetView()
{
	CMasterView	*pView = STATIC_DOWNCAST(CMasterView, CRowDlg::GetView());
	ASSERT(pView != NULL);
	return pView;
}

void CMasterRowDlg::Update(double fVal)
{
	m_sliderVal.SetVal(fVal);
}

void CMasterRowDlg::GetPropEditRange(int iProp, DBL_RANGE& range)
{
	ASSERT(IsValidMasterProp(iProp));
	range = m_arrEditRange[iProp];
	if (!range.fMinVal && !range.fMaxVal) {
		const CEditSliderCtrl::INFO& info = m_arrSliderInfo[iProp];
		range.fMinVal = info.nRangeMin * info.fEditScale;
		range.fMaxVal = info.nRangeMax * info.fEditScale;
	}
}

inline double CMasterRowDlg::Log(double Base, double x)
{
	return log(x) / log(Base);	// log with arbitrary base
}

inline double CMasterRowDlg::LogNorm(double x, double Base, double Scale)
{
	// x is exponential from 0..1, result is linear from 0..Scale
	return Log(Base, x * (Base - 1) + 1) * Scale;
}

inline double CMasterRowDlg::ExpNorm(double x, double Base, double Scale)
{
	// x is linear from 0..Scale, result is exponential from 0..1
	return (pow(Base, x / Scale) - 1) / (Base - 1);
}

int CMasterRowDlg::GetRowIdx(const CWnd& wnd)
{
	CMasterRowDlg	*pRowDlg = STATIC_DOWNCAST(CMasterRowDlg, wnd.GetParent());
	return pRowDlg->GetRowIndex();
}

double CMasterRowDlg::SliderNorm(int iProp, double fVal)
{
	const CEditSliderCtrl::INFO&	info = m_arrSliderInfo[iProp];
	switch (iProp) {
	case MASTER_Damping:
		return info.fSliderScale - LogNorm(fVal, info.fLogBase, info.fSliderScale);
	case MASTER_Trail:
		return LogNorm(fVal, info.fLogBase, info.fSliderScale);
	case MASTER_Rings:
		return LogNorm(fVal / CPatch::MAX_RINGS, info.fLogBase, info.fSliderScale);
	default:
		return CEditSliderCtrl::Norm(info, fVal);
	}
}

double CMasterRowDlg::SliderDenorm(int iProp, double fVal)
{
	const CEditSliderCtrl::INFO&	info = m_arrSliderInfo[iProp];
	switch (iProp) {
	case MASTER_Damping:
		return ExpNorm(info.fSliderScale - fVal, info.fLogBase, info.fSliderScale);
	case MASTER_Trail:
		return ExpNorm(fVal, info.fLogBase, info.fSliderScale);
	case MASTER_Rings:
		return ExpNorm(fVal, info.fLogBase, info.fSliderScale) * CPatch::MAX_RINGS;
	default:
		return CEditSliderCtrl::Denorm(info, fVal);
	}
}

double CMasterRowDlg::Norm(int iProp, double fVal)
{
	const CEditSliderCtrl::INFO&	info = m_arrSliderInfo[iProp];
	fVal = SliderNorm(iProp, fVal);
	return (fVal - info.nRangeMin) / (info.nRangeMax - info.nRangeMin);
}

double CMasterRowDlg::Denorm(int iProp, double fVal)
{
	const CEditSliderCtrl::INFO&	info = m_arrSliderInfo[iProp];
	fVal = fVal * (info.nRangeMax - info.nRangeMin) + info.nRangeMin;
	return SliderDenorm(iProp, fVal);
}

double CMasterRowDlg::CMyEditSliderCtrl::Norm(double fVal) const
{
	int	iProp = GetRowIdx(*this);
	return CMasterRowDlg::SliderNorm(iProp, fVal);
}

double CMasterRowDlg::CMyEditSliderCtrl::Denorm(double fVal) const
{
	int	iProp = GetRowIdx(*this);
	return CMasterRowDlg::SliderDenorm(iProp, fVal);
}

void CMasterRowDlg::CMyNumEdit::StrToVal(LPCTSTR Str)
{
	int	iProp = GetRowIdx(*this);
	const CEditSliderCtrl::INFO&	info = m_arrSliderInfo[iProp];
	switch (iProp) {
	case MASTER_Damping:
		m_fVal = (info.fEditScale - _tstof(Str)) / info.fEditScale;
		break;
	default:
		CNumEdit::StrToVal(Str);
	}
}

void CMasterRowDlg::CMyNumEdit::ValToStr(CString& Str)
{
	int	iProp = GetRowIdx(*this);
	const CEditSliderCtrl::INFO&	info = m_arrSliderInfo[iProp];
	switch (iProp) {
	case MASTER_Damping:
		Str.Format(_T("%.*f"), info.nEditPrecision, info.fEditScale - m_fVal * info.fEditScale);
		break;
	default:
		CNumEdit::ValToStr(Str);
	}
}

void CMasterRowDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_MAST_NAME_STATIC, m_staticName);
	DDX_Control(pDX, IDC_MAST_VAL_SLIDER, m_sliderVal);
	DDX_Control(pDX, IDC_MAST_VAL_EDIT, m_editVal);
	CRowDlg::DoDataExchange(pDX);
}

BOOL CMasterRowDlg::OnInitDialog()
{
	CRowDlg::OnInitDialog();

	int	iMaster = m_iRow;
	m_staticName.SetWindowText(GetMasterName(iMaster));
	DBL_RANGE	range;
	GetPropEditRange(iMaster, range);
	m_editVal.SetRange(range.fMinVal, range.fMaxVal);
	const CEditSliderCtrl::INFO&	info = m_arrSliderInfo[iMaster];
	m_sliderVal.SetInfo(info, &m_editVal);
	m_sliderVal.SetPageSize((info.nRangeMax - info.nRangeMin) / SLIDER_PAGES);

	return TRUE;  // return TRUE unless you set the focus to a control
}

// CMasterRowDlg message map

BEGIN_MESSAGE_MAP(CMasterRowDlg, CRowDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_MAST_VAL_EDIT, OnChangedVal)
END_MESSAGE_MAP()

// CMasterRowDlg message handlers

void CMasterRowDlg::OnChangedVal(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	theApp.GetDocument()->SetMasterProp(m_iRow, m_editVal.GetVal());
	*pResult = 0;
}
