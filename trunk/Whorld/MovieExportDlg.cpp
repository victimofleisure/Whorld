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
#include "Options.h"	// for scale to fit types

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg dialog

IMPLEMENT_DYNAMIC(CMovieExportDlg, CDialog);

CMovieExportDlg::CMovieExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMovieExportDlg::IDD, pParent)
{
	m_iFrameSizePreset = FSP_MOVIE;
	m_szFrame = CSize(0, 0);
	m_iScaleToFit = 0;
	m_iFrameSelType = FST_ALL;
	m_nRangeStart = 0;
	m_nRangeEnd = 0;
	m_nDuration = 0;
	m_iTimeUnit = UNIT_TIME;
	m_fFrameRate = 0;
	m_nFrameCount = 0;
}

UINT CMovieExportDlg::GetExportFlags() const
{
	return COptions::GetExportFlags(m_iFrameSizePreset == FSP_VIEW, m_iScaleToFit);
}

void CMovieExportDlg::UpdateFrameSize(int iFrameSizePreset)
{
	D2D1_SIZE_F	szTarget;
	if (iFrameSizePreset == FSP_MOVIE) {	// if getting frame size from movie
		szTarget = theApp.m_thrRender.GetMovieFrameSize();
	} else {	// get frame size from render target size
		szTarget = theApp.m_thrRender.GetTargetSize();
	}
	m_szFrame.cx = Round(szTarget.width);
	m_szFrame.cy = Round(szTarget.height);
}

void CMovieExportDlg::FrameToTime(int iFrame, COleDateTime& dt, float fFrameRate)
{
	dt.m_dt = iFrame / fFrameRate / SECONDS_PER_DAY;
}

int CMovieExportDlg::TimeToFrame(const COleDateTime& dt, float fFrameRate)
{
	return Round(dt.m_dt * fFrameRate * SECONDS_PER_DAY);
}

void CMovieExportDlg::FrameToTimeString(int iFrame, CString& sTime, float fFrameRate)
{
	COleDateTime	dt;
	FrameToTime(iFrame, dt, fFrameRate);
	sTime = dt.Format(_T("%H:%M:%S"));
}

bool CMovieExportDlg::TimeStringToFrame(CString sTime, int& iFrame, float fFrameRate)
{
	COleDateTime	dt;
	if (!dt.ParseDateTime(sTime)) {	// if can't parse time
		return false;	// fail
	}
	iFrame = TimeToFrame(dt, fFrameRate);
	return true;
}

void CMovieExportDlg::FrameToTimeString(int iFrame, CString& sTime) const
{
	FrameToTimeString(iFrame, sTime, m_fFrameRate);
}

bool CMovieExportDlg::TimeStringToFrame(CString sTime, int& iFrame) const
{
	return TimeStringToFrame(sTime, iFrame, m_fFrameRate);
}

