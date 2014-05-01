// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06jul05	initial version
		01		13jul05	separate state from controls
		02		23jul05	convert to use edit slider
		03		03aug05	add wheel sensitivity
		04		28jan08	support Unicode

        color options property page
 
*/

// OptsInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "OptsInputDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsInputDlg property page

IMPLEMENT_DYNCREATE(COptsInputDlg, CPropertyPage)

const CEditSliderCtrl::INFO COptsInputDlg::m_SliderInfo[SLIDERS] = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	{-100,	100,	4,		100,	0,		3,		.01f,	0},	// MouseSens
	{-100,	100,	4,		100,	0,		3,		.01f,	0},	// WheelSens
	{100,	900,	0,		100000,	250,	3,		.01f,	3}	// TempoNudge
};

const COptsInputDlg::STATE COptsInputDlg::m_Default =
{
	1,	// MouseSens
	1,	// WheelSens
	m_SliderInfo[TEMPO_NUDGE].DefaultPos	// TempoNudge
		/ m_SliderInfo[TEMPO_NUDGE].SliderScale
};

#define REG_VALUE(name) m_Reg##name(_T(#name), m_st.name, m_Default.name)

COptsInputDlg::COptsInputDlg() : CPropertyPage(COptsInputDlg::IDD),
	REG_VALUE(MouseSens),
	REG_VALUE(WheelSens),
	REG_VALUE(TempoNudge)
{
	//{{AFX_DATA_INIT(COptsInputDlg)
	//}}AFX_DATA_INIT
}

void COptsInputDlg::SetDefaults()
{
	m_st = m_Default;
}

void COptsInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsInputDlg)
	DDX_Control(pDX, IDC_OP_WHEEL_SENS_SLIDER, m_WheelSensSlider);
	DDX_Control(pDX, IDC_OP_WHEEL_SENS_EDIT, m_WheelSensEdit);
	DDX_Control(pDX, IDC_OP_TEMPO_NUDGE_SLIDER, m_TempoNudgeSlider);
	DDX_Control(pDX, IDC_OP_TEMPO_NUDGE_EDIT, m_TempoNudgeEdit);
	DDX_Control(pDX, IDC_OP_MOUSE_SENS_SLIDER, m_MouseSensSlider);
	DDX_Control(pDX, IDC_OP_MOUSE_SENS_EDIT, m_MouseSensEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptsInputDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsInputDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsInputDlg message handlers

BOOL COptsInputDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_MouseSensSlider.SetInfo(m_SliderInfo[MOUSE_SENS], &m_MouseSensEdit);
	m_MouseSensSlider.SetTicCount(3);
	m_MouseSensSlider.SetVal(m_st.MouseSens);
	m_WheelSensSlider.SetInfo(m_SliderInfo[WHEEL_SENS], &m_WheelSensEdit);
	m_WheelSensSlider.SetTicCount(3);
	m_WheelSensSlider.SetVal(m_st.WheelSens);
	m_TempoNudgeSlider.SetInfo(m_SliderInfo[TEMPO_NUDGE], &m_TempoNudgeEdit);
	m_TempoNudgeSlider.SetTicCount(3);
	m_TempoNudgeSlider.SetVal(m_st.TempoNudge);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void COptsInputDlg::OnOK() 
{
	CPropertyPage::OnOK();
	m_st.MouseSens = m_MouseSensSlider.GetVal();
	m_st.WheelSens = m_WheelSensSlider.GetVal();
	m_st.TempoNudge = m_TempoNudgeSlider.GetVal();
}
