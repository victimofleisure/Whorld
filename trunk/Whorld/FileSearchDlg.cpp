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
		03		29mar25	modernize style

        abstract class to search for files recursively
 
*/

// FileSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "FileSearchDlg.h"
#include "SubFileFind.h"
#include "PathStr.h"

/////////////////////////////////////////////////////////////////////////////
// CFileSearchDlg dialog

IMPLEMENT_DYNAMIC(CFileSearchDlg, CDialog);

CFileSearchDlg::CFileSearchDlg(LPCTSTR pszFolder, CWnd* pParentWnd)
	: m_sFolder(pszFolder), CDialog(CFileSearchDlg::IDD, pParentWnd)
{
	m_bCancel = false;
	m_nDriveTypeMask = DTM_FIXED;
}

CFileSearchDlg::~CFileSearchDlg()
{
}

void CFileSearchDlg::SetDriveTypeMask(int nMask)
{
	m_nDriveTypeMask = nMask;
}

void CFileSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

bool CFileSearchDlg::SearchFolder(LPCTSTR pszFolder)
{
	CSubFileFind	sff;
	CFileFind		*pFF;
	CPathStr	sPath(pszFolder);
	sPath.Append(_T("*.*"));
	BOOL	Working = sff.FindFile(sPath);
	while (Working) {
		Working = sff.FindNextFile(pFF);
		if (!OnFile(pFF->GetFilePath()) || m_bCancel)
			return false;
	}
	return true;
}

bool CFileSearchDlg::SearchAllDrives()
{
	DWORD	nListLen = GetLogicalDriveStrings(0, NULL);	// get list size in characters
	CString	DriveList;
	LPTSTR	pList = DriveList.GetBufferSetLength(nListLen);
	GetLogicalDriveStrings(nListLen, pList);
	while (*pList) {
		size_t	nPathLen = _tcslen(pList);
		// if drive type is selected by type mask
		if ((1 << GetDriveType(pList)) & m_nDriveTypeMask) {
			if (!SearchFolder(pList))
				return false;
		}
		pList += nPathLen + 1;	// one extra for null terminator
	}
	return true;
}

void CFileSearchDlg::Search()
{
	if (m_sFolder.IsEmpty())
		SearchAllDrives();
	else
		SearchFolder(m_sFolder);
	PostMessage(WM_USER);
}

UINT CFileSearchDlg::SearchThread(LPVOID pParam)
{
	((CFileSearchDlg *)pParam)->Search();
	return 0;
}

BEGIN_MESSAGE_MAP(CFileSearchDlg, CDialog)
	ON_MESSAGE(WM_USER, OnUser)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileSearchDlg message handlers

BOOL CFileSearchDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_bCancel = false;
	AfxBeginThread(SearchThread, this);
	return true;
}

void CFileSearchDlg::OnCancel() 
{
	m_bCancel = true;
}

LRESULT CFileSearchDlg::OnUser(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	EndDialog(m_bCancel ? IDCANCEL : IDOK);
	return 0;
}
