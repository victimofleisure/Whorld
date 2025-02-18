// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		13aug05	initial version
		01		17sep05	add MIDI sync
		02		28jan08	support Unicode

        MIDI options property page
 
*/

// OptsMidiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "OptsMidiDlg.h"
#include "BackBufDD.h"
#include "MidiIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsMidiDlg dialog

IMPLEMENT_DYNCREATE(COptsMidiDlg, CPropertyPage)

const COptsMidiDlg::STATE COptsMidiDlg::m_Default = {
	CMidiIO::NO_DEVICE,	// MidiDev
	FALSE	// MidiSync
};

#define REG_MIDI_DEVICE _T("MidiDevice")

#define REG_VALUE(name) m_Reg##name(_T(#name), m_st.name, m_Default.name)

COptsMidiDlg::COptsMidiDlg() : CPropertyPage(COptsMidiDlg::IDD),
	REG_VALUE(MidiSync)
{
	//{{AFX_DATA_INIT(COptsMidiDlg)
	m_MidiSync = FALSE;
	//}}AFX_DATA_INIT
	CString	DevName = CPersist::GetString(REG_SETTINGS, REG_MIDI_DEVICE);
	int	Devs = CMidiIO::GetInputDeviceNames(m_DevList);
	m_st.MidiDev = CMidiIO::NO_DEVICE;
	for (int i = 0; i < Devs; i++) {
		if (m_DevList[i] == DevName) {
			m_st.MidiDev = i;
			break;
		}
	}
}

COptsMidiDlg::~COptsMidiDlg()
{
	CPersist::WriteString(REG_SETTINGS, REG_MIDI_DEVICE,
		m_st.MidiDev == CMidiIO::NO_DEVICE ? "" : m_DevList[m_st.MidiDev]);
}

void COptsMidiDlg::SetDefaults()
{
	m_st = m_Default;
}

void COptsMidiDlg::UpdateUI()
{
	int	sel = m_MidiDev.GetCurSel();
	GetDlgItem(IDC_OP_MIDI_SYNC)->EnableWindow(sel);
}

void COptsMidiDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsMidiDlg)
	DDX_Control(pDX, IDC_OP_MIDI_DEV, m_MidiDev);
	DDX_Check(pDX, IDC_OP_MIDI_SYNC, m_MidiSync);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptsMidiDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsMidiDlg)
	ON_CBN_SELCHANGE(IDC_OP_MIDI_DEV, OnSelchangeMidiDev)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsMidiDlg message handlers

BOOL COptsMidiDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	m_MidiDev.AddString(LDS(IDS_OP_NO_MIDI_IN));
	for (int i = 0; i < m_DevList.GetSize(); i++)
		m_MidiDev.AddString(m_DevList[i]);
	m_MidiDev.SetCurSel(m_st.MidiDev + 1);
	m_MidiSync = m_st.MidiSync;
	UpdateData(FALSE);	// init dialog
	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void COptsMidiDlg::OnOK() 
{
	CPropertyPage::OnOK();
	m_st.MidiDev = m_MidiDev.GetCurSel() - 1;
	m_st.MidiSync = m_MidiSync != 0;
}

void COptsMidiDlg::OnSelchangeMidiDev() 
{
	UpdateUI();
}
