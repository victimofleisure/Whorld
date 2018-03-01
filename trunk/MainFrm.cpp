// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		27apr05	add snapshots
		02		28apr05	add crossfader
		03		29apr05	update the view in OnTimer
		04		04may05	move row oscillators here
		05		07may05	add export bitmap
		06		09may05	paint after pause was causing spurious ring step
		07		09may05	crossfader edit buttons now show params dialog
		08		16may05	OnClose now disables full screen mode
		09		16may05	add OnNewDocument to update title
		10		16may05	allow parms dlg and view parms order to differ
		11		16may05	add drag and drop for snapshot files
		12		16may05	seed random number generator
		13		17may05	add pulse width
		14		20may05	add ring fill and outline
		15		21may05	add parm info object
		16		22may05	add view/edit info pointers
		17		25may05	add split
		18		26may05	add context menu
		19		28may05	add save argument to new doc message
		20		02jun05	add playlist dialog and demo mode
        21		05jun05	add mirror
		22		09jun05	add rotate hue
		23		10jun05	create dialogs in OnCreate
		24		20jun05	add x-ray draw mode
		25		22jun05	add options dialog
		26		23jun05	add master speed
		27		25jun05	disable timer while iconic
        28      28jun05	add read parms only and export size
		29		01jul05	add snapshot preview
		30		02jul05	add hue wheel
		31		04jul05	add hue loop
		32		04jul05	add snapshot backup
		33		06jul05	add origin damping
		34		07jul05	add snapshot array
		35		08jul05	add zoom
		36		09jul05	add snapshot menu
		37		10jul05	add stretch and resolution to export
		38		11jul05	don't repaint screen during print job
		39		13jul05	add auto origin
		40		14jul05	add tap tempo
		41		15jul05	remove split
		42		16jul05	add tempo timer
		43		18jul05	add averaging for tap tempo
		44		19jul05	add DirectInput
		45		20jul05	add detached view
		46		21jul05	deactivating app must cancel input modes
		47		23jul05	use master dialog's edit sliders
		48		27jul05	add get/set info
		49		01aug05	add VJ mode
		50		03aug05	simplify mouse modes
		51		04aug05	use wheel to control damping
		52		04aug05	add reverse mode
		53		05aug05	limit tempo doublings
		54		06aug05	don't force view mix in OpenPlaylist
		55		06aug05	remove hue wheel, move hue loop to right button
		56		10aug05	add zoom damping
		57		12aug05	add invert fill, invert outline, invert xray
		58		13aug05	add MIDI
		59		15aug05	override GetMenu so it works in full screen
		60		16aug05	pass MIDI input to MIDI setup dialog
		61		17aug05	add VJ accelerators
		62		18aug05	don't update bars in full screen
		63		21aug05	add playlist popup to file menu
		64		27aug05	move oscillators into view
		65		29aug05	add HTML help
		66		30aug05	add movie support
		67		02sep05	move AVI export to its own process
		68		04sep05	add MIDI control of miscellaneous properties
		69		05sep05	move view and edit select here
		70		06sep05	replace DirectInput keyboard callback with hook
		71		09sep05	add playlist's most recent file list
		72		12sep05	use app path to find demo
		73		13sep05	use mouse buttons as input modifiers
		74		15sep05	record to snapshot folder in VJ mode
		75		17sep05	add MIDI sync
		76		27sep05	add movie resize
		77		02oct05	get selection names from edit menu resource
		78		02oct05	add make patch
		79		05oct05	add support for notes
		80		08oct05	allow random patch in VJ mode
		81      14oct05	add exclusive mode
		82		17oct05	update master info on exiting VJ mode
		83		17oct05	allow damping for zoom via MIDI
		84		18oct05	EnableTimer and SetReverse needn't step rings now
		85		19oct05	don't flush history on master speed change
		86		20oct05	in OnTimer, record frame before view's timer hook
		87		25oct05	panic shouldn't exit full screen when view is detached
		88		26oct05	move exclusive-mode handling to SetExclusive
		89		02nov05	in KeybdProc, don't block F10, we're using it now
		90		02nov05	in SetViewSel, flush history to prevent glitches
		91		02nov05	in OnClose, don't save check if we're in print preview
		92		03nov05	in SetExclusive, re-hide thin frame if needed
		93		09nov05 in OnFileExportBitmap, add default file name
		94		11nov05 add max rings
		95		21dec05	in OnMidiCtrlChange, add HueLoopLength
		96		22dec05	add repaint flag to Mirror
		97		17feb06	add oscillator override
		98		20feb06	add master parameters to info struct
		99		03mar06	add convex
        100     04mar06	add trail
		101		08mar06	in OnMidiCtrlChange, clamp oscillator waveform
		102		10mar06	in OnMidiCtrlChange, add trail
		103		15mar06	in OnMouseMove, use SetGlobalOrigin
		104		23mar06	add Hue, LoopHue, InvertColor, Pause to INFO
		105		24mar06	when exiting Veejay, only park origin if dragging
		106		28mar06	add ZoomCenter
		107		08apr06	panic now resets trail
		108		12apr06	In DetachView, must remove aux view from document
		109		17apr06	add CPatch
		110		18apr06	add patch modes
		111		24apr06	in OnMidiCtrlChange, add CanvasScale and PatchMode
		112		04may06	in panic, center origin, disable reverse, default speed
		113		06may06	add video raster operations
		114		09may06	add video cycling
		115		12may06	add atomic bank/patch select
		116		16may06	make video cycling state public
		117		19may06	panic now disables video cycling
		118		22may06	add video list info
		119		02jun06	add AVI to OpenOtherDoc
		120		05jun06	add video auto-rewind
		121		07jun06	add video path recording
		122		09jun06	remove duplicate message map entries
		123		17jun06	stop recording in OnClose
		124		21jun06	rename some MasterDlg accessors
		125		24jun06	add Copies and Spread
		126		29jun06	add video origin
		127		10jul06	add video random origin
		128		16jul06	add video origin MIDI support
		129		24jul06	in OnShellOpen, add start in VJ mode
		130		15sep06	in TimerHook, record frame after view timer hook
		131		10dec07	add global parameters
		132		15dec07	add MIDI drag origin mode
		133		21dec07	hide caption bar in dual-monitor exclusive mode
		134		21dec07	replace AfxGetMainWnd with m_This
		135		22jan08	in SetEditSel, set offset mode for globals page
		136		22jan08	use FastIsVisible for tool dialogs
		137		23jan08	replace MIDI range scaler with start/end
		138		25jan08	in OnWndPanic, clear screen
		139		25jan08	in MidiZeroCtrls, reset global parameters
		140		28jan08	support Unicode
		141		29jan08	make DragQueryFile arg unsigned to fix warning
		142		30jan08	add extra keyboard accelerators when we have focus
		143		30jan08	show actual frame rate in status bar
		144		31jan08	if print preview, don't set capture in OnParentNotify 
		145		08feb08	in MiscPropChanged, bang on make only; ignore break
		146		05apr08	in SetInfo, reset MIDI learn mode

        Whorld main frame window

*/

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include <math.h>	// for fabs
#include "Resource.h"
#include "MainFrm.h"
#include "WhorldDoc.h"
#include "WhorldViewDD.h"
#include "Persist.h"
#include "PathStr.h"
#include "FolderDialog.h"
#include "MultiFileDlg.h"
#include "Benchmark.h"
#include "AuxFrame.h"
#include "htmlhelp.h"	// needed for HTML Help API
#include "ProgressDlg.h"
#include "MoviePlayerDlg.h"
#include "MovieResizeDlg.h"
#include "Playlist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

#define REG_MAIN_FRAME		_T("MainFrame")
#define REG_DEMO_PATH		_T("DemoPath")

// don't show this again keys; see MessageBoxCheck
#define REG_VIEW_DEMO		_T("WhorldViewDemo")
#define REG_VEEJAY_MODE		_T("WhorldVeejayMode")

#define	PREFIX_SNAPSHOT		_T("Snapshot")
#define	PREFIX_MOVIE		_T("Movie")
#define	PREFIX_PATCH		_T("Patch")

// macros for passing a float to a message handler by value
#define FLOAT_TO_INT(x) (*((int *)(&x)))
#define INT_TO_FLOAT(x) (*((float *)(&x)))

const double CMainFrame::DAMP_TOLER = 1e-3;

const double CMainFrame::TAP_AVG_TOLERANCE		= .10;

// these nominals are scaled by master settings; bigger means more sensitive
const double CMainFrame::SPEED_MOUSE_SENS		= .001;
const double CMainFrame::ZOOM_MOUSE_SENS		= .001;
const double CMainFrame::HUE_MOUSE_SENS			= .001;
const double CMainFrame::RINGS_MOUSE_SENS		= .001;
const double CMainFrame::VIDEO_ORG_MOUSE_SENS	= .001;
const double CMainFrame::DAMPING_WHEEL_SENS		= .025;
const double CMainFrame::XFADE_TIME_WHEEL_SENS	= .025;
const double CMainFrame::TRAIL_WHEEL_SENS		= .025;

const LPCTSTR CMainFrame::m_DemoPlaylist = _T("Patches\\demo.whl");

const UINT CMainFrame::m_Indicators[] = {
	ID_SEPARATOR,		// status line indicator
	ID_INDICATOR_TEMPO,
	ID_INDICATOR_ORG_MOTION,
	ID_INDICATOR_HUE_LOOP,
	ID_INDICATOR_MIX,
	ID_INDICATOR_AUTOPLAY,
	ID_INDICATOR_RECORD,
	ID_INDICATOR_PAUSE,
	ID_INDICATOR_PATCH_MODE,
	ID_INDICATOR_FRAME_RATE,
};

const UINT CMainFrame::m_OrgMotionID[] = {
	0,	// no motion
	IDS_ORG_MOTION_DRAG,
	IDS_ORG_MOTION_RANDOM,
	0	// MIDI drag mode
};

const UINT CMainFrame::m_SelNameID[SELS] = {	// order must match selection enum
	ID_EDIT_DOCUMENT,
	ID_EDIT_XFADER_A,
	ID_EDIT_XFADER_B,
	ID_EDIT_XFADER_MIX,
	ID_EDIT_MASTER_OFFSETS,
	ID_EDIT_GLOBAL_PARMS
};

// in VJ mode, disable disruptive commands (e.g. ones that display a dialog)
const UINT CMainFrame::m_VJCmdDisable[] = {
	ID_FILE_NEW,
	ID_FILE_OPEN,
	ID_FILE_SAVE,
	ID_FILE_SAVE_AS,
	ID_FILE_PRINT,
	ID_FILE_PRINT_PREVIEW,
	ID_FILE_PRINT_SETUP,
	ID_FILE_EXPORT_BITMAP,
	ID_FILE_LOAD_SNAPSHOT,
	ID_FILE_MOVIE_PLAY,
	ID_FILE_MOVIE_EXPORT,
	ID_VIEW_PARMS,
	ID_VIEW_NUMBERS,
	ID_VIEW_CROSSFADER,
	ID_VIEW_PLAYLIST,
	ID_VIEW_MASTER,
	ID_VIEW_OPTIONS,
	ID_VIEW_MIDI_SETUP,
	ID_WND_FULL_SCREEN,
	ID_WND_DETACH,
	0	// list terminator: don't delete
};

CMainFrame	*CMainFrame::m_This;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
	m_OptsDlg(IDS_OPTIONS),
	m_TapAvg(TAP_AVG_HISTORY_SIZE)
{
	srand(UINT(time(NULL)));	// seed random number generator
	m_Snapshot.SetFrame(this);
	m_View = NULL;
	m_ViewInfo = NULL;
	m_EditInfo = NULL;
	m_ViewSel = SEL_DOC;
	m_EditSel = SEL_DOC;
	m_WasShown = FALSE;
	m_IsFullScreen = FALSE;
	m_IsPaused = FALSE;
	m_HideCursor = FALSE;
	m_ScreenRect.SetRectEmpty();
	m_ShowToolbar = FALSE;
	m_ShowStatusBar = FALSE;
	m_Menu.LoadMenu(MAKEINTRESOURCE(IDR_MAINFRAME));
	m_Timer = 0;
	m_OrgMotion = OM_PARK;
	m_MouseHook = NULL;
	m_KeybdHook = NULL;
	m_PreIconicTimer = -1;
	m_PrePrintTimer = FALSE;
	m_DemoMode = FALSE;
	m_IsVeejay = FALSE;
	m_IsCursorValid = FALSE;
	m_PrevTapTime = 0;
	m_Clock = 0;
	m_PrevClock = 0;
	m_InputMode = 0;
	m_MousePos = DPoint(0, 0);
	m_TargetOrg = DPoint(0, 0);
	m_AuxFrm = NULL;
	m_AuxView = NULL;
	m_GrowDir = 1;
	m_TempoMult = 0;
	m_Zoom = 0;
	m_TargetZoom = 0;
	m_MidiIO.SetInputCallback(MidiCallback, this);
	m_MidiDev = CMidiIO::NO_DEVICE;
	ZeroMemory(m_MastOfs, sizeof(m_MastOfs));
	ZeroMemory(&m_MastInfo, sizeof(CParmInfo));
	m_HelpCookie = 0;
	m_FirstNewDoc = TRUE;
	m_HideChrome = FALSE;
	m_IsExclusive = FALSE;
	m_InHouse = CPersist::GetInt(_T("Settings"), _T("InHouse"), FALSE) != 0;
	m_PrevHue = 0;
	m_MidiClock = 0;
	m_MidiQuant = QUANT_DEFAULT;
	ZeroMemory(&m_NoteToggle, sizeof(m_NoteToggle));
	memset(m_MPState, -1, sizeof(m_MPState));
	m_KbdLedMask = 0;
	m_PatchMode = PM_FULL;
	m_CycleVideo = FALSE;
	m_VideoOrgMoving = FALSE;
	m_VideoRandOrg = FALSE;
	m_TargetVideoOrg = DPoint(1, 1);
	ZeroMemory(&m_GlobParm, sizeof(m_GlobParm));
	ZeroMemory(&m_IsGlobParm, sizeof(m_IsGlobParm));
	for (int i = 0; i < GLOBAL_PARMS; i++) { 
		int	gpi = CParmInfo::m_GlobParm[i];
		m_IsGlobParm[gpi] = TRUE;
	}
	ZeroMemory(&m_GlobParmTarg, sizeof(m_GlobParmTarg));
	m_FocusAccel = LoadAccelerators(AfxGetResourceHandle(), 
		MAKEINTRESOURCE(IDR_MAIN_FOCUS));
	m_FrameRateTimer = 0;
	m_FrameCount = 0;
	m_PrevFrameCount = 0;
	m_ActualFrameRate = 0;
	m_This = this;
}

