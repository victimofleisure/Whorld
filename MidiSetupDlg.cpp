// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
        00      16aug05	add learn mode, shadow values
		01		21aug05	in zero controllers, set values to 64
		02		04sep05	add miscellaneous properties
		03		04oct05	add support for notes
		04		17oct05	in Assign, reset prev mapping's event only
		05		17feb06	add oscillator properties
		06		11oct06	remove OnClose remnant from message map
		07		10dec07	add global parameters
		08		21dec07	replace AfxGetMainWnd with GetThis 
		09		22jan08	add special caption for global rotation
		10		23jan08	replace MIDI range scaler with start/end
		11		28jan08	support Unicode
		12		29jan08	in MakeMidiMap, add static cast to fix warning
		13		30jan08	use main keyboard accelerators
		14		31mar08	redo default MIDI controller scheme

		MIDI setup dialog
 
*/

// MidiSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MidiSetupDlg.h"
#include "MidiSetupRow.h"
#include "Oscillator.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg dialog

IMPLEMENT_DYNAMIC(CMidiSetupDlg, CRowDialog);

const CRowDialog::COLINFO CMidiSetupDlg::m_ColInfo[COLS] = {
	{IDC_MS_TITLE,			0},
	{IDC_MS_RANGE_START,	IDS_MS_RANGE},
	{IDC_MS_RANGE_START,	IDS_MS_RANGE},
	{IDC_MS_EVENT,			IDS_MS_EVENT},
	{IDC_MS_CHAN,			IDS_MS_CHAN},
	{IDC_MS_CTRL,			IDS_MS_CTRL},
	{IDC_MS_VALUE,			IDS_MS_VALUE}
};

#undef MIDI_PROP
#define MIDI_PROP(name, tag, start, end, ctrl) {_T(#name), IDS_MP_##tag, {start, end}, ctrl},
const CMidiSetupDlg::MISCPROPDATA CMidiSetupDlg::m_MiscPropData[MISC_PROPS] = {
	#include "MidiProps.h"
};

// never change the suffix strings, else playlist tagged I/O will break
const CMidiSetupDlg::OSCPROPDATA CMidiSetupDlg::m_OscPropData[OSC_PROPS] = {
//	suffix			title
	{_T("Wave"),	IDS_MS_OSC_WAVE},
	{_T("Amp"),		IDS_MS_OSC_AMP},
	{_T("Freq"),	IDS_MS_OSC_FREQ},
	{_T("PW"),		IDS_MS_OSC_PW}
};

CMidiSetupDlg::CMidiSetupDlg(CWnd* pParent /*=NULL*/)
	: CRowDialog(CMidiSetupDlg::IDD, IDR_MAINFRAME, _T("MidiSetupDlg"), pParent)
{
	//{{AFX_DATA_INIT(CMidiSetupDlg)
	//}}AFX_DATA_INIT
	ZeroMemory(m_CtrlMap, sizeof(m_CtrlMap));
	ZeroMemory(m_NoteMap, sizeof(m_NoteMap));
	ZeroMemory(m_PitchMap, sizeof(m_PitchMap));
	memset(m_Value, CENTER_POS, sizeof(m_Value));	// center all control shadows
	ZeroMemory(&m_Info, sizeof(m_Info));
	MakeRowTables();
	MakeDefaults(m_DefInfo);
	m_RowSel = -1;			// no row selection
	m_ShowRowSel = FALSE;
	m_Learn = FALSE;
}

CWnd *CMidiSetupDlg::CreateRow(int Idx, int& Pos)
{
	ASSERT(Idx >= 0 && Idx < ROWS);
	CMidiSetupRow	*rp = new CMidiSetupRow;
	rp->Create(IDD_MIDI_SETUP_ROW);
	CString	s = GetRowCaption(Idx);
	rp->SetCaption(s + ":");
	Pos = m_RowPos[Idx];	// pass row's position in list to base class
	return(rp);	// return address of created row object to base class
}

void CMidiSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CRowDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMidiSetupDlg)
	DDX_Control(pDX, IDC_MS_ADVANCED, m_AdvancedChk);
	DDX_Control(pDX, IDC_MS_LEARN, m_LearnChk);
	//}}AFX_DATA_MAP
}

void CMidiSetupDlg::SelectRow(int RowIdx)
{
	if (RowIdx != m_RowSel) {
		if (m_ShowRowSel) {
			if (m_RowSel >= 0)
				GetRow(m_RowSel)->SetSelected(FALSE);	// remove previous selection
			if (RowIdx >= 0)
				GetRow(RowIdx)->SetSelected(TRUE);
		}
		m_RowSel = RowIdx;
	}
}

void CMidiSetupDlg::ShowRowSel(bool Enable)
{
	if (Enable != m_ShowRowSel) {
		if (Enable) {
			m_ShowRowSel = Enable;	// order matters
			int	row = m_RowSel;	// save selection
			m_RowSel = -1;	// force SelectRow to update
			SelectRow(row);	// show selection
		} else {
			SelectRow(-1);	// hide selection
			m_ShowRowSel = Enable;	// order matters
		}
	}
}

void CMidiSetupDlg::SetLearn(bool Enable)
{
	m_Learn = Enable;
	m_LearnChk.SetCheck(Enable);
	ShowRowSel(Enable);
}

void CMidiSetupDlg::GetDefaults(INFO& Info) const
{
	Info = m_DefInfo;
}

void CMidiSetupDlg::MakeRowTables()
{
	m_RowName.SetSize(ROWS);
	m_RowCaption.SetSize(ROWS);
	int	i;
	for (i = 0; i < PARMS; i++)	{	// for parameters, storage and display orders differ
		m_RowIdx[i] = CParmInfo::m_RowOrder[i];
		m_RowName[i] = CParmInfo::m_RowData[i].Name;
		m_RowCaption[i].LoadString(CParmInfo::m_RowData[i].TitleID);
	}
	for (; i < FIRST_GLOB_ROW; i++) {	// for miscellaneous properties
		m_RowIdx[i] = i;	// storage and display orders are identical
		m_RowName[i] = m_MiscPropData[i - PARMS].Name;
		m_RowCaption[i].LoadString(m_MiscPropData[i - PARMS].TitleID);
	}
	CString	GlobPrefix((LPCTSTR)IDS_MS_GLOB_PREFIX);
	for (; i < FIRST_OSC_ROW; i++) {	// for global parameters
		int	ParmIdx = i - FIRST_GLOB_ROW;
		m_RowIdx[i] = FIRST_GLOB_ROW + ParmIdx;
		int	gpi = CParmInfo::m_GlobParm[ParmIdx];
		m_RowName[i] = GlobPrefix + CParmInfo::m_RowData[gpi].Name;
		int	TitleID = gpi == CParmInfo::ROTATE_SPEED ?
			IDS_VP_ROTATION : CParmInfo::m_RowData[gpi].TitleID;
		m_RowCaption[i] = GlobPrefix + " " + LDS(TitleID);
	}
	CStringArray	OscPropTitle;
	OscPropTitle.SetSize(OSC_PROPS);
	for (int k = 0; k < OSC_PROPS; k++)
		OscPropTitle[k].LoadString(m_OscPropData[k].TitleID);
	CString	s;
	int	j;
	for (j = 0; j < PARMS; j++) {	// one group of oscillator rows per parameter
		for (int k = 0; k < OSC_PROPS; k++) {	// inner loop for oscillator properties
			m_RowIdx[i] = FIRST_OSC_ROW + CParmInfo::m_RowOrder[j] * OSC_PROPS + k;
			s = CParmInfo::m_RowData[j].Name;
			m_RowName[i] = s + m_OscPropData[k].Suffix;
			s.LoadString(CParmInfo::m_RowData[j].TitleID);
			m_RowCaption[i] = s + " " + OscPropTitle[k];
			i++;
		}
	}
	GlobPrefix = "G ";	// so titles fit in column
	for (j = 0; j < GLOBAL_PARMS; j++) {	// one group of oscillator rows per global
		for (int k = 0; k < OSC_PROPS; k++) {	// inner loop for oscillator properties
			m_RowIdx[i] = FIRST_GLOB_OSC_ROW + j * OSC_PROPS + k;
			int	gpi = CParmInfo::m_GlobParm[j];
			s = CParmInfo::m_RowData[gpi].Name;
			m_RowName[i] = s + m_OscPropData[k].Suffix;
			int	TitleID = gpi == CParmInfo::ROTATE_SPEED ?
				IDS_VP_ROTATION : CParmInfo::m_RowData[gpi].TitleID;
			s.LoadString(TitleID);
			m_RowCaption[i] = GlobPrefix + s + " " + OscPropTitle[k];
			i++;
		}
	}
	for (i = 0; i < ROWS; i++)
		m_RowPos[m_RowIdx[i]] = i;	// reverse lookup to obtain display position
}

