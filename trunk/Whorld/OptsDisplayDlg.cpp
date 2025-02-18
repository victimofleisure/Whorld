// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22jun05	initial version
        01      29jun05	add mirror precision option
        02      10jul05	add auto memory option
		03		13jul05	separate state from controls
		04		26jul05	add print from bitmap
		05		29aug05	add frame rate
        06      14oct05	add exclusive mode
		07		28jan08	support Unicode
		08		29jan08	remove duplicate return in GetOptions

        display options property page
 
*/

// OptsDisplayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "OptsDisplayDlg.h"
#include "WhorldViewDD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsDisplayDlg dialog

IMPLEMENT_DYNCREATE(COptsDisplayDlg, CPropertyPage)

const COptsDisplayDlg::STATE COptsDisplayDlg::m_Default = {
	TRUE,		// UseDirDraw
	TRUE,		// MirrorPrecise
	TRUE,		// AllowExclusive
	FALSE,		// PrintFromBitmap
	BB_AUTO,	// BackBufLoc
	FPS_DEF,	// FrameRate
};

#define REG_VALUE(name) m_Reg##name(_T(#name), m_st.name, m_Default.name)

COptsDisplayDlg::COptsDisplayDlg() : CPropertyPage(COptsDisplayDlg::IDD),
	REG_VALUE(UseDirDraw),
	REG_VALUE(MirrorPrecise),
	REG_VALUE(AllowExclusive),
	REG_VALUE(PrintFromBitmap),
	REG_VALUE(BackBufLoc),
	REG_VALUE(FrameRate)
{
	//{{AFX_DATA_INIT(COptsDisplayDlg)
	m_UseDirDraw = FALSE;
	m_BackBufLoc = 0;
	m_PrintFromBitmap = FALSE;
	m_MirrorPrecise = FALSE;
	m_AllowExclusive = FALSE;
	//}}AFX_DATA_INIT
}

void COptsDisplayDlg::SetDefaults()
{
	m_st = m_Default;
}

int COptsDisplayDlg::GetOptions() const
{
	return(
		(m_st.BackBufLoc == BB_AUTO ? CWhorldViewDD::OPT_AUTO_MEMORY :
		(m_st.BackBufLoc == BB_VIDEO ? CWhorldViewDD::OPT_USE_VIDEO_MEM : 0)) |
		(m_st.MirrorPrecise ? CWhorldViewDD::OPT_MIRROR_PRECISE : 0) |
		(m_st.PrintFromBitmap ? CWhorldViewDD::OPT_PRINT_FROM_BITMAP : 0));
}

void COptsDisplayDlg::UpdateUI()
{
	UpdateData(TRUE);	// retrieve data
	bool	dd = m_UseDirDraw != 0;
	GetDlgItem(IDC_OP_BACK_BUF_LOC)->EnableWindow(dd);
	GetDlgItem(IDC_OP_MIRROR_PRECISE)->EnableWindow(dd);
	GetDlgItem(IDC_OP_ALLOW_EXCLUSIVE)->EnableWindow(dd);
}

void COptsDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsDisplayDlg)
	DDX_Control(pDX, IDC_OP_FRAME_RATE_SPIN, m_FrameRateSpin);
	DDX_Control(pDX, IDC_OP_FRAME_RATE, m_FrameRate);
	DDX_Check(pDX, IDC_OP_USE_DIRDRAW, m_UseDirDraw);
	DDX_CBIndex(pDX, IDC_OP_BACK_BUF_LOC, m_BackBufLoc);
	DDX_Check(pDX, IDC_OP_PRINT_FROM_BITMAP, m_PrintFromBitmap);
	DDX_Check(pDX, IDC_OP_MIRROR_PRECISE, m_MirrorPrecise);
	DDX_Check(pDX, IDC_OP_ALLOW_EXCLUSIVE, m_AllowExclusive);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptsDisplayDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsDisplayDlg)
	ON_BN_CLICKED(IDC_OP_USE_DIRDRAW, OnUseDirdraw)
	ON_CBN_SELCHANGE(IDC_OP_BACK_BUF_LOC, OnSelchangeBackBufLoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsDisplayDlg message handlers

BOOL COptsDisplayDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	m_UseDirDraw		= m_st.UseDirDraw;
	m_MirrorPrecise		= m_st.MirrorPrecise;
	m_AllowExclusive		= m_st.AllowExclusive;
	m_BackBufLoc		= m_st.BackBufLoc;
	m_PrintFromBitmap	= m_st.PrintFromBitmap;
	m_FrameRate.SetVal(m_st.FrameRate);
	m_FrameRate.SetFormat(CNumEdit::DF_INT);
	m_FrameRate.SetRange(FPS_MIN, FPS_MAX);
	UpdateData(FALSE);	// init dialog
	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void COptsDisplayDlg::OnOK() 
{
	CPropertyPage::OnOK();
	m_st.UseDirDraw			= m_UseDirDraw != 0;
	m_st.MirrorPrecise		= m_MirrorPrecise != 0;
	m_st.AllowExclusive		= m_AllowExclusive != 0;
	m_st.BackBufLoc			= m_BackBufLoc;
	m_st.PrintFromBitmap	= m_PrintFromBitmap != 0;
	m_st.FrameRate			= m_FrameRate.GetIntVal();
}

void COptsDisplayDlg::OnUseDirdraw()
{
	AfxMessageBox(IDS_OP_MUST_RESTART);
	UpdateUI();
}

void COptsDisplayDlg::OnSelchangeBackBufLoc() 
{
	UpdateData(TRUE);	// retrieve data
	if (m_BackBufLoc == BB_VIDEO)
		AfxMessageBox(IDS_OP_VIDEO_MEM);
}
