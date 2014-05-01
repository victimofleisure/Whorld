// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28apr05	initial version
		01		29apr05	add view select, separate from radio
		02		04may05	separate view select from edit select
		03		05may05	set phase from fader pos
		04		05may05	implement non-looped fade
		05		06may05	set filename after save
		06		18may05	add random patch generator
		07		21may05	add parm info object
		08		10jun05	pass frame to ctor
		09		27jul05	experimental change: set image attributes
		10		21aug05	clamp pos and secs positive
		11		27aug05	don't interpolate waveform
		12		28aug05	in Open, save check after dialog
		13		03sep05	in FadeTo, optionally set effects
		14		05sep05	move view and edit select to frame
		15		06sep05	in FadeTo, update parms dialog
		16		02oct05	in Open, refresh parms dialog
		17		02oct05	if editing mix, don't update it too
		18		12oct05	add combo boxes for playlist patches
		19		27oct05	in FadeTo, if in-between pos, copy mix to source
		20		27oct05	if resuming previous auto-fade, don't set phase
		21		29oct05	include document and A/B/Mix in combo boxes
		22		11dec05	in FadeTo, don't reset loop and waveform
		23		20dec05	if fade time is too small, complete fade and stop
		24		23mar06	change INFO booleans from int to bool
		25		17apr06	add CPatch
		26		18apr06	in FadeTo, replace ReadParmsOnly with patch mode
		27		25jan08	in Play, if zero seconds, flush history to avoid glitch
		28		28jan08	support Unicode
		29		30jan08	use main keyboard accelerators

        crossfade between two patches
 
*/

// CrossDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CrossDlg.h"
#include "Persist.h"
#include "MainFrm.h"
#include "WhorldDoc.h"
#include "PathStr.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCrossDlg dialog

IMPLEMENT_DYNAMIC(CCrossDlg, CToolDlg);

CCrossDlg::CCrossDlg(CWnd* pParent) :
	CToolDlg(CCrossDlg::IDD, IDR_MAINFRAME, _T("CrossDlg"), pParent),
	m_Osc(0, DEFAULT_WAVEFORM, SecsToFreq(DEFAULT_SECS))
{
	//{{AFX_DATA_INIT(CCrossDlg)
	//}}AFX_DATA_INIT
	m_Frm = NULL;
	for (int i = 0; i < SELS; i++) {
		m_Info[i].SetDefaults();
		m_Modified[i] = FALSE;
	}
	m_Play = FALSE;
	m_Loop = FALSE;
	m_MinSecs = 0;
	m_PrevPos = -1;
	m_PrevDir = 0;
	m_OneShot = FALSE;
}

BOOL CCrossDlg::Create(UINT nIDTemplate, CMainFrame *pFrame)
{
	m_Frm = pFrame;
	return CToolDlg::Create(nIDTemplate, pFrame);
}

void CCrossDlg::GetDefaults(INFO& Info) const
{
	Info.Pos = .5;
	Info.Seconds = DEFAULT_SECS;
	Info.Loop = FALSE;
	Info.Waveform = DEFAULT_WAVEFORM;
}

void CCrossDlg::GetInfo(INFO& Info) const
{
	Info.Pos = m_PosEdit.GetVal();
	Info.Seconds = m_SecsEdit.GetVal();
	Info.Loop = m_Loop;
	Info.Waveform = m_Waveform.GetCurSel();
}

void CCrossDlg::SetInfo(const INFO& Info)
{
	SetPos(Info.Pos);
	SetSeconds(Info.Seconds);
	Loop(Info.Loop);
	SetWaveform(Info.Waveform);
}

void CCrossDlg::RestoreDefaults()
{
	INFO	Info;
	GetDefaults(Info);
	SetInfo(Info);
	Play(FALSE);
}

