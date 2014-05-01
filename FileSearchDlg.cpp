// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jan04	initial version
		01		17jan04	add drive type mask
		02		23nov07	support Unicode

        abstract class to search for files recursively
 
*/

// FileSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "FileSearchDlg.h"
#include "SubFileFind.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileSearchDlg dialog

IMPLEMENT_DYNAMIC(CFileSearchDlg, CDialog);

CFileSearchDlg::CFileSearchDlg(LPCTSTR Folder, CWnd* pParent)
	: m_Folder(Folder), CDialog(CFileSearchDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileSearchDlg)
	//}}AFX_DATA_INIT
	m_DriveTypeMask = DTM_FIXED;
}

CFileSearchDlg::~CFileSearchDlg()
{
}

void CFileSearchDlg::SetDriveTypeMask(int Mask)
{
	m_DriveTypeMask = Mask;
}

void CFileSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileSearchDlg)
	//}}AFX_DATA_MAP
}

bool CFileSearchDlg::SearchFolder(LPCTSTR Folder)
{
	CSubFileFind	sff;
	CFileFind		*ff;
	BOOL	Working = sff.FindFile(CString(Folder) + "\\*.*");
	while (Working) {
		Working = sff.FindNextFile(ff);
		if (!OnFile(ff->GetFilePath()) || m_Cancel)
			return(FALSE);
	}
	return(TRUE);
}

bool CFileSearchDlg::SearchAllDrives()
{
	DWORD	len = GetLogicalDriveStrings(0, NULL);	// get list size
	CString	DriveList;
	LPTSTR	p = DriveList.GetBufferSetLength(len);
	GetLogicalDriveStrings(len, p);
	while (*p) {
		// if drive type is selected by type mask
		if ((1 << GetDriveType(p)) & m_DriveTypeMask) {
			p[2] = 0;	// remove backslash
			if (!SearchFolder(p))
				return(FALSE);
		}
		p += 4;		// drive letter, colon, backslash, null
	}
	return(TRUE);
}

void CFileSearchDlg::Search()
{
	if (m_Folder.IsEmpty())
		SearchAllDrives();
	else
		SearchFolder(m_Folder);
	PostMessage(WM_USER);
}

UINT CFileSearchDlg::SearchThread(LPVOID pParam)
{
	((CFileSearchDlg *)pParam)->Search();
	return(0);
}

BEGIN_MESSAGE_MAP(CFileSearchDlg, CDialog)
	//{{AFX_MSG_MAP(CFileSearchDlg)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER, OnUser)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileSearchDlg message handlers

BOOL CFileSearchDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_Cancel = FALSE;
	AfxBeginThread(SearchThread, this);
	return(TRUE);
}

void CFileSearchDlg::OnCancel() 
{
	m_Cancel = TRUE;
}

LRESULT CFileSearchDlg::OnUser(WPARAM wParam, LPARAM lParam)
{
	EndDialog(m_Cancel ? IDCANCEL : IDOK);
	return(0);
}