void CMidiSetupDlg::MakeDefaults(INFO& Info) const
{
	ZeroMemory(&Info, sizeof(INFO));
	int	i, j, ctrl = 1;
	for (i = 0; i < PARMS; i++) {
		int	pos = CParmInfo::m_RowOrder[i];
		double	maxval = CParmInfo::m_RowData[pos].MaxVal;
		ROWINFO&	ri = Info.Row[pos];
		ri.Range = MakeRange(-maxval, maxval);
		ri.Event = ET_CTRL;
		ri.Ctrl = ctrl++;
	}
	for (; i < FIRST_GLOB_ROW; i++) {
		int	rdi = i - PARMS;
		ROWINFO&	ri = Info.Row[i];
		ri.Range = m_MiscPropData[rdi].DefRange;
		ri.Event = ET_CTRL;
		if (rdi != MP_BANK)
			ri.Ctrl = ctrl++;
	}
	for (; i < FIRST_OSC_ROW; i++) {
		int	rdi = i - FIRST_GLOB_ROW;
		ROWINFO&	ri = Info.Row[i];
		int	gpi = CParmInfo::m_GlobParm[rdi];
		double	maxval = CParmInfo::m_RowData[gpi].MaxVal;
		ri.Range = MakeRange(-maxval, maxval);
		ri.Event = ET_CTRL;
		ri.Ctrl = ctrl++;
	}
	for (j = 0; j < PARMS; j++) {
		int	pos = CParmInfo::m_RowOrder[m_RowPos[j]];
		Info.Row[i++].Range = MakeRange(0, COscillator::WAVEFORMS);
		Info.Row[i++].Range = MakeRange(0, CParmInfo::m_RowData[pos].MaxVal);
		Info.Row[i++].Range = MakeRange(0, 1);
		Info.Row[i++].Range = MakeRange(0, 1);
	}
	for (j = 0; j < GLOBAL_PARMS; j++) {
		int	pos = CParmInfo::m_GlobParm[j];
		Info.Row[i++].Range = MakeRange(0, COscillator::WAVEFORMS);
		Info.Row[i++].Range = MakeRange(0, CParmInfo::m_RowData[pos].MaxVal);
		Info.Row[i++].Range = MakeRange(0, 1);
		Info.Row[i++].Range = MakeRange(0, 1);
	}
}

bool CMidiSetupDlg::IsDefault(int Idx) const
{
	ASSERT(Idx >= 0 && Idx < ROWS);
	return(!memcmp(&m_Info.Row[Idx], &m_DefInfo.Row[Idx], sizeof(ROWINFO)));
}

void CMidiSetupDlg::GetInfo(INFO& Info) const
{
	Info = m_Info;
}

void CMidiSetupDlg::SetInfo(const INFO& Info)
{
	m_Info = Info;
	for (int i = 0; i < GetCount(); i++)	// update all row dialogs
		GetRow(i)->SetInfo(Info.Row[i]);
	MakeMidiMap();
}

void CMidiSetupDlg::RestoreDefaults()
{
	INFO	Info;
	GetDefaults(Info);
	SetInfo(Info);
}

