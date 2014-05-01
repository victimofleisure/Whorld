// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      03feb06	initial version
        01      16aug06	move non-GUI implementation to VideoComprList
		02		05mar07	set combo selection from compressor state
		03		23nov07	support Unicode

        Video Compression dialog

*/

// VideoComprDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "VideoComprDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVideoComprDlg dialog

IMPLEMENT_DYNAMIC(CVideoComprDlg, CDialog);

CVideoComprDlg::CVideoComprDlg(CWnd* pParent)
	: CDialog(CVideoComprDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVideoComprDlg)
	m_KeyFrameEdit = 0;
	m_PFramesEdit = 0;
	m_DataRateEdit = 0;
	m_WndSizeEdit = 0;
	//}}AFX_DATA_INIT
}

CVideoComprDlg::~CVideoComprDlg()
{
}

bool CVideoComprDlg::CreateCompr()
{
	if (!m_vc.CreateList())	// populate compressor list
		return(FALSE);
	if (!m_vc.RestoreState(m_State))	// select compressor and restore its settings
		return(FALSE);
	m_vc.DisconnectCompr();	// make compressor available for use
	return(TRUE);
}

void CVideoComprDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVideoComprDlg)
	DDX_Control(pDX, IDC_VCD_QUAL_NUM, m_QualNum);
	DDX_Control(pDX, IDC_VCD_QUALITY, m_Quality);
	DDX_Control(pDX, IDC_VCD_COMBO, m_Combo);
	DDX_Text(pDX, IDC_VCD_KEY_FRAME_EDIT, m_KeyFrameEdit);
	DDX_Text(pDX, IDC_VCD_P_FRAMES_EDIT, m_PFramesEdit);
	DDX_Text(pDX, IDC_VCD_DATA_RATE_EDIT, m_DataRateEdit);
	DDX_Text(pDX, IDC_VCD_WND_SIZE_EDIT, m_WndSizeEdit);
	//}}AFX_DATA_MAP
}

bool CVideoComprDlg::SetFilters(IBaseFilter *pSource, IBaseFilter *pDest)
{
	if (!m_vc.SetFilters(pSource, pDest)) {
		AfxMessageBox(IDS_VCDE_GET_GRAPH);
		return(FALSE);
	}
	return(TRUE);
}

bool CVideoComprDlg::SelectCompr()
{
	m_vc.DisconnectCompr();	// disconnect compressor first in case we fail
	int sel = m_Combo.GetCurSel();	// get current selection from combo box
	if (sel < 0)	// no selection
		return FALSE;
	int	ComprIdx = m_Combo.GetItemData(sel);	// map item position to compressor index
	if (ComprIdx < 0)	// combo box error
		return FALSE;
	return(m_vc.SelectCompr(ComprIdx));	// select and connect compressor
}

void CVideoComprDlg::UpdateQuality()
{
	CAPS	Caps;
	m_vc.GetCaps(Caps);
	int qual = Caps.CanQuality ? int(m_vc.GetQuality() * 100.0 + .5) : 0;
	m_Quality.SetPos(qual);
	CString s;
	s.Format(_T("%d"), qual);
	m_QualNum.SetWindowText(s);
}

void CVideoComprDlg::UpdateUI()
{
	CAPS	Caps;
	m_vc.GetCaps(Caps);
	// update quality
	GetDlgItem(IDC_VCD_CONFIG)->EnableWindow(Caps.CanConfig);
	GetDlgItem(IDC_VCD_ABOUT)->EnableWindow(Caps.CanAbout);
	GetDlgItem(IDC_VCD_QUAL_CAP)->EnableWindow(Caps.CanQuality);
	m_Quality.EnableWindow(Caps.CanQuality);
	m_QualNum.EnableWindow(Caps.CanQuality);
	// update data rate
	GetDlgItem(IDC_VCD_DATA_RATE_EDIT)->EnableWindow(Caps.CanCrunch);
	GetDlgItem(IDC_VCD_DATA_RATE_CAP)->EnableWindow(Caps.CanCrunch);
	GetDlgItem(IDC_VCD_DATA_RATE_UNIT)->EnableWindow(Caps.CanCrunch);
	m_DataRateEdit = Caps.CanCrunch ? m_vc.GetDataRate() / 1000 : 0;
	// update key frame rate
	GetDlgItem(IDC_VCD_KEY_FRAME_EDIT)->EnableWindow(Caps.CanKeyFrame);
	GetDlgItem(IDC_VCD_KEY_FRAME_CAP)->EnableWindow(Caps.CanKeyFrame);
	GetDlgItem(IDC_VCD_KEY_FRAME_UNIT)->EnableWindow(Caps.CanKeyFrame);
	m_KeyFrameEdit = Caps.CanKeyFrame ? m_vc.GetKeyFrameRate() : 0;
	// update P frames per key
	GetDlgItem(IDC_VCD_P_FRAMES_EDIT)->EnableWindow(Caps.CanBFrame);
	GetDlgItem(IDC_VCD_P_FRAMES_CAP)->EnableWindow(Caps.CanBFrame);
	GetDlgItem(IDC_VCD_P_FRAMES_UNIT)->EnableWindow(Caps.CanBFrame);
	m_PFramesEdit = Caps.CanBFrame ? m_vc.GetPFramesPerKey() : 0;
	// update window size
	GetDlgItem(IDC_VCD_WND_SIZE_EDIT)->EnableWindow(Caps.CanWindow);
	GetDlgItem(IDC_VCD_WND_SIZE_CAP)->EnableWindow(Caps.CanWindow);
	GetDlgItem(IDC_VCD_WND_SIZE_UNIT)->EnableWindow(Caps.CanWindow);
	m_WndSizeEdit = Caps.CanWindow ? m_vc.GetWindowSize() : 0;
	// update dialog from member data
	UpdateData(FALSE);
	UpdateQuality();
}

