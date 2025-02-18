// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27sep05	initial version
		01		07jun06	set view before opening movie
		02		21dec07	rename GetMainFrame to GetThis
		03		29jan08	in DoModal, create CSize explicitly to fix warning

        movie export options dialog
 
*/

// MovieResizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MovieResizeDlg.h"
#include "SnapMovie.h"
#include "MultiFileDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovieResizeDlg dialog


CMovieResizeDlg::CMovieResizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMovieResizeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMovieResizeDlg)
	m_Height = 0;
	m_Width = 0;
	//}}AFX_DATA_INIT
}

void CMovieResizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMovieResizeDlg)
	DDX_Text(pDX, IDC_MRS_HEIGHT, m_Height);
	DDX_Text(pDX, IDC_MRS_WIDTH, m_Width);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMovieResizeDlg, CDialog)
	//{{AFX_MSG_MAP(CMovieResizeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovieResizeDlg message handlers

int CMovieResizeDlg::DoModal() 
{
	CMultiFileDlg	fd(TRUE, EXT_MOVIE, NULL, OFN_HIDEREADONLY,
		LDS(IDS_FILTER_MOVIE), LDS(IDS_MOVIE_OPEN));
	if (fd.DoModal() != IDOK)
		return(IDCANCEL);
	CSnapMovie	sm;
	CMainFrame	*Frm = CMainFrame::GetThis();
	sm.SetView(Frm->GetView());
	if (!sm.Open(fd.GetPathName(), TRUE))
		return(IDABORT);
	m_Width = sm.GetFrameSize().cx;
	m_Height = sm.GetFrameSize().cy;
	if (CDialog::DoModal() == IDOK) {
		sm.Close();
		CSize	sz(m_Width, m_Height);
		sm.Resize(fd.GetPathName(), sz);
	}
	return(IDOK);
}
