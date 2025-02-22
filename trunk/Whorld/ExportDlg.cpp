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

        export options dialog
 
*/

// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Whorld.h"
#include "ExportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

IMPLEMENT_DYNAMIC(CExportDlg, CDialog);

CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent)
{
	m_bUseViewSize = theApp.m_options.m_Export_bUseViewSize;
	m_nWidth = theApp.m_options.m_Export_nImageWidth;
	m_nHeight = theApp.m_options.m_Export_nImageHeight;
	m_nResizing = theApp.m_options.m_Export_bScaleToFit;
}

void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_EXPORT_USE_VIEW_SIZE, m_bUseViewSize);
	DDX_Text(pDX, IDC_EXPORT_WIDTH, m_nWidth);
	DDX_Text(pDX, IDC_EXPORT_HEIGHT, m_nHeight);
	DDX_Radio(pDX, IDC_EXPORT_RESIZING1, m_nResizing);
}

BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_WIDTH, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_HEIGHT, OnUpdateSize)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

BOOL CExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CExportDlg::OnOK() 
{
	CDialog::OnOK();
	theApp.m_options.m_Export_bUseViewSize = m_bUseViewSize != 0;
	theApp.m_options.m_Export_nImageWidth = m_nWidth;
	theApp.m_options.m_Export_nImageHeight = m_nHeight;
	theApp.m_options.m_Export_bScaleToFit = m_nResizing != 0;
}

LRESULT CExportDlg::OnKickIdle(WPARAM, LPARAM)
{
	UpdateDialogControls(this, FALSE); 
	return 0;
}

void CExportDlg::OnUpdateSize(CCmdUI *pCmdUI)
{
	BOOL	bUseViewSize = IsDlgButtonChecked(IDC_EXPORT_USE_VIEW_SIZE);
	pCmdUI->Enable(!bUseViewSize);
}