BEGIN_MESSAGE_MAP(CVideoComprDlg, CDialog)
	//{{AFX_MSG_MAP(CVideoComprDlg)
	ON_BN_CLICKED(IDC_VCD_ABOUT, OnAbout)
	ON_BN_CLICKED(IDC_VCD_CONFIG, OnConfig)
	ON_CBN_SELCHANGE(IDC_VCD_COMBO, OnSelchangeCombo)
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_EN_KILLFOCUS(IDC_VCD_KEY_FRAME_EDIT, OnKillfocusKeyFrameEdit)
	ON_EN_KILLFOCUS(IDC_VCD_P_FRAMES_EDIT, OnKillfocusPFramesEdit)
	ON_EN_KILLFOCUS(IDC_VCD_DATA_RATE_EDIT, OnKillfocusDataRateEdit)
	ON_EN_KILLFOCUS(IDC_VCD_WND_SIZE_EDIT, OnKillfocusWndSizeEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoComprDlg message handlers

BOOL CVideoComprDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (!m_vc.CreateList()) {	// initialize COM and build compressor list
		AfxMessageBox(IDS_VCDE_INIT_COM);
		return TRUE;
	}
	if (HaveState())	// if saved compressor state
		m_vc.RestoreState(m_State);	// restore it
	// populate combo box
	int devs = m_vc.GetCount();
	int	sel = devs - 1;	// selection defaults to last item (uncompressed video)
	for (int i = 0; i < devs; i++) {	// for each compressor
		CString	name = m_vc.GetName(i);		// get compressor name
		// item position and compressor index can differ if combo box is sorted
		int	pos = m_Combo.AddString(name);	// add compressor name to combo box
		m_Combo.SetItemData(pos, i);		// store compressor index in item data
		if (name == m_State.m_Name)			// if name matches saved state's name
			sel = i;							// set selection
	}
	m_Combo.SetCurSel(sel);	// set selection in combo box
	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CVideoComprDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	m_vc.DisconnectCompr();	// make compressor available to user
}

void CVideoComprDlg::OnOK() 
{
	CAPS	Caps;
	m_vc.GetCaps(Caps);
	if (m_vc.GetCompr() != NULL && !Caps.CanConnect) {	// if compressor can't connect
		CString	Err((LPCTSTR)IDS_VCDE_CONNECT), DSErr;
		CDirShowU::GetErrorString(m_vc.GetLastError(), DSErr);
		AfxMessageBox(Err + "\n" + DSErr);	// display error message
	} else {
		m_vc.SaveState(m_State);	// save selected compressor's state
		CDialog::OnOK();
	}
}

void CVideoComprDlg::OnConfig() 
{
	if (!m_vc.ShowConfigDlg(m_hWnd))
		AfxMessageBox(IDS_VCDE_SHOW_CONFIG);
}

void CVideoComprDlg::OnAbout() 
{
	if (!m_vc.ShowAboutDlg(m_hWnd))
		AfxMessageBox(IDS_VCDE_SHOW_ABOUT);
}

void CVideoComprDlg::OnSelchangeCombo() 
{
	SelectCompr();
	UpdateUI();
}

void CVideoComprDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CAPS	Caps;
	m_vc.GetCaps(Caps);
	if (Caps.CanQuality) {
		if (!m_vc.SetQuality(m_Quality.GetPos() / 100.0))
			AfxMessageBox(IDS_VCDE_SET_QUALITY);
		UpdateQuality();
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVideoComprDlg::OnKillfocusDataRateEdit() 
{
	UpdateData(TRUE);	// retrieve member data from dialog
	if (!m_vc.SetDataRate(m_DataRateEdit * 1000))	// convert from Kbps to bps
		AfxMessageBox(IDS_VCDE_SET_DATA_RATE);
}

void CVideoComprDlg::OnKillfocusKeyFrameEdit() 
{
	UpdateData(TRUE);	// retrieve member data from dialog
	if (!m_vc.SetKeyFrameRate(m_KeyFrameEdit))
		AfxMessageBox(IDS_VCDE_SET_KEY_FRAMES);
}

void CVideoComprDlg::OnKillfocusPFramesEdit() 
{
	UpdateData(TRUE);	// retrieve member data from dialog
	if (!m_vc.SetPFramesPerKey(m_PFramesEdit))
		AfxMessageBox(IDS_VCDE_SET_PFRAMES);
}

void CVideoComprDlg::OnKillfocusWndSizeEdit() 
{
	UpdateData(TRUE);	// retrieve member data from dialog
	if (!m_vc.SetWindowSize(m_WndSizeEdit))
		AfxMessageBox(IDS_VCDE_SET_WND_SIZE);
}