void CMovieExportDlg::DDX_FrameTime(CDataExchange* pDX, int nIDC, int& value) const
{
	if (m_iTimeUnit == UNIT_TIME) {	// if unit is time
		CString	sTime;
		if (!pDX->m_bSaveAndValidate) {	// if initializing controls from data
			FrameToTimeString(value, sTime);
		}
		DDX_Text(pDX, nIDC, sTime);
		if (pDX->m_bSaveAndValidate) {	// if retrieving data from controls
			if (!TimeStringToFrame(sTime, value)) {	// if can't parse time
				AfxMessageBox(IDS_MEX_BAD_TIME);
				DDV_Fail(pDX, nIDC);
			}
			// if retrieving range end, and range end is greater than zero
			if (nIDC == IDC_MEX_RANGE_END_EDIT && value > 0) {
				value--;	// ensure range end is one less than duration
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

void CMovieExportDlg::UpdateFrameSelection(int iFrameSelType)
{
	if (iFrameSelType == FST_ALL) {
		m_nRangeStart = 0;
		m_nRangeEnd = m_nFrameCount - 1;
		m_nDuration = m_nFrameCount;
	}
}

void CMovieExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEX_FRAME_SIZE_COMBO, m_comboFrameSize);
	DDX_Control(pDX, IDC_MEX_SCALE_TO_FIT_COMBO, m_comboScaleToFit);
	DDX_Text(pDX, IDC_MEX_FRAME_WIDTH_EDIT, m_szFrame.cx);
	DDX_Text(pDX, IDC_MEX_FRAME_HEIGHT_EDIT, m_szFrame.cy);
	DDX_Radio(pDX, IDC_MEX_FRAME_SEL_TYPE_1, m_iFrameSelType);
	if (!pDX->m_bSaveAndValidate) {	// if initializing controls from data
		// order matters: initialize time unit BEFORE initializing times
		DDX_Radio(pDX, IDC_MEX_TIME_UNIT_1, m_iTimeUnit);
	}
	DDX_FrameTime(pDX, IDC_MEX_RANGE_START_EDIT, m_nRangeStart);
	DDX_FrameTime(pDX, IDC_MEX_RANGE_END_EDIT, m_nRangeEnd);
	DDX_FrameTime(pDX, IDC_MEX_DURATION_EDIT, m_nDuration);
	// if retrieving data from controls and not canceling dialog
	if (pDX->m_bSaveAndValidate && m_nModalResult != IDCANCEL) {
		// order matters: retrieve time unit AFTER retrieving times
		DDX_Radio(pDX, IDC_MEX_TIME_UNIT_1, m_iTimeUnit);
		if (m_nRangeStart > m_nRangeEnd) {	// if range out of order
			AfxMessageBox(IDS_MEX_BAD_RANGE);	// gong user
			DDV_Fail(pDX, IDC_MEX_RANGE_END_EDIT);	// fail
		}
	}
	DDX_Text(pDX, IDC_MEX_FRAME_RATE_EDIT, m_fFrameRate);
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
	ON_UPDATE_COMMAND_UI(IDC_MEX_FRAME_RATE_EDIT, OnUpdateFrameRate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovieExportDlg message handlers

BOOL CMovieExportDlg::OnInitDialog() 
{
	m_nDuration = 1;	// avoids spurious DDV error
	m_nFrameCount = static_cast<int>(theApp.m_thrRender.GetMovieFrameCount());
	m_fFrameRate = theApp.m_thrRender.GetMovieFrameRate();
	ASSERT(m_nFrameCount > 0);
	ASSERT(m_fFrameRate > 0);
	UpdateFrameSize(m_iFrameSizePreset);
	// Initialize the frame range regardless of the current frame selection type,
	// because the frame range isn't an input to this dialog, it's output only.
	UpdateFrameSelection(FST_ALL);

	CDialog::OnInitDialog();

	m_comboFrameSize.SetCurSel(CLAMP(m_iFrameSizePreset, 0, FRAME_SIZE_PRESETS - 1));
	// initialize scale to fit combo; scale to fit types are owned by options
	for (int iFitType = 0; iFitType < COptions::SCALE_TO_FIT_TYPES; iFitType++) {
		m_comboScaleToFit.AddString(LDS(COptions::m_oiScaleToFit[iFitType].nNameID));
	}
	m_comboScaleToFit.SetCurSel(CLAMP(m_iScaleToFit, 0, COptions::SCALE_TO_FIT_TYPES - 1));

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMovieExportDlg::OnOK()
{
	CDialog::OnOK();
	// get index of frame size preset from combo
	m_iFrameSizePreset = m_comboFrameSize.GetCurSel();
	ASSERT(m_iFrameSizePreset >= 0 && m_iFrameSizePreset < FRAME_SIZE_PRESETS);
	m_iFrameSizePreset = CLAMP(m_iFrameSizePreset, 0, FRAME_SIZE_PRESETS - 1);
	// get index of scale to fit type from combo
	m_iScaleToFit = m_comboScaleToFit.GetCurSel();
	ASSERT(m_iScaleToFit >= 0 && m_iScaleToFit < COptions::SCALE_TO_FIT_TYPES);
	m_iScaleToFit = CLAMP(m_iScaleToFit, 0, COptions::SCALE_TO_FIT_TYPES - 1);
	// If the frame selection type is ALL, update the frame range from the
	// movie's actual frame count, avoiding imprecision due to time rounding.
	UpdateFrameSelection(m_iFrameSelType);
	if (m_iFrameSizePreset == FSP_MOVIE) {	// if frame size preset is movie
		UpdateFrameSize(m_iFrameSizePreset);	// refresh frame size just in case
	}
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
		UpdateFrameSize(iPreset);	// set frame size
		UpdateData(false);	// initialize controls from data
	}
}

void CMovieExportDlg::OnClickedFrameSelType(UINT nID)
{
	UNREFERENCED_PARAMETER(nID);
	int	iFrameSelType = IsDlgButtonChecked(IDC_MEX_FRAME_SEL_TYPE_1);
	if (iFrameSelType == FST_ALL) {	// if frame selection type is ALL
		UpdateData(true);	// retrieve data from controls
		UpdateFrameSelection(iFrameSelType);	// set frame range
		UpdateData(false);	// initialize controls from data
	}
}

void CMovieExportDlg::OnClickedTimeUnit(UINT nID)
{
	UNREFERENCED_PARAMETER(nID);
	UpdateData(true);	// retrieve data from controls
    UpdateRangeEnd();	// set range end from duration
	UpdateData(false);	// initialize controls from data
}

void CMovieExportDlg::OnKillFocusRange()
{
	UpdateData(true);	// retrieve data from controls
    UpdateDuration();	// set duration from range end
	UpdateData(false);	// initialize controls from data
}

void CMovieExportDlg::OnKillFocusDuration()
{
	UpdateData(true);	// retrieve data from controls
    UpdateRangeEnd();	// set range end from duration
	UpdateData(false);	// initialize controls from data
}

void CMovieExportDlg::OnUpdateFrameRate(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(false);	// frame rate isn't editable
}
