// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05oct05	initial version

        playlist MIDI mapping dialog
 
*/

// PlaylistMidiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PlaylistMidiDlg.h"
#include "MidiSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlaylistMidiDlg dialog

IMPLEMENT_DYNAMIC(CPlaylistMidiDlg, CDialog);

const CPlaylistMidiDlg::INFO CPlaylistMidiDlg::m_Defaults = {
	MS_PROG_CHG,	// Scheme
	0,	// Chan
	0,	// First
	CMidiSetupDlg::MIDI_PARMS - 1	// Last
};

CPlaylistMidiDlg::CPlaylistMidiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlaylistMidiDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlaylistMidiDlg)
	m_Scheme = 0;
	//}}AFX_DATA_INIT
	m_Info = m_Defaults;
}

void CPlaylistMidiDlg::GetDefaults(INFO& Info)
{
	Info = m_Defaults;
}

void CPlaylistMidiDlg::UpdateUI()
{
	UpdateData(TRUE);	// retrieve data
	bool	IsNote = (m_Scheme == MS_NOTES);
	m_First.SetNoteEntry(IsNote);
	m_Last.SetNoteEntry(IsNote);
	m_Chan.EnableWindow(m_Scheme);
	m_First.EnableWindow(m_Scheme);
	m_Last.EnableWindow(m_Scheme);
}

void CPlaylistMidiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaylistMidiDlg)
	DDX_Control(pDX, IDC_PLM_LAST_SPIN, m_LastSpin);
	DDX_Control(pDX, IDC_PLM_LAST, m_Last);
	DDX_Control(pDX, IDC_PLM_FIRST_SPIN, m_FirstSpin);
	DDX_Control(pDX, IDC_PLM_FIRST, m_First);
	DDX_Control(pDX, IDC_PLM_CHAN_SPIN, m_ChanSpin);
	DDX_Control(pDX, IDC_PLM_CHAN, m_Chan);
	DDX_Radio(pDX, IDC_PLM_SCHEME, m_Scheme);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPlaylistMidiDlg, CDialog)
	//{{AFX_MSG_MAP(CPlaylistMidiDlg)
	ON_BN_CLICKED(IDC_PLM_SCHEME, OnScheme)
	ON_BN_CLICKED(IDC_PLM_SCHEME2, OnScheme)
	ON_BN_CLICKED(IDC_PLM_SCHEME3, OnScheme)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaylistMidiDlg message handlers

BOOL CPlaylistMidiDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Chan.SetRange(1, CMidiSetupDlg::MIDI_CHANS);
	m_First.SetRange(0, CMidiSetupDlg::MIDI_PARMS - 1);
	m_Last.SetRange(0, CMidiSetupDlg::MIDI_PARMS - 1);
	m_Scheme = m_Info.Scheme;
	m_Chan.SetVal(m_Info.Chan + 1);
	m_First.SetVal(m_Info.First);
	m_Last.SetVal(m_Info.Last);
	UpdateData(FALSE);	// update dialog
	UpdateUI();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlaylistMidiDlg::OnOK() 
{
	if (!UpdateData(TRUE))	// retrieve data
		return;
	m_Info.Scheme	= m_Scheme;
	m_Info.Chan		= m_Chan.GetIntVal() - 1;
	m_Info.First	= m_First.GetIntVal();
	m_Info.Last 	= m_Last.GetIntVal();
	CDialog::OnOK();
}

void CPlaylistMidiDlg::OnScheme() 
{
	UpdateUI();
}