void CCrossDlg::TimerHook()
{
	if (m_Play) {
		m_Osc.TimerHook();
		double pos = (m_Osc.GetVal() + 1) / 2;
		if (!m_Loop) {
			if (m_OneShot) {
				m_OneShot = FALSE;
				m_PrevPos = pos;
				m_PrevDir = -1;	// direction can't be determined yet
			} else {
				int	Dir = pos > m_PrevPos;
				if (m_PrevDir >= 0 && Dir != m_PrevDir) {
					Play(FALSE);
					pos = pos > .5 ? 1 : 0;
					m_PrevPos = -1;	// force next play to start new fade
				} else
					m_PrevPos = pos;
				m_PrevDir = Dir;
			}
		}
		UpdateMix(pos);
		m_PosEdit.SetVal(pos);
		m_PosSlider.SetPos(round(pos * SLIDER_RANGE));
	}
}

void CCrossDlg::SetViewSel(int Sel)
{
	m_MixView.SetCheck(Sel == SEL_MIX);
}

void CCrossDlg::SetEditSel(int Sel)
{
	m_EditA.SetCheck(Sel == SEL_A);
	m_EditB.SetCheck(Sel == SEL_B);
	m_EditMix.SetCheck(Sel == SEL_MIX);
}

void CCrossDlg::SetInfo(int Sel, const CParmInfo& Info)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	m_Info[Sel] = Info;
	UpdateMix(m_PosEdit.GetVal());
}

void CCrossDlg::Play(bool Enable)
{
	if (Enable && !m_Play) {	// if transition from stop to play
		double	secs = m_SecsEdit.GetVal();
		if (!secs)
			m_Frm->GetView()->FlushHistory();	// prevent glitch
		if (secs <= m_MinSecs) {	// prevent stuck thumb
			Enable = FALSE;
			SetPos(GetPos() < .5 ? 1 : 0);
		} else {
			if (GetPos() != m_PrevPos)	// if fader isn't where we left it
				m_Osc.SetPhaseFromVal(m_PosEdit.GetVal() * 2 - 1);	// start new fade
			m_OneShot = TRUE;
		}
	}
	m_PlayBtn.SetIcon(AfxGetApp()->LoadIcon(Enable ? IDI_PLAYD : IDI_PLAYU));
	m_PlayBtn.SetCheck(Enable != 0);
	m_StopBtn.SetIcon(AfxGetApp()->LoadIcon(Enable ? IDI_STOPU : IDI_STOPD));
	m_StopBtn.SetCheck(Enable == 0);
	m_Play = Enable;
	m_Frm->GetPlaylist().SetCurPatchState(Enable);
}

void CCrossDlg::Loop(bool Enable)
{
	m_LoopBtn.SetIcon(AfxGetApp()->LoadIcon(Enable ? IDI_LOOPD : IDI_LOOPU));
	m_LoopBtn.SetCheck(Enable != 0);
	m_Loop = Enable;
	if (!Enable && m_Play) {	// if unlooped and fade in progress
		if (m_SecsEdit.GetVal() <= m_MinSecs)	// if fade time is too small
			Play(FALSE);	// stop
		else
			m_OneShot = TRUE;	// tell timer hook to calculate direction
	}
}

void CCrossDlg::SetPos(double Pos)
{
	Pos = CLAMP(Pos, 0, 1);
	m_PosSlider.SetPos(round(Pos * SLIDER_RANGE));
	m_PosEdit.SetVal(Pos);
	UpdateMix(Pos);
}

void CCrossDlg::SetSeconds(double Seconds)
{
	m_SecsEdit.SetVal(Seconds);
	m_Osc.SetFreq(SecsToFreq(Seconds));
	// if unlooped fade in progress, and fade time is too small
	if (m_Play && !m_Loop && m_SecsEdit.GetVal() <= m_MinSecs) {
		SetPos(max(m_PrevDir, 0));	// complete fade immediately and stop
		Play(FALSE);
		m_Frm->GetView()->FlushHistory();	// prevent glitch
	}
}

void CCrossDlg::SetWaveform(int Waveform)
{
	m_Waveform.SetCurSel(Waveform);
	m_Osc.SetWaveform(Waveform);
}

void CCrossDlg::SetTimerFreq(double Freq)
{
	m_Osc.SetTimerFreq(Freq);
	m_MinSecs = 2 / Freq;	// reciprocal of timer's nyquist freq
}