inline int CMidiSetupDlg::GetMapping(int Event, int Chan, int Ctrl) const
{
	switch (Event) {
	case ET_CTRL:
		return(GetCtrlMapping(Chan, Ctrl));
	case ET_NOTE:
		return(GetNoteMapping(Chan, Ctrl));
	case ET_PITCH:
		return(GetPitchMapping(Chan));
	default:
		return(-1);
	}
}

void CMidiSetupDlg::Assign(int Idx, int Event, int Chan, int Ctrl)
{
	ASSERT(Idx >= 0 && Idx < ROWS);
	if (Event != ET_OFF) {
		int	PrevMap = GetMapping(Event, Chan, Ctrl);
		if (PrevMap >= 0 && PrevMap != Idx) {	// if already mapped to a different property
			ROWINFO&	pmri = m_Info.Row[PrevMap];	// previous mapping's row info
			pmri.Event = ET_OFF;	// remove previous mapping
			if (PrevMap < GetCount())	// if property has a row dialog, update it
				GetRow(PrevMap)->Assign(pmri.Event, pmri.Chan, pmri.Ctrl);
		}
	}
	ROWINFO&	ri = m_Info.Row[Idx];
	ri.Event = Event;	// create new mapping
	ri.Chan = Chan;
	ri.Ctrl = Ctrl;
	if (Idx < GetCount())	// if property has a row dialog, update it
		GetRow(Idx)->Assign(Event, Chan, Ctrl);
	MakeMidiMap();
}

void CMidiSetupDlg::MakeMidiMap()
{
	ZeroMemory(m_CtrlMap, sizeof(m_CtrlMap));
	ZeroMemory(m_NoteMap, sizeof(m_NoteMap));
	ZeroMemory(m_PitchMap, sizeof(m_PitchMap));
	for (int i = 0; i < ROWS; i++) {
		ROWINFO&	ri = m_Info.Row[i];
		ASSERT(ri.Chan >= 0 && ri.Chan < MIDI_CHANS);
		ASSERT(ri.Ctrl >= 0 && ri.Ctrl < MIDI_PARMS);
		BYTE	targ = static_cast<BYTE>(i + 1);
		switch (ri.Event) {
		case ET_CTRL:
			m_CtrlMap[ri.Chan][ri.Ctrl] = targ;
			break;
		case ET_NOTE:
			m_NoteMap[ri.Chan][ri.Ctrl] = targ;
			break;
		case ET_PITCH:
			m_PitchMap[ri.Chan] = targ;
			break;
		}
	}
}

void CMidiSetupDlg::ZeroControllers()
{
	for (int i = 0; i < GetCount(); i++)	// update all row dialogs
		GetRow(i)->SetValue(0);
	memset(m_Value, CENTER_POS, sizeof(m_Value));	// center all control shadows
}

bool CMidiSetupDlg::AutoAssign()
{
	if (m_AutoDlg.DoModal() != IDOK)
		return(FALSE);
	CMidiAutoAssignDlg::INFO	Info;
	m_AutoDlg.GetInfo(Info);
	SetLearn(FALSE);	// end learn mode
	int	Delta = Info.PropLast - Info.PropFirst;
	if (Delta >= 0)	// forward iteration
		Delta = 1;
	else {	// reverse iteration
		int	tmp = Info.PropFirst;	// swap first and last properties
		Info.PropFirst = Info.PropLast;
		Info.PropLast = tmp;
		if (Info.ParamRange)	// if generating parameter range
			Info.Param -= Delta;	// decrement to start value
		Delta = -1;
	}
	int	ChanDelta = Info.ChanRange ? Delta : 0;
	int	ParamDelta = Info.ParamRange ? Delta : 0;
	for (int i = Info.PropFirst; i <= Info.PropLast; i++) {
		if (Info.SetChanOnly) {	// if setting MIDI channel only
			ROWINFO&	ri = m_Info.Row[GetRowIdx(i)];
			Info.Event = ri.Event;	// preserve row's event type
			Info.Param = ri.Ctrl;	// preserve row's controller/note #
		}
		Assign(GetRowIdx(i), Info.Event, CLAMP(Info.Chan, 0, MIDI_CHANS - 1), 
			CLAMP(Info.Param, 0, MIDI_PARMS - 1));
		Info.Chan += ChanDelta;
		Info.Param += ParamDelta;
	}
	CMainFrame::GetThis()->SendMessage(UWM_MIDIROWEDIT, 0, IDC_MS_EVENT);	// set modify flag
	return(TRUE);
}

