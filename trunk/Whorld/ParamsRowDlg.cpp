// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb25	initial version
		02		01mar25	exclude globals property

*/

#include "stdafx.h"
#include "Whorld.h"
#include "ParamsBar.h"
#include "ParamsRowDlg.h"
#include "WhorldDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CParamsRowDlg

// define aliases for parameter property controls, accounting for the control type prefix
// in our child control names; allows initialization code to be generated via preprocessor
#define m_Val m_editVal
#define m_Wave m_comboWave
#define m_Amp m_editAmp
#define m_Freq m_editFreq
#define m_PW m_editPW

CParamsRowDlg::CParamsRowDlg() : CRowDlg(IDD_PARAM_ROW)
{
}

CParamsRowDlg::~CParamsRowDlg()
{
}

inline CParamsView* CParamsRowDlg::GetView()
{
	CParamsView	*pView = STATIC_DOWNCAST(CParamsView, CRowDlg::GetView());
	ASSERT(pView != NULL);
	return pView;
}

void CParamsRowDlg::Update(const PARAM_ROW& row)
{
	// update all child controls from row data
	#define	PARAMPROPDEF_GLOBAL 0	// exclude global property
	#define PARAMPROPDEF(name, type, prefix, variant) m_##name.SetVal(row.prefix##name);
	#include "WhorldDef.h"	// generate code to update controls
}

void CParamsRowDlg::Update(const PARAM_ROW& row, int iProp)
{
	// update specified child control from row data
	switch (iProp) {
	#define	PARAMPROPDEF_GLOBAL 0	// exclude global property
	#define PARAMPROPDEF(name, type, prefix, variant) \
	case PARAM_PROP_##name: m_##name.SetVal(row.prefix##name); break;
	#include "WhorldDef.h"	// generate code to update controls
	case PARAM_PROP_Global:
		break;	// ignore global property
	default:
		ASSERT(0);	// unknown parameter property; logic error
	}
}

double CParamsRowDlg::CMyEditSliderCtrl::Norm(double x) const
{
	CParamsRowDlg	*pRowDlg = STATIC_DOWNCAST(CParamsRowDlg, GetParent());
	int	iRow = pRowDlg->GetRowIndex();
	const PARAM_INFO&	info = GetParamInfo(iRow);
	return (x - info.fMinVal) / (info.fMaxVal - info.fMinVal) * info.nSteps;
}

double CParamsRowDlg::CMyEditSliderCtrl::Denorm(double x) const
{
	CParamsRowDlg	*pRowDlg = STATIC_DOWNCAST(CParamsRowDlg, GetParent());
	int	iRow = pRowDlg->GetRowIndex();
	const PARAM_INFO&	info = GetParamInfo(iRow);
	return x / info.nSteps * (info.fMaxVal - info.fMinVal) + info.fMinVal;
}

void CParamsRowDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PROW_NAME_STATIC, m_staticName);
	DDX_Control(pDX, IDC_PROW_VAL_SLIDER, m_sliderVal);
	DDX_Control(pDX, IDC_PROW_Val, m_editVal);
	DDX_Control(pDX, IDC_PROW_VAL_SPIN, m_spinVal);
	DDX_Control(pDX, IDC_PROW_Wave, m_comboWave);
	DDX_Control(pDX, IDC_PROW_Amp, m_editAmp);
	DDX_Control(pDX, IDC_PROW_AMP_SPIN, m_spinAmp);
	DDX_Control(pDX, IDC_PROW_Freq, m_editFreq);
	DDX_Control(pDX, IDC_PROW_FREQ_SPIN, m_spinFreq);
	DDX_Control(pDX, IDC_PROW_PW, m_editPW);
	DDX_Control(pDX, IDC_PROW_PW_SPIN, m_spinPW);
	CRowDlg::DoDataExchange(pDX);
}

BOOL CParamsRowDlg::OnInitDialog()
{
	CRowDlg::OnInitDialog();

	// initialize child controls
	int	iParam = m_iRow;
	const PARAM_INFO&	info = GetParamInfo(iParam);
	m_staticName.SetWindowText(GetParamName(iParam));
	m_editVal.SetRange(info.fMinVal, info.fMaxVal);
	m_editVal.SetScale(info.fScale);
	m_sliderVal.SetRange(0, info.nSteps);
	m_sliderVal.SetDefaultPos(info.nSteps / 2);
	m_sliderVal.SetEditCtrl(&m_editVal);
	m_spinVal.SetDelta((info.fMaxVal - info.fMinVal) / info.nSteps);
	for (int iWave = 0; iWave < WAVEFORM_COUNT; iWave++) {	// for each waveform
		m_comboWave.AddString(GetWaveformName(iWave));	// add name to combo box
	}
	m_editFreq.SetRange(0, INT_MAX);
	m_editAmp.SetScale(info.fScale);
	m_spinAmp.SetDelta(0.01);
	m_spinFreq.SetDelta(0.01);
	m_editPW.SetRange(0, 1);
	m_spinPW.SetDelta(0.01);

	return TRUE;  // return TRUE unless you set the focus to a control
}

inline void CParamsRowDlg::SetParam(int iProp, const CComVariant& prop)
{
	theApp.GetDocument()->SetParam(m_iRow, iProp, prop, GetView());
}

// CParamsRowDlg message map

BEGIN_MESSAGE_MAP(CParamsRowDlg, CRowDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_PROW_Val, OnChangedVal)
	ON_CBN_SELCHANGE(IDC_PROW_Wave, OnSelChangeWave)
	ON_NOTIFY(NEN_CHANGED, IDC_PROW_Amp, OnChangedAmp)
	ON_NOTIFY(NEN_CHANGED, IDC_PROW_Freq, OnChangedFreq)
	ON_NOTIFY(NEN_CHANGED, IDC_PROW_PW, OnChangedPW)
END_MESSAGE_MAP()

// CParamsRowDlg message handlers

void CParamsRowDlg::OnChangedVal(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	SetParam(PARAM_PROP_Val, m_editVal.GetVal());
	*pResult = 0;
}

void CParamsRowDlg::OnSelChangeWave()
{
	SetParam(PARAM_PROP_Wave, m_comboWave.GetCurSel());
}

void CParamsRowDlg::OnChangedAmp(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	SetParam(PARAM_PROP_Amp, m_editAmp.GetVal());
	*pResult = 0;
}

void CParamsRowDlg::OnChangedFreq(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	SetParam(PARAM_PROP_Freq, m_editFreq.GetVal());
	*pResult = 0;
}

void CParamsRowDlg::OnChangedPW(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	SetParam(PARAM_PROP_PW, m_editPW.GetVal());
	*pResult = 0;
}
