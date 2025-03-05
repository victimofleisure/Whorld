// Copyleft 2017 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26mar17	initial version

        about dialog
 
*/

#include "stdafx.h"
#include "Whorld.h"
#include "AboutDlg.h"
#include "VersionInfo.h"

CAboutDlg::CAboutDlg() 
	: CDialog(IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ABOUT_LICENSE, m_License);
	DDX_Control(pDX, IDC_ABOUT_VERSION, m_Version);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	VS_FIXEDFILEINFO	AppInfo;
	CVersionInfo::GetFileInfo(AppInfo, NULL);
	m_Version.SetWindowText(CString(theApp.m_pszAppName) + ' ' + theApp.GetVersionString());
	m_License.SetWindowText(LDS(IDS_APP_LICENSE));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
