// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01jul05	initial version
		01		27oct06	add GetFolder
		02		29jan08	in DoModal, fix untyped constant

        file dialog with enhanced multiple selection
 
*/

// MultiFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MultiFileDlg.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiFileDlg

IMPLEMENT_DYNAMIC(CMultiFileDlg, CFileDialog)

CMultiFileDlg::CMultiFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							 DWORD dwFlags, LPCTSTR lpszFilter, LPCTSTR lpszTitle, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd),
	m_Title(lpszTitle)
{
	//{{AFX_DATA_INIT(CMultiFileDlg)
	m_Preview = FALSE;
	//}}AFX_DATA_INIT
	m_ExStyle = 0;
	m_NotifyWnd = NULL;
	m_OwnTemplate = FALSE;
}

void CMultiFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CFileDialog::DoDataExchange(pDX);
	if (m_OwnTemplate) {	// only DDX if we're using our own template
	//{{AFX_DATA_MAP(CMultiFileDlg)
	DDX_Check(pDX, IDC_MULTI_FILE_PREVIEW, m_Preview);
	//}}AFX_DATA_MAP
	}
}

void CMultiFileDlg::OnFileNameChange()
{
	if (m_NotifyWnd != NULL)
		m_NotifyWnd->SendMessage(UWM_MULTIFILESEL, (WPARAM)this);
}

void CMultiFileDlg::SetPreview(bool Preview)
{
	m_Preview = Preview;
	if (IsWindow(this->m_hWnd))
		UpdateData(FALSE);
}

BEGIN_MESSAGE_MAP(CMultiFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CMultiFileDlg)
	ON_BN_CLICKED(IDC_MULTI_FILE_PREVIEW, OnMultiFilePreview)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CMultiFileDlg::DoModal()
{
	m_OwnTemplate = m_ExStyle & ES_SHOWPREVIEW;
	if (m_OwnTemplate) {
		m_ofn.Flags |= OFN_ENABLETEMPLATE;
		m_ofn.hInstance = AfxGetApp()->m_hInstance;
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_MULTI_FILE);
	}
	CString	Buffer;
	if (m_ofn.Flags & OFN_ALLOWMULTISELECT) {
		const int	BUFSIZE = 0x7fff;
		LPTSTR	FileBuf = Buffer.GetBufferSetLength(BUFSIZE);
		ZeroMemory(FileBuf, BUFSIZE);
		m_ofn.lpstrFile = FileBuf;
		m_ofn.nMaxFile = BUFSIZE;
		m_ofn.nFileOffset = 0;
	}
	if (!m_Title.IsEmpty())
		m_ofn.lpstrTitle = m_Title;
	int	retc = CFileDialog::DoModal();
	if (retc == IDOK) {
		m_Path.RemoveAll();
		if (m_ofn.Flags & OFN_ALLOWMULTISELECT) {
			POSITION	FilePos = GetStartPosition();
			while (FilePos != NULL)
				m_Path.Add(GetNextPathName(FilePos));
		} else
			m_Path.Add(GetPathName());
	}
	return(retc);
}

void CMultiFileDlg::OnMultiFilePreview() 
{
	UpdateData(TRUE);
	if (m_Preview)
		OnFileNameChange();
}

void CMultiFileDlg::GetFolder(CString& Folder)
{
	CPathStr	s(GetPathName());
	s.RemoveFileSpec();
	Folder = s;
}
