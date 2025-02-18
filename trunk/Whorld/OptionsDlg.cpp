// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22jun05	initial version
		01		06jul05	add color page
		02		31jul05	add reset all
		03		12aug05	remove color page
		04		13aug05	add MIDI page
		05		15sep05	add folder page
		06		18apr06	remove general page
		07		28jan08	support Unicode

        options property sheet
 
*/

// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "OptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg

IMPLEMENT_DYNAMIC(COptionsDlg, CPropertySheet)

COptionsDlg::COptionsDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	Construct();
}

COptionsDlg::COptionsDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	Construct();
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::Construct()
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	// add new pages to OnResetAll also
	AddPage(&m_Display);
	AddPage(&m_Input);
	AddPage(&m_Midi);
	AddPage(&m_Folder);
	m_CurPage = 0;
}

BEGIN_MESSAGE_MAP(COptionsDlg, CPropertySheet)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_OP_RESET_ALL, OnResetAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	SetActivePage(m_CurPage);
	
	// create a reset all button
	CRect	r, rt;
	GetDlgItem(IDOK)->GetWindowRect(r);
	GetTabControl()->GetWindowRect(rt);
	ScreenToClient(r);
	ScreenToClient(rt);
	int	w = r.Width();
	r.left = rt.left;
	r.right = rt.left + w;
	CString	Title(LPCTSTR(IDC_OP_RESET_ALL));
	m_ResetAll.Create(Title, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		r, this, IDC_OP_RESET_ALL);
	m_ResetAll.SetFont(GetFont());

	return bResult;
}

void COptionsDlg::OnDestroy() 
{
	m_CurPage = GetActiveIndex();

	CPropertySheet::OnDestroy();
}

void COptionsDlg::OnResetAll() 
{
	if (AfxMessageBox(IDS_OP_RESET_ALL, MB_YESNO) == IDYES) {
		EndDialog(IDOK);
		m_Display.SetDefaults();
		m_Input.SetDefaults();
		m_Midi.SetDefaults();
		m_Folder.SetDefaults();
	}
}
