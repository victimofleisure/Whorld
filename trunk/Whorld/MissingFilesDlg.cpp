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
		05		29mar25	modernize style

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

CMissingFilesDlg::CMissingFilesDlg(CStringArrayEx& arrPath, LPCTSTR pszDefaultExt, 
								   LPCTSTR pszFilter, CWnd* pParentWnd /*=NULL*/)
	: m_arrPath(arrPath), m_pszDefaultExt(pszDefaultExt), 
	m_pszFilter(pszFilter), CDialog(CMissingFilesDlg::IDD, pParentWnd)
{
}

void CMissingFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMissingFilesDlg, CDialog)
	ON_BN_CLICKED(IDC_MISF_PROCEED, OnProceed)
	ON_BN_CLICKED(IDC_MISF_OPENDLG, OnOpenDlg)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissingFilesDlg message handlers

void CMissingFilesDlg::OnProceed()
{
	EndDialog(IDIGNORE);
}

void CMissingFilesDlg::OnOpenDlg() 
{
	EndDialog(IDC_MISF_OPENDLG);
}

INT_PTR	CMissingFilesDlg::Check()
{
	bool	bMissingFiles = false;
	INT_PTR	retc = IDOK;
	for (int iPath = 0; iPath < m_arrPath.GetSize(); iPath++) {
		if (m_arrPath[iPath].GetLength() && !PathFileExists(m_arrPath[iPath]))
			bMissingFiles = true;
	}
	if (bMissingFiles) {
		{
			CReplaceFilesDlg	rfd(m_arrPath, m_pszDefaultExt, m_pszFilter);
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
		// older versions blanked any remaining missing files, but it's unhelpful
	}
	return retc;
}

