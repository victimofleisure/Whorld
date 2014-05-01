// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov03 initial version
		01		16jan04	add file filter argument to ctor
		02		01oct04	use IDC_ prefix for all controls
		03		05aug05	pass default extension to ctor
		04		23nov07	support Unicode

		missing files dialog
 
*/

// MissingFilesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MissingFilesDlg.h"
#include "ReplaceFilesDlg.h"
#include "Shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMissingFilesDlg dialog

IMPLEMENT_DYNAMIC(CMissingFilesDlg, CDialog);

CMissingFilesDlg::CMissingFilesDlg(CStringArray& Path, LPCTSTR DefaultExt, 
								   LPCTSTR Filter, CWnd* pParent /*=NULL*/)
	: m_Path(Path), m_DefaultExt(DefaultExt), 
	m_Filter(Filter), CDialog(CMissingFilesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMissingFilesDlg)
	//}}AFX_DATA_INIT
}


void CMissingFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMissingFilesDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMissingFilesDlg, CDialog)
	//{{AFX_MSG_MAP(CMissingFilesDlg)
	ON_BN_CLICKED(IDC_MISF_PROCEED, OnProceed)
	ON_BN_CLICKED(IDC_MISF_OPENDLG, OnOpendlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissingFilesDlg message handlers

void CMissingFilesDlg::OnProceed()
{
	EndDialog(IDIGNORE);
}

void CMissingFilesDlg::OnOpendlg() 
{
	EndDialog(IDC_MISF_OPENDLG);
}

int	CMissingFilesDlg::Check()
{
	bool	MissingFiles = FALSE;
	int	retc = IDOK;
	for (int i = 0; i < m_Path.GetSize(); i++) {
		if (m_Path[i].GetLength() && !PathFileExists(m_Path[i]))
			MissingFiles = TRUE;
	}
	if (MissingFiles) {
		{
			CReplaceFilesDlg	rfd(m_Path, m_DefaultExt, m_Filter);
			switch (retc = DoModal()) {
			case IDC_MISF_OPENDLG:
				retc = rfd.DoModal();
				break;
			case IDOK:
				if (!rfd.Search())
					retc = rfd.DoModal();
				break;
			}
		}
		// delete any remaining missing files
		for (int i = 0; i < m_Path.GetSize(); i++) {
			if (m_Path[i] && !PathFileExists(m_Path[i]))
				m_Path[i].Empty();
		}
	}
	return(retc);
}

