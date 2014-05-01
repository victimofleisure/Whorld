// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00      04oct05	initial version
		01		17feb06	allow variable property count
		02		21dec07	rename GetMainFrame to GetThis
		03		28jan08	support Unicode

        MIDI auto assign dialog
 
*/

// MidiAutoAssignDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MidiSetupDlg.h"
#include "MidiAutoAssignDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiAutoAssignDlg dialog

IMPLEMENT_DYNAMIC(CMidiAutoAssignDlg, CDialog);

const CMidiAutoAssignDlg::INFO CMidiAutoAssignDlg::m_Defaults = {
	FALSE,	// PropRange
	0,		// PropFirst
	0,		// PropLast
	CMidiSetupDlg::ET_CTRL,		// Event
	0,		// Chan
	FALSE,	// ChanRange
	0,		// Param
	TRUE	// ParamRange
};

CMidiAutoAssignDlg::CMidiAutoAssignDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMidiAutoAssignDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMidiAutoAssignDlg)
	m_Event = 0;
	m_ChanRange = FALSE;
	m_ParamRange = 0;
	m_SetChanOnly = FALSE;
	m_PropRange = 0;
	//}}AFX_DATA_INIT
	m_Info = m_Defaults;
}

void CMidiAutoAssignDlg::UpdateUI()
{
	UpdateData(TRUE);	// retrieve data
	m_PropFirst.EnableWindow(m_PropRange);
	m_PropLast.EnableWindow(m_PropRange);
	if (!m_PropRange) {
		m_PropFirst.SetCurSel(0);
		m_PropLast.SetCurSel(m_Props - 1);
	}
	bool	IsNote = (m_Event == CMidiSetupDlg::ET_NOTE);
	CString	s((LPCTSTR)(IsNote ? IDS_MAA_NOTE : IDS_MAA_CTRL));
	m_ParmCap.SetWindowText(s);	// change parameter caption to match event type
	m_Param.SetNoteEntry(IsNote);	// if note event, enable note entry
}

void CMidiAutoAssignDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMidiAutoAssignDlg)
	DDX_Control(pDX, IDC_MAA_PARAM_SPIN, m_ParamSpin);
	DDX_Control(pDX, IDC_MAA_CHAN_SPIN, m_ChanSpin);
	DDX_Control(pDX, IDC_MAA_PROP_LAST, m_PropLast);
	DDX_Control(pDX, IDC_MAA_PROP_FIRST, m_PropFirst);
	DDX_Control(pDX, IDC_MAA_PARM_CAP, m_ParmCap);
	DDX_Radio(pDX, IDC_MAA_EVENT, m_Event);
	DDX_Check(pDX, IDC_MAA_CHAN_RANGE, m_ChanRange);
	DDX_Control(pDX, IDC_MAA_PARAM, m_Param);
	DDX_Check(pDX, IDC_MAA_PARAM_RANGE, m_ParamRange);
	DDX_Control(pDX, IDC_MAA_CHAN, m_Chan);
	DDX_Check(pDX, IDC_MAA_SET_CHAN_ONLY, m_SetChanOnly);
	DDX_Radio(pDX, IDC_MAA_PROP_RANGE, m_PropRange);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMidiAutoAssignDlg, CDialog)
	//{{AFX_MSG_MAP(CMidiAutoAssignDlg)
	ON_BN_CLICKED(IDC_MAA_EVENT, OnEventType)
	ON_BN_CLICKED(IDC_MAA_PROP_RANGE, OnPropRange)
	ON_BN_CLICKED(IDC_MAA_EVENT2, OnEventType)
	ON_BN_CLICKED(IDC_MAA_EVENT3, OnEventType)
	ON_BN_CLICKED(IDC_MAA_PROP_RANGE2, OnPropRange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiAutoAssignDlg message handlers

BOOL CMidiAutoAssignDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CMainFrame	*Frm = CMainFrame::GetThis();
	ASSERT(Frm != NULL);
	CMidiSetupDlg&	msd = Frm->GetMidiSetup();
	m_Props = msd.GetCount();
	for (int i = 0; i < m_Props; i++) {	// populate list boxes
		LPCTSTR	Caption = msd.GetRowCaption(msd.GetRowIdx(i));
		m_PropFirst.AddString(Caption);
		m_PropLast.AddString(Caption);
	}
	m_PropRange = m_Info.PropRange;
	if (m_Info.PropFirst >= m_Props)	// clamp range in case prop count changed
		m_Info.PropFirst = 0;
	if (m_Info.PropLast >= m_Props)
		m_Info.PropLast = m_Props - 1;
	m_PropFirst.SetCurSel(m_Info.PropFirst);
	m_PropLast.SetCurSel(m_Info.PropLast);
	m_Event = m_Info.Event;
	m_Chan.SetVal(m_Info.Chan + 1);
	m_ChanRange = m_Info.ChanRange;
	m_Param.SetVal(m_Info.Param);
	m_ParamRange = m_Info.ParamRange;
	m_SetChanOnly = m_Info.SetChanOnly;
	UpdateData(FALSE);	// update dialog
	m_Chan.SetRange(1, CMidiSetupDlg::MIDI_CHANS);
	m_Chan.SetFormat(CNumEdit::DF_INT);
	m_Param.SetRange(0, CMidiSetupDlg::MIDI_PARMS - 1);
	m_Param.SetFormat(CNumEdit::DF_INT);
	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMidiAutoAssignDlg::OnOK() 
{
	if (!UpdateData(TRUE))	// retrieve data
		return;
	m_Info.PropRange = m_PropRange != 0;
	m_Info.PropFirst = m_PropFirst.GetCurSel();
	m_Info.PropLast = m_PropLast.GetCurSel();
	m_Info.Event = m_Event;
	m_Info.Chan = m_Chan.GetIntVal() - 1;
	m_Info.ChanRange = m_ChanRange != 0;
	m_Info.Param = m_Param.GetIntVal();
	m_Info.ParamRange = m_ParamRange != 0;
	m_Info.SetChanOnly = m_SetChanOnly != 0;
	CDialog::OnOK();
}

void CMidiAutoAssignDlg::OnEventType() 
{
	UpdateUI();
}

void CMidiAutoAssignDlg::OnPropRange() 
{
	UpdateUI();
}