void CCrossDlg::OnParamUpdate(int Sel)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	if (Sel != SEL_MIX) {	// if editing mix, don't update it: would clobber change
		UpdateMix(m_PosEdit.GetVal());	// update mix to reflect changed input
		SetModifiedFlag(Sel, TRUE);
	}
}

#define INTERPOLATE(x) \
	Interpolate(m_Info[SEL_A].m_Row[i].x, m_Info[SEL_B].m_Row[i].x, Pos)

void CCrossDlg::UpdateMix(double Pos)
{
	for (int i = 0; i < ROWS; i++) {
		m_Info[SEL_MIX].m_Row[i].Val	= INTERPOLATE(Val);
		m_Info[SEL_MIX].m_Row[i].Wave =	// don't interpolate waveform, threshold it
			Pos < .5 ? m_Info[SEL_A].m_Row[i].Wave : m_Info[SEL_B].m_Row[i].Wave;
		m_Info[SEL_MIX].m_Row[i].Amp	= INTERPOLATE(Amp);
		m_Info[SEL_MIX].m_Row[i].Freq	= INTERPOLATE(Freq);
		m_Info[SEL_MIX].m_Row[i].PW		= INTERPOLATE(PW);
	}
	m_Frm->OnNewParms(SEL_MIX);	// update parameters dialog
}

#define RANDOMIZE(x) \
	Randomize(m_Info[SEL_A].m_Row[i].x, m_Info[SEL_B].m_Row[i].x)

void CCrossDlg::RandomPatch()
{
	Play(FALSE);	// prevent auto-fade from clobbering result
	for (int i = 0; i < ROWS; i++) {
		m_Info[SEL_MIX].m_Row[i].Val	= RANDOMIZE(Val);
		m_Info[SEL_MIX].m_Row[i].Wave	= round(RANDOMIZE(Wave));
		m_Info[SEL_MIX].m_Row[i].Amp	= RANDOMIZE(Amp);
		m_Info[SEL_MIX].m_Row[i].Freq	= RANDOMIZE(Freq);
		m_Info[SEL_MIX].m_Row[i].PW		= RANDOMIZE(PW);
	}
	m_Frm->OnNewParms(SEL_MIX);	// update parameters dialog
	m_PosEdit.SetVal(.5);	// center fader, so FadeTo fades from random mix
	m_PosSlider.SetPos(round(.5 * SLIDER_RANGE));
}

void CCrossDlg::GetName(int Sel, CString& Name) const
{
	ASSERT(Sel >= 0 && Sel < SELS);
	switch (Sel) {
	case SEL_DOC:
		Name = m_Frm->GetDoc()->GetTitle();
		break;
	case SEL_A:
		m_NameA.GetWindowText(Name);
		break;
	case SEL_B:
		m_NameB.GetWindowText(Name);
		break;
	case SEL_MIX:
		Name = m_MixName;
		break;
	default:
		Name.Empty();
	}
}

void CCrossDlg::SetName(int Sel, LPCTSTR Name)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	switch (Sel) {
	case SEL_A:
		m_NameA.SetWindowText(Name);
		break;
	case SEL_B:
		m_NameB.SetWindowText(Name);
		break;
	}
}

bool CCrossDlg::Open(int Sel, LPCTSTR Path)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	if (!SaveCheck(Sel))
		return(FALSE);
	CStdioFile	fp;
	CFileException	e;
	if (!fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	CPatch	Patch;
	if (!Patch.Read(fp))
		return(FALSE);
	m_Info[Sel] = Patch;
	UpdateMix(m_PosEdit.GetVal());
	CPathStr	FileName(PathFindFileName(Path));
	FileName.RemoveExtension();
	SetName(Sel, FileName);
	if (Sel == m_Frm->GetEditSel())	// if input is selected for editing
		m_Frm->SetEditSel(Sel);	// refresh parameters dialog
	return(TRUE);
}

bool CCrossDlg::Open(int Sel)
{
	CFileDialog	fd(TRUE, EXT_PATCH, NULL, OFN_HIDEREADONLY, LDS(IDS_FILTER_PATCH));
	if (fd.DoModal() != IDOK)
		return(FALSE);
	return(Open(Sel, fd.GetPathName()));
}

