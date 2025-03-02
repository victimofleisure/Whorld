// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02mar25	initial version

*/

#include "stdafx.h"
#include "Whorld.h"
#include "GlobalsBar.h"
#include "GlobalsRowDlg.h"
#include "WhorldDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CGlobalsRowDlg

// define aliases for parameter property controls, accounting for the control type prefix
// in our child control names; allows initialization code to be generated via preprocessor
#define m_Val m_editVal

CGlobalsRowDlg::CGlobalsRowDlg() : CRowDlg(IDD_PARAM_ROW)
{
}

CGlobalsRowDlg::~CGlobalsRowDlg()
{
}

inline CGlobalsView* CGlobalsRowDlg::GetView()
{
	CGlobalsView	*pView = STATIC_DOWNCAST(CGlobalsView, CRowDlg::GetView());
	ASSERT(pView != NULL);
	return pView;
}

void CGlobalsRowDlg::Update(double fVal)
{
	m_Val.SetVal(fVal);	// only one property
}

double CGlobalsRowDlg::CMyEditSliderCtrl::Norm(double x) const
{
	CGlobalsRowDlg	*pRowDlg = STATIC_DOWNCAST(CGlobalsRowDlg, GetParent());
	int	iRow = pRowDlg->GetRowPos();
	const PARAM_INFO&	info = GetParamInfo(iRow);
	// use maximum value and its negation for symmetrical range
	return (x + info.fMaxVal) / (info.fMaxVal * 2) * info.nSteps;
}

double CGlobalsRowDlg::CMyEditSliderCtrl::Denorm(double x) const
{
	CGlobalsRowDlg	*pRowDlg = STATIC_DOWNCAST(CGlobalsRowDlg, GetParent());
	int	iRow = pRowDlg->GetRowPos();
	const PARAM_INFO&	info = GetParamInfo(iRow);
	// use maximum value and its negation for symmetrical range
	return x / info.nSteps * (info.fMaxVal * 2) - info.fMaxVal;
}

void CGlobalsRowDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PROW_NAME_STATIC, m_staticName);
	DDX_Control(pDX, IDC_PROW_VAL_SLIDER, m_sliderVal);
	DDX_Control(pDX, IDC_PROW_Val, m_editVal);
	DDX_Control(pDX, IDC_PROW_VAL_SPIN, m_spinVal);
	CRowDlg::DoDataExchange(pDX);
}

BOOL CGlobalsRowDlg::OnInitDialog()
{
	CRowDlg::OnInitDialog();

	// initialize child controls
	int	iParam = m_iPos;	// map from row index to parameter index
	const PARAM_INFO&	info = GetParamInfo(iParam);
	m_staticName.SetWindowText(GetParamName(iParam));
	m_editVal.SetRange(info.fMinVal, info.fMaxVal);
	m_editVal.SetScale(info.fScale);
	m_sliderVal.SetRange(0, info.nSteps);
	m_sliderVal.SetDefaultPos(info.nSteps / 2);
	m_sliderVal.SetEditCtrl(&m_editVal);
	double	fMinVal = -info.fMaxVal;	// minimum is negative maximum
	m_spinVal.SetDelta((info.fMaxVal - fMinVal) / info.nSteps);

	return TRUE;  // return TRUE unless you set the focus to a control
}

inline void CGlobalsRowDlg::SetParam(double fVal)
{
	// get the parameter index from the row's position, not its index, 
	// because the globals are a discontinuous subset of the parameters,
	// and the position tells us to which row the parameter corresponds
	theApp.GetDocument()->SetParam(m_iPos, PARAM_PROP_Global, fVal, GetView());
}

// CGlobalsRowDlg message map

BEGIN_MESSAGE_MAP(CGlobalsRowDlg, CRowDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_PROW_Val, OnChangedVal)
END_MESSAGE_MAP()

// CGlobalsRowDlg message handlers

void CGlobalsRowDlg::OnChangedVal(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	SetParam(m_editVal.GetVal());
	*pResult = 0;
}
