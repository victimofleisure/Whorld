// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28jun05	initial version
		01		10jul05	add stretch and resolution
		02		13jul05	separate state from controls
		03		28jan08	support Unicode

        export options dialog
 
*/

// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ExportDlg.h"
#include "Persist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

IMPLEMENT_DYNAMIC(CExportDlg, CDialog);

#define REG_VALUE(name, defval) m_Reg##name(_T("Exp")_T(#name), m_st.name, defval)

CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent),
	REG_VALUE(UseWndSize, TRUE),
	REG_VALUE(Width, 0),
	REG_VALUE(Height, 0),
	REG_VALUE(ScaleToFit, FALSE),
	REG_VALUE(Resolution, 0)
{
	//{{AFX_DATA_INIT(CExportDlg)
	m_UseWndSize = FALSE;
	m_Width = 0;
	m_Height = 0;
	m_Resolution = 0;
	m_ScaleToFit = -1;
	//}}AFX_DATA_INIT
}

bool CExportDlg::GetSize(CSize& Size) const
{
	Size = CSize(m_st.Width, m_st.Height);
	return(m_st.UseWndSize != 0);
}

void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
	DDX_Check(pDX, IDC_EXP_USE_WND_SIZE, m_UseWndSize);
	DDX_Text(pDX, IDC_EXP_WIDTH, m_Width);
	DDX_Text(pDX, IDC_EXP_HEIGHT, m_Height);
	DDX_Text(pDX, IDC_EXP_RESOLUTION, m_Resolution);
	DDX_Radio(pDX, IDC_EXP_SCALE_TO_FIT, m_ScaleToFit);
	//}}AFX_DATA_MAP
}

void CExportDlg::UpdateUI()
{
	UpdateData(TRUE);	// retrieve data
	if (m_UseWndSize) {
		CRect	r;
		CFrameWnd	*frm = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd());
		ASSERT(frm != NULL);
		CView	*vp = frm->GetActiveView();
		vp->GetClientRect(r);
		m_Width = r.Width();
		m_Height = r.Height();
		m_ScaleToFit = FALSE;
		m_Resolution = DEFAULT_RESOLUTION;
		UpdateData(FALSE);	// init dialog
	}
	GetDlgItem(IDC_EXP_WIDTH)->EnableWindow(!m_UseWndSize);
	GetDlgItem(IDC_EXP_HEIGHT)->EnableWindow(!m_UseWndSize);
	GetDlgItem(IDC_EXP_SCALE_TO_FIT)->EnableWindow(!m_UseWndSize);
	GetDlgItem(IDC_EXP_SCALE_TO_FIT2)->EnableWindow(!m_UseWndSize);
	GetDlgItem(IDC_EXP_RESOLUTION)->EnableWindow(!m_UseWndSize);
}

BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	//{{AFX_MSG_MAP(CExportDlg)
	ON_BN_CLICKED(IDC_EXP_USE_WND_SIZE, OnExpUseWndSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

BOOL CExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_UseWndSize	= m_st.UseWndSize;
	m_Height		= m_st.Height;
	m_Width			= m_st.Width;
	m_ScaleToFit	= m_st.ScaleToFit;
	m_Resolution	= m_st.Resolution;
	UpdateData(FALSE);	// init dialog
	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CExportDlg::OnExpUseWndSize() 
{
	UpdateUI();
}

void CExportDlg::OnOK() 
{
	CDialog::OnOK();
	m_st.UseWndSize	= m_UseWndSize != 0;
	m_st.Height		= m_Height;
	m_st.Width		= m_Width;
	m_st.ScaleToFit	= m_ScaleToFit != 0;
	m_st.Resolution = m_Resolution;
}
