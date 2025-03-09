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
        02      09mar25	add export scaling types

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
	DDX_Control(pDX, IDC_EXPORT_SCALING_TYPE_COMBO, m_comboScalingType);
}

BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_WIDTH_EDIT, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_HEIGHT_EDIT, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_SCALING_TYPE_COMBO, OnUpdateSize)
	ON_CBN_SELCHANGE(IDC_EXPORT_SCALING_TYPE_COMBO, &CExportDlg::OnCbnSelchangeExportScalingTypeCombo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

BOOL CExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// initialize scaling type drop list from options
	for (int iScaT = 0; iScaT < COptions::SCALING_TYPES; iScaT++) {
		m_comboScalingType.AddString(LDS(COptions::m_oiScalingType[iScaT].nNameID));
	}
	m_comboScalingType.SetCurSel(theApp.m_options.m_Export_nScalingType);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CExportDlg::OnOK() 
{
	CDialog::OnOK();
	theApp.m_options.m_Export_bUseViewSize = m_bUseViewSize != 0;
	theApp.m_options.m_Export_nImageWidth = m_nWidth;
	theApp.m_options.m_Export_nImageHeight = m_nHeight;
	theApp.m_options.m_Export_nScalingType = max(m_comboScalingType.GetCurSel(), 0);
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


void CExportDlg::OnCbnSelchangeExportScalingTypeCombo()
{
	// TODO: Add your control notification handler code here
}
