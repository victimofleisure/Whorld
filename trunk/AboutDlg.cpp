// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		18may05	add version info
		02		02aug05	add license and URL
		03		22dec05	add ShowWindow to init in case main is hogging
		04		10feb06	change URL
		05		28jan08	support Unicode

        about dialog
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "AboutDlg.h"
#include "VersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

const LPCTSTR CAboutDlg::HOME_PAGE_URL = _T("http://whorld.org");

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_ABOUT_CREDITS, m_Credits);
	DDX_Control(pDX, IDC_ABOUT_LICENSE, m_License);
	DDX_Control(pDX, IDC_ABOUT_URL, m_AboutUrl);
	DDX_Control(pDX, IDC_ABOUT_TEXT, m_AboutText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	VS_FIXEDFILEINFO	AppInfo;
	CVersionInfo::GetFileInfo(AppInfo, NULL);
	CString	s;
	s.Format(IDS_APP_ABOUT_TEXT,
		HIWORD(AppInfo.dwFileVersionMS), LOWORD(AppInfo.dwFileVersionMS),
		HIWORD(AppInfo.dwFileVersionLS), LOWORD(AppInfo.dwFileVersionLS));
	m_AboutText.SetWindowText(s);
	m_AboutUrl.SetUrl(HOME_PAGE_URL);
	m_License.SetWindowText(LDS(IDS_APP_LICENSE));
	m_Credits.SetWindowText(LDS(IDS_APP_CREDITS));
	CenterWindow();	// required if we do our own show
	ShowWindow(SW_SHOW);	// else if main is hogging CPU, we won't show
	
	return TRUE;  // return TRUE unless you set the focus to a control
}
