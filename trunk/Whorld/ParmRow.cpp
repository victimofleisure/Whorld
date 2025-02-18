// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		27apr05	remove log base, add scale
		02		04may05	remove oscillator
		03		17may05	add SetNotifyWnd
		04		17may05	add pulse width
		05		13aug05	derive from CRowDialogRow
		06		06sep05	add EnableLFO
		07		15mar06	add random ramp waveform
		08		28jan08	support Unicode

        parameter row form
 
*/

// ParmRow.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ParmRow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParmRow dialog

IMPLEMENT_DYNAMIC(CParmRow, CRowDialogRow);

// this list must match the Oscillator::WAVEFORM enum
const int CParmRow::m_WaveID[WAVEFORMS] = {
	IDS_WAVE_TRIANGLE,
	IDS_WAVE_SINE,
	IDS_WAVE_RAMP_UP,
	IDS_WAVE_RAMP_DOWN,
	IDS_WAVE_SQUARE,
	IDS_WAVE_PULSE,
	IDS_WAVE_RANDOM,
	IDS_WAVE_RANDOM_RAMP
};

CParmRow::CParmRow(CWnd* pParent /*=NULL*/)
	: CRowDialogRow(CParmRow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CParmRow)
	//}}AFX_DATA_INIT
	m_MinVal = 0;
	m_MaxVal = 0;
	m_Steps = 100;
}

void CParmRow::DoDataExchange(CDataExchange* pDX)
{
	CRowDialogRow::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParmRow)
	DDX_Control(pDX, IDC_PM_PARM_SPIN, m_ParmSpin);
	DDX_Control(pDX, IDC_PM_MOD_AMP_SPIN, m_ModAmpSpin);
	DDX_Control(pDX, IDC_PM_MOD_FREQ_SPIN, m_ModFreqSpin);
	DDX_Control(pDX, IDC_PM_MOD_PW_SPIN, m_ModPWSpin);
	DDX_Control(pDX, IDC_PM_TITLE, m_Title);
	DDX_Control(pDX, IDC_PM_PARM_EDIT, m_ParmEdit);
	DDX_Control(pDX, IDC_PM_PARM_SLIDER, m_ParmSlider);
	DDX_Control(pDX, IDC_PM_MOD_WAVE, m_ModWave);
	DDX_Control(pDX, IDC_PM_MOD_AMP, m_ModAmp);
	DDX_Control(pDX, IDC_PM_MOD_FREQ, m_ModFreq);
	DDX_Control(pDX, IDC_PM_MOD_PW, m_ModPW);
	//}}AFX_DATA_MAP
}

void CParmRow::UpdateSlider()
{
	m_ParmSlider.SetPos(round((m_ParmEdit.GetVal() - m_MinVal) 
		/ (m_MaxVal - m_MinVal) * m_Steps));
}

void CParmRow::NotifyEdit(int CtrlID)
{
	if (m_NotifyWnd != NULL)
		m_NotifyWnd->SendMessage(UWM_PARMROWEDIT, m_RowIdx, CtrlID);
}

void CParmRow::SetVal(double Val)
{
	m_ParmEdit.SetVal(Val);
	UpdateSlider();
}

double CParmRow::GetVal()
{
	return(m_ParmEdit.GetVal());
}

void CParmRow::SetCaption(LPCTSTR Title)
{
	m_Title.SetWindowText(Title);
}

void CParmRow::GetInfo(CParmInfo::ROW& Info) const
{
	Info.Val = m_ParmEdit.GetVal();
	Info.Wave = m_ModWave.GetCurSel();
	Info.Amp = m_ModAmp.GetVal();
	Info.Freq = m_ModFreq.GetVal();
	Info.PW = m_ModPW.GetVal();
}

void CParmRow::SetInfo(const CParmInfo::ROW& Info)
{
	SetVal(Info.Val);
	m_ModWave.SetCurSel(Info.Wave);
	m_ModAmp.SetVal(Info.Amp);
	m_ModFreq.SetVal(Info.Freq);
	m_ModPW.SetVal(Info.PW);
}

void CParmRow::SetSliderRange(double MinVal, double MaxVal, int Steps)
{
	m_MinVal = MinVal;
	m_MaxVal = MaxVal;
	m_Steps = Steps;
	m_ParmSlider.SetRange(0, Steps);
	m_ParmSlider.SetDefaultPos(Steps / 2);
	m_ParmSpin.SetDelta((MaxVal - MinVal) / Steps);
}

void CParmRow::SetScale(double Scale)
{
	m_ParmEdit.SetScale(Scale);
	m_ModAmp.SetScale(Scale);
}

void CParmRow::EnableLFO(bool Enable)
{
	m_ModWave.EnableWindow(Enable);
	m_ModAmp.EnableWindow(Enable);
	m_ModFreq.EnableWindow(Enable);
	m_ModPW.EnableWindow(Enable);
}

BEGIN_MESSAGE_MAP(CParmRow, CRowDialogRow)
	//{{AFX_MSG_MAP(CParmRow)
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_PM_MOD_WAVE, OnSelchangeModWave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParmRow message handlers

BOOL CParmRow::OnInitDialog() 
{
	CRowDialogRow::OnInitDialog();

	CString	s;
	for (int i = 0; i < WAVEFORMS; i++) {
		s.LoadString(m_WaveID[i]);
		m_ModWave.AddString(s);
	}
	m_ModWave.SetCurSel(0);
	m_ModAmpSpin.SetDelta(.01);
	m_ModFreq.SetRange(0, INT_MAX);
	m_ModFreqSpin.SetDelta(.01);
	m_ModPW.SetRange(0, 1);
	m_ModPWSpin.SetDelta(.01);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CParmRow::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CWnd	*psb = pScrollBar;
	if (psb == &m_ParmSlider) {
		SetVal(double(m_ParmSlider.GetPos()) 
			/ m_Steps * (m_MaxVal - m_MinVal) + m_MinVal);
		NotifyEdit(IDC_PM_PARM_EDIT);
	}

	CRowDialogRow::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CParmRow::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch (wParam) {
	case IDC_PM_PARM_EDIT:
		UpdateSlider();
		NotifyEdit(wParam);
		break;
	case IDC_PM_MOD_AMP:
	case IDC_PM_MOD_FREQ:
	case IDC_PM_MOD_PW:
		NotifyEdit(wParam);
		break;
	}
	return CRowDialogRow::OnNotify(wParam, lParam, pResult);
}

void CParmRow::OnSelchangeModWave() 
{
	NotifyEdit(IDC_PM_MOD_WAVE);
}