bool CCrossDlg::Save(int Sel)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	CPathStr	FileName;
	GetName(Sel, FileName);
	CFileDialog	fd(FALSE, EXT_PATCH, FileName, OFN_OVERWRITEPROMPT, LDS(IDS_FILTER_PATCH));
	if (fd.DoModal() != IDOK)
		return(FALSE);
	CStdioFile	fp;
	CFileException	e;
	if (!fp.Open(fd.GetPathName(), CFile::modeCreate | CFile::modeWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	CPatch	Patch;
	Patch = m_Info[Sel];	// assign parm info
	m_Frm->GetPatch(Patch);	// get master and main info
	if (!Patch.Write(fp))
		return(FALSE);
	m_Modified[Sel] = FALSE;
	FileName = fd.GetFileName();
	FileName.RemoveExtension();
	SetName(Sel, FileName);
	return(TRUE);
}

bool CCrossDlg::SaveCheck(int Sel)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	if (!m_Modified[Sel])
		return(TRUE);
	CString	s, fname;
	GetName(Sel, fname);
	AfxFormatString2(s, IDS_CF_SAVE_PROMPT, m_Frm->GetSelName(Sel), fname);
	int	retc = AfxMessageBox(s, MB_YESNOCANCEL);
	if (retc == IDCANCEL)
		return(FALSE);
	if (retc == IDNO) {
		m_Modified[Sel] = FALSE;
		return(TRUE);
	}
	return(Save(Sel));
}

bool CCrossDlg::SaveCheck()
{
	return(SaveCheck(SEL_A) && SaveCheck(SEL_B));
}

bool CCrossDlg::FadeTo(const CPatch& Patch, LPCTSTR Name, double Secs, bool RandPhase)
{
	if (!m_Frm->IsVeejay() && !SaveCheck())
		return(FALSE);
	m_Play = FALSE;					// halt fader first, without updating UI
	double	Pos = GetPos();
	bool	Fwd = Pos <= .5;		// set fade direction
	int		Src = SEL_A + !Fwd;		// index of source
	int		Dst = SEL_A + Fwd;		// index of destination
	if (Pos > 0 && Pos < 1) {		// if fader is between A and B
		SetInfo(Src, GetInfo(SEL_MIX));	// copy mix to source
		SetName(Src, m_MixName);		// set source name to mix
		m_Frm->OnNewParms(Src);		// update parameters dialog
	}
	SetPos(!Fwd);		// move fader to source
	SetInfo(Dst, Patch);	// copy arg to destination
	SetName(Dst, Name);	// set destination file name
	SetSeconds(Secs);
	if (Secs)
		Play(TRUE);	// start fading to new patch
	else {
		Play(FALSE);
		SetPos(Fwd);	// jump to new patch
		m_Frm->GetView()->FlushHistory();	// prevent glitch
	}
	if (RandPhase) {
		TimerHook();	// make mod freqs non-zero so randomize phase works
		for (int i = 0; i < ROWS; i++) {
			// if modulator is unused in source, but active in destination
			if (!m_Info[Src].m_Row[i].Freq && m_Info[Dst].m_Row[i].Freq)
				m_Frm->GetView()->SetPhase(i, double(rand()) / RAND_MAX);	// randomize phase
		}
	}
	if (m_Frm->GetPatchMode() == CMainFrame::PM_FULL)
		m_Frm->SetPatch(Patch);	// restore master and main settings
	m_Frm->OnNewParms(Dst);	// update parameters dialog
	return(TRUE);
}

void CCrossDlg::OnEditButton(int Sel, BOOL Check)
{
	m_Frm->SetEditSel(Check ? Sel : CMainFrame::SEL_DOC);
	if (Check)
		m_Frm->GetParms().ShowWindow(SW_SHOW);
}

