// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov03 initial version
		01		03jan04	duplicate files confuse search; use while, not for
		02		05jan04	search all fixed disks
		03		09jan04	move actual searching to file search dialog
		04		16jan04	add file filter argument to ctor
		05		17jan04	add search in folder
		06		24jan04	reset focus after folder browse or file search
		07		05aug05	make all strings resources
		08		05aug05	pass default extension to ctor
		09		23nov07	support Unicode
		10		29jan08	create CMissing explicitly to fix warnings
		11		29mar25	modernize style

        replace files dialog
 
*/

// ReplaceFilesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ReplaceFilesDlg.h"
#include "Shlwapi.h"
#include "SubFileFind.h"
#include "FolderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReplaceFilesDlg dialog

IMPLEMENT_DYNAMIC(CReplaceFilesDlg, CDialog);

CReplaceFilesDlg::CReplaceFilesDlg(CStringArrayEx& arrPath, LPCTSTR pszDefaultExt, 
								   LPCTSTR pszFilter, CWnd* pParentWnd /*=NULL*/)
	: m_arrPath(arrPath), m_pszDefaultExt(pszDefaultExt), 
	m_pszFilter(pszFilter), CDialog(CReplaceFilesDlg::IDD, pParentWnd)
{
}

bool CReplaceFilesDlg::Search() 
{
	m_arrMissing.RemoveAll();
	for (int iPath = 0; iPath < m_arrPath.GetSize(); iPath++) {
		if (m_arrPath[iPath].GetLength() && !PathFileExists(m_arrPath[iPath])) {
			CMissing	miss(PathFindFileName(m_arrPath[iPath]), -1, iPath);
			m_arrMissing.Add(miss);
		}
	}
	return Search(NULL);	// search all disks
}

bool CReplaceFilesDlg::Search(LPCTSTR pszFolder)
{
	CMyFileSearchDlg	fsd(pszFolder, this);
	return fsd.DoModal() == IDOK;
}

bool CReplaceFilesDlg::OnFile(LPCTSTR Path) 
{
	int iMiss = 0;
	while (iMiss < m_arrMissing.GetSize()) {
		const CMissing&	miss = m_arrMissing[iMiss];
		if (!_tcsicmp(PathFindFileName(Path), miss.m_sFileName)) {
			if (miss.m_iItem >= 0)
				m_wndList.SetItemText(miss.m_iItem, 1, m_sReplaced);
			if (miss.m_iPath >= 0)
				m_arrPath[miss.m_iPath] = Path;
			m_arrMissing.RemoveAt(iMiss);	// this decrements the loop limit
		} else {
			iMiss++;
		}
	}
	return m_arrMissing.GetSize() != 0;	// return true to keep searching
}

CReplaceFilesDlg::CMissing::CMissing(LPCTSTR FileName, int iItem, int iPath)
{
	m_sFileName = FileName;
	m_iItem = iItem;
	m_iPath = iPath;
}

CReplaceFilesDlg::CMissing::CMissing()
{
	m_iItem = -1;
	m_iPath = -1;
}

CReplaceFilesDlg::CMissing::CMissing(const CMissing& Missing)
{
	m_sFileName = Missing.m_sFileName;
	m_iItem = Missing.m_iItem;
	m_iPath = Missing.m_iPath;
}

CReplaceFilesDlg::CMyFileSearchDlg::CMyFileSearchDlg(LPCTSTR Folder, CReplaceFilesDlg *pRFD)
	: CFileSearchDlg(Folder), m_pRFD(pRFD)
{
}

bool CReplaceFilesDlg::CMyFileSearchDlg::OnFile(LPCTSTR Path)
{
	return m_pRFD->OnFile(Path);
}

void CReplaceFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RPLF_LIST, m_wndList);
}

BEGIN_MESSAGE_MAP(CReplaceFilesDlg, CDialog)
	ON_BN_CLICKED(IDC_RPLF_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_RPLF_SEARCH_ALL, OnSearchAll)
	ON_BN_CLICKED(IDC_RPLF_SEARCH_FOLDER, OnSearchFolder)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplaceFilesDlg message handlers

BOOL CReplaceFilesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_sMissing.LoadString(IDS_RPLF_MISSING);
	m_sReplaced.LoadString(IDS_RPLF_REPLACED);
	CString	sTitle;
	sTitle.LoadString(IDS_RPLF_FILENAME);
	m_wndList.InsertColumn(0, sTitle, LVCFMT_LEFT, 300);
	sTitle.LoadString(IDS_RPLF_STATUS);
	m_wndList.InsertColumn(1, sTitle, LVCFMT_LEFT, 100);
	for (int iPath = 0; iPath < m_arrPath.GetSize(); iPath++) {
		if (m_arrPath[iPath].GetLength() && !PathFileExists(m_arrPath[iPath])) {
			int	pos = m_wndList.InsertItem(0, PathFindFileName(m_arrPath[iPath]));
			m_wndList.SetItemText(pos, 1, m_sMissing);
			m_wndList.SetItemData(pos, iPath);
		}
	}
	m_wndList.SetItemState(0, (UINT)-1, LVIS_SELECTED);
	m_wndList.SetSelectionMark(0);
	m_wndList.SetFocus();

	return false;	// return true unless you set the focus to a control
}

void CReplaceFilesDlg::OnBrowse() 
{
	int	iSel = m_wndList.GetSelectionMark();
	if (iSel < 0)
		return;
	int	iPath = static_cast<int>(m_wndList.GetItemData(iSel));
	CFileDialog	fd(true, m_pszDefaultExt, NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, m_pszFilter);
	if (fd.DoModal() == IDOK && PathFileExists(fd.GetPathName())) {
		m_arrPath[iPath] = fd.GetPathName();
		m_wndList.SetItemText(iSel, 1, m_sReplaced);
	}
}

BOOL CReplaceFilesDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// if no valid selection, disable browse
	if (((NMHDR*)lParam)->idFrom == IDC_RPLF_LIST)
		GetDlgItem(IDC_RPLF_BROWSE)->EnableWindow(m_wndList.GetSelectedCount() > 0);
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CReplaceFilesDlg::InitMissing()
{
	m_arrMissing.RemoveAll();
	for (int iItem = 0; iItem < m_wndList.GetItemCount(); iItem++) {
		if (m_wndList.GetItemText(iItem, 1) == m_sMissing) {
			int	iPath = static_cast<int>(m_wndList.GetItemData(iItem));
			CMissing	ms(m_wndList.GetItemText(iItem, 0), iItem, iPath);
			m_arrMissing.Add(ms);
		}
	}
}

void CReplaceFilesDlg::OnSearchAll() 
{
	InitMissing();
	Search(NULL);	// search all disks
	SetFocus();
}

void CReplaceFilesDlg::OnSearchFolder() 
{
	CString	sFolder;
	UINT	nFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON;
	if (CFolderDialog::BrowseFolder(NULL, sFolder, NULL, nFlags)) {
		InitMissing();
		Search(sFolder);
	}
	SetFocus();
}