void CMidiSetupDlg::FixMidiRange(int RowIdx, CMidiSetupRow::INFO& Info)
{
	// in previous versions, MIDI "Range" wasn't a range at all, it was just a
	// multiplier, so for older projects we try to create a compatible range
	if (RowIdx < FIRST_MISC_ROW) {
		Info.Range.Start = -Info.Range.End;
	} else if (RowIdx < FIRST_GLOB_ROW) {
		int	PropIdx = RowIdx - FIRST_MISC_ROW;
		switch (PropIdx) {
		case MP_SPEED:
		case MP_ZOOM:
			// property wasn't offset
			Info.Range.Start = -Info.Range.End;
			break;
		case MP_DAMPING:
		case MP_TRAIL:
		case MP_RINGS:
		case MP_SPREAD:
		case MP_PATCH_SECS:
		case MP_XFADE_SECS:
		case MP_XFADE_POS:
		case MP_HUE_LOOP_LENGTH:
		case MP_VIDEO_SELECT:
		case MP_VIDEO_BLENDING:
		case MP_VIDEO_CYCLE_LEN:
			// property was offset by Range
			Info.Range.End *= 2;
			Info.Range.Start = 0;
			break;
		case MP_ORIGIN_X:
		case MP_ORIGIN_Y:
		case MP_VIDEO_ORIGIN_X:
		case MP_VIDEO_ORIGIN_Y:
			// property was offset by 0.5
			Info.Range.Start = .5 - Info.Range.End;
			Info.Range.End += .5;
			break;
		case MP_CANVAS_SCALE:
			Info.Range.Start = 2 - Info.Range.End;
			Info.Range.End += 2;
			break;
		case MP_COPIES:
			Info.Range.End = Info.Range.End * 2 + 1;
			Info.Range.Start = 1;
			break;
		default:	// assume it's a binary property
			if (Info.Range.End >= 0) {
				Info.Range.Start = 0;
				Info.Range.End = 1;
			} else {
				Info.Range.Start = 1;
				Info.Range.End = 0;
			}
		}
	} else
		Info.Range.Start = 0;
}