CMainFrame::~CMainFrame()
{
	HookMouse(FALSE);
	m_This = NULL;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	// CS_HREDRAW and CS_VREDRAW are OK: view doesn't support partial painting
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame attributes

CRuntimeClass *CMainFrame::GetViewClass()
{
	// determine view type (GDI or DirectDraw) from registry entry
	int	UseDirDraw = CPersist::GetInt(REG_SETTINGS, REG_USE_DIR_DRAW, TRUE);
	return(UseDirDraw ? RUNTIME_CLASS(CWhorldViewDD) : RUNTIME_CLASS(CWhorldView));
}

void CMainFrame::SetMasterInfo(const MASTER_INFO& Master)
{
	m_MasterDlg.SetInfo(Master);
	for (int i = 0; i < CMasterDlg::SLIDERS; i++)
		UpdateMaster(i);	// apply changes
}

void CMainFrame::GetMainInfo(MAIN_INFO& Main) const
{
	m_View->GetNormOrigin(Main.Origin);
	Main.DrawMode = GetDrawMode();
	Main.OrgMotion = GetOrgMotion();
	Main.Hue = m_View->GetHue();
	Main.Mirror = IsMirrored();
	Main.Reverse = GetReverse();
	Main.Convex = GetConvex();
	Main.InvertColor = IsColorInverted();
	Main.LoopHue = IsHueLooped();
	Main.Pause = IsPaused();
	Main.ZoomCenter = GetZoomCenter();
}

void CMainFrame::SetMainInfo(const MAIN_INFO& Main)
{
	m_View->SetNormOrigin(Main.Origin);
	SetDrawMode(Main.DrawMode);
	SetOrgMotion(Main.OrgMotion);
	Mirror(Main.Mirror);
	SetReverse(Main.Reverse);
	SetConvex(Main.Convex);
	InvertColor(Main.InvertColor);
	LoopHue(Main.LoopHue);	// set loop state before setting hue
	m_View->SetHue(Main.Hue);
	Pause(Main.Pause);
	SetZoomCenter(Main.ZoomCenter);
	m_View->GetNormOrigin(m_TargetOrg);	// prevents subsequent drift
}

void CMainFrame::GetPatch(CPatch& Patch) const
{
	GetMasterInfo(Patch.m_Master);
	GetMainInfo(Patch.m_Main);
}

void CMainFrame::SetPatch(const CPatch& Patch)
{
	SetMasterInfo(Patch.m_Master);
	SetMainInfo(Patch.m_Main);
	MidiZeroCtrls();
}

void CMainFrame::GetInfo(INFO& Info) const
{
	m_CrossDlg.GetInfo(Info.XFader);
	m_PlaylistDlg.GetInfo(Info.Playlist);
	m_MidiSetupDlg.GetInfo(Info.MidiSetup);
	m_View->GetVideo().GetInfo(Info.VideoList);
	Info.Patch = m_MastInfo;	// assign to patch base class
	Info.GlobParm = m_GlobParm;
	GetMasterInfo(Info.Patch.m_Master);
	GetMainInfo(Info.Patch.m_Main);
	Info.ViewSel = m_ViewSel;
	Info.EditSel = m_EditSel;
	Info.PatchMode = m_PatchMode;
}

void CMainFrame::SetInfo(const INFO& Info)
{
	SetViewSel(Info.ViewSel);	// these have global effects, do first
	SetEditSel(Info.EditSel);
	SetPatchMode(Info.PatchMode);
	m_GlobParm = Info.GlobParm;	// order matters
	SetMasterInfo(Info.Patch.m_Master);	// then our member objects
	m_CrossDlg.SetInfo(Info.XFader);
	m_PlaylistDlg.SetInfo(Info.Playlist);
	m_MidiSetupDlg.SetInfo(Info.MidiSetup);
	m_MidiSetupDlg.SetLearn(FALSE);
	m_View->GetVideo().SetInfo(Info.VideoList);
	m_CycleVideo = Info.VideoList.Cycle;	// update video cycling shadow
	m_MastInfo = Info.Patch;	// automatically upcast patch to base class
	int	i;
	for (i = 0; i < ROWS; i++)
		m_MastOfs[i] = (float)m_MastInfo.m_Row[i].Val;	// update master offsets
	for (i = 0; i < GLOBAL_PARMS; i++) {
		int	gpi = CParmInfo::m_GlobParm[i];
		const	CParmInfo::ROW& GlobInfo = Info.GlobParm.m_Row[gpi];
		m_GlobParmTarg[gpi] = GlobInfo.Val;	// update global parameter target
		m_GlobOsc[gpi].SetWaveform(GlobInfo.Wave);
		m_GlobOsc[gpi].SetPulseWidth(GlobInfo.PW);
	}
	switch (m_EditSel) {
	case SEL_OFFSETS:
	case SEL_GLOBALS:
		m_ParmsDlg.SetInfo(*m_EditInfo);
		break;
	}	
	SetMainInfo(Info.Patch.m_Main);	// then our remaining properties
	m_View->ClearScreen();
}

void CMainFrame::GetDefaults(INFO& Info) const
{
	m_CrossDlg.GetDefaults(Info.XFader);
	m_PlaylistDlg.GetDefaults(Info.Playlist);
	m_MidiSetupDlg.GetDefaults(Info.MidiSetup);
	m_View->GetVideo().GetDefaults(Info.VideoList);
	ZeroMemory(&Info.Patch, sizeof(CParmInfo));
	m_MasterDlg.GetDefaults(Info.Patch.m_Master);
	Info.Patch.m_Main = CPatch::m_MainDefaults;
	ZeroMemory(&Info.GlobParm, sizeof(Info.GlobParm));
	Info.ViewSel = SEL_DOC;
	Info.EditSel = SEL_DOC;
	Info.PatchMode = PM_FULL;
}

void CMainFrame::RestoreDefaults()
{
	INFO	Info;
	GetDefaults(Info);
	SetInfo(Info);
}

void CMainFrame::SetViewSel(int Sel)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	m_ViewSel = Sel;
	switch (Sel) {
	case SEL_MIX:
		m_ViewInfo = &m_CrossDlg.GetInfo(Sel);
		break;
	case SEL_GLOBALS:
		m_ViewInfo = &m_GlobParm;
		break;
	default:
		m_ViewInfo = &GetDoc()->m_Patch;
	}
	m_View->FlushHistory();	// prevents glitches
	m_CrossDlg.SetViewSel(Sel);
	UpdateBars();
}

void CMainFrame::SetEditSel(int Sel)
{
	ASSERT(Sel >= 0 && Sel < SELS);
	if (m_EditSel != Sel && (m_EditSel == SEL_GLOBALS || Sel == SEL_GLOBALS)) {
		CRowDialog	*rd = &m_ParmsDlg;
		bool	Show = Sel != SEL_GLOBALS;
		for (int i = 0; i < ROWS; i++) {	// show or hide non-global rows
			if (!m_IsGlobParm[i]) {
				CWnd	*wp = rd->GetRow(i)->GetWindow(GW_CHILD);
				while (wp != NULL) {
					wp->EnableWindow(Show);
					wp = wp->GetNextWindow();
				}
			}
		}
	}
	m_EditSel = Sel;
	switch (Sel) {
	case SEL_DOC:
		m_EditInfo = &GetDoc()->m_Patch;
		break;
	case SEL_OFFSETS:
		m_EditInfo = &m_MastInfo;
		break;
	case SEL_GLOBALS:
		m_EditInfo = &m_GlobParm;
		break;
	default:
		m_EditInfo = &m_CrossDlg.GetInfo(Sel);
	}
	m_ParmsDlg.SetOffsetMode(Sel == SEL_OFFSETS || Sel == SEL_GLOBALS);
	m_CrossDlg.SetEditSel(Sel);
	m_ParmsDlg.SetInfo(*m_EditInfo);
	UpdateParmsDlgTitle();
	UpdateBars();
}

void CMainFrame::SetDrawMode(int Mode)
{
	m_View->SetDrawMode(Mode);
	UpdateBars();
}

void CMainFrame::Mirror(bool Enable)
{
	m_View->Mirror(Enable);
	UpdateBars();
}

void CMainFrame::LoopHue(bool Enable)
{
	m_View->LoopHue(Enable);
	UpdateBars();
}

void CMainFrame::InvertColor(bool Enable)
{
	m_View->InvertColor(Enable);
	UpdateBars();
}

void CMainFrame::SetCaption(LPCTSTR Filename)
{
	SetWindowText(CString(Filename) + " - " + AfxGetAppName());
}

void CMainFrame::SetTempo(double Tempo, bool Resync)
{
	m_Tempo.SetFreq(float(Tempo / 60.0), Resync);
	m_MasterDlg.GetTempo().SetVal(Tempo);
	UpdateTempo();
}

void CMainFrame::SetTempoMult(int Mult)
{
	m_TempoMult = Mult;
}

void CMainFrame::SetOrgMotion(int Motion)
{
	if (Motion == m_OrgMotion)
		return;	// nothing to do
	switch (Motion) {
	case OM_DRAG:
		SetMousePosFromOrigin();
		break;
	case OM_RANDOM:
		m_View->GetNormOrigin(m_TargetOrg);	// prevent initial move
		break;
	}
	bool	MouseDrag = (Motion == OM_DRAG);
	HideCursor(MouseDrag);	// hide cursor while dragging origin
	m_OrgMotion = Motion;
	UpdateBars();
}

void CMainFrame::SetReverse(bool Enable)
{
	int	Dir = Enable ? -1 : 1;
	if (Dir == m_GrowDir)
		return;	// nothing to do
	m_GrowDir = Dir;
	UpdateBars();
}

void CMainFrame::SetConvex(bool Enable)
{
	m_View->SetConvex(Enable);
	UpdateBars();
}

void CMainFrame::SetAutoplay(bool Enable)
{
	m_PlaylistDlg.Play(Enable);
	UpdateBars();
}

void CMainFrame::SetXFadeFrac(double Frac)
{
	m_PlaylistDlg.SetXFadeFrac(Frac);
	if (!m_CrossDlg.IsPlaying())
		m_CrossDlg.SetSeconds(m_PlaylistDlg.GetXFadeSecs());
}

void CMainFrame::GetDefaultFileName(CString& Name)
{
	CPathStr	ps;
	if (SnapCount())	// if we're in snapshot mode
		ps = m_Snapshot.GetCurPath();	// use current snap path
	else if (m_ViewSel == SEL_DOC)	// if we're viewing document
		ps = GetDoc()->GetPathName();	// use document path
	if (!ps.IsEmpty()) {
		ps = PathFindFileName(ps);
		ps.RemoveExtension();
	}
	Name = ps;
}

void CMainFrame::SetZoomCenter(bool Enable)
{
	m_View->SetZoomType(Enable ? 
		CWhorldView::ZT_WND_CENTER : CWhorldView::ZT_RING_ORIGIN);
}

void CMainFrame::SetPatchMode(int Mode)
{
	m_PatchMode = Mode;
	UpdateBars();
}

void CMainFrame::CycleVideo(bool Enable)
{
	m_CycleVideo = Enable;
	m_View->GetVideo().Cycle(Enable);
}

void CMainFrame::SetVideoCycleLength(int Length)
{
	Length = Length ? min(Length, CVideoList::MAX_VIDEOS) : CVideoList::MAX_VIDEOS;
	m_View->GetVideo().SetCycleLength(Length);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame operations

bool CMainFrame::EnableTimer(bool Enable)
{
	bool	PrevEnab = m_Timer != 0;
	if (Enable) {
		if (!m_Timer) {
			m_Timer = SetTimer(TIMER_ID, 
				round(1000.0 / m_OptsDlg.m_Display.GetFrameRate()), NULL);
		}
	} else {
		if (m_Timer) {
			KillTimer(m_Timer);
			m_Timer = 0;
		}
	}
	m_View->SetPause(!Enable);
	return(PrevEnab);
}

void CMainFrame::Pause(bool Enable)
{
	if (Enable == m_IsPaused)
		return;	// nothing to do
	switch (m_OrgMotion) {
	case OM_DRAG:
		if (!Enable)
			SetMousePosFromOrigin();
		else
			m_IsCursorValid = FALSE;
		break;
	case OM_RANDOM:
		if (!Enable)
			SetMousePosFromOrigin();
		break;
	}
	if (!Enable)
		m_Snapshot.Unload();
	m_IsPaused = Enable;
	EnableTimer(!Enable);
	HookMouse(Enable);	// if no timer, drag origin uses mouse move messages
	UpdateBars();
}

void CMainFrame::HideChrome(bool Enable)
{
	if (Enable == m_HideChrome)
		return;	// nothing to do
	if (IsViewDetached()) {
		if (Enable)
			ModifyStyle(WS_CAPTION | WS_THICKFRAME, 0);	// hide caption and frame
		else
			ModifyStyle(0, WS_CAPTION | WS_THICKFRAME);	// restore caption and frame
	} else {
		if (Enable) {	// order matters here
			m_ShowToolbar = m_Toolbar.IsWindowVisible();
			m_ShowStatusBar = m_StatusBar.IsWindowVisible();
			ShowControlBar(&m_Toolbar, FALSE, TRUE);	// hide toolbar, delay paint
			ShowControlBar(&m_StatusBar, FALSE, TRUE);	// hide status bar, delta paint
			ModifyStyle(WS_CAPTION | WS_THICKFRAME, 0);	// hide caption and frame
			SetMenu(NULL);	// hide our menus
		} else {
			ShowControlBar(&m_Toolbar, m_ShowToolbar, FALSE);	// restore toolbar
			ShowControlBar(&m_StatusBar, m_ShowStatusBar, FALSE);	// retore status bar
			m_Toolbar.OnUpdateCmdUI(this, TRUE);
			m_StatusBar.OnUpdateCmdUI(this, TRUE);
			ModifyStyle(0, WS_CAPTION | WS_THICKFRAME);	// restore caption and frame
			SetMenu(&m_Menu);	// restore our menus
		}
	}
	m_HideChrome = Enable;
}

void CMainFrame::FullScreen(bool Enable)
{
	if (IsViewDetached())
		m_AuxFrm->FullScreen(Enable);	// let detached frame handle it
	if (Enable == m_IsFullScreen || m_IsExclusive)
		return;	// nothing to do
	CRect	rc;
	if (Enable) {
		GetWindowRect(m_ScreenRect);
		CBackBufDD::GetFullScreenRect(*this, rc);
		rc.InflateRect(2, 2);	// hide thin frame
	} else
		rc = m_ScreenRect;
	HideChrome(Enable);
	SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
	m_IsFullScreen = Enable;
}

void CMainFrame::SetExclusive(bool Enable)
{
	if (Enable == m_IsExclusive)
		return;	// nothing to do
	CWhorldViewDD	*ViewDD = DYNAMIC_DOWNCAST(CWhorldViewDD, m_View);
	if (ViewDD != NULL) {
		if (Enable && !m_IsFullScreen)
			HideChrome(TRUE);
		HWND	hWnd = IsViewDetached() ? m_AuxFrm->m_hWnd : m_hWnd;
		ViewDD->SetExclusive(hWnd, Enable);
		if (!Enable) {
			if (m_IsFullScreen) {
				if (IsZoomed()) {	// if we were maximized before full screen
					CRect	rc;		// view leaves us maximized with thin frame
					GetWindowRect(rc);
					rc.InflateRect(2, 2);	// re-hide thin frame
					SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(),
						SWP_NOZORDER);
				}
			} else
				HideChrome(FALSE);
		}
		m_IsExclusive = Enable;
	}
}

bool CMainFrame::AllowExclusive() const
{
	CWhorldViewDD	*ViewDD = DYNAMIC_DOWNCAST(CWhorldViewDD, m_View);
	return(ViewDD != NULL && m_OptsDlg.m_Display.AllowExclusive());
}

void CMainFrame::Veejay(bool Enable)
{
	if (m_IsVeejay == Enable)
		return;	// nothing to do
	if (Enable) {	// warn first-time users
		if (MessageBoxCheck(m_hWnd, IDS_MF_VEEJAY_MODE_TEXT, IDS_MF_VEEJAY_MODE_TITLE,
			MB_OKCANCEL | MB_ICONINFORMATION, 0, REG_VEEJAY_MODE) == IDCANCEL)
			return;
	}
	m_PlaylistDlg.Cache(Enable);	// read all patches into memory
	if (AllowExclusive())
		SetExclusive(Enable);
	else
		FullScreen(Enable);
	ShowOwnedWindows(!Enable);	// hide dialogs in VJ mode
	ShowCursor(!Enable);		// hide cursor in VJ mode
	if (Enable) {
		if (m_OrgMotion == OM_PARK && !m_IsPaused)
			SetOrgMotion(OM_DRAG);	// start out in drag mode
	} else {
		if (m_OrgMotion == OM_DRAG)	// if dragging origin, park it
			SetOrgMotion(OM_PARK);
		for (int i = 0; i < ROWS; i++)	// master info could be stale
			m_MastInfo.m_Row[i].Val = m_MastOfs[i];
		m_ParmsDlg.SetInfo(*m_EditInfo);	// parms dialog could be stale
	}
	m_IsVeejay = Enable;
}