void CCrossDlg::PopulateCombo(int Sel)
{
	CComboBox& Combo = (Sel == SEL_A ? m_NameA : m_NameB);
	CString	s;
	Combo.GetWindowText(s);
	Combo.ResetContent();
	int	i;
	for (i = 0; i < SELS; i++) {
		if (i != Sel) {	// exclude self-assignment
			int	pos = Combo.AddString(m_Frm->GetSelName(i));
			Combo.SetItemData(pos, i);	// store selection index in item
		}
	}
	int	Patches = m_Frm->GetPlaylist().GetCount();
	for (i = 0; i < Patches; i++) {
		CPathStr	Name;
		Name = PathFindFileName(m_Frm->GetPlaylist().GetPath(i));
		Name.RemoveExtension();
		Combo.AddString(Name);
	}
	Combo.SetWindowText(s);
}

void CCrossDlg::SelectCombo(int Sel)
{
	CComboBox& Combo = (Sel == SEL_A ? m_NameA : m_NameB);
	Combo.GetWindowText(m_ComboText);	// in case selection is canceled
	int	Item = Combo.GetCurSel();
	if (Item == LB_ERR)
		return;
	if (Item < SELS - 1) {
		if (SaveCheck(Sel)) {
			int	Src = Combo.GetItemData(Item);	// retrieve selection index
			SetInfo(Sel, Src == SEL_DOC ? m_Frm->GetDoc()->m_Patch : GetInfo(Src));
			GetName(Src, m_ComboText);	// combo gets new text via post message
			m_Frm->OnNewParms(Sel);	// update parameters dialog
		}
	} else {
		if (Open(Sel, m_Frm->GetPlaylist().GetPath(Item - (SELS - 1))))
			GetName(Sel, m_ComboText);	// combo gets new text via post message
	}
	// posting WM_SETTEXT to combo doesn't work, combo still overwrites our text
	PostMessage(UWM_SETTEXT, WPARAM(Combo.m_hWnd), LPARAM(LPCTSTR(m_ComboText)));
}

void CCrossDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCrossDlg)
	DDX_Control(pDX, IDC_XF_NAME_B, m_NameB);
	DDX_Control(pDX, IDC_XF_NAME_A, m_NameA);
	DDX_Control(pDX, IDC_XF_SECS_SPIN, m_SecsSpin);
	DDX_Control(pDX, IDC_XF_POS_SPIN, m_PosSpin);
	DDX_Control(pDX, IDC_XF_EDIT_MIX, m_EditMix);
	DDX_Control(pDX, IDC_XF_MIX_VIEW, m_MixView);
	DDX_Control(pDX, IDC_XF_EDIT_B, m_EditB);
	DDX_Control(pDX, IDC_XF_EDIT_A, m_EditA);
	DDX_Control(pDX, IDC_XF_WAVEFORM, m_Waveform);
	DDX_Control(pDX, IDC_XF_PLAY, m_PlayBtn);
	DDX_Control(pDX, IDC_XF_STOP, m_StopBtn);
	DDX_Control(pDX, IDC_XF_LOOP, m_LoopBtn);
	DDX_Control(pDX, IDC_XF_POS_SLIDER, m_PosSlider);
	DDX_Control(pDX, IDC_XF_POS_EDIT, m_PosEdit);
	DDX_Control(pDX, IDC_XF_SECS_EDIT, m_SecsEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCrossDlg, CToolDlg)
	//{{AFX_MSG_MAP(CCrossDlg)
	ON_NOTIFY(NEN_CHANGED, IDC_XF_SECS_EDIT, OnChangedSecsEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_XF_POS_EDIT, OnChangedPosEdit)
	ON_BN_CLICKED(IDC_XF_OPEN_A, OnOpenA)
	ON_BN_CLICKED(IDC_XF_SAVE_A, OnSaveA)
	ON_BN_CLICKED(IDC_XF_OPEN_B, OnOpenB)
	ON_BN_CLICKED(IDC_XF_SAVE_B, OnSaveB)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_XF_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_XF_STOP, OnStop)
	ON_BN_CLICKED(IDC_XF_LOOP, OnLoop)
	ON_CBN_SELCHANGE(IDC_XF_WAVEFORM, OnSelchangeWaveform)
	ON_BN_CLICKED(IDC_XF_MIX_VIEW, OnMixView)
	ON_BN_CLICKED(IDC_XF_EDIT_A, OnEditA)
	ON_BN_CLICKED(IDC_XF_EDIT_B, OnEditB)
	ON_BN_CLICKED(IDC_XF_MIX_SAVE, OnMixSave)
	ON_BN_CLICKED(IDC_XF_EDIT_MIX, OnEditMix)
	ON_CBN_SELCHANGE(IDC_XF_NAME_A, OnSelchangeNameA)
	ON_CBN_SELCHANGE(IDC_XF_NAME_B, OnSelchangeNameB)
	ON_CBN_SETFOCUS(IDC_XF_NAME_A, OnSetfocusNameA)
	ON_CBN_SETFOCUS(IDC_XF_NAME_B, OnSetfocusNameB)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCrossDlg message handlers

