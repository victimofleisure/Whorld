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
	m_nFrameSizePreset = 0;
	m_nScaleToFit = 0;
	m_nFrameSelType = 0;
	m_nRangeStart = 0;
	m_nRangeEnd = 0;
	m_nDuration = 0;
	m_nTimeUnit = 0;
	m_nFrameCount = 0;
	m_fFrameRate = 0;
}

UINT CMovieExportDlg::GetExportFlags() const
{
	return COptions::GetExportFlags(m_nFrameSizePreset > 0, m_nScaleToFit);
}

void CMovieExportDlg::FrameToTime(int nFrames, COleDateTime& dt, float fFrameRate)
{
	dt.m_dt = nFrames / fFrameRate / SECONDS_PER_DAY;
}

int CMovieExportDlg::TimeToFrame(const COleDateTime& dt, float fFrameRate)
{
	return Round(dt.m_dt * fFrameRate * SECONDS_PER_DAY);
}

CString CMovieExportDlg::FrameToTimeString(int nFrames, float fFrameRate)
{
	COleDateTime	dt;
	FrameToTime(nFrames, dt, fFrameRate);
	return dt.Format(_T("%H:%M:%S"));
}

bool CMovieExportDlg::TimeStringToFrame(CString sTime, int& nFrame, float fFrameRate)
{
	COleDateTime	dt;
	if (!dt.ParseDateTime(sTime)) {	// if can't parse time
		return false;	// fail
	}
	nFrame = TimeToFrame(dt, fFrameRate);
	return true;
}

CString CMovieExportDlg::FrameToTimeString(int nFrame) const
{
	return FrameToTimeString(nFrame, m_fFrameRate);
}

bool CMovieExportDlg::TimeStringToFrame(CString sTime, int& nFrame) const
{
	return TimeStringToFrame(sTime, nFrame, m_fFrameRate);
}

void CMovieExportDlg::DDX_FrameTime(CDataExchange* pDX, int nIDC, int& value) const
{
	if (m_nTimeUnit == UNIT_TIME) {	// if unit is time
		CString	sTime;
		if (!pDX->m_bSaveAndValidate) {	// if initializing controls from data
			sTime = FrameToTimeString(value);
		}
		DDX_Text(pDX, nIDC, sTime);
		if (pDX->m_bSaveAndValidate) {	// if retrieving data from controls
			if (!TimeStringToFrame(sTime, value)) {	// if can't parse time
				AfxMessageBox(IDS_MEX_BAD_TIME);
				DDV_Fail(pDX, nIDC);
			}
		}
	} else {	// unit is frames
		DDX_Text(pDX, nIDC, value);	// ordinary integer
	}
	if (m_nModalResult != IDCANCEL) {	// if not canceling dialog
		if (pDX->m_bSaveAndValidate) {	// if retrieving data from controls
			bool	bIsDuration = nIDC == IDC_MEX_DURATION_EDIT;
			int	nMinVal = bIsDuration ? 1 : 0;
			int	nMaxVal = bIsDuration ? m_nFrameCount : m_nFrameCount - 1;
			value = CLAMP(value, nMinVal, nMaxVal);
		}
	}
}

void CMovieExportDlg::UpdateDuration()
{
    m_nDuration = m_nRangeEnd - m_nRangeStart + 1;
	m_nDuration = max(m_nDuration, 1);
}

void CMovieExportDlg::UpdateRangeEnd()
{
    m_nRangeEnd = m_nRangeStart + m_nDuration - 1;
	m_nRangeEnd = max(m_nRangeEnd, m_nRangeStart);
}

void CMovieExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEX_FRAME_SIZE_COMBO, m_comboFrameSize);
	DDX_Control(pDX, IDC_MEX_SCALE_TO_FIT_COMBO, m_comboScaleToFit);
	DDX_Text(pDX, IDC_MEX_FRAME_WIDTH_EDIT, m_szFrame.cx);
	DDX_Text(pDX, IDC_MEX_FRAME_HEIGHT_EDIT, m_szFrame.cy);
	DDX_Radio(pDX, IDC_MEX_FRAME_SEL_TYPE_1, m_nFrameSelType);
	if (!pDX->m_bSaveAndValidate) {	// if initializing controls from data
		// order matters: initialize time unit BEFORE initializing times
		DDX_Radio(pDX, IDC_MEX_TIME_UNIT_1, m_nTimeUnit);
	}
	DDX_FrameTime(pDX, IDC_MEX_RANGE_START_EDIT, m_nRangeStart);
	DDX_FrameTime(pDX, IDC_MEX_RANGE_END_EDIT, m_nRangeEnd);
	DDX_FrameTime(pDX, IDC_MEX_DURATION_EDIT, m_nDuration);
	// if retrieving data from controls and not canceling dialog
	if (pDX->m_bSaveAndValidate && m_nModalResult != IDCANCEL) {
		// order matters: retrieve time unit AFTER retrieving times
		DDX_Radio(pDX, IDC_MEX_TIME_UNIT_1, m_nTimeUnit);
		if (m_nRangeStart > m_nRangeEnd) {	// if range out of order
			AfxMessageBox(IDS_MEX_BAD_RANGE);
			DDV_Fail(pDX, IDC_MEX_RANGE_END_EDIT);
		}
	}
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
	ON_CONTROL_RANGE(BN_CLICKED, IDC_MEX_FRAME_SEL_TYPE_1, IDC_MEX_FRAME_SEL_TYPE_2, OnClickedFrameSelType)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_MEX_TIME_UNIT_1, IDC_MEX_TIME_UNIT_2, OnClickedTimeUnit)
	ON_EN_KILLFOCUS(IDC_MEX_RANGE_START_EDIT, OnKillFocusRange)
	ON_EN_KILLFOCUS(IDC_MEX_RANGE_END_EDIT, OnKillFocusRange)
	ON_EN_KILLFOCUS(IDC_MEX_DURATION_EDIT, OnKillFocusDuration)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg message handlers

BOOL CMovieExportDlg::OnInitDialog() 
{
	m_nDuration = 1;
	m_nFrameCount = static_cast<int>(theApp.m_thrRender.GetMovieFrameCount());
	m_fFrameRate = theApp.m_thrRender.GetMovieFrameRate();

	CDialog::OnInitDialog();

	// initialize scale to fit drop list from options
	for (int iFitType = 0; iFitType < COptions::SCALE_TO_FIT_TYPES; iFitType++) {
		m_comboScaleToFit.AddString(LDS(COptions::m_oiScaleToFit[iFitType].nNameID));
	}
	m_comboFrameSize.SetCurSel(m_nFrameSizePreset);
	m_comboScaleToFit.SetCurSel(m_nScaleToFit);
	OnSelchangeFrameSize();
	OnClickedFrameSelType(0);

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
	int	iPreset = m_comboFrameSize.GetCurSel();
	pCmdUI->Enable(iPreset == FSP_CUSTOM);
}

void CMovieExportDlg::OnUpdateScaleToFit(CCmdUI *pCmdUI)
{
	int	iFitType = m_comboFrameSize.GetCurSel();
	pCmdUI->Enable(iFitType > COptions::SCALE_TO_FIT_None);
}

void CMovieExportDlg::OnUpdateFrameSelType(CCmdUI *pCmdUI)
{
	int	iSelType = IsDlgButtonChecked(IDC_MEX_FRAME_SEL_TYPE_2);
	pCmdUI->Enable(iSelType > FST_ALL);
}

void CMovieExportDlg::OnSelchangeFrameSize()
{
	int	iPreset = m_comboFrameSize.GetCurSel();
	if (iPreset < FSP_CUSTOM) {
		// display render target size in width/height edit controls
		UpdateData(true);	// retrieve data from controls
		D2D1_SIZE_F	szTarget;
		if (iPreset == FSP_MOVIE) {	// if getting frame size from movie
			szTarget = theApp.m_thrRender.GetMovieFrameSize();
		} else {	// get frame size from render target size
			szTarget = theApp.m_thrRender.GetTargetSize();
		}
		m_szFrame.cx = Round(szTarget.width);
		m_szFrame.cy = Round(szTarget.height);
		UpdateData(false);	// initialize controls from data
	}
}

void CMovieExportDlg::OnClickedFrameSelType(UINT nID)
{
	UNREFERENCED_PARAMETER(nID);
	int	iSelType = IsDlgButtonChecked(IDC_MEX_FRAME_SEL_TYPE_1);
	if (iSelType > FST_ALL) {
		UpdateData(true);	// retrieve data from controls
		int	nFrames = static_cast<int>(m_nFrameCount);
		m_nRangeStart = 0;
		m_nRangeEnd = nFrames - 1;
		m_nDuration = nFrames;
		UpdateData(false);	// initialize controls from data
	}
}

void CMovieExportDlg::OnClickedTimeUnit(UINT nID)
{
	UNREFERENCED_PARAMETER(nID);
	UpdateData(true);	// retrieve data from controls
    UpdateRangeEnd();
	UpdateData(false);	// initialize controls from data
}

void CMovieExportDlg::OnKillFocusRange()
{
	UpdateData(true);	// retrieve data from controls
    UpdateDuration();
	UpdateData(false);	// initialize controls from data
}

void CMovieExportDlg::OnKillFocusDuration()
{
	UpdateData(true);	// retrieve data from controls
    UpdateRangeEnd();
	UpdateData(false);	// initialize controls from data
}