void CMainFrame::DetachView(bool Enable)
{
	if (Enable == IsViewDetached())
		return;	// nothing to do
	FullScreen(FALSE);
	if (Enable) {
		m_AuxFrm = new CAuxFrame;
		if (m_AuxFrm != NULL && m_AuxFrm->Create(NULL, AfxGetAppName())) {
			m_AuxView = m_AuxFrm->GetActiveView();
			// exchange views with aux frame
			m_View->SetParent(m_AuxFrm);	// must set parent before setting active
			m_AuxFrm->SetActiveView(m_View);	// aux frame takes our view
			m_AuxView->SetParent(this);
			SetActiveView(m_AuxView);	// we take aux frame's view
			GetDoc()->AddView(m_AuxView);		// add aux view to our document
			RecalcLayout();
			m_AuxFrm->ShowWindow(SW_SHOW);
		} else {
			delete m_AuxFrm;
			m_AuxFrm = NULL;
			AfxMessageBox(IDS_CANT_DETACH);
		}
	} else {
		// undo exchange of views
		GetDoc()->RemoveView(m_AuxView);	// remove aux view from our document
		m_AuxView->SetParent(m_AuxFrm);	// must set parent before setting active
		m_AuxFrm->SetActiveView(m_AuxView);	// aux frame takes back its view
		m_View->SetParent(this);
		SetActiveView(m_View);	// we take back our view
		RecalcLayout();
		m_AuxFrm->PostMessage(WM_CLOSE);	// deletes aux frame and view
		m_AuxFrm = NULL;
	}
}

void CMainFrame::ShowDemo(bool Enable, bool Prompt)
{
	if (Enable == m_DemoMode)
		return;	// nothing to do
	if (Enable) {
		CPathStr	DemoPath = CPersist::GetString(REG_SETTINGS, REG_DEMO_PATH);
		if (!PathFileExists(DemoPath)) {
			DemoPath = GetAppPath();
			DemoPath.Append(m_DemoPlaylist);
		}
		if (Prompt) {
			if (!PathFileExists(DemoPath))	// only prompt if demo is available
				return;
			if (MessageBoxCheck(m_hWnd, IDS_MF_VIEW_DEMO_TEXT, IDS_MF_VIEW_DEMO_TITLE,
				MB_YESNO | MB_ICONQUESTION, 0, REG_VIEW_DEMO) != IDYES)
				return;
		}
		bool	retc = m_PlaylistDlg.Open(DemoPath);
		if (!retc && AfxMessageBox(IDS_MF_DEMO_BROWSE, MB_YESNO) == IDYES)
			retc = m_PlaylistDlg.Open();
		if (retc) {
			DemoPath = m_PlaylistDlg.GetListPath();
			CPersist::WriteString(REG_SETTINGS, REG_DEMO_PATH, DemoPath);
			m_PlaylistDlg.PlayNext(0);	// jump to first patch, no crossfade
			m_PlaylistDlg.PlayNext();	// crossfade to second patch
			m_View->ClearScreen();
			m_DemoMode = TRUE;
		}
	} else {
		m_PlaylistDlg.Play(FALSE);
		m_DemoMode = FALSE;
	}
}

bool CMainFrame::ShowSnapshot(int SnapIdx)
{
	return(m_Snapshot.Show(SnapIdx));
}

bool CMainFrame::ExportBitmap(LPCTSTR Path)
{
	CFile	fp;
	CFileException	e;
	if (!fp.Open(Path, CFile::modeCreate | CFile::modeWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	CSize	size;
	bool	UseWndSize = m_ExportDlg.GetSize(size);
	bool	ScaleToFit = m_ExportDlg.GetScaleToFit();
	int		Resolution = m_ExportDlg.GetResolution();
	if (!m_View->ExportBitmap(fp, UseWndSize ? NULL : &size, NULL, ScaleToFit, Resolution)) {
		AfxMessageBox(IDS_MF_CANT_EXPORT);
		return(FALSE);
	}
	return(TRUE);
}

bool CMainFrame::OpenPlaylist(LPCTSTR Path)
{
	return(m_PlaylistDlg.Open(Path));
}

bool CMainFrame::OpenVideo(LPCTSTR Path)
{
	CVideoList&	vl = m_View->GetVideo();
	bool	retc = vl.Open(Path);
	if (!retc && !m_IsVeejay) {
		LPCTSTR	err = vl.GetLastErrorString();
		CString	s;
		AfxFormatString2(s, IDS_PL_BAD_VIDEO, Path, err);
		AfxMessageBox(s);
	}
	return(retc);
}

bool CMainFrame::OpenOtherDoc(LPCTSTR Path)
{
	CString	Ext = PathFindExtension(Path);
	if (!_tcsicmp(Ext, EXT_PLAYLIST))
		m_PlaylistDlg.Open(Path);
	else if (!_tcsicmp(Ext, EXT_SNAPSHOT))
		m_Snapshot.Serialize(Path, TRUE);
	else if (!_tcsicmp(Ext, EXT_MOVIE))
		CMoviePlayerDlg::Play(Path);
	else if (!_tcsicmp(Ext, EXT_AVI))
		OpenVideo(Path);
	else
		return(FALSE);
	return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame helpers

void CMainFrame::UpdateParmsDlgTitle()
{
	CString	ViewName;
	CPathStr	Title;
	switch (m_EditSel) {
	case SEL_DOC:
		Title = GetDoc()->GetTitle();
		break;
	default:
		Title = GetSelName(m_EditSel);
	}
	Title.RemoveExtension();
	m_ParmsDlg.SetDocTitle(Title);
}

void CMainFrame::UpdateBars()
{
	if (!m_HideChrome) {
		m_Toolbar.OnUpdateCmdUI(this, TRUE);
		m_StatusBar.OnUpdateCmdUI(this, TRUE);
	}
}

void CMainFrame::UpdateTempo()
{
	UpdateBars();
}

void CMainFrame::UpdateOptions()
{
	m_View->SetOptions(m_OptsDlg.m_Display.GetOptions());
	int	Dev = m_OptsDlg.m_Midi.GetDevice();
	if (Dev != m_MidiDev) {
		m_MidiIO.EnableInput(FALSE);	// close previous MIDI device if any
		if (Dev != CMidiIO::NO_DEVICE) {
			if (!m_MidiIO.EnableInput(TRUE, Dev)) {
				CString	s, msg;
				m_MidiIO.GetLastErrorString(s);
				AfxFormatString1(msg, IDS_MIDI_CANT_CREATE, s); 
				AfxMessageBox(msg);
			}
		}
		m_MidiDev = Dev;
	}
	m_Tempo.Run(!m_OptsDlg.m_Midi.IsSync());	// run tempo clock unless MIDI sync
	double	TimerFreq = m_OptsDlg.m_Display.GetFrameRate();
	m_CrossDlg.SetTimerFreq(TimerFreq);
	m_View->SetTimerFreq(TimerFreq);
	for (int i = 0; i < GLOBAL_PARMS; i++) {
		int	gpi = CParmInfo::m_GlobParm[i];
		m_GlobOsc[gpi].SetTimerFreq(TimerFreq);
	}
	TimerState	ts(FALSE);	// restart timer to set frequency
}

void CMainFrame::UpdateMaster(int Idx)
{
	switch (Idx) {
	case CMasterDlg::SPEED:
		{
			double	Speed = m_MasterDlg.GetSpeed().GetVal();
			for (int i = 0; i < GLOBAL_PARMS; i++) {
				int	gpi = CParmInfo::m_GlobParm[i];
				double	Freq = m_GlobParm.m_Row[gpi].Freq;
				if (Freq)
					m_GlobOsc[gpi].SetFreq(Freq * Speed);
			}
		}
		break;
	case CMasterDlg::ZOOM:
		m_View->SetZoom(m_MasterDlg.GetZoom().GetVal());
		m_Zoom = m_MasterDlg.GetZoom().GetValNorm();
		m_TargetZoom = m_Zoom;	// no damping, jump to new value
		break;
	case CMasterDlg::TEMPO:
		m_Tempo.SetFreq(float(GetTempo() / 60.0));
		SetTempoMult(0);
		UpdateTempo();
		break;
	case CMasterDlg::RINGS:
		{
			int	Limit = round(m_MasterDlg.GetRings().GetVal());
			if (Limit >= CPatch::MAX_RINGS)
				Limit = INT_MAX;	// no limit
			m_View->SetMaxRings(Limit);
		}
		break;
	case CMasterDlg::TRAIL:
		m_View->SetTrail(m_MasterDlg.GetTrail().GetVal());
		break;
	case CMasterDlg::HUE_LOOP:
		m_View->SetHueLoopLength(m_MasterDlg.GetHueLoopLength().GetVal());
		break;
	case CMasterDlg::CANVAS:
		m_View->SetCanvasScale(m_MasterDlg.GetCanvasScale().GetVal());
		break;
	case CMasterDlg::COPIES:
		m_View->SetCopyCount(round(m_MasterDlg.GetCopies().GetVal()));
		break;
	case CMasterDlg::SPREAD:
		m_View->SetCopySpread(round(m_MasterDlg.GetSpread().GetVal()));
		break;
	}
}

void CMainFrame::HideCursor(bool Enable)
{
	if (Enable == m_HideCursor)
		return;	// nothing to do
	ShowCursor(!Enable);
	m_HideCursor = Enable;
}

LRESULT CALLBACK CMainFrame::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CMainFrame	*frm = CMainFrame::GetThis();
	if (nCode == HC_ACTION) {	// ignore peek
		if (wParam == WM_MOUSEMOVE) {
			// if any buttons are down, we've captured the mouse, so don't relay,
			// otherwise OnMouseMove will receive duplicate mouse move messages
			if (!(frm->m_InputMode & IM_BUTTONS)) {
				LPMOUSEHOOKSTRUCT	p = (LPMOUSEHOOKSTRUCT)lParam;
				CPoint	pos(p->pt);
				frm->ScreenToClient(&pos);
				frm->SendMessage(WM_MOUSEMOVE, 0, MAKELONG(pos.x, pos.y));
			}
		}
	}
	return(CallNextHookEx(frm->m_MouseHook, nCode, wParam, lParam));
}

bool CMainFrame::HookMouse(bool Enable)
{
	bool	retc = TRUE;
	if (Enable != (m_MouseHook != NULL)) {	// if not in requested state
		if (Enable) {
			m_MouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc,
				NULL, AfxGetApp()->m_nThreadID);
			retc = m_MouseHook != NULL;
		} else {
			retc = UnhookWindowsHookEx(m_MouseHook) != 0;
			m_MouseHook = NULL;
		}
	}
	return(retc);
}

LRESULT CALLBACK CMainFrame::KeybdProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CMainFrame	*frm = CMainFrame::GetThis();
	if (nCode == HC_ACTION) {	// ignore peek
		LPKBDLLHOOKSTRUCT	khs = (LPKBDLLHOOKSTRUCT)lParam;
		if (frm->m_IsVeejay) {	// disable task-switching keys in VJ mode
			switch (khs->vkCode) {
			case VK_LWIN:	// Windows keys
			case VK_RWIN:
				return(TRUE);	// eat key
			case VK_ESCAPE:
				if ((khs->flags & LLKHF_ALTDOWN)	// Alt+Escape 
				|| (GetAsyncKeyState(VK_CONTROL) & GKS_DOWN))	// Ctrl+Escape
					return(TRUE);	// eat key
				break;
			case VK_TAB:
				if (khs->flags & LLKHF_ALTDOWN)	// Alt+Tab
					return(TRUE);	// eat key
				break;
			}
		}
		switch (khs->vkCode) {
		case VK_TAB:
			if (wParam == WM_KEYDOWN) {
				double	Time = CBenchmark::Time();
				float	Delta = float(Time - frm->m_PrevTapTime);
				frm->PostMessage(UWM_TEMPOTAP, FLOAT_TO_INT(Delta));
				frm->m_PrevTapTime = Time;
			}
			break;
		case VK_LSHIFT:
		case VK_RSHIFT:
			if (wParam == WM_KEYDOWN)
				frm->m_InputMode |= IM_SHIFT;
			else if (wParam == WM_KEYUP)
				frm->m_InputMode &= ~IM_SHIFT;
			break;
		case VK_LCONTROL:
		case VK_RCONTROL:
			if (wParam == WM_KEYDOWN)
				frm->m_InputMode |= IM_CONTROL;
			else if (wParam == WM_KEYUP)
				frm->m_InputMode &= ~IM_CONTROL;
			break;
		case VK_LMENU:
		case VK_RMENU:
			if (khs->flags & LLKHF_ALTDOWN) {
				frm->m_InputMode |= IM_ALT;
				if (frm->m_HideChrome)	// if no menus
					return(TRUE);	// eat key to avoid entering menu mode
			} else
				frm->m_InputMode &= ~IM_ALT;
			break;
		case VK_CAPITAL:
			if (frm->IsRecording() && wParam == WM_KEYDOWN)
				frm->m_KbdLedMask ^= CKeyboardLeds::CAPS_LOCK;
			break;
		case VK_NUMLOCK:
			if (frm->IsRecording() && wParam == WM_KEYDOWN)
				frm->m_KbdLedMask ^= CKeyboardLeds::NUM_LOCK;
			break;
		}
	}
	return(CallNextHookEx(frm->m_KeybdHook, nCode, wParam, lParam));
}

bool CMainFrame::HookKeybd(bool Enable)
{
	bool	retc = TRUE;
	if (Enable != (m_KeybdHook != NULL)) {	// if not in requested state
		if (Enable) {
			m_KeybdHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeybdProc, 
				AfxGetApp()->m_hInstance, 0);
			retc = m_KeybdHook != NULL;
		} else {
			retc = UnhookWindowsHookEx(m_KeybdHook) != 0;
			m_KeybdHook = NULL;
		}
	}
	return(retc);
}

void CMainFrame::TimerCallback(LPVOID Cookie)
{
	((CMainFrame *)Cookie)->m_Clock++;	// trigger random jump
}

