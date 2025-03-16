// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28jun05	initial version
        01      21feb25	refactor
        02      09mar25	add export scale to fit
		03		12mar25	display view size when use view size is checked

        export options dialog
 
*/

// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Whorld.h"
#include "ExportDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

IMPLEMENT_DYNAMIC(CExportDlg, CDialog);

CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent)
{
	m_bUseViewSize = theApp.m_options.m_Export_bUseViewSize;
	m_nWidth = theApp.m_options.m_Export_nImageWidth;
	m_nHeight = theApp.m_options.m_Export_nImageHeight;
}

void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_EXPORT_USE_VIEW_SIZE_CHECK, m_bUseViewSize);
	DDX_Text(pDX, IDC_EXPORT_WIDTH_EDIT, m_nWidth);
	DDX_Text(pDX, IDC_EXPORT_HEIGHT_EDIT, m_nHeight);
	DDX_Control(pDX, IDC_EXPORT_SCALE_TO_FIT_COMBO, m_comboScaleToFit);
}

BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_WIDTH_EDIT, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_HEIGHT_EDIT, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_SCALE_TO_FIT_COMBO, OnUpdateSize)
	ON_BN_CLICKED(IDC_EXPORT_USE_VIEW_SIZE_CHECK, OnClickedUseViewSize)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

BOOL CExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// initialize scale to fit drop list from options
	for (int iSTF = 0; iSTF < COptions::SCALE_TO_FIT_TYPES; iSTF++) {
		m_comboScaleToFit.AddString(LDS(COptions::m_oiScaleToFit[iSTF].nNameID));
	}
	m_comboScaleToFit.SetCurSel(theApp.m_options.m_Export_nScaleToFit);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CExportDlg::OnOK() 
{
	CDialog::OnOK();
	theApp.m_options.m_Export_bUseViewSize = m_bUseViewSize != 0;
	theApp.m_options.m_Export_nImageWidth = m_nWidth;
	theApp.m_options.m_Export_nImageHeight = m_nHeight;
	theApp.m_options.m_Export_nScaleToFit = max(m_comboScaleToFit.GetCurSel(), 0);
}

LRESULT CExportDlg::OnKickIdle(WPARAM, LPARAM)
{
	UpdateDialogControls(this, FALSE); 
	return 0;
}

void CExportDlg::OnUpdateSize(CCmdUI *pCmdUI)
{
	BOOL	bUseViewSize = IsDlgButtonChecked(IDC_EXPORT_USE_VIEW_SIZE_CHECK);
	pCmdUI->Enable(!bUseViewSize);
}

void CExportDlg::OnClickedUseViewSize()
{
	BOOL	bUseViewSize = IsDlgButtonChecked(IDC_EXPORT_USE_VIEW_SIZE_CHECK);
	if (bUseViewSize) {	// if using view size
		// display render target size in width/height edit controls
		UpdateData(true);	// retrieve data from controls
		D2D1_SIZE_F	szTarget = theApp.m_thrRender.GetTargetSize();
		m_nWidth = Round(szTarget.width);
		m_nHeight = Round(szTarget.height);
		UpdateData(false);	// initialize controls from data
	}
}
