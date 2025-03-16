// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03sep05	initial version
        01      15mar25	refactor

        movie export options dialog
 
*/

// MovieExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Whorld.h"
#include "MovieExportDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg dialog

IMPLEMENT_DYNAMIC(CMovieExportDlg, CDialog);

CMovieExportDlg::CMovieExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMovieExportDlg::IDD, pParent)
{
	m_szFrame = CSize(0, 0);
	m_nFrameSelType = 0;
	m_nTimeUnit = 0;
	m_nFrameSizePreset = 0;
	m_nScaleToFit = 0;
	m_nRangeStart = 0;
	m_nRangeEnd = 0;
}

void CMovieExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEX_FRAME_SIZE_COMBO, m_comboFrameSize);
	DDX_Control(pDX, IDC_MEX_SCALE_TO_FIT_COMBO, m_comboScaleToFit);
	DDX_Text(pDX, IDC_MEX_FRAME_WIDTH_EDIT, m_szFrame.cx);
	DDX_Text(pDX, IDC_MEX_FRAME_HEIGHT_EDIT, m_szFrame.cy);
	DDX_Radio(pDX, IDC_MEX_FRAME_SEL_TYPE_1, m_nFrameSelType);
	DDX_Radio(pDX, IDC_MEX_TIME_UNIT_1, m_nTimeUnit);
	DDX_Text(pDX, IDC_MEX_RANGE_START_EDIT, m_nRangeStart);
	DDX_Text(pDX, IDC_MEX_RANGE_END_EDIT, m_nRangeEnd);
	DDX_Text(pDX, IDC_MEX_DURATION_EDIT, m_nDuration);
}

BEGIN_MESSAGE_MAP(CMovieExportDlg, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDC_MEX_FRAME_WIDTH_EDIT, OnUpdateFrameSize)
	ON_UPDATE_COMMAND_UI(IDC_MEX_FRAME_HEIGHT_EDIT, OnUpdateFrameSize)
	ON_UPDATE_COMMAND_UI(IDC_MEX_SCALE_TO_FIT_COMBO, OnUpdateScaleToFit)
	ON_UPDATE_COMMAND_UI(IDC_MEX_RANGE_START_EDIT, OnUpdateFrameSelType)
	ON_UPDATE_COMMAND_UI(IDC_MEX_RANGE_END_EDIT, OnUpdateFrameSelType)
	ON_UPDATE_COMMAND_UI(IDC_MEX_DURATION_EDIT, OnUpdateFrameSelType)
	ON_CBN_SELCHANGE(IDC_MEX_FRAME_SIZE_COMBO, OnSelchangeFrameSize)
	ON_BN_CLICKED(IDC_MEX_FRAME_SEL_TYPE_1, OnClickedFrameSelType)
	ON_BN_CLICKED(IDC_MEX_FRAME_SEL_TYPE_2, OnClickedFrameSelType)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg message handlers

BOOL CMovieExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// initialize scale to fit drop list from options
	for (int iSTF = 0; iSTF < COptions::SCALE_TO_FIT_TYPES; iSTF++) {
		m_comboScaleToFit.AddString(LDS(COptions::m_oiScaleToFit[iSTF].nNameID));
	}
	m_comboFrameSize.SetCurSel(m_nFrameSizePreset);
	m_comboScaleToFit.SetCurSel(m_nScaleToFit);
	OnSelchangeFrameSize();
	OnClickedFrameSelType();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMovieExportDlg::OnOK() 
{
	CDialog::OnOK();
}

LRESULT CMovieExportDlg::OnKickIdle(WPARAM, LPARAM)
{
	UpdateDialogControls(this, FALSE); 
	return 0;
}

void CMovieExportDlg::OnUpdateFrameSize(CCmdUI *pCmdUI)
{
	int	nOpt = m_comboFrameSize.GetCurSel();
	pCmdUI->Enable(nOpt == 2);
}

void CMovieExportDlg::OnUpdateScaleToFit(CCmdUI *pCmdUI)
{
	int	nOpt = m_comboFrameSize.GetCurSel();
	pCmdUI->Enable(nOpt > 0);
}

void CMovieExportDlg::OnUpdateFrameSelType(CCmdUI *pCmdUI)
{
	int	nOpt = IsDlgButtonChecked(IDC_MEX_FRAME_SEL_TYPE_2);
	pCmdUI->Enable(nOpt > 0);
}

void CMovieExportDlg::OnSelchangeFrameSize()
{
	int	nOpt = m_comboFrameSize.GetCurSel();
	if (nOpt < 2) {
		// display render target size in width/height edit controls
		UpdateData(true);	// retrieve data from controls
		D2D1_SIZE_F	szTarget;
		if (nOpt == 0) {
			szTarget = theApp.m_thrRender.GetMovieFrameSize();
		} else {
			szTarget = theApp.m_thrRender.GetTargetSize();
		}
		m_szFrame.cx = Round(szTarget.width);
		m_szFrame.cy = Round(szTarget.height);
		UpdateData(false);	// initialize controls from data
	}
}

void CMovieExportDlg::OnClickedFrameSelType()
{
	int	nOpt = IsDlgButtonChecked(IDC_MEX_FRAME_SEL_TYPE_1);
	if (nOpt) {
		UpdateData(true);	// retrieve data from controls
		int	nFrames = static_cast<int>(theApp.m_thrRender.GetMovieFrameCount());
		m_nRangeStart = 0;
		m_nRangeEnd = nFrames - 1;
		m_nDuration = nFrames;
		UpdateData(false);	// initialize controls from data
	}
}