void CMainFrame::GetMouseInput()
{
	DWORD	MouseMode = m_InputMode;	// modified by keyboard hook
	CDirInput::STATE	st;
	m_DirInput.GetState(CDirInput::DEV_MOUSE, st);
	double	MouseSens = m_OptsDlg.m_Input.GetMouseSens();
	if (ViewHasFocus()) {
		switch (MouseMode) {
		case 0:	// unmodified mouse
			if (m_OrgMotion != OM_RANDOM || !m_IsVeejay)
				break;	// if random motion in VJ mode, fall through to zoom
		case MM_ZOOM:
			if (st.Mouse.lY) {	// if y-axis changed
				double	r = m_TargetZoom;
				r -= st.Mouse.lY * ZOOM_RANGE * ZOOM_MOUSE_SENS * MouseSens;
				m_TargetZoom = CLAMP(r, -1, 1);
			}
			break;
		case MM_SPEED:
			if (st.Mouse.lY) {	// if y-axis changed
				double	r = GetSpeedNorm();
				r -= st.Mouse.lY * SPEED_RANGE * SPEED_MOUSE_SENS * MouseSens;
				SetSpeedNorm(CLAMP(r, -1, 1));
			}
			break;
		case MM_CYCLE_HUE:
			if (st.Mouse.lY) {	// if y-axis changed
				double	r = -st.Mouse.lY * HUE_RANGE * HUE_MOUSE_SENS * MouseSens;
				m_View->RotateHue(r);
			}
			break;
		case MM_RINGS:
			if (st.Mouse.lY && m_IsVeejay) {	// if y-axis changed (VJ mode only)
				double	r = GetRingsNorm();
				r -= st.Mouse.lY * SPEED_RANGE * RINGS_MOUSE_SENS * MouseSens;
				SetRingsNorm(CLAMP(r, 0, 1));
			}
			break;
		case MM_VIDEO_ORG:
			{
				if (st.Mouse.lX || st.Mouse.lY) {	// only update if mouse moved
					m_TargetVideoOrg.x += st.Mouse.lX * VIDEO_ORG_MOUSE_SENS * MouseSens;
					m_TargetVideoOrg.y += st.Mouse.lY * VIDEO_ORG_MOUSE_SENS * MouseSens;
					m_TargetVideoOrg.x = CLAMP(m_TargetVideoOrg.x, 0, 1);
					m_TargetVideoOrg.y = CLAMP(m_TargetVideoOrg.y, 0, 1);
					m_VideoOrgMoving = TRUE;
				}
			}
			break;
		}
	}
	double	delta = m_Zoom - m_TargetZoom;
	if (fabs(delta) > DAMP_TOLER) {
		m_Zoom -= delta * m_MasterDlg.GetDamping().GetVal();
		m_MasterDlg.GetZoom().SetValNorm(m_Zoom);
		DPoint	p1, p2;
		m_View->GetNormOrigin(p1);
		m_View->SetZoom(m_MasterDlg.GetZoom().GetVal());
		m_View->GetNormOrigin(p2);
		m_TargetOrg -= (p1 - p2);	// zoom can change origin, so compensate target
	}
	if (m_VideoRandOrg) {
		if (m_Clock != m_PrevClock) {	// time for a new random target
			m_TargetVideoOrg.x = double(rand()) / RAND_MAX;
			m_TargetVideoOrg.y = double(rand()) / RAND_MAX;
			m_VideoOrgMoving = TRUE;
		}
	}
	if (m_VideoOrgMoving) {	// if not close enough to target yet
		DPoint	org, delta;
		m_View->GetVideoOrigin(org);
		delta = m_TargetVideoOrg - org;
		if (fabs(delta.x) < DAMP_TOLER && fabs(delta.y) < DAMP_TOLER)
			m_VideoOrgMoving = FALSE;	// close enough to target
		org += delta * m_MasterDlg.GetDamping().GetVal();
		m_View->SetVideoOrigin(org);
	}
	switch (m_OrgMotion) {
	case OM_DRAG:
		{
			if (!MouseMode) {	// unmodified mouse updates drag target
				if (st.Mouse.lX || st.Mouse.lY) {	// only update if mouse moved
					CRect	r;
					m_View->GetCanvasRect(r);	// limit origin to canvas
					CSize	sz = m_View->GetSize();
					if (r.Width() < sz.cx)	// if canvas is smaller than view
						r = CRect(0, 0, sz.cx, sz.cy);	// limit origin to view instead
					m_View->ClientToScreen(r);
					m_MousePos.x += st.Mouse.lX * MouseSens;
					m_MousePos.y += st.Mouse.lY * MouseSens;
					m_MousePos.x = CLAMP(m_MousePos.x, r.left, r.right);
					m_MousePos.y = CLAMP(m_MousePos.y, r.top, r.bottom);
					if (sz.cx > 0 && sz.cy > 0) {	// avoid divide by zero
						CPoint	pt = m_MousePos;
						m_View->ScreenToClient(&pt);
						m_TargetOrg.x = double(pt.x) / sz.cx;
						m_TargetOrg.y = double(pt.y) / sz.cy;
					}
				}
			} 
			DPoint	org;
			m_View->GetNormOrigin(org);
			org += (m_TargetOrg - org) * m_MasterDlg.GetDamping().GetVal();
			m_View->SetNormOrigin(org);
		}
		break;
	case OM_RANDOM:
		{
			if (m_Clock != m_PrevClock) {	// time for a new random target
				m_TargetOrg.x = double(rand()) / RAND_MAX;
				m_TargetOrg.y = double(rand()) / RAND_MAX;
			}
			DPoint	org;
			m_View->GetNormOrigin(org);
			org += (m_TargetOrg - org) * m_MasterDlg.GetDamping().GetVal();
			m_View->SetNormOrigin(org);
		}
		break;
	case OM_MIDI_DRAG:
		{
			DPoint	org;
			m_View->GetNormOrigin(org);
			org += (m_TargetOrg - org) * m_MasterDlg.GetDamping().GetVal();
			m_View->SetNormOrigin(org);
		}
		break;
	}
}

void CMainFrame::SetMousePosFromOrigin()
{
	CPoint	pt;
	pt = m_View->GetOrigin();
	CSize	sz = m_View->GetSize();
	if (sz.cx > 0 && sz.cy > 0) {	// avoid divide by zero
		m_TargetOrg.x = double(pt.x) / sz.cx;
		m_TargetOrg.y = double(pt.y) / sz.cy;
	}
	m_View->ClientToScreen(&pt);
	m_MousePos = pt;
	CDirInput::STATE	st;
	m_DirInput.GetState(CDirInput::DEV_MOUSE, st);	// flush mouse deltas
}

inline bool CMainFrame::MiscPropChanged(int PropIdx, int Toggle, float CtrlVal)
{
	if (Toggle)
		return(TRUE);
	BYTE	State = (CtrlVal >= .5);	// assume range is 0..1
	if (State != m_MPState[PropIdx]) {
		m_MPState[PropIdx] = State;
		return(State != 0);	// bang on make only; ignore break
	}
	return(FALSE);
}

void CMainFrame::SetMidiProperty(int Idx, int Val, bool Toggle)
{
	m_MidiSetupDlg.SetValue(Idx, Val);	// update property's shadow
	float	r;
	CMidiSetupDlg::DRANGE	MSRange;
	m_MidiSetupDlg.GetRange(Idx, MSRange);
	double	Range = MSRange.End - MSRange.Start;
	if (Toggle) {	// alternate between extremes
		r = float(m_NoteToggle[Idx] ? MSRange.Start : MSRange.End);
		m_NoteToggle[Idx] ^= 1;
	} else {	// pan style: 0 = hard left, 64 = center, 127 = hard right
		int		n = Val - 64;	// convert from offset to signed
		double	HalfRange = Range / 2;
		r = float(MSRange.Start + HalfRange + n / float(n < 0 ? 64 : 63) * HalfRange);
	}
	if (Idx < CParmInfo::ROWS) {	// assume it's a master offset
		m_MastOfs[Idx] = r;
		if (m_IsVeejay)
			return;	// don't bother updating parameters dialog in VJ mode
		m_MastInfo.m_Row[Idx].Val = r;
		if (m_EditSel == SEL_OFFSETS)
			m_ParmsDlg.SetVal(Idx, r);
	} else {	// assume it's a miscellaneous MIDI property
		Idx -= CParmInfo::ROWS;	// skip master offsets
		switch (Idx) {	// keep cases in MidiProps.h order for clarity
		case MP_BANK:
			m_PlaylistDlg.SetBank(round(r));
			break;
		case MP_SPEED:
			SetSpeedNorm(r);
			break;
		case MP_ZOOM:
			SetZoomNormDamped(r);
			break;
		case MP_DAMPING:
			SetDampingNorm(r);
			break;
		case MP_TRAIL:
			SetTrailNorm(r);
			break;
		case MP_RINGS:
			SetRingsNorm(r);
			break;
		case MP_COPIES:
			SetCopies(round(r));
			break;
		case MP_SPREAD:
			SetSpread(round(r));
			break;
		case MP_PATCH_SECS:
			m_PlaylistDlg.SetPatchSecs(r);
			break;
		case MP_XFADE_SECS:
			m_PlaylistDlg.SetXFadeSecs(r);
			if (!m_CrossDlg.IsPlaying())
				m_CrossDlg.SetSeconds(r);
			break;
		case MP_XFADE_POS:
			m_CrossDlg.SetPos(r);
			break;
		case MP_ORIGIN_X:
			if (m_OrgMotion == OM_PARK)
				SetOrgMotion(OM_MIDI_DRAG);
			if (m_OrgMotion == OM_DRAG || m_OrgMotion == OM_MIDI_DRAG)
				m_TargetOrg.x = r;
			break;
		case MP_ORIGIN_Y:
			if (m_OrgMotion == OM_PARK)
				SetOrgMotion(OM_MIDI_DRAG);
			if (m_OrgMotion == OM_DRAG || m_OrgMotion == OM_MIDI_DRAG)
				m_TargetOrg.y = r;
			break;
		case MP_HUE:
			m_View->RotateHue((r - m_PrevHue) * 360);
			m_PrevHue = r;
			break;
		case MP_HUE_LOOP_LENGTH:
			SetHueLoopLength(r);
			break;
		case MP_CANVAS_SCALE:
			SetCanvasScale(r);
			break;
		case MP_PATCH_MODE:
			SetPatchMode(round(r));
			break;
		case MP_RANDOM_PATCH:	// momentary
			if (MiscPropChanged(Idx, Toggle, r))
				OnEditRandomPatch();
			break;
		case MP_LOAD_NEXT_A:	// momentary
			if (MiscPropChanged(Idx, Toggle, r))
				OnEditNextPatchToA();
			break;
		case MP_LOAD_NEXT_B:	// momentary
			if (MiscPropChanged(Idx, Toggle, r))
				OnEditNextPatchToB();
			break;
		case MP_LOOP_HUE:
			LoopHue(Toggle ? !IsHueLooped() : r >= .5);
			break;
		case MP_ROTATE_HUE:	// momentary
			if (MiscPropChanged(Idx, Toggle, r))
				OnImgRotateHue();
			break;
		case MP_INVERT_COLOR:
			InvertColor(Toggle ? !IsColorInverted() : r >= .5);
			break;
		case MP_MIRROR:
			Mirror(Toggle ? !IsMirrored() : r >= .5);
			break;
		case MP_FILL:
			SetFill(Toggle ? !GetFill() : r >= .5);
			break;
		case MP_OUTLINE:
			SetOutline(Toggle ? !GetOutline() : r >= .5);
			break;
		case MP_XRAY:
			SetXRay(Toggle ? !GetXRay() : r >= .5);
			break;
		case MP_RANDOM_PHASE:	// momentary
			if (MiscPropChanged(Idx, Toggle, r))
				OnImgRandomPhase();
			break;
		case MP_REVERSE:
			SetReverse(Toggle ? !GetReverse() : r >= .5);
			break;
		case MP_TOGGLE_ORIGIN:
			if (Toggle)
				OnImgToggleOrigin();
			else
				SetOrgMotion(r >= .5 ? OM_RANDOM : OM_DRAG);
			break;
		case MP_AUTOPLAY:
			SetAutoplay(Toggle ? !GetAutoplay() : r >= .5);
			break;
		case MP_PAUSE:
			Pause(Toggle ? !IsPaused() : r >= .5);
			break;
		case MP_CLEAR:	// momentary
			if (MiscPropChanged(Idx, Toggle, r))
				OnWndClear();
			break;
		case MP_XFADE_PLAY:
			m_CrossDlg.Play(Toggle ? !m_CrossDlg.IsPlaying() : r >= .5);
			break;
		case MP_XFADE_LOOP:
			m_CrossDlg.Loop(Toggle ? !m_CrossDlg.IsLooped() : r >= .5);
			break;
		case MP_CONVEX:
			SetConvex(Toggle ? !GetConvex() : r >= .5);
			break;
		case MP_ZOOM_CENTER:
			SetZoomCenter(Toggle ? !GetZoomCenter() : r >= .5);
			break;
		case MP_VIDEO_SELECT:
			m_View->GetVideo().Select(int(r));
			break;
		case MP_VIDEO_BLENDING:
			m_View->SetVideoROP(int(r));
			break;
		case MP_VIDEO_CYCLE:
			CycleVideo(Toggle ? !IsVideoCycling() : r >= .5);
			break;
		case MP_VIDEO_CYCLE_LEN:
			SetVideoCycleLength(int(r));
			break;
		case MP_VIDEO_ORIGIN_X:
			m_TargetVideoOrg.x = r;
			m_VideoOrgMoving = TRUE;
			break;
		case MP_VIDEO_ORIGIN_Y:
			m_TargetVideoOrg.y = r;
			m_VideoOrgMoving = TRUE;
			break;
		case MP_VIDEO_RAND_ORG:
			m_VideoRandOrg = Toggle ? !m_VideoRandOrg : r >= .5;
			break;
		default:
			Idx -= CMidiSetupDlg::MISC_PROPS;	// skip misc properties
			// if it's a global parameter
			if (Idx >= 0 && Idx < GLOBAL_PARMS) {
				int	gpi = CParmInfo::m_GlobParm[Idx];
				m_GlobParmTarg[gpi] = r;
			} else {
				Idx -= GLOBAL_PARMS;	// skip global parameters
				// if it's an oscillator property
				if (Idx >= 0 && Idx < CMidiSetupDlg::OSC_ROWS) {
					int	ParmIdx = Idx / CMidiSetupDlg::OSC_PROPS;
					int	OscProp = Idx % CMidiSetupDlg::OSC_PROPS;
					CParmInfo::ROW&	MastRow = m_MastInfo.m_Row[ParmIdx];
					switch (OscProp) {
					case CParmInfo::OSC_WAVE:
						{
							int	i = round(r);	// round and clamp to waveform range
							MastRow.Wave = CLAMP(i, 0, COscillator::WAVEFORMS - 1);
						}
						break;
					case CParmInfo::OSC_AMP:
						MastRow.Amp = r;
						break;
					case CParmInfo::OSC_FREQ:
						MastRow.Freq = r;
						break;
					case CParmInfo::OSC_PW:
						MastRow.PW = r;
						break;
					}
					if (!m_IsVeejay && m_EditSel == SEL_OFFSETS)
						m_ParmsDlg.SetRow(ParmIdx, MastRow);
				} else {
					Idx -= CMidiSetupDlg::OSC_ROWS;	// skip oscillator properties
					// if it's a global oscillator property
					if (Idx >= 0 && Idx < CMidiSetupDlg::GLOB_OSC_ROWS) {
						int	ParmIdx = Idx / CMidiSetupDlg::OSC_PROPS;
						int	OscProp = Idx % CMidiSetupDlg::OSC_PROPS;
						int	gpi = CParmInfo::m_GlobParm[ParmIdx];
						CParmInfo::ROW&	GlobRow = m_GlobParm.m_Row[gpi];
						switch (OscProp) {
						case CParmInfo::OSC_WAVE:
							{
								int	i = round(r);	// round and clamp to waveform range
								GlobRow.Wave = CLAMP(i, 0, COscillator::WAVEFORMS - 1);
								m_GlobOsc[gpi].SetWaveform(GlobRow.Wave);
							}
							break;
						case CParmInfo::OSC_AMP:
							GlobRow.Amp = r;
							break;
						case CParmInfo::OSC_FREQ:
							GlobRow.Freq = r;
							m_GlobOsc[gpi].SetFreq(r);
							break;
						case CParmInfo::OSC_PW:
							GlobRow.PW = r;
							m_GlobOsc[gpi].SetPulseWidth(r);
							break;
						}
						if (!m_IsVeejay && m_EditSel == SEL_GLOBALS)
							m_ParmsDlg.SetRow(gpi, GlobRow);
					}
				}
			}
		}
	}
}

void CMainFrame::OnBankPatchSel(int Bank, int Patch)
{
	if (Bank == 0x7f) {	// bank 127 is reserved for video select
		m_View->GetVideo().Select(Patch);	// assume patch is video index
	} else {
		int	CurBank = m_PlaylistDlg.GetBank();	// save and restore current bank
		m_PlaylistDlg.FadeToBank(Bank, Patch);
		m_PlaylistDlg.SetBank(CurBank);
	}
}

LRESULT CMainFrame::OnMidiIn(WPARAM wParam, LPARAM lParam)
{
	CMidiIO::MSG	msg;
	msg.dw = wParam;
	// pass to setup dialog first, so learned assignments take effect immediately
	if (m_MidiSetupDlg.FastIsVisible())
		m_MidiSetupDlg.OnMidiIn(msg);
	int	idx;
	int	chan = msg.s.cmd & 0x0f;
	switch (msg.s.cmd & 0xf0) {
	case MC_NOTE_ON:
		if (!msg.s.p2)	// ignore zero velocity note off
			return(0);
		idx = m_MidiSetupDlg.GetNoteMapping(chan, msg.s.p1);
		if (idx >= 0)	// if message is mapped
			SetMidiProperty(idx, msg.s.p2, TRUE);	// toggle property
		idx = m_PlaylistDlg.GetNoteMapping(chan, msg.s.p1);
		if (idx >= 0)
			OnMidiProgChange(idx);
		break;
	case MC_KEY_AFT:
		if (m_InHouse)	{	// if hidden features are enabled
			if (!chan)	// key aftertouch on channel 0 is atomic bank/patch select
				OnBankPatchSel(msg.s.p1, msg.s.p2);
		}
		break;
	case MC_CTRL_CHG:
		idx = m_MidiSetupDlg.GetCtrlMapping(chan, msg.s.p1);
		if (idx >= 0) 	// if message is mapped
			SetMidiProperty(idx, msg.s.p2, FALSE);	// set property
		break;
	case MC_PROG_CHG:
		idx = m_PlaylistDlg.GetProgMapping(chan, msg.s.p1);
		if (idx >= 0)
			OnMidiProgChange(idx);
		break;
	case MC_CHAN_AFT:
		break;
	case MC_PITCH_BEND:
		idx = m_MidiSetupDlg.GetPitchMapping(chan);
		if (idx >= 0) 	// if message is mapped
			SetMidiProperty(idx, msg.s.p2, FALSE);	// set property
		break;
	}
	return(0);
}