BOOL CCrossDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();

	m_MixName.LoadString(IDS_CF_MIX_NAME);
	m_PosEdit.SetPrecision(3);
	m_PosEdit.SetRange(0, 1);
	m_PosSpin.SetDelta(.01);
	m_PosSlider.SetRange(0, SLIDER_RANGE);
	m_PosSlider.SetPos(SLIDER_RANGE / 2);
	m_PosSlider.SetDefaultPos(SLIDER_RANGE / 2);
	m_PosSlider.SetTicFreq(SLIDER_RANGE / 10);
	m_SecsEdit.SetRange(0, INT_MAX);
	CString	s;
	for (int i = 0; i < WAVEFORMS; i++) {
		s.LoadString(CParmRow::GetWaveID(i));
		m_Waveform.AddString(s);
	}
	RestoreDefaults();
	m_PosSlider.SetFocus();
	((CEdit *)m_NameA.GetWindow(GW_CHILD))->SetReadOnly(TRUE);
	((CEdit *)m_NameB.GetWindow(GW_CHILD))->SetReadOnly(TRUE);

	return FALSE; // set the focus to a control
}

void CCrossDlg::OnChangedPosEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetPos(m_PosEdit.GetVal());
}

void CCrossDlg::OnChangedSecsEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetSeconds(m_SecsEdit.GetVal());
}

void CCrossDlg::OnOpenA() 
{
	Open(SEL_A);
}

void CCrossDlg::OnSaveA() 
{
	Save(SEL_A);
}

void CCrossDlg::OnOpenB() 
{
	Open(SEL_B);
}

void CCrossDlg::OnSaveB() 
{
	Save(SEL_B);
}

void CCrossDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetPos(double(m_PosSlider.GetPos()) / SLIDER_RANGE);
	CToolDlg::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCrossDlg::OnPlay() 
{
	Play(TRUE);
}

void CCrossDlg::OnStop() 
{
	Play(FALSE);
}

void CCrossDlg::OnLoop() 
{
	Loop(!m_Loop);
}

void CCrossDlg::OnSelchangeWaveform() 
{
	m_Osc.SetWaveform(m_Waveform.GetCurSel());
}

void CCrossDlg::OnEditA()
{
	OnEditButton(SEL_A, m_EditA.GetCheck());
}

void CCrossDlg::OnEditB() 
{
	OnEditButton(SEL_B, m_EditB.GetCheck());
}

void CCrossDlg::OnEditMix() 
{
	OnEditButton(SEL_MIX, m_EditMix.GetCheck());
}

void CCrossDlg::OnMixView()
{
	m_Frm->SetViewSel(m_MixView.GetCheck() ? SEL_MIX : CMainFrame::SEL_DOC);
}

void CCrossDlg::OnMixSave() 
{
	Save(SEL_MIX);
}

void CCrossDlg::OnSetfocusNameA() 
{
	PopulateCombo(SEL_A);
}

void CCrossDlg::OnSetfocusNameB() 
{
	PopulateCombo(SEL_B);
}

void CCrossDlg::OnSelchangeNameA() 
{
	SelectCombo(SEL_A);
}

void CCrossDlg::OnSelchangeNameB() 
{
	SelectCombo(SEL_B);
}

LRESULT	CCrossDlg::OnSetText(WPARAM wParam, LPARAM lParam)
{
	::SetWindowText((HWND)wParam, (LPCTSTR)lParam);
	return Default();
}