BEGIN_MESSAGE_MAP(CMidiSetupDlg, CRowDialog)
	//{{AFX_MSG_MAP(CMidiSetupDlg)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_MS_RESTORE_DEFAULTS, OnRestoreDefaults)
	ON_BN_CLICKED(IDC_MS_ZERO_CONTROLS, OnZeroControls)
	ON_BN_CLICKED(IDC_MS_AUTO_ASSIGN, OnAutoAssign)
	ON_BN_CLICKED(IDC_MS_ADVANCED, OnAdvanced)
	ON_BN_CLICKED(IDC_MS_LEARN, OnLearn)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_MIDIROWEDIT, OnMidiRowEdit)
	ON_MESSAGE(UWM_MIDIROWSEL, OnMidiRowSel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg message handlers

BOOL CMidiSetupDlg::OnInitDialog() 
{
	CRowDialog::OnInitDialog();
	CreateCols(COLS, m_ColInfo);
	CreateRows(FIRST_OSC_ROW, TOP_MARGIN);
	RestoreDefaults();

	return FALSE;	// CreateRows sets focus to a control
}

LRESULT CMidiSetupDlg::OnMidiRowEdit(WPARAM wParam, LPARAM lParam)
{
	switch (lParam) {
	case IDC_MS_EVENT:
	case IDC_MS_CHAN:
	case IDC_MS_CTRL:
		{
			ROWINFO	Info;
			GetRow(wParam)->GetInfo(Info);
			Assign(wParam, Info.Event, Info.Chan, Info.Ctrl);
		}
		break;
	case IDC_MS_RANGE_START:
	case IDC_MS_RANGE_END:
		GetRow(wParam)->GetRange(m_Info.Row[wParam].Range);
		break;
	}
	CMainFrame::GetThis()->SendMessage(UWM_MIDIROWEDIT, wParam, lParam);	// relay to main
	return(TRUE);
}

LRESULT CMidiSetupDlg::OnMidiRowSel(WPARAM wParam, LPARAM lParam)
{
	SelectRow(wParam);
	return TRUE;
}

void CMidiSetupDlg::OnMidiIn(CMidiIO::MSG msg)
{
	static const int CmdToEvent[] = {
		0, 0, 0, 0, 0, 0, 0, 0,	// unused
		-1,			// note off
		ET_NOTE,	// note on
		-1,			// key aftertouch
		ET_CTRL,	// control change
		-1,			// program change
		-1,			// channel aftertouch
		ET_PITCH,	// pitch bend
		-1,			// system
	};
	int	event = CmdToEvent[(msg.s.cmd >> 4)];
	if (event >= 0) {
		int	chan = msg.s.cmd & 0x0f;
		int	ctrl = (event == ET_PITCH ? 0 : msg.s.p1);
		if (m_Learn) {	// if we're learning MIDI assignments
			if (m_RowSel >= 0) {
				Assign(m_RowSel, event, chan, ctrl);
				CMainFrame::GetThis()->SendMessage(UWM_MIDIROWEDIT, m_RowSel, IDC_MS_CTRL);
			}
		}
		int	idx = GetMapping(event, chan, ctrl);
		if (idx >= 0 && idx < GetCount())	// if property has a row dialog, update it
			GetRow(idx)->SetValue(msg.s.p2 - CENTER_POS);	// convert to signed
	}
}

void CMidiSetupDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CRowDialog::OnShowWindow(bShow, nStatus);
	if (bShow) {
		for (int i = 0; i < GetCount(); i++)	// update all row dialogs
			GetRow(i)->SetValue(m_Value[i] - CENTER_POS);	// update control from shadow
	} else
		SetLearn(FALSE);	// closing dialog ends learn mode
}

void CMidiSetupDlg::OnRestoreDefaults() 
{
	if (AfxMessageBox(IDS_MS_RESTORE_DEFAULT, MB_YESNO) == IDYES) {
		SetLearn(FALSE);	// end learn mode
		RestoreDefaults();
		for (int i = 0; i < ROWS; i++)
			CMainFrame::GetThis()->SendMessage(UWM_MIDIROWEDIT, i, IDC_MS_RANGE_START);
		OnZeroControls();
	}
}

void CMidiSetupDlg::OnZeroControls()
{
	CMainFrame::GetThis()->SendMessage(WM_COMMAND, ID_EDIT_ZERO_CONTROLLERS);	// defer to main
}

void CMidiSetupDlg::OnOK() 
{
	SetLearn(FALSE);	// closing dialog ends learn mode
	CRowDialog::OnOK();
}

void CMidiSetupDlg::OnCancel() 
{
	SetLearn(FALSE);	// closing dialog ends learn mode
	CRowDialog::OnCancel();
}

void CMidiSetupDlg::OnAutoAssign() 
{
	AutoAssign();
}

void CMidiSetupDlg::OnLearn() 
{
	m_Learn ^= 1;
	ShowRowSel(m_Learn);
}

void CMidiSetupDlg::OnAdvanced() 
{
	CWaitCursor	wc;
	ReplaceRows(m_AdvancedChk.GetCheck() ? ROWS : FIRST_OSC_ROW);
	for (int i = 0; i < GetCount(); i++) {	// update all row dialogs
		GetRow(i)->SetInfo(m_Info.Row[i]);
		GetRow(i)->SetValue(m_Value[i] - CENTER_POS);
	}
	m_AdvancedChk.SetFocus();
}