void CALLBACK CMainFrame::MidiCallback(HMIDIIN handle, UINT uMsg, DWORD dwInstance,
								  DWORD dwParam1, DWORD dwParam2)
{
	// This function runs in a system thread, so beware of data corruption;
	// also avoid calling any system functions except for those listed in the
	// SDK under MidiInProc, and keep processing time to the absolute minimum.
	if (uMsg != MIM_DATA)
		return;
	CMainFrame	*frm = (CMainFrame *)dwInstance;
	CMidiIO::MSG	msg;
	msg.dw = dwParam1;
	if (msg.s.cmd >= 0xf0) {	// system message
		if (msg.s.cmd == 0xf8 && frm->m_OptsDlg.m_Midi.IsSync()) {	// if clock sync
			static const int MIN_QUANT_CLOCKS = MIDI_CLOCK_RATE >> QUANT_QUARTER;
			if (!(frm->m_MidiClock % (MIN_QUANT_CLOCKS << frm->m_MidiQuant)))
				frm->m_Clock++;	// we're on a measure boundary, so trigger random jump
			__asm {
				mov		esi, frm;
				inc		[esi].m_MidiClock;	// atomic increment for thread-safety
			}
		}
	} else 	// channel message
		::PostMessage(frm->m_hWnd, UWM_MIDIIN, msg.dw, 0);
}

void CMainFrame::MidiZeroCtrls()
{
	for (int i = 0; i < ROWS; i++) {
		m_MastInfo.m_Row[i].Val = 0;	// zero master offsets
		m_GlobParm.m_Row[i].Val = 0;	// zero globals
	}
	ZeroMemory(m_MastOfs, sizeof(m_MastOfs));	// zero master offset shadows
	ZeroMemory(m_GlobParmTarg, sizeof(m_GlobParmTarg));	// zero global targets
	if (!m_IsVeejay && (m_EditSel == SEL_OFFSETS || m_EditSel == SEL_GLOBALS))
		m_ParmsDlg.SetInfo(*m_EditInfo);
	m_MidiSetupDlg.ZeroControllers();
}

CString CMainFrame::GetAppPath()
{
	CString	s = GetCommandLine();
	s.TrimLeft();	// trim leading whitespace just in case
	if (s[0] == '"')	// if first char is a quote
		s = s.Mid(1).SpanExcluding(_T("\""));	// span to next quote
	else
		s = s.SpanExcluding(_T(" \t"));	// span to next whitespace
	TCHAR	*p = s.GetBuffer(0);
	PathRemoveFileSpec(p);
	s.ReleaseBuffer();
	return(s);
}

bool CMainFrame::MakeUniquePath(LPCTSTR Folder, LPCTSTR Prefix, LPCTSTR Extension, CString& Path)
{
	CTime	t(CTime::GetCurrentTime());
	CString	fname = CString(Prefix) + t.Format("_%y%m%d%H%M%S") + Extension;
	CPathStr	ps(Folder);
	if (ps.IsEmpty())
		ps = GetAppPath();	// last resort
	if (!PathIsDirectory(ps)) {
		if (!CreateDirectory(ps, NULL))
			return(FALSE);
	}
	ps.Append(fname);
	if (PathFileExists(ps))	{	// time is wrong, or multiple calls per second
		CString	s;
		s.Format(_T("_%d"), clock());
		ps.Insert(ps.ReverseFind('.'), s);	// append milliseconds to filename
		if (PathFileExists(ps))	// shouldn't happen
			return(FALSE);
	}
	Path = ps;
	return(TRUE);
}

bool CMainFrame::SaveMix()
{
	bool	retc;
	CString	Path;
	if (m_IsVeejay || m_OptsDlg.m_Folder.SnapFolderAlways()) {
		retc = MakeUniquePath(m_OptsDlg.m_Folder.GetSnapshotFolder(),
			PREFIX_PATCH, EXT_PATCH, Path);
	} else {
		CMultiFileDlg	fd(FALSE, EXT_PATCH, NULL, OFN_OVERWRITEPROMPT,
			LDS(IDS_FILTER_PATCH), LDS(IDS_MF_SAVE_MIX));
		if (retc = (fd.DoModal() == IDOK))
			Path = fd.GetPathName();
	}
	if (retc) {
		CPatch	Patch;
		Patch = *m_ViewInfo;	// save whatever we're viewing; assign to base class
		for (int i = 0; i < ROWS; i++) {
			Patch.m_Row[i].Val += m_MastOfs[i];	// add master offset to parameter
			if (m_MastInfo.m_Row[i].Freq)		// if master frequency is nonzero
				Patch.SetOscProps(m_MastInfo, i);	// override oscillator settings
		}
		GetPatch(Patch);	// get master and main info
		retc = Patch.Write(Path);
	}
	return(retc);
}

