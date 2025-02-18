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

CReplaceFilesDlg::CReplaceFilesDlg(CStringArray& Path, LPCTSTR DefaultExt, 
								   LPCTSTR Filter, CWnd* pParent /*=NULL*/)
	: m_Path(Path), m_DefaultExt(DefaultExt), 
	m_Filter(Filter), CDialog(CReplaceFilesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReplaceFilesDlg)
	//}}AFX_DATA_INIT
}

bool CReplaceFilesDlg::Search() 
{
	m_Missing.RemoveAll();
	for (int i = 0; i < m_Path.GetSize(); i++) {
		if (m_Path[i].GetLength() && !PathFileExists(m_Path[i])) {
			CMissing	ms(PathFindFileName(m_Path[i]), -1, i);
			m_Missing.Add(ms);
		}
	}
	return(Search(NULL));	// search all disks
}

bool CReplaceFilesDlg::Search(LPCTSTR Folder)
{
	CMyFileSearchDlg	fsd(Folder, this);
	return(fsd.DoModal() == IDOK);
}

bool CReplaceFilesDlg::OnFile(LPCTSTR Path) 
{
	int i = 0;
	while (i < m_Missing.GetSize()) {
		CMissing	*mp = &m_Missing.ElementAt(i);
		if (!_tcsicmp(PathFindFileName(Path), mp->m_FileName)) {
			if (mp->m_Item >= 0)
				m_List.SetItemText(mp->m_Item, 1, m_ReplacedStr);
			if (mp->m_Chan >= 0)
				m_Path[mp->m_Chan] = Path;
			m_Missing.RemoveAt(i);	// this decrements the loop limit
		} else
			i++;
	}
	return(m_Missing.GetSize() != 0);	// return true to keep searching
}

CReplaceFilesDlg::CMissing::CMissing(LPCTSTR FileName, int Item, int Chan)
{
	m_FileName = FileName;
	m_Item = Item;
	m_Chan = Chan;
}

CReplaceFilesDlg::CMissing::CMissing()
{
	m_Item = -1;
	m_Chan = -1;
}

CReplaceFilesDlg::CMissing::CMissing(const CMissing& Missing)
{
	m_FileName = Missing.m_FileName;
	m_Item = Missing.m_Item;
	m_Chan = Missing.m_Chan;
}

CReplaceFilesDlg::CMyFileSearchDlg::CMyFileSearchDlg(LPCTSTR Folder, CReplaceFilesDlg *rfd)
	: CFileSearchDlg(Folder), m_rfd(rfd)
{
}

bool CReplaceFilesDlg::CMyFileSearchDlg::OnFile(LPCTSTR Path)
{
	return(m_rfd->OnFile(Path));
}

void CReplaceFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplaceFilesDlg)
	DDX_Control(pDX, IDC_RPLF_LIST, m_List);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReplaceFilesDlg, CDialog)
	//{{AFX_MSG_MAP(CReplaceFilesDlg)
	ON_BN_CLICKED(IDC_RPLF_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_RPLF_SEARCH_ALL, OnSearchAll)
	ON_BN_CLICKED(IDC_RPLF_SEARCH_FOLDER, OnSearchFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplaceFilesDlg message handlers

BOOL CReplaceFilesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_MissingStr.LoadString(IDS_RPLF_MISSING);
	m_ReplacedStr.LoadString(IDS_RPLF_REPLACED);
	CString	Title;
	Title.LoadString(IDS_RPLF_FILENAME);
	m_List.InsertColumn(0, Title, LVCFMT_LEFT, 300);
	Title.LoadString(IDS_RPLF_STATUS);
	m_List.InsertColumn(1, Title, LVCFMT_LEFT, 100);
	for (int i = 0; i < m_Path.GetSize(); i++) {
		if (m_Path[i].GetLength() && !PathFileExists(m_Path[i])) {
			int	pos = m_List.InsertItem(0, PathFindFileName(m_Path[i]));
			m_List.SetItemText(pos, 1, m_MissingStr);
			m_List.SetItemData(pos, i);
		}
	}
	m_List.SetItemState(0, (UINT)-1, LVIS_SELECTED);
	m_List.SetSelectionMark(0);
	m_List.SetFocus();

	return FALSE;	// return TRUE unless you set the focus to a control
}

void CReplaceFilesDlg::OnBrowse() 
{
	int	sel = m_List.GetSelectionMark();
	if (sel < 0)
		return;
	int	pos = m_List.GetItemData(sel);
	CFileDialog	fd(TRUE, m_DefaultExt, NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, m_Filter);
	if (fd.DoModal() == IDOK && PathFileExists(fd.GetPathName())) {
		m_Path[pos] = fd.GetPathName();
		m_List.SetItemText(sel, 1, m_ReplacedStr);
	}
}

BOOL CReplaceFilesDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// if no valid selection, disable browse
	if (((NMHDR*)lParam)->idFrom == IDC_RPLF_LIST)
		GetDlgItem(IDC_RPLF_BROWSE)->EnableWindow(m_List.GetSelectedCount() > 0);
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CReplaceFilesDlg::InitMissing()
{
	m_Missing.RemoveAll();
	for (int i = 0; i < m_List.GetItemCount(); i++) {
		if (m_List.GetItemText(i, 1) == m_MissingStr) {
			CMissing	ms(m_List.GetItemText(i, 0), i, m_List.GetItemData(i));
			m_Missing.Add(ms);
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
	CString	Folder;
	if (CFolderDialog::BrowseFolder(NULL, Folder)) {
		InitMissing();
		Search(Folder);
	}
	SetFocus();
}
