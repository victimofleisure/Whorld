// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15sep05	initial version
		01		08nov05	create default snapshot folder in MyDocuments
		02		22dec05	in SetDefaults, reset folder to MyDocuments
		03		28jan08	support Unicode

        folder options property page
 
*/

// OptsFolderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "OptsFolderDlg.h"
#include "FolderDialog.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsFolderDlg property page

IMPLEMENT_DYNCREATE(COptsFolderDlg, CPropertyPage)

const COptsFolderDlg::STATE COptsFolderDlg::m_Default = {
	FALSE	// SnapFolderAlways
};

#define REG_SNAPSHOT_FOLDER		_T("SnapshotFolder")

#define REG_VALUE(name) m_Reg##name(_T(#name), m_st.name, m_Default.name)

COptsFolderDlg::COptsFolderDlg() : CPropertyPage(COptsFolderDlg::IDD),
	REG_VALUE(SnapFolderAlways)
{
	//{{AFX_DATA_INIT(COptsFolderDlg)
	m_SnapFolderAlways = 0;
	//}}AFX_DATA_INIT
	m_SnapshotFolder = CPersist::GetString(REG_SETTINGS, REG_SNAPSHOT_FOLDER, _T("\n"));
	if (m_SnapshotFolder == _T("\n"))
		GetMyDocs(m_SnapshotFolder);
}

COptsFolderDlg::~COptsFolderDlg()
{
	CPersist::WriteString(REG_SETTINGS, REG_SNAPSHOT_FOLDER, m_SnapshotFolder);
}

bool COptsFolderDlg::GetMyDocs(CString& str)
{
	CPathStr	Path;
	TCHAR	*p = Path.GetBuffer(MAX_PATH);
	bool	retc = SUCCEEDED(SHGetSpecialFolderPath(NULL, p, CSIDL_PERSONAL, 0));
	if (retc) {
		Path.ReleaseBuffer();
		Path.Append(LDS(AFX_IDS_APP_TITLE));
		str = Path;
	} else
		str.Empty();
	return(retc);
}

void COptsFolderDlg::SetDefaults()
{
	m_st = m_Default;
	GetMyDocs(m_SnapshotFolder);
}

void COptsFolderDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsFolderDlg)
	DDX_Control(pDX, IDC_OP_SNAP_FOLDER_EDIT, m_SnapFolderEdit);
	DDX_Radio(pDX, IDC_OP_SNAP_FOLDER_ALWAYS, m_SnapFolderAlways);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptsFolderDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsFolderDlg)
	ON_BN_CLICKED(IDC_OP_SNAP_FOLDER_BROWSE, OnSnapFolderBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsFolderDlg message handlers

BOOL COptsFolderDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_SnapFolderEdit.SetWindowText(m_SnapshotFolder);
	m_SnapFolderAlways = m_st.SnapFolderAlways;
	UpdateData(FALSE);	// init dialog

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptsFolderDlg::OnOK() 
{
	CPropertyPage::OnOK();
	m_SnapFolderEdit.GetWindowText(m_SnapshotFolder);
	m_st.SnapFolderAlways = m_SnapFolderAlways != 0;
}

void COptsFolderDlg::OnSnapFolderBrowse() 
{
	CString	s;
	if (CFolderDialog::BrowseFolder(NULL, s, NULL))
		m_SnapFolderEdit.SetWindowText(s);
}