void CMainFrame::Record(bool Enable)
{
	if (Enable == IsRecording())
		return;	// nothing to do
	if (Enable) {
		TimerState	ts(FALSE);	// stop timer, dtor restores it
		CString	Path;
		if (m_IsVeejay || m_OptsDlg.m_Folder.SnapFolderAlways()) {
			if (!MakeUniquePath(m_OptsDlg.m_Folder.GetSnapshotFolder(),
			PREFIX_MOVIE, EXT_MOVIE, Path))
				return;
		} else {
			CMultiFileDlg	fd(FALSE, EXT_MOVIE, NULL, OFN_OVERWRITEPROMPT,
				LDS(IDS_FILTER_MOVIE), LDS(IDS_MOVIE_RECORD));
			if (fd.DoModal() == IDOK)
				Path = fd.GetPathName();
		}
		if (!Path.IsEmpty()) {
			m_Movie.SetView(m_View);
			m_Movie.SetFrameRate(round(GetTimerFreq()));
			m_Movie.SetFrameSize(m_View->GetSize());
			m_Movie.Open(Path, FALSE);
			m_View->GetVideo().Record(TRUE);	// enable video path recording
			m_KbdLeds.Open(TRUE);	// use overlapped I/O
			m_KbdLeds.Get(m_KbdLedMask);	// initialize LED shadow state
		}
	} else {
		m_KbdLeds.Set(m_KbdLedMask & ~CKeyboardLeds::SCROLL_LOCK);	// scroll LED off
		CWaitCursor	wc;	// close can take a while
		m_Movie.Close();
		m_View->GetVideo().Record(FALSE);	// disable video path recording
		m_KbdLeds.Close();
	}
	UpdateBars();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message map

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_WM_CREATE()
	ON_WM_ACTIVATEAPP()
	ON_WM_CHAR()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_EDIT_DOCUMENT, OnEditDocument)
	ON_COMMAND(ID_EDIT_END, OnEditEnd)
	ON_COMMAND(ID_EDIT_GLOBAL_PARMS, OnEditGlobalParms)
	ON_COMMAND(ID_EDIT_HOME, OnEditHome)
	ON_COMMAND(ID_EDIT_MASTER_OFFSETS, OnEditMasterOffsets)
	ON_COMMAND(ID_EDIT_NEXT_PATCH_TO_A, OnEditNextPatchToA)
	ON_COMMAND(ID_EDIT_NEXT_PATCH_TO_B, OnEditNextPatchToB)
	ON_COMMAND(ID_EDIT_PAGE_DOWN, OnEditPageDown)
	ON_COMMAND(ID_EDIT_PAGE_UP, OnEditPageUp)
	ON_COMMAND(ID_EDIT_PATCH_FULL, OnEditPatchFull)
	ON_COMMAND(ID_EDIT_PATCH_GEOMETRY, OnEditPatchGeometry)
	ON_COMMAND(ID_EDIT_RANDOM_PATCH, OnEditRandomPatch)
	ON_COMMAND(ID_EDIT_XFADE_LOOP, OnEditXFadeLoop)
	ON_COMMAND(ID_EDIT_XFADE_PLAY, OnEditXFadePlay)
	ON_COMMAND(ID_EDIT_XFADER_A, OnEditXFaderA)
	ON_COMMAND(ID_EDIT_XFADER_B, OnEditXFaderB)
	ON_COMMAND(ID_EDIT_XFADER_MIX, OnEditXFaderMix)
	ON_COMMAND(ID_EDIT_XFADE_TIME_DOWN, OnEditXFadeTimeDown)
	ON_COMMAND(ID_EDIT_XFADE_TIME_UP, OnEditXFadeTimeUp)
	ON_COMMAND(ID_EDIT_ZERO_CONTROLLERS, OnEditZeroControllers)
	ON_COMMAND(ID_FILE_EXPORT_BITMAP, OnFileExportBitmap)
	ON_COMMAND(ID_FILE_LOAD_SNAPSHOT, OnFileLoadSnapshot)
	ON_COMMAND(ID_FILE_MOVIE_EXPORT, OnFileMovieExport)
	ON_COMMAND(ID_FILE_MOVIE_PLAY, OnFileMoviePlay)
	ON_COMMAND(ID_FILE_MOVIE_RECORD, OnFileMovieRecord)
	ON_COMMAND(ID_FILE_MOVIE_RESIZE, OnFileMovieResize)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE_MIX, OnFileSaveMix)
	ON_COMMAND(ID_FILE_TAKE_SNAPSHOT, OnFileTakeSnapshot)
	ON_COMMAND(ID_IMG_CENTER_ORIGIN, OnImgCenterOrigin)
	ON_COMMAND(ID_IMG_CONVEX, OnImgConvex)
	ON_COMMAND(ID_IMG_DAMPING_DOWN, OnImgDampingDown)
	ON_COMMAND(ID_IMG_DAMPING_UP, OnImgDampingUp)
	ON_COMMAND(ID_IMG_DRAG_ORIGIN, OnImgDragOrigin)
	ON_COMMAND(ID_IMG_FILL, OnImgFill)
	ON_COMMAND(ID_IMG_INVERT_COLOR, OnImgInvertColor)
	ON_COMMAND(ID_IMG_INVERT_FILL, OnImgInvertFill)
	ON_COMMAND(ID_IMG_INVERT_OUTLINE, OnImgInvertOutline)
	ON_COMMAND(ID_IMG_INVERT_XRAY, OnImgInvertXray)
	ON_COMMAND(ID_IMG_LOOP_HUE, OnImgLoopHue)
	ON_COMMAND(ID_IMG_MIRROR, OnImgMirror)
	ON_COMMAND(ID_IMG_OUTLINE, OnImgOutline)
	ON_COMMAND(ID_IMG_RANDOM_ORIGIN, OnImgRandomOrigin)
	ON_COMMAND(ID_IMG_RANDOM_PHASE, OnImgRandomPhase)
	ON_COMMAND(ID_IMG_REVERSE, OnImgReverse)
	ON_COMMAND(ID_IMG_ROTATE_HUE, OnImgRotateHue)
	ON_COMMAND(ID_IMG_SET_ORIGIN, OnImgSetOrigin)
	ON_COMMAND(ID_IMG_STEP_BACKWARD, OnImgStepBackward)
	ON_COMMAND(ID_IMG_STEP_FORWARD, OnImgStepForward)
	ON_COMMAND(ID_IMG_TOGGLE_ORIGIN, OnImgToggleOrigin)
	ON_COMMAND(ID_IMG_VIDEO_AUTO_REWIND, OnImgVideoAutoRewind)
	ON_COMMAND(ID_IMG_VIDEO_CYCLE, OnImgVideoCycle)
	ON_COMMAND(ID_IMG_VIDEO_RANDOM_ORIGIN, OnImgVideoRandomOrigin)
	ON_COMMAND(ID_IMG_VIDEO_SELECT_NONE, OnImgVideoSelectNone)
	ON_COMMAND(ID_IMG_XRAY, OnImgXRay)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_NCRBUTTONUP()
	ON_WM_PARENTNOTIFY()
	ON_COMMAND(ID_PL_FILE_NEW, OnPlaylistFileNew)
	ON_COMMAND(ID_PL_FILE_OPEN, OnPlaylistFileOpen)
	ON_COMMAND(ID_PL_FILE_SAVE, OnPlaylistFileSave)
	ON_COMMAND(ID_PL_FILE_SAVE_AS, OnPlaylistFileSaveAs)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_COMMAND(ID_SNAP_EXPORT_ALL, OnSnapExportAll)
	ON_COMMAND(ID_SNAP_SLIDE_SHOW, OnSnapSlideShow)
	ON_COMMAND(ID_TEMPO_DOUBLE, OnTempoDouble)
	ON_COMMAND(ID_TEMPO_HALF, OnTempoHalf)
	ON_COMMAND(ID_TEMPO_NUDGE_DOWN, OnTempoNudgeDown)
	ON_COMMAND(ID_TEMPO_NUDGE_UP, OnTempoNudgeUp)
	ON_COMMAND(ID_TEMPO_RESYNC, OnTempoResync)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DOCUMENT, OnUpdateEditDocument)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GLOBAL_PARMS, OnUpdateEditGlobalParms)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MASTER_OFFSETS, OnUpdateEditMasterOffsets)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PATCH_FULL, OnUpdateEditPatchFull)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PATCH_GEOMETRY, OnUpdateEditPatchGeometry)
	ON_UPDATE_COMMAND_UI(ID_EDIT_XFADE_LOOP, OnUpdateEditXFadeLoop)
	ON_UPDATE_COMMAND_UI(ID_EDIT_XFADE_PLAY, OnUpdateEditXFadePlay)
	ON_UPDATE_COMMAND_UI(ID_EDIT_XFADER_A, OnUpdateEditXFaderA)
	ON_UPDATE_COMMAND_UI(ID_EDIT_XFADER_B, OnUpdateEditXFaderB)
	ON_UPDATE_COMMAND_UI(ID_EDIT_XFADER_MIX, OnUpdateEditXFaderMix)
	ON_UPDATE_COMMAND_UI(ID_FILE_MOVIE_RECORD, OnUpdateFileMovieRecord)
	ON_UPDATE_COMMAND_UI(ID_IMG_CONVEX, OnUpdateImgConvex)
	ON_UPDATE_COMMAND_UI(ID_IMG_DRAG_ORIGIN, OnUpdateImgDragOrigin)
	ON_UPDATE_COMMAND_UI(ID_IMG_FILL, OnUpdateImgFill)
	ON_UPDATE_COMMAND_UI(ID_IMG_INVERT_COLOR, OnUpdateImgInvertColor)
	ON_UPDATE_COMMAND_UI(ID_IMG_LOOP_HUE, OnUpdateImgLoopHue)
	ON_UPDATE_COMMAND_UI(ID_IMG_MIRROR, OnUpdateImgMirror)
	ON_UPDATE_COMMAND_UI(ID_IMG_OUTLINE, OnUpdateImgOutline)
	ON_UPDATE_COMMAND_UI(ID_IMG_RANDOM_ORIGIN, OnUpdateImgRandomOrigin)
	ON_UPDATE_COMMAND_UI(ID_IMG_REVERSE, OnUpdateImgReverse)
	ON_UPDATE_COMMAND_UI(ID_IMG_VIDEO_AUTO_REWIND, OnUpdateImgVideoAutoRewind)
	ON_UPDATE_COMMAND_UI(ID_IMG_VIDEO_CYCLE, OnUpdateImgVideoCycle)
	ON_UPDATE_COMMAND_UI(ID_IMG_VIDEO_RANDOM_ORIGIN, OnUpdateImgVideoRandomOrigin)
	ON_UPDATE_COMMAND_UI(ID_IMG_XRAY, OnUpdateImgXRay)
	ON_UPDATE_COMMAND_UI(ID_PL_FILE_MRU_FILE1, OnUpdatePlaylistFileMru)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AUTOPLAY, OnUpdateViewAutoplay)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CROSSFADER, OnUpdateViewCrossfader)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DEMO, OnUpdateViewDemo)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MASTER, OnUpdateViewMaster)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MIDI_SETUP, OnUpdateViewMidiSetup)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MIX, OnUpdateViewMix)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NUMBERS, OnUpdateViewNumbers)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PARMS, OnUpdateViewParms)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PLAYLIST, OnUpdateViewPlaylist)
	ON_UPDATE_COMMAND_UI(ID_WND_DETACH, OnUpdateWndDetach)
	ON_UPDATE_COMMAND_UI(ID_WND_FULL_SCREEN, OnUpdateWndFullScreen)
	ON_UPDATE_COMMAND_UI(ID_WND_PAUSE, OnUpdateWndPause)
	ON_UPDATE_COMMAND_UI(ID_WND_VEEJAY, OnUpdateWndVeejay)
	ON_UPDATE_COMMAND_UI(ID_WND_ZOOM_CENTER, OnUpdateWndZoomCenter)
	ON_COMMAND(ID_VIEW_AUTOPLAY, OnViewAutoplay)
	ON_COMMAND(ID_VIEW_CROSSFADER, OnViewCrossfader)
	ON_COMMAND(ID_VIEW_DEMO, OnViewDemo)
	ON_COMMAND(ID_VIEW_MASTER, OnViewMaster)
	ON_COMMAND(ID_VIEW_MIDI_SETUP, OnViewMidiSetup)
	ON_COMMAND(ID_VIEW_MIX, OnViewMix)
	ON_COMMAND(ID_VIEW_NUMBERS, OnViewNumbers)
	ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
	ON_COMMAND(ID_VIEW_PARMS, OnViewParms)
	ON_COMMAND(ID_VIEW_PLAYLIST, OnViewPlaylist)
	ON_WM_WINDOWPOSCHANGED()
	ON_COMMAND(ID_WND_CLEAR, OnWndClear)
	ON_COMMAND(ID_WND_DETACH, OnWndDetach)
	ON_COMMAND(ID_WND_FULL_SCREEN, OnWndFullScreen)
	ON_COMMAND(ID_WND_PANIC, OnWndPanic)
	ON_COMMAND(ID_WND_PAUSE, OnWndPause)
	ON_COMMAND(ID_WND_STEP, OnWndStep)
	ON_COMMAND(ID_WND_VEEJAY, OnWndVeejay)
	ON_COMMAND(ID_WND_ZOOM_CENTER, OnWndZoomCenter)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_MIDIIN, OnMidiIn)
	ON_COMMAND_RANGE(ID_PL_FILE_MRU_FILE1, ID_PL_FILE_MRU_FILE4, OnPlaylistFileMru)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TEMPO, OnUpdateIndicatorTempo)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_ORG_MOTION, OnUpdateIndicatorOrgMotion)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_HUE_LOOP, OnUpdateIndicatorHueLoop)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_MIX, OnUpdateIndicatorMix)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_AUTOPLAY, OnUpdateIndicatorAutoplay)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RECORD, OnUpdateIndicatorRecord)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_PAUSE, OnUpdateIndicatorPause)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_PATCH_MODE, OnUpdateIndicatorPatchMode)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FRAME_RATE, OnUpdateIndicatorActualFrameRate)
	ON_MESSAGE(WM_SETTINGCHANGE, OnSettingChange)
	ON_MESSAGE(UWM_PARMROWEDIT, OnParmRowEdit)
	ON_MESSAGE(UWM_NEWDOCUMENT, OnNewDocument)
	ON_MESSAGE(UWM_MASTEREDIT, OnMasterEdit)
	ON_MESSAGE(UWM_MULTIFILESEL, OnMultiFileSel)
	ON_MESSAGE(UWM_VIEWPRINT, OnViewPrint)
	ON_MESSAGE(UWM_SHELLOPEN, OnShellOpen)
	ON_MESSAGE(UWM_TEMPOTAP, OnTempoTap)
	ON_MESSAGE(UWM_AUXFRAMECLOSE, OnAuxFrameClose)
	ON_MESSAGE(UWM_MIDIROWEDIT, OnMidiRowEdit)
	ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
	ON_COMMAND_RANGE(ID_IMG_VIDEO_SELECT0, ID_IMG_VIDEO_SELECT9, OnImgVideoSelect)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_VIDEO_SELECT0, ID_IMG_VIDEO_SELECT9, OnUpdateImgVideoSelect)
	ON_COMMAND_RANGE(ID_IMG_VIDEO_ROP0, ID_IMG_VIDEO_ROP9, OnImgVideoRop)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_VIDEO_ROP0, ID_IMG_VIDEO_ROP9, OnUpdateImgVideoRop)
	ON_COMMAND_RANGE(ID_IMG_VIDEO_CYCLE_LEN0, ID_IMG_VIDEO_CYCLE_LEN9, OnImgVideoCycleLen)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMG_VIDEO_CYCLE_LEN0, ID_IMG_VIDEO_CYCLE_LEN9, OnUpdateImgVideoCycleLen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create toolbar and status bar
	if (!m_Toolbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_Toolbar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	if (!m_StatusBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM) ||
		!m_StatusBar.SetIndicators(m_Indicators,
		  sizeof(m_Indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_Toolbar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_Toolbar);
	LoadBarState(REG_SETTINGS);
	DragAcceptFiles();
	SetMenu(&m_Menu);	// set our own menu object
	CMenu	*EditMenu = m_Menu.GetSubMenu(MENU_EDIT);
	for (int i = 0; i < SELS; i++) {	// get selection names from edit menu
		EditMenu->GetMenuString(m_SelNameID[i], m_SelName[i], MF_BYCOMMAND);
		m_SelName[i].Remove('&');	// remove mnemonic key prefix
	}
	m_View = CWhorldView::GetMainView();	// set by first view creation
	if (m_View == NULL)
		return -1;
	AfxGetApp()->m_pMainWnd = this;	// so components can use AfxGetMainWnd
	// must set parent explicitly, else dialogs won't stay in front
	if (!m_NumbersDlg.Create(IDD_NUMBERS, this))	// order matters here
		return -1;
	if (!m_PlaylistDlg.Create(IDD_PLAYLIST, this))
		return -1;
	if (!m_ParmsDlg.Create(IDD_PARMS, this))
		return -1;
	if (!m_CrossDlg.Create(IDD_CROSS, this))
		return -1;
	if (!m_MasterDlg.Create(IDD_MASTER, this))
		return -1;
	if (!m_MidiSetupDlg.Create(IDD_MIDI_SETUP, this))
		return -1;
	if (m_Tempo.Launch(TimerCallback, this, THREAD_PRIORITY_TIME_CRITICAL))
		UpdateMaster(CMasterDlg::TEMPO);	// set default tempo
	else
		AfxMessageBox(IDS_CANT_LAUNCH_TIMER);
	m_EditInfo = &GetDoc()->m_Patch;
	m_ViewInfo = &GetDoc()->m_Patch;
	UpdateOptions();
	EnableTimer(TRUE);
	PostMessage(UWM_NEWDOCUMENT);
	if (!m_DirInput.Create(m_hWnd,	// create DirectInput instance and acquire mouse
		CDirInput::OPT_MOUSE_ACQUIRE | CDirInput::OPT_SYS_MOUSE_ACCEL,
		THREAD_PRIORITY_ABOVE_NORMAL, NULL, this))
	{
		CString	s;
		AfxFormatString1(s, IDS_DI_CANT_CREATE, m_DirInput.GetLastErrorString());
		AfxMessageBox(s);
	}
	if (!HookKeybd(TRUE))
		AfxMessageBox(IDS_CANT_HOOK_KEYBD);
	m_FrameRateTimer = SetTimer(FRAME_RATE_TIMER_ID, 
		FRAME_RATE_TIMER_PERIOD, FrameTimerProc);
	m_PrevFrameTime = GetTickCount();

	return 0;
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFrameWnd::OnShowWindow(bShow, nStatus);
	if (!m_WasShown && !IsWindowVisible()) {
		m_WasShown = TRUE;
		CPersist::LoadWnd(REG_SETTINGS, this, REG_MAIN_FRAME, CPersist::NO_MINIMIZE);
	}
}

BOOL CMainFrame::DestroyWindow() 
{
	SaveBarState(REG_SETTINGS);
	// if HTML help was initialized, uninitialize it
	if (m_HelpCookie) {
		HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
		HtmlHelp(NULL, NULL, HH_UNINITIALIZE, m_HelpCookie);
	}
	HookKeybd(FALSE);
	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnDestroy() 
{
	CPersist::SaveWnd(REG_SETTINGS, this, REG_MAIN_FRAME);
	CFrameWnd::OnDestroy();
	EnableTimer(FALSE);
	KillTimer(m_FrameRateTimer);
}

void CMainFrame::OnClose() 
{
	if (m_IsVeejay)	// app can't be closed in VJ mode
		return;
	if (m_lpfnCloseProc == NULL) {	// if we're not in print preview
		if (!m_CrossDlg.SaveCheck())
			return;
		if (!m_PlaylistDlg.SaveCheck())
			return;
		Record(FALSE);
		FullScreen(FALSE);	// order matters
		DetachView(FALSE);
		m_Tempo.Run(FALSE);
	}
	CFrameWnd::OnClose();
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
	if (m_CrossDlg.IsPlaying())
		m_CrossDlg.TimerHook();
	GetMouseInput();
	CParmInfo	Info = *m_ViewInfo;
	double	Speed = m_MasterDlg.GetSpeed().GetVal();
	int	i;
	for (i = 0; i < ROWS; i++) {
		Info.m_Row[i].Val += m_MastOfs[i];	// add master offset to parameter
		if (m_MastInfo.m_Row[i].Freq)		// if master frequency is nonzero
			Info.SetOscProps(m_MastInfo, i);	// override oscillator settings
		Info.m_Row[i].Freq *= Speed;	// compensate frequency for master speed
	}
	CWhorldView::PARMS	GlobParm;		// global parameter values
	for (i = 0; i < GLOBAL_PARMS; i++) {
		int	gpi = CParmInfo::m_GlobParm[i];
		CParmInfo::ROW& GlobRow = m_GlobParm.m_Row[gpi];
		if (m_GlobParmTarg[gpi] != GlobRow.Val) {	// if target differs from value
			double delta = GlobRow.Val - m_GlobParmTarg[gpi];
			if (fabs(delta) > DAMP_TOLER)
				GlobRow.Val -= delta * m_MasterDlg.GetDamping().GetVal();
			else	// close enough to target
				GlobRow.Val = m_GlobParmTarg[gpi];	// arrive at target
			if (!m_IsVeejay && m_EditSel == SEL_GLOBALS)
				m_ParmsDlg.SetVal(gpi, GlobRow.Val);
		}
		if (GlobRow.Freq) {
			COscillator&	Osc = m_GlobOsc[gpi];
			Osc.TimerHook();
			((double *)&GlobParm)[gpi] = GlobRow.Val + Osc.GetVal() * GlobRow.Amp;
		} else
			((double *)&GlobParm)[gpi] = GlobRow.Val;
	}
	m_View->TimerHook(Info, GlobParm, Speed * m_GrowDir);	// generate new frame
	if (IsRecording()) {
		if (!(m_Movie.GetFrameCount() & 0x1f)) {	// every 32 frames
			m_KbdLedMask ^= CKeyboardLeds::SCROLL_LOCK;
			m_KbdLeds.Set(m_KbdLedMask);	// toggle scroll lock LED
		}
		m_Movie.Write();	// record current frame
	}
	if (m_NumbersDlg.FastIsVisible())
		m_NumbersDlg.TimerHook();
	if (m_CycleVideo) {
		if (m_Clock != m_PrevClock)
			m_View->GetVideo().SelectNext();
	}
	m_PrevClock = m_Clock;
	m_FrameCount++;
}

LRESULT CMainFrame::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
	if (!wParam) {	// if open or new; we're also called when doc is saved
		m_Snapshot.Unload();
		m_ParmsDlg.SetInfo(*m_EditInfo);
		m_View->FlushHistory();
		if (m_PatchMode == PM_FULL)
			SetPatch(GetDoc()->m_Patch);	// restore master and main settings
		if (m_FirstNewDoc) {
			m_FirstNewDoc = FALSE;	// shut gate first: modal loop allows reentrance
			ShowDemo(TRUE, TRUE);	// prompt for view demo, with never again option
		}
	}
	UpdateParmsDlgTitle();
	return(TRUE);
}

LRESULT CMainFrame::OnParmRowEdit(WPARAM wParam, LPARAM lParam)
{
	m_ParmsDlg.GetRow(wParam, m_EditInfo->m_Row[wParam]);
	switch (m_EditSel) {
	case SEL_DOC:
		GetDoc()->SetModifiedFlag(TRUE);
		break;
	case SEL_OFFSETS:
		m_MastOfs[wParam] = float(m_MastInfo.m_Row[wParam].Val);
		m_PlaylistDlg.SetModify();	// master offsets are stored in playlist
		break;
	case SEL_GLOBALS:
		{
			const CParmInfo::ROW&	GlobRow = m_GlobParm.m_Row[wParam];
			COscillator&	GlobOsc = m_GlobOsc[wParam];
			switch (lParam) {
			case IDC_PM_PARM_EDIT:
				m_GlobParmTarg[wParam] = GlobRow.Val;
				break;
			case IDC_PM_MOD_WAVE:
				GlobOsc.SetWaveform(GlobRow.Wave);
				break;
			case IDC_PM_MOD_FREQ:
				GlobOsc.SetFreq(GlobRow.Freq * m_MasterDlg.GetSpeed().GetVal());
				break;
			case IDC_PM_MOD_PW:
				GlobOsc.SetPulseWidth(GlobRow.PW);
				break;
			}
		}
		m_PlaylistDlg.SetModify();	// globals are stored in playlist
		break;
	default:
		m_CrossDlg.OnParamUpdate(m_EditSel);
	}
	return(TRUE);
}

LRESULT CMainFrame::OnMidiRowEdit(WPARAM wParam, LPARAM lParam)
{
	m_PlaylistDlg.SetModify();
	int	Idx = wParam;
	ASSERT(Idx >= 0 && Idx <= MIDI_ROWS);
	switch (lParam) {
	case IDC_MS_RANGE_START:
	case IDC_MS_RANGE_END:
		if (Idx < CParmInfo::ROWS)
			SetMidiProperty(Idx, m_MidiSetupDlg.GetValue(Idx), FALSE);
		break;
	}
	return(TRUE);
}

void CMainFrame::OnMidiProgChange(int ProgIdx)
{
	m_PlaylistDlg.FadeTo(ProgIdx);
}

LRESULT CMainFrame::OnMasterEdit(WPARAM wParam, LPARAM lParam)
{
	UpdateMaster(wParam);
	return(TRUE);
}

LRESULT	CMainFrame::OnViewPrint(WPARAM wParam, LPARAM lParam)
{
	static const int PRN_DELAYED_END = CWhorldView::PRN_END + 1000;
	CPrintInfo *pInfo = (CPrintInfo *)lParam;
	switch (wParam) {
	case CWhorldView::PRN_PREPARE:
		m_PrePrintTimer = EnableTimer(FALSE);	// stop timer
		if (SnapCount())
			pInfo->SetMaxPage(SnapCount());
		break;
	case CWhorldView::PRN_PRINT:
		if (SnapCount() > 1) {
			LPCTSTR	Path = m_Snapshot.GetPath(pInfo->m_nCurPage - 1);
			if (pInfo->m_bPreview)
				SetCaption(PathFindFileName(Path));
			m_View->Serialize(Path, TRUE);
			// must prevent screen repaint, else pages may be corrupted
			m_View->ValidateRect(NULL);
		}
		break;
	case CWhorldView::PRN_END:
	case CWhorldView::PRN_CANCEL:
		if (SnapCount() > 1)
			ShowSnapshot(GetCurSnap());	// restore current snapshot
		EnableTimer(m_PrePrintTimer);	// restore timer
		// delay restoring caption, else base class overwrites it
		PostMessage(UWM_VIEWPRINT, PRN_DELAYED_END);
		break;
	case PRN_DELAYED_END:
		if (SnapCount())
			SetCaption(PathFindFileName(m_Snapshot.GetCurPath()));
		break;
	}
	return(TRUE);
}

LRESULT	CMainFrame::OnShellOpen(WPARAM wParam, LPARAM lParam)
{
	if (wParam != NULL)
		OpenOtherDoc((LPCTSTR)wParam);
	if (lParam)	// if non-zero, enter veejay mode
		Veejay(TRUE);
	return(TRUE);
}

void CMainFrame::OnFileOpen() 
{
	CFileDialog	fd(TRUE, EXT_PATCH, NULL, OFN_HIDEREADONLY, LDS(IDS_FILTER_MAIN));
	if (fd.DoModal() == IDOK) {
		if (!OpenOtherDoc(fd.GetFileName()))
			AfxGetApp()->OpenDocumentFile(fd.GetFileName());
	}
}

void CMainFrame::OnFileTakeSnapshot() 
{
	if (m_IsVeejay || m_OptsDlg.m_Folder.SnapFolderAlways()) {
		CString	Path;
		if (MakeUniquePath(m_OptsDlg.m_Folder.GetSnapshotFolder(),
		PREFIX_SNAPSHOT, EXT_SNAPSHOT, Path))
			m_Snapshot.Take(Path);
	} else
		m_Snapshot.Take();
}

void CMainFrame::OnFileLoadSnapshot()
{
	m_Snapshot.Load();
}

LRESULT	CMainFrame::OnMultiFileSel(WPARAM wParam, LPARAM lParam)
{
	return(m_Snapshot.OnMultiFileSel(wParam, lParam));
}

void CMainFrame::OnFileExportBitmap() 
{
	TimerState	ts(FALSE);	// stop timer, dtor restores it
	CPathStr	DefName;
	GetDefaultFileName(DefName);
	CMultiFileDlg	fd(FALSE, EXT_BITMAP, DefName, OFN_OVERWRITEPROMPT, 
		LDS(IDS_FILTER_BMP), LDS(IDS_MF_EXPORT));
	if (fd.DoModal() == IDOK && m_ExportDlg.DoModal() == IDOK) {
		CWaitCursor	wc;
		ExportBitmap(fd.GetPathName());
	}
}

void CMainFrame::OnFileSaveMix() 
{
	SaveMix();
}

void CMainFrame::OnFileMovieRecord()
{
	Record(!IsRecording());
}

void CMainFrame::OnUpdateFileMovieRecord(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsRecording());
}

void CMainFrame::OnFileMoviePlay() 
{
	CMoviePlayerDlg::Play();
}

void CMainFrame::OnFileMovieExport()
{
	m_MovieExportDlg.Export();
}

void CMainFrame::OnFileMovieResize() 
{
	CMovieResizeDlg	dlg;
	dlg.DoModal();
}

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	UINT	Files = DragQueryFile(hDropInfo, 0xFFFFFFFF, 0, 0);
	TCHAR	Path[MAX_PATH];
	DragQueryFile(hDropInfo, 0, Path, MAX_PATH);
	CStringArray	SnapPath;
	for (UINT i = 0; i < Files; i++) {
		DragQueryFile(hDropInfo, i, Path, MAX_PATH);
		CString	Ext = PathFindExtension(Path); 
		if (!_tcsicmp(Ext, EXT_SNAPSHOT))
			SnapPath.Add(Path);
		else if (!OpenOtherDoc(Path))
			AfxGetApp()->OpenDocumentFile(Path);
	}
	if (SnapPath.GetSize())
		m_Snapshot.SetList(SnapPath);
}

void CMainFrame::OnUpdateIndicatorTempo(CCmdUI *pCmdUI)
{
	CString	s;
	if (m_OptsDlg.m_Midi.IsSync()) {
		int	i = QUANT_QUARTER + 2 - m_MidiQuant; // remap so 0 = whole, 1 = half, etc
		s.Format(_T("%d/%d"), (i < 0) ? (1 << -i) : 1, (i < 0) ? 1 : (1 << i));
	} else
		s.Format(_T("%.2f"), GetTempo());
	pCmdUI->SetText(s);
}

void CMainFrame::OnUpdateIndicatorOrgMotion(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(LDS(m_OrgMotionID[m_OrgMotion]));
}

void CMainFrame::OnUpdateIndicatorHueLoop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_View->IsHueLooped());
}

void CMainFrame::OnUpdateIndicatorMix(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_ViewSel == SEL_MIX);
}

void CMainFrame::OnUpdateIndicatorAutoplay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_PlaylistDlg.IsPlaying());
}

void CMainFrame::OnUpdateIndicatorRecord(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsRecording());
}

void CMainFrame::OnUpdateIndicatorPause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_IsPaused);
}

void CMainFrame::OnUpdateIndicatorPatchMode(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_PatchMode);
}

void CMainFrame::OnUpdateIndicatorActualFrameRate(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(m_ActualFrameRateStr);
}

void CMainFrame::OnViewParms() 
{
	m_ParmsDlg.ShowWindow(m_ParmsDlg.FastIsVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewParms(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ParmsDlg.FastIsVisible());
}

void CMainFrame::OnViewNumbers()
{
	m_NumbersDlg.ShowWindow(m_NumbersDlg.FastIsVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewNumbers(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_NumbersDlg.FastIsVisible());
}

void CMainFrame::OnViewCrossfader() 
{
	m_CrossDlg.ShowWindow(m_CrossDlg.FastIsVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewCrossfader(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_CrossDlg.FastIsVisible());
}

void CMainFrame::OnViewPlaylist() 
{
	m_PlaylistDlg.ShowWindow(m_PlaylistDlg.FastIsVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewPlaylist(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_PlaylistDlg.FastIsVisible());
}

void CMainFrame::OnViewMaster() 
{
	m_MasterDlg.ShowWindow(m_MasterDlg.FastIsVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewMaster(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MasterDlg.FastIsVisible());
}

void CMainFrame::OnViewMidiSetup() 
{
	m_MidiSetupDlg.ShowWindow(m_MidiSetupDlg.FastIsVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewMidiSetup(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MidiSetupDlg.FastIsVisible());
}

void CMainFrame::OnViewMix() 
{
	SetViewSel(m_ViewSel == SEL_MIX ? SEL_DOC : SEL_MIX);
}

void CMainFrame::OnUpdateViewMix(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ViewSel == SEL_MIX);
}

void CMainFrame::OnViewDemo() 
{
	ShowDemo(!m_DemoMode);
}

void CMainFrame::OnUpdateViewDemo(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_DemoMode);
}

void CMainFrame::OnViewOptions()
{
	if (m_OptsDlg.DoModal() == IDOK)
		UpdateOptions();
}

void CMainFrame::OnEditDocument()
{
	SetEditSel(SEL_DOC);
}

void CMainFrame::OnUpdateEditDocument(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_EditSel == SEL_DOC);
}

void CMainFrame::OnEditXFaderA()
{
	SetEditSel(SEL_A);
}

void CMainFrame::OnUpdateEditXFaderA(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_EditSel == SEL_A);
}

void CMainFrame::OnEditXFaderB()
{
	SetEditSel(SEL_B);
}

void CMainFrame::OnUpdateEditXFaderB(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_EditSel == SEL_B);
}

void CMainFrame::OnEditXFaderMix()
{
	SetEditSel(SEL_MIX);
}

void CMainFrame::OnUpdateEditXFaderMix(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_EditSel == SEL_MIX);
}

void CMainFrame::OnEditMasterOffsets() 
{
	SetEditSel(SEL_OFFSETS);
}

void CMainFrame::OnEditGlobalParms()
{
	SetEditSel(SEL_GLOBALS);
}

void CMainFrame::OnUpdateEditGlobalParms(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_EditSel == SEL_GLOBALS);
}

void CMainFrame::OnUpdateEditMasterOffsets(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_EditSel == SEL_OFFSETS);
}

void CMainFrame::OnEditZeroControllers() 
{
	MidiZeroCtrls();
}

void CMainFrame::OnEditPageDown() 
{
	if (SnapCount())
		ShowSnapshot(GetCurSnap() + 1);
	else
		m_View->GetVideo().SelectNext();
}

void CMainFrame::OnEditPageUp() 
{
	if (SnapCount())
		ShowSnapshot(GetCurSnap() - 1);
	else
		m_View->GetVideo().SelectPrev();
}

void CMainFrame::OnEditHome() 
{
	if (SnapCount())
		ShowSnapshot(0);
	else 
		m_View->GetVideo().Rewind();
}

void CMainFrame::OnEditEnd() 
{
	if (SnapCount())
		ShowSnapshot(SnapCount() - 1);
}

void CMainFrame::OnEditRandomPatch() 
{
	CParmInfo	*a = &m_CrossDlg.GetInfo(SEL_A);
	CParmInfo	*b = &m_CrossDlg.GetInfo(SEL_B);
	if (memcmp(a, b, sizeof(CParmInfo))) {	// if crossfader inputs are different
		m_CrossDlg.RandomPatch();	// set crossfader mix to random patch
		m_View->FlushHistory();	// prevent glitches
	} else {	// crossfader inputs are identical
		if (!m_IsVeejay)	// fail quietly in VJ mode
			AfxMessageBox(IDS_MF_CANT_RANDOM_PATCH);
	}
}

void CMainFrame::OnEditPatchFull() 
{
	SetPatchMode(PM_FULL);
}

void CMainFrame::OnUpdateEditPatchFull(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_PatchMode == PM_FULL);
}

void CMainFrame::OnEditPatchGeometry() 
{
	SetPatchMode(PM_GEOMETRY);
}

void CMainFrame::OnUpdateEditPatchGeometry(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_PatchMode == PM_GEOMETRY);
}

void CMainFrame::OnEditXFadePlay() 
{
	m_CrossDlg.Play(!m_CrossDlg.IsPlaying());
}

void CMainFrame::OnUpdateEditXFadePlay(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_CrossDlg.IsPlaying());
}

void CMainFrame::OnEditXFadeLoop() 
{
	m_CrossDlg.Loop(!m_CrossDlg.IsLooped());
}

void CMainFrame::OnUpdateEditXFadeLoop(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_CrossDlg.IsLooped());
}

void CMainFrame::OnEditNextPatchToA() 
{
	m_PlaylistDlg.LoadNext(SEL_A);
}

void CMainFrame::OnEditNextPatchToB() 
{
	m_PlaylistDlg.LoadNext(SEL_B);
}

void CMainFrame::OnEditXFadeTimeDown()
{
	double	r = GetXFadeFrac() - XFADE_TIME_WHEEL_SENS;
	SetXFadeFrac(CLAMP(r, 0, 1));
}

void CMainFrame::OnEditXFadeTimeUp() 
{
	double	r = GetXFadeFrac() + XFADE_TIME_WHEEL_SENS;
	SetXFadeFrac(CLAMP(r, 0, 1));
}

void CMainFrame::OnImgStepBackward()
{
	if (m_IsPaused) {
		m_View->StepRings(FALSE);
		m_View->Invalidate();
	}
}

void CMainFrame::OnImgStepForward() 
{
	if (m_IsPaused) {
		m_View->StepRings(TRUE);
		m_View->Invalidate();
	}
}

void CMainFrame::OnImgMirror() 
{
	Mirror(!IsMirrored());
}

void CMainFrame::OnUpdateImgMirror(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsMirrored());
}

void CMainFrame::OnImgFill() 
{
	SetFill(!GetFill());
}

void CMainFrame::OnUpdateImgFill(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetFill());
}

void CMainFrame::OnImgOutline() 
{
	SetOutline(!GetOutline());
}

void CMainFrame::OnUpdateImgOutline(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetOutline());
}

void CMainFrame::OnImgXRay() 
{
	SetXRay(!GetXRay());
}

void CMainFrame::OnUpdateImgXRay(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetXRay());
}

void CMainFrame::OnImgInvertColor() 
{
	InvertColor(!IsColorInverted());
	UpdateBars();
}

void CMainFrame::OnUpdateImgInvertColor(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsColorInverted());
}

void CMainFrame::OnImgInvertFill() 
{
	m_View->InvertDrawMode(CWhorldView::DM_FILL);
	UpdateBars();
}

void CMainFrame::OnImgInvertOutline() 
{
	m_View->InvertDrawMode(CWhorldView::DM_OUTLINE);
	UpdateBars();
}

void CMainFrame::OnImgInvertXray() 
{
	m_View->InvertDrawMode(CWhorldView::DM_XRAY);
	UpdateBars();
}

void CMainFrame::OnImgRotateHue()
{
	m_View->RotateHue(90);
}

void CMainFrame::OnImgLoopHue()
{
	LoopHue(!IsHueLooped());
}

void CMainFrame::OnImgToggleOrigin() 
{
	SetOrgMotion(m_OrgMotion == OM_RANDOM ? OM_DRAG : OM_RANDOM);
}

void CMainFrame::OnUpdateImgLoopHue(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_View->IsHueLooped());
}

void CMainFrame::OnImgReverse()
{
	SetReverse(!GetReverse());
}

void CMainFrame::OnUpdateImgReverse(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetReverse());
}

void CMainFrame::OnImgConvex() 
{
	SetConvex(!GetConvex());
	UpdateBars();
}

void CMainFrame::OnUpdateImgConvex(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_View->GetConvex());
}

void CMainFrame::OnImgSetOrigin() 
{
	SetOrgMotion(OM_PARK);
	CPoint	pt;
	GetCursorPos(&pt);
	m_View->ScreenToClient(&pt);
	m_View->SetOrigin(pt);
}

void CMainFrame::OnImgCenterOrigin() 
{
	static const DPoint NormCenter(.5, .5);
	SetOrgMotion(OM_PARK);
	if (m_IsPaused)
		m_View->SetGlobalNormOrigin(NormCenter);
	else
		m_View->SetNormOrigin(NormCenter);
}

void CMainFrame::OnImgDragOrigin() 
{
	SetOrgMotion(m_OrgMotion == OM_DRAG ? OM_PARK : OM_DRAG);
}

void CMainFrame::OnUpdateImgDragOrigin(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_OrgMotion == OM_DRAG);
}

void CMainFrame::OnImgRandomOrigin() 
{
	SetOrgMotion(m_OrgMotion == OM_RANDOM ? OM_PARK : OM_RANDOM);
}

void CMainFrame::OnUpdateImgRandomOrigin(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_OrgMotion == OM_RANDOM);
}

void CMainFrame::OnImgRandomPhase() 
{
	for (int i = 0; i < ROWS; i++)
		m_View->SetPhase(i, double(rand()) / RAND_MAX);
}

void CMainFrame::OnImgDampingDown() 
{
	double	r = GetDampingNorm() - DAMPING_WHEEL_SENS;
	SetDampingNorm(CLAMP(r, 0, 1));
}

void CMainFrame::OnImgDampingUp() 
{
	double	r = GetDampingNorm() + DAMPING_WHEEL_SENS;
	SetDampingNorm(CLAMP(r, 0, 1));
}

void CMainFrame::OnImgVideoSelect(UINT nID)
{
	int	sel = nID - ID_IMG_VIDEO_SELECT0;
	if (m_InputMode & IM_ALT)
		SetVideoCycleLength(sel);
	else
		m_View->GetVideo().Select(sel);
}

void CMainFrame::OnUpdateImgVideoSelect(CCmdUI *pCmdUI)
{
	int	vid = pCmdUI->m_nID - ID_IMG_VIDEO_SELECT0;
	pCmdUI->SetCheck(vid == m_View->GetVideo().GetCurSel());
	pCmdUI->Enable(m_View->GetVideo().IsOpen(vid));
	CString	s;
	s.Format(_T("&%d %s\tNum%d"), vid, 
		PathFindFileName(m_View->GetVideo().GetPath(vid)), vid);
	pCmdUI->SetText(s);
}

void CMainFrame::OnImgVideoSelectNone() 
{
	m_View->GetVideo().Deselect();
}

void CMainFrame::OnImgVideoRop(UINT nID)
{
	int	rop = nID - ID_IMG_VIDEO_ROP0;
	m_View->SetVideoROP(rop);
}

void CMainFrame::OnUpdateImgVideoRop(CCmdUI *pCmdUI)
{
	int	rop = pCmdUI->m_nID - ID_IMG_VIDEO_ROP0;
	pCmdUI->SetCheck(rop == m_View->GetVideoROP());
}

void CMainFrame::OnImgVideoCycle() 
{
	CycleVideo(!m_CycleVideo);
}

void CMainFrame::OnUpdateImgVideoCycle(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_CycleVideo);
}

void CMainFrame::OnImgVideoCycleLen(UINT nID) 
{
	int	len = nID - ID_IMG_VIDEO_CYCLE_LEN0;
	SetVideoCycleLength(len);
}

void CMainFrame::OnUpdateImgVideoCycleLen(CCmdUI* pCmdUI) 
{
	int	len = pCmdUI->m_nID - ID_IMG_VIDEO_CYCLE_LEN0;
	if (!len)	// zero means cycle all videos
		len = CVideoList::MAX_VIDEOS;
	pCmdUI->SetCheck(GetVideoCycleLength() == len);
}

void CMainFrame::OnImgVideoAutoRewind() 
{
	m_View->GetVideo().SetAutoRewind(!m_View->GetVideo().GetAutoRewind());
}

void CMainFrame::OnUpdateImgVideoAutoRewind(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_View->GetVideo().GetAutoRewind());
}

void CMainFrame::OnImgVideoRandomOrigin() 
{
	m_VideoRandOrg ^= 1;
}

void CMainFrame::OnUpdateImgVideoRandomOrigin(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_VideoRandOrg);
}

void CMainFrame::OnWndFullScreen()
{
	FullScreen(!IsFullScreen());
}

void CMainFrame::OnWndVeejay() 
{
	Veejay(!m_IsVeejay);
}

void CMainFrame::OnUpdateWndVeejay(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_IsVeejay);
	pCmdUI->Enable(!IsViewDetached());	// if detached, disable VJ mode
}

void CMainFrame::OnWndPause() 
{
	Pause(!m_IsPaused);
}

void CMainFrame::OnUpdateWndPause(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_IsPaused);
}

void CMainFrame::OnWndClear() 
{
	m_View->ClearScreen();
}

void CMainFrame::OnWndPanic() 
{
	if (SnapCount()) {	// if we're in snapshot mode
		Pause(FALSE);	// just exit snapshot mode
		return;
	}
	Pause(FALSE);	// disable pause
	OnImgCenterOrigin();	// park and center origin
	if (m_IsVeejay)
		SetOrgMotion(OM_DRAG);	// default to drag origin
	else {
		if (!IsViewDetached())	// unless we're dual-monitor
			FullScreen(FALSE);	// exit full-screen mode
	}
	m_View->SetDrawModeAllRings(0);	// make all rings lines
	SetReverse(FALSE);	// make rings grow outward
	InvertColor(FALSE);	// no color inversion
	SetSpeedNorm(0);	// default speed
	SetZoomNorm(0);		// default zoom
	SetDampingNorm(.5);	// default damping
	SetTrailNorm(0);	// default trail
	SetRingsNorm(1);	// no ring limit
	SetCopies(1);		// one instance
	MidiZeroCtrls();	// reset all MIDI controllers
	m_View->FlushHistory();	// prevent glitching
	CycleVideo(FALSE);	// disable video cycling
	m_View->GetVideo().Deselect();	// disable video
	m_View->ClearScreen();	// delete all rings
}

void CMainFrame::OnWndStep() 
{
	if (m_IsPaused)
		OnTimer(TIMER_ID);
	else
		Pause(TRUE);
}

void CMainFrame::OnWndZoomCenter() 
{
	SetZoomCenter(!GetZoomCenter());
}

void CMainFrame::OnUpdateWndZoomCenter(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(GetZoomCenter());
}

void CMainFrame::OnWndDetach() 
{
	DetachView(!IsViewDetached());
}

void CMainFrame::OnUpdateWndDetach(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsViewDetached());
}

void CMainFrame::OnUpdateWndFullScreen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsFullScreen());
}

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	MSG	msg = {m_hWnd, WM_KEYDOWN, nChar, MAKELONG(nRepCnt, nFlags)};
	if (TranslateAccelerator(m_hWnd, m_FocusAccel, &msg))
		return;
	m_PlaylistDlg.HandleHotKey(nChar);	// assume playlist hot key
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMainFrame::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	MSG	msg = {m_hWnd, WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags)};
	if (TranslateAccelerator(m_hWnd, m_FocusAccel, &msg))
		return;
	CFrameWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CMainFrame::OnSnapSlideShow() 
{
	m_Snapshot.SlideShow();
}

void CMainFrame::OnSnapExportAll() 
{
	CString	Folder;
	CString	Title((LPCTSTR)ID_SNAP_EXPORT_ALL);
	Title.Replace('\n', 0);	// use first line only
	if (CFolderDialog::BrowseFolder(Title, Folder) && m_ExportDlg.DoModal() == IDOK) {
		CProgressDlg	pd;
		pd.Create();
		pd.SetRange(0, SnapCount());
		for (int i = 0; i < SnapCount(); i++) {
			pd.SetPos(i);
			ShowSnapshot(i);
			CPathStr	Path(Folder);
			Path.Append(PathFindFileName(m_Snapshot.GetPath(i)));
			Path.RenameExtension(EXT_BITMAP);
			ExportBitmap(Path);
			if (pd.Canceled())
				break;
		}
	}
}

void CMainFrame::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_IsPaused) {
		if (m_OrgMotion == OM_DRAG) {
			if (m_IsCursorValid) {
				MapWindowPoints(m_View, &point, 1);	// map from frame to view
				m_View->SetGlobalOrigin(point);
			} else {	// first mouse move after pause
				CPoint	pt = m_View->GetOrigin();
				m_View->ClientToScreen(&pt);
				SetCursorPos(pt.x, pt.y);	// update windows cursor from origin
				m_IsCursorValid = TRUE;
			}
		}
	}
	CFrameWnd::OnMouseMove(nFlags, point);
}

BOOL CMainFrame::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	switch (m_InputMode) {
	case 0:
		{
			double	r = GetDampingNorm() + zDelta / 120.0 
				* DAMPING_WHEEL_SENS * m_OptsDlg.m_Input.GetWheelSens();
			SetDampingNorm(CLAMP(r, 0, 1));
		}
		break;
	case IM_CONTROL:
		{
			double	r = GetXFadeFrac() + zDelta / 120.0 
				* XFADE_TIME_WHEEL_SENS * m_OptsDlg.m_Input.GetWheelSens();
			SetXFadeFrac(CLAMP(r, 0, 1));
		}
		break;
	case IM_SHIFT:
		{
			double	r = GetTrailNorm() + zDelta / 120.0 
				* TRAIL_WHEEL_SENS * m_OptsDlg.m_Input.GetWheelSens();
			SetTrailNorm(CLAMP(r, 0, 1));
		}
	}
	return CFrameWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CMainFrame::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CFrameWnd::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_FRAMECHANGED) {
		if (IsIconic()) {	// if minimized, kill timer if we didn't already
			if (m_PreIconicTimer < 0 && !IsViewDetached())
				m_PreIconicTimer = EnableTimer(FALSE);
		} else {	// if a pre-minimize timer state is available, restore it
			if (m_PreIconicTimer >= 0) {
				EnableTimer(m_PreIconicTimer > 0);
				m_PreIconicTimer = -1;	// invalid timer state
			}
		}
	}
}

void CMainFrame::OnTempoNudgeUp() 
{
	SetTempo(GetTempo() * (1 + m_OptsDlg.m_Input.GetTempoNudge()));
	m_Tempo.Resync();
}

void CMainFrame::OnTempoNudgeDown() 
{	
	SetTempo(GetTempo() * (1 - m_OptsDlg.m_Input.GetTempoNudge()));
	m_Tempo.Resync();
}

void CMainFrame::OnTempoResync() 
{
	m_Tempo.Resync();
	m_MidiClock = 0;
}

void CMainFrame::OnTempoDouble() 
{
	if (m_OptsDlg.m_Midi.IsSync()) {
		if (m_MidiQuant > 0)
			m_MidiQuant--;
	} else {
		if (m_TempoMult < MAX_TEMPO_DOUBLINGS) {
			SetTempoMult(m_TempoMult + 1);
			SetTempo(GetTempo() * 2);
		}
	}
	UpdateTempo();
}

void CMainFrame::OnTempoHalf() 
{
	if (m_OptsDlg.m_Midi.IsSync()) {
		if (m_MidiQuant < MIDI_QUANTS - 1)
			m_MidiQuant++;
	} else {
		if (m_TempoMult > -MAX_TEMPO_DOUBLINGS) {
			SetTempoMult(m_TempoMult - 1);
			SetTempo(GetTempo() / 2);
		}
	}
	UpdateTempo();
}

LRESULT	CMainFrame::OnTempoTap(WPARAM wParam, LPARAM lParam)
{
	if (ViewHasFocus()) {
		float	Delta = INT_TO_FLOAT(wParam);	// assume wParam is a float
		if (Delta > 0) {	// avoid divide by zero
			double	Freq = 1.0 / Delta;
			double	PrevFreq = m_Tempo.GetFreq();
			// if new sample is close to previous sample, use averaging
			if (fabs(Freq - PrevFreq) < PrevFreq * TAP_AVG_TOLERANCE) {
				m_TapAvg.Update(Freq);
				Freq = m_TapAvg.GetAvg();
			} else	// sample is too different, so flush the history
				m_TapAvg.Reset();
			SetTempo(Freq * 60.0, TRUE);	// resync
			SetTempoMult(0);
		}
	}
	return(TRUE);
}

LRESULT	CMainFrame::OnAuxFrameClose(WPARAM wParam, LPARAM lParam)
{
	DetachView(FALSE);
	return(TRUE);
}

void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	CFrameWnd::OnActivateApp(bActive, hTask);
	if (!bActive) {	// cancel input modes so they don't get stuck
		ReleaseCapture();
		m_InputMode = 0;
		if (IsViewDetached() && IsExclusive())	// if dual-monitor and exclusive
			m_AuxFrm->FullScreen(FALSE);	// restore cooperative mode
	}
}

void CMainFrame::OnViewAutoplay() 
{
	SetAutoplay(!GetAutoplay());
}

void CMainFrame::OnUpdateViewAutoplay(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_PlaylistDlg.IsPlaying());
}

void CMainFrame::OnParentNotify(UINT message, LPARAM lParam)
{
	if (m_View != NULL && m_lpfnCloseProc == NULL) {	// exclude print preview
		CRect	r;
		m_View->GetWindowRect(r);
		ScreenToClient(r);
		CPoint	pos(LOWORD(lParam), HIWORD(lParam));
		if (r.PtInRect(pos)) {	// if mouse is within view
			switch (message) {
			case WM_LBUTTONDOWN:
				m_InputMode |= IM_LBUTTON;
				SetCapture();
				break;
			case WM_RBUTTONDOWN:
				m_InputMode |= IM_RBUTTON;
				SetCapture();
				break;
			case WM_MBUTTONDOWN:
				m_InputMode |= IM_MBUTTON;
				SetCapture();
				break;
			}
		}
	}
	CFrameWnd::OnParentNotify(message, lParam);
}

void CMainFrame::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_InputMode |= IM_LBUTTON;
	SetCapture();
	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CMainFrame::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_InputMode |= IM_RBUTTON;
	SetCapture();
	CFrameWnd::OnRButtonDown(nFlags, point);
}

void CMainFrame::OnMButtonDown(UINT nFlags, CPoint point) 
{
	m_InputMode |= IM_MBUTTON;
	SetCapture();
	CFrameWnd::OnMButtonDown(nFlags, point);
}

void CMainFrame::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_InputMode = 0;
	ReleaseCapture();
	CFrameWnd::OnLButtonUp(nFlags, point);
}

void CMainFrame::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_InputMode = 0;
	ReleaseCapture();
	CFrameWnd::OnRButtonUp(nFlags, point);
}

void CMainFrame::OnMButtonUp(UINT nFlags, CPoint point) 
{
	m_InputMode = 0;
	ReleaseCapture();
	CFrameWnd::OnMButtonUp(nFlags, point);
}

LRESULT CMainFrame::OnSettingChange(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case SPI_SETMOUSE:
		m_DirInput.GetSystemMouseAccel();	// update mouse acceleration params
		break;
	}
	return Default();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	if (m_View != NULL) {
		DPoint	PrvOrg, NewOrg;
		m_View->GetNormOrigin(PrvOrg);
		CFrameWnd::OnSize(nType, cx, cy);
		m_View->GetNormOrigin(NewOrg);
		m_TargetOrg += NewOrg - PrvOrg;	// adjust target for origin shift
	} else
		CFrameWnd::OnSize(nType, cx, cy);
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (m_IsVeejay) {	// disable disruptive commands
		UINT	CmdID = LOWORD(wParam);
		for (int i = 0; m_VJCmdDisable[i]; i++) {
			if (CmdID == m_VJCmdDisable[i])
				return(TRUE);
		}
	}
	return CFrameWnd::OnCommand(wParam, lParam);
}

void CMainFrame::OnPlaylistFileNew() 
{
	m_PlaylistDlg.New();
}

void CMainFrame::OnPlaylistFileOpen() 
{
	m_PlaylistDlg.Open();
}

void CMainFrame::OnPlaylistFileSave() 
{
	m_PlaylistDlg.Save();
}

void CMainFrame::OnPlaylistFileSaveAs() 
{
	m_PlaylistDlg.SaveAs();
}

void CMainFrame::OnPlaylistFileMru(UINT nID) 
{
	m_PlaylistDlg.OpenRecent(nID - ID_PL_FILE_MRU_FILE1);
}

void CMainFrame::OnUpdatePlaylistFileMru(CCmdUI* pCmdUI) 
{
	m_PlaylistDlg.UpdateRecentFileMenu(pCmdUI);
}

void CMainFrame::WinHelp(DWORD dwData, UINT nCmd) 
{
	static const LPCTSTR	HELP_FILE_NAME = _T("whorld.chm");
	// if HTML help hasn't been initialized yet, initialize it
	if (!m_HelpCookie)
		HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&m_HelpCookie);
	HWND retc = HtmlHelp(m_hWnd, HELP_FILE_NAME, HH_DISPLAY_TOC, 0);
	if (!retc) {	// not found, try appending help file name to app path
		CPathStr	HelpPath = GetAppPath();
		HelpPath.Append(HELP_FILE_NAME);
		retc = HtmlHelp(m_hWnd, HelpPath, HH_DISPLAY_TOC, 0);	// try again
		if (!retc) {	// not found, give up
			CString	s;
			AfxFormatString1(s, IDS_MF_HELP_FILE_MISSING, HELP_FILE_NAME);
			AfxMessageBox(s);
		}
	}
}

void CMainFrame::OnHelpIndex()
{
	if (m_IsVeejay)
		OnImgToggleOrigin();	// in VJ mode, F1 toggles origin motion
	else
		CFrameWnd::OnHelpIndex();
}

void CMainFrame::FrameTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	CMainFrame::GetThis()->OnFrameTimer(dwTime);
}

void CMainFrame::OnFrameTimer(DWORD dwTime)
{
	double	dt = double(dwTime) - m_PrevFrameTime;	// delta time
	int	df = m_FrameCount - m_PrevFrameCount;	// delta frames
	m_ActualFrameRate = df / dt * 1000;
	m_ActualFrameRateStr.Format(_T("%.2f"), m_ActualFrameRate);
	m_StatusBar.SetPaneText(SBP_ACTUAL_FRAME_RATE, m_ActualFrameRateStr);
	m_PrevFrameCount = m_FrameCount;
	m_PrevFrameTime = dwTime;
}

void CMainFrame::OnNcRButtonDown(UINT nHitTest, CPoint point)
{
	switch (nHitTest) {
	case HTCAPTION:
		break;	// do nothing, and display context menu in OnNcRButtonUp
	default:
		CFrameWnd::OnNcRButtonDown(nHitTest, point);
	}
}

void CMainFrame::OnNcRButtonUp(UINT nHitTest, CPoint point)
{
	switch (nHitTest) {
	case HTCAPTION:
		SendMessage(WM_CONTEXTMENU, (LONG)m_hWnd, MAKELONG(point.x, point.y));
		break;
	default:
		CFrameWnd::OnNcRButtonUp(nHitTest, point);
	}
}
