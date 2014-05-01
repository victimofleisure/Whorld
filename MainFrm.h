// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		20feb06	add master parameters to info struct
		02		03mar06	add convex
        03      04mar06	add trail
		04		23mar06	add Hue, LoopHue, InvertColor, Pause to INFO
		05		28mar06	add ZoomCenter
		06		17apr06	add CPatch
		07		18apr06	add patch modes
		08		03may06	add GetMovieExport
		09		06may06	add video raster operations
		10		09may06	add video cycling
		11		12may06	add atomic bank/patch select
		12		16may06	make video cycling state public
		13		22may06	add video list info
		14		02jun06	add OpenVideo
		15		05jun06	add video auto-rewind
		16		17jun06	stop recording in OnClose
		17		21jun06	rename some MasterDlg accessors
		18		24jun06	add Copies and Spread
		19		29jun06	add video origin
		20		10jul06	add video random origin
		21		10dec07	add global parameters
		22		15dec07	add MIDI drag origin mode
		23		21dec07	make IsFullScreen inline
		24		21dec07	replace AfxGetMainWnd with m_This
		25		22jan08	simplify MIDI message handling
		26		23jan08	replace MIDI range scaler with start/end
		27		28jan08	support Unicode
		28		30jan08	add extra keyboard accelerators when we have focus
		29		30jan08	show actual frame rate in status bar

        Whorld main frame window
 
*/

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__F54EFF6B_1538_48D8_83C5_2B526B2A8248__INCLUDED_)
#define AFX_MAINFRM_H__F54EFF6B_1538_48D8_83C5_2B526B2A8248__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ParmsDlg.h"
#include "NumbersDlg.h"
#include "CrossDlg.h"
#include "PlaylistDlg.h"
#include "MasterDlg.h"
#include "ExportDlg.h"
#include "OptionsDlg.h"
#include "RealTimer.h"
#include "RunAvg.h"
#include "DirInput.h"
#include "Snapshot.h"
#include "MidiSetupDlg.h"
#include "MidiIO.h"
#include "WhorldView.h"
#include "SnapMovie.h"
#include "MovieExportDlg.h"
#include "KeyboardLeds.h"

class CWhorldDoc;
class CAuxFrame;

class CMainFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CMainFrame)
protected: // create from serialization only
	CMainFrame();

// Constants
public:
	enum {	// main menus; must match menu resource
		MENU_FILE,
		MENU_EDIT,
		MENU_SNAPSHOT = MENU_EDIT,	// snapshot menu is dynamic
		MENU_IMAGE,
		MENU_VIEW,
		MENU_WINDOW,
		MENU_HELP
	};
	enum {	// origin motion types
		OM_PARK,	// no motion
		OM_DRAG,	// cursor drag mode
		OM_RANDOM,	// random jump mode
		OM_MIDI_DRAG	// MIDI drag mode
	};
	enum {	// patch change modes
		PM_FULL,	// affects all settings
		PM_GEOMETRY	// affects geometry only
	};
	enum {	// selection indices: first four are shared with CCrossDlg
		SEL_DOC,	// active document
		SEL_A,		// crossfader input A
		SEL_B,		// crossfader input B
		SEL_MIX,	// crossfader output mix
		SEL_OFFSETS,	// master offsets
		SEL_GLOBALS,	// global parameters
		SELS		// number of selections
	};
	enum {	// input modifiers
		IM_SHIFT	= 0x01,
		IM_CONTROL	= 0x02,
		IM_ALT		= 0x04,
		IM_LBUTTON	= 0x08,
		IM_RBUTTON	= 0x10,
		IM_MBUTTON	= 0x20,
		IM_KEYS		= IM_SHIFT | IM_CONTROL | IM_ALT,
		IM_BUTTONS	= IM_LBUTTON | IM_RBUTTON | IM_MBUTTON
	};

// Types
	typedef CPatch::MAIN MAIN_INFO;
	typedef CPatch::MASTER MASTER_INFO;
	typedef struct tagINFO {	// persistent information including components
		CCrossDlg::INFO		XFader;		// crossfader dialog info
		CPlaylistDlg::INFO	Playlist;	// playlist dialog info
		CMidiSetupDlg::INFO	MidiSetup;	// MIDI setup dialog info
		CVideoList::INFO	VideoList;	// video list info
		CPatch	Patch;		// master patch
		CParmInfo	GlobParm;	// global parameters
		int		ViewSel;	// view selection: document or mix
		int		EditSel;	// edit selection: document, A, B, mix, master offsets
		int		PatchMode;	// patch change mode; see enum above
	} INFO;

// Nested classes
	class TimerState {
	public:
		TimerState(bool NewState);
		~TimerState();
	private:
		bool	m_PrevState;
	};

// Attributes
	static	CRuntimeClass *GetViewClass();
	CParmsDlg&	GetParms();
	CCrossDlg&	GetXFader();
	CMasterDlg&	GetMaster();
	CPlaylistDlg&	GetPlaylist();
	CMidiSetupDlg&	GetMidiSetup();
	COptionsDlg&	GetOptions();
	CMovieExportDlg&	GetMovieExport();
	static	CMainFrame *GetThis();
	CWhorldDoc	*GetDoc() const;
	CWhorldView	*GetView() const;
	void	GetMasterInfo(MASTER_INFO& Master) const;
	void	SetMasterInfo(const MASTER_INFO& Master);
	void	GetMainInfo(MAIN_INFO& Main) const;
	void	SetMainInfo(const MAIN_INFO& Main);
	void	GetPatch(CPatch& Patch) const;
	void	SetPatch(const CPatch& Patch);
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);
	void	GetDefaults(INFO& Info) const;
	void	SetViewSel(int Sel);
	int		GetViewSel() const;
	void	SetEditSel(int Sel);
	int		GetEditSel() const;
	void	SetDrawMode(int Mode);
	int		GetDrawMode() const;
	void	Mirror(bool Enable);
	bool	IsMirrored() const;
	void	LoopHue(bool Enable);
	bool	IsHueLooped() const;
	void	InvertColor(bool Enable);
	bool	IsColorInverted() const;
	int		SnapCount() const;
	int		GetCurSnap() const;
	void	SetCaption(LPCTSTR Filename);
	double	GetSpeedNorm() const;
	void	SetSpeedNorm(double Speed);
	double	GetZoomNorm() const;
	void	SetZoomNorm(double Zoom);
	void	SetZoomNormDamped(double Zoom);
	double	GetDampingNorm() const;
	void	SetDampingNorm(double Damping);
	double	GetTrailNorm() const;
	void	SetTrailNorm(double Damping);
	double	GetRingsNorm() const;
	void	SetRingsNorm(double Rings);
	void	SetTempo(double Tempo, bool Resync = FALSE);
	double	GetTempo();
	void	SetTempoMult(int Mult);
	int		GetTempoMult(int Mult) const;
	void	SetOrgMotion(int Motion);
	int		GetOrgMotion() const;
	void	SetReverse(bool Enable);
	bool	GetReverse() const;
	void	SetConvex(bool Enable);
	bool	GetConvex() const;
	void	SetAutoplay(bool Enable);
	bool	GetAutoplay() const;
	void	SetXFadeFrac(double Frac);
	double	GetXFadeFrac() const;
	CMenu	*GetMenu();
	double	GetTimerFreq() const;
	LPCTSTR	GetSelName(int Sel) const;
	void	SetFill(bool Enable);
	bool	GetFill() const;
	void	SetOutline(bool Enable);
	bool	GetOutline() const;
	void	SetXRay(bool Enable);
	bool	GetXRay() const;
	DWORD	GetInputMode() const;
	void	GetDefaultFileName(CString& Name);
	void	SetZoomCenter(bool Enable);
	bool	GetZoomCenter() const;
	void	SetPatchMode(int Mode);
	int		GetPatchMode() const;
	void	SetHueLoopLength(double Length);
	double	GetHueLoopLength() const;
	void	SetCanvasScale(double Scale);
	double	GetCanvasScale() const;
	void	SetCopies(double Copies);
	double	GetCopies() const;
	void	SetSpread(double Spread);
	double	GetSpread() const;
	void	CycleVideo(bool Enable);
	bool	IsVideoCycling() const;
	void	SetVideoCycleLength(int Length);
	int		GetVideoCycleLength() const;

// Operations
	void	RestoreDefaults();
	bool	EnableTimer(bool Enable);
	void	Pause(bool Enable);
	bool	IsPaused() const;
	void	FullScreen(bool Enable);
	bool	IsFullScreen() const;
	void	SetExclusive(bool Enable);
	bool	IsExclusive() const;
	bool	AllowExclusive() const;
	void	DetachView(bool Enable);
	bool	IsViewDetached() const;
	void	Veejay(bool Enable);
	bool	IsVeejay() const;
	void	Record(bool Enable);
	bool	IsRecording() const;
	void	ShowDemo(bool Enable, bool Prompt = FALSE);
	bool	ShowSnapshot(int SnapIdx);
	bool	ExportBitmap(LPCTSTR Path);
	bool	OpenPlaylist(LPCTSTR Path);
	bool	OpenVideo(LPCTSTR Path);
	bool	OpenOtherDoc(LPCTSTR Path);
	void	MidiZeroCtrls();
	bool	SaveMix();

// Handlers
	void	OnNewParms(int Sel);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	// This list is kept in alphabetical order EXCEPT OnTimer comes first
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnEditDocument();
	afx_msg void OnEditEnd();
	afx_msg void OnEditGlobalParms();
	afx_msg void OnEditHome();
	afx_msg void OnEditMasterOffsets();
	afx_msg void OnEditNextPatchToA();
	afx_msg void OnEditNextPatchToB();
	afx_msg void OnEditPageDown();
	afx_msg void OnEditPageUp();
	afx_msg void OnEditPatchFull();
	afx_msg void OnEditPatchGeometry();
	afx_msg void OnEditRandomPatch();
	afx_msg void OnEditXFadeLoop();
	afx_msg void OnEditXFadePlay();
	afx_msg void OnEditXFaderA();
	afx_msg void OnEditXFaderB();
	afx_msg void OnEditXFaderMix();
	afx_msg void OnEditXFadeTimeDown();
	afx_msg void OnEditXFadeTimeUp();
	afx_msg void OnEditZeroControllers();
	afx_msg void OnFileExportBitmap();
	afx_msg void OnFileLoadSnapshot();
	afx_msg void OnFileMovieExport();
	afx_msg void OnFileMoviePlay();
	afx_msg void OnFileMovieRecord();
	afx_msg void OnFileMovieResize();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSaveMix();
	afx_msg void OnFileTakeSnapshot();
	afx_msg void OnImgCenterOrigin();
	afx_msg void OnImgConvex();
	afx_msg void OnImgDampingDown();
	afx_msg void OnImgDampingUp();
	afx_msg void OnImgDragOrigin();
	afx_msg void OnImgFill();
	afx_msg void OnImgInvertColor();
	afx_msg void OnImgInvertFill();
	afx_msg void OnImgInvertOutline();
	afx_msg void OnImgInvertXray();
	afx_msg void OnImgLoopHue();
	afx_msg void OnImgMirror();
	afx_msg void OnImgOutline();
	afx_msg void OnImgRandomOrigin();
	afx_msg void OnImgRandomPhase();
	afx_msg void OnImgReverse();
	afx_msg void OnImgRotateHue();
	afx_msg void OnImgSetOrigin();
	afx_msg void OnImgStepBackward();
	afx_msg void OnImgStepForward();
	afx_msg void OnImgToggleOrigin();
	afx_msg void OnImgVideoAutoRewind();
	afx_msg void OnImgVideoCycle();
	afx_msg void OnImgVideoRandomOrigin();
	afx_msg void OnImgVideoSelectNone();
	afx_msg void OnImgXRay();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnPlaylistFileNew();
	afx_msg void OnPlaylistFileOpen();
	afx_msg void OnPlaylistFileSave();
	afx_msg void OnPlaylistFileSaveAs();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSnapExportAll();
	afx_msg void OnSnapSlideShow();
	afx_msg void OnTempoDouble();
	afx_msg void OnTempoHalf();
	afx_msg void OnTempoNudgeDown();
	afx_msg void OnTempoNudgeUp();
	afx_msg void OnTempoResync();
	afx_msg void OnUpdateEditDocument(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditGlobalParms(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditMasterOffsets(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPatchFull(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPatchGeometry(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditXFadeLoop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditXFadePlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditXFaderA(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditXFaderB(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditXFaderMix(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileMovieRecord(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgConvex(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgDragOrigin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgFill(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgInvertColor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgLoopHue(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgMirror(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgOutline(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgRandomOrigin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgReverse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgVideoAutoRewind(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgVideoCycle(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgVideoRandomOrigin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateImgXRay(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlaylistFileMru(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewAutoplay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewCrossfader(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewDemo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMaster(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMidiSetup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMix(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewNumbers(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewParms(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewPlaylist(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWndDetach(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWndFullScreen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWndPause(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWndVeejay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWndZoomCenter(CCmdUI* pCmdUI);
	afx_msg void OnViewAutoplay();
	afx_msg void OnViewCrossfader();
	afx_msg void OnViewDemo();
	afx_msg void OnViewMaster();
	afx_msg void OnViewMidiSetup();
	afx_msg void OnViewMix();
	afx_msg void OnViewNumbers();
	afx_msg void OnViewOptions();
	afx_msg void OnViewParms();
	afx_msg void OnViewPlaylist();
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnWndClear();
	afx_msg void OnWndDetach();
	afx_msg void OnWndFullScreen();
	afx_msg void OnWndPanic();
	afx_msg void OnWndPause();
	afx_msg void OnWndStep();
	afx_msg void OnWndVeejay();
	afx_msg void OnWndZoomCenter();
	//}}AFX_MSG
	afx_msg LRESULT OnMidiIn(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPlaylistFileMru(UINT nID);
	afx_msg void OnUpdateIndicatorTempo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorOrgMotion(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorHueLoop(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorMix(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorAutoplay(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorRecord(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorPause(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorPatchMode(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorActualFrameRate(CCmdUI *pCmdUI);
	afx_msg LRESULT	OnSettingChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnParmRowEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMasterEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnMultiFileSel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnViewPrint(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnShellOpen(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnTempoTap(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnAuxFrameClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMidiRowEdit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelpIndex();
	afx_msg void OnImgVideoSelect(UINT nID);
	afx_msg void OnUpdateImgVideoSelect(CCmdUI *pCmdUI);
	afx_msg void OnImgVideoRop(UINT nID);
	afx_msg void OnUpdateImgVideoRop(CCmdUI *pCmdUI);
	afx_msg void OnImgVideoCycleLen(UINT nID);
	afx_msg void OnUpdateImgVideoCycleLen(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// status bar panes; must match array of indicator IDs
		SBP_TEXT,
		SBP_TEMPO,
		SBP_ORG_MOTION,
		SBP_HUE_LOOP,
		SBP_MIX,
		SBP_AUTOPLAY,
		SBP_RECORD,
		SBP_PAUSE,
		SBP_PATCH_MODE,
		SBP_ACTUAL_FRAME_RATE,
	};
	enum {	// mouse modes
		MM_ZOOM			= IM_LBUTTON,
		MM_CYCLE_HUE	= IM_RBUTTON,
		MM_SPEED		= IM_MBUTTON,
		MM_RINGS		= IM_ALT,
		MM_VIDEO_ORG	= IM_SHIFT
	};
	enum {	// normalized ranges
		SPEED_RANGE		= 2,	// -1..1
		ZOOM_RANGE		= 2,	// -1..1
		HUE_RANGE		= 360,	// 0..360
		DAMPING_RANGE	= 1		// 0..1
	};
	enum {	// MIDI channel messages
		MC_NOTE_OFF		= 0x80,
		MC_NOTE_ON		= 0x90,
		MC_KEY_AFT		= 0xa0,
		MC_CTRL_CHG		= 0xb0,
		MC_PROG_CHG		= 0xc0,
		MC_CHAN_AFT		= 0xd0,
		MC_PITCH_BEND	= 0xe0
	};
	enum {	// MIDI-related constants
		MIDI_CLOCK_RATE = 24,		// MIDI clocks per quarter note
		MIDI_QUANTS = 7,			// number of quantizations in MIDI sync mode
		QUANT_QUARTER = 2,			// which quantization equals a quarter note
		QUANT_DEFAULT = QUANT_QUARTER + 2,	// default quantization (whole note)
		MIDI_ROWS = CMidiSetupDlg::ROWS		// number of MIDI property rows
	};
	enum {	// miscellaneous MIDI properties
		#define MIDI_PROP(name, tag, start, end, ctrl) MP_##tag,
		#include "MidiProps.h"
		MISC_PROPS
	};
	enum {	// miscellaneous constants
		TIMER_ID = 1,				// identifies our timer instance
		FRAME_RATE_TIMER_ID = 2,	// identifies frame rate timer
		FRAME_RATE_TIMER_PERIOD = 1000,	// how often to update frame rate, in ms
		ROWS = CParmInfo::ROWS,		// number of parameter rows
		TAP_AVG_HISTORY_SIZE = 4,	// max samples for tempo tap averaging
		MAX_TEMPO_DOUBLINGS = 3,		// limits tempo doubling/halving
		GLOBAL_PARMS = CParmInfo::GLOBAL_PARMS	// number of global parameters
	};
	static const double	TAP_AVG_TOLERANCE;
	static const double	SPEED_MOUSE_SENS;
	static const double	ZOOM_MOUSE_SENS;
	static const double	HUE_MOUSE_SENS;
	static const double	RINGS_MOUSE_SENS;
	static const double	VIDEO_ORG_MOUSE_SENS;
	static const double	DAMPING_WHEEL_SENS;
	static const double	XFADE_TIME_WHEEL_SENS;
	static const double	TRAIL_WHEEL_SENS;
	static const LPCTSTR	m_DemoPlaylist;		// app-relative path to demo playlist
	static const UINT	m_Indicators[];		// string IDs for status bar indicators
	static const UINT	m_OrgMotionID[];	// string IDs for origin motion types
	static const UINT	m_SelNameID[SELS];	// string IDs for selection names
	static const UINT	m_VJCmdDisable[];	// list of commands disabled in VJ mode
	static const double	DAMP_TOLER;

// Member data
	static	CMainFrame *m_This;	// for finding our one and only instance
	CToolBar	m_Toolbar;		// standard toolbar
	CStatusBar	m_StatusBar;	// standard status bar
	CParmsDlg	m_ParmsDlg;		// parameters dialog
	CNumbersDlg	m_NumbersDlg;	// numbers dialog
	CCrossDlg	m_CrossDlg;		// crossfader dialog
	CPlaylistDlg	m_PlaylistDlg;	// playlist dialog
	CExportDlg	m_ExportDlg;	// export dialog
	COptionsDlg	m_OptsDlg;		// options property sheet
	CMasterDlg	m_MasterDlg;	// master dialog
	CMidiSetupDlg	m_MidiSetupDlg;	// MIDI setup dialog
	CSnapshot	m_Snapshot;		// snapshot container
	CRealTimer	m_Tempo;		// self-correcting timer based on performance counter
	CRunAvg<double>	m_TapAvg;	// running average of tempo taps
	CDirInput	m_DirInput;		// DirectInput wrapper
	CMovieExportDlg	m_MovieExportDlg;	// movie export dialog
	CSnapMovie	m_Movie;		// snapshot movie container
	CString	m_SelName[SELS];	// selection name strings
	CWhorldView	*m_View;		// pointer to our view
	CParmInfo	*m_ViewInfo;	// pointer to parameters being viewed
	CParmInfo	*m_EditInfo;	// pointer to parameters being edited
	int		m_ViewSel;			// view selection: document or mix
	int		m_EditSel;			// edit selection: document, A, B, mix, master offsets
	bool	m_WasShown;			// true if dialog should save its window position
	bool	m_IsFullScreen;		// true if we're in full-screen mode
	bool	m_IsPaused;			// true if we're paused
	bool	m_HideCursor;		// true if cursor is hidden
	CRect	m_ScreenRect;		// window rect to restore when full-screen ends
	BOOL	m_ShowToolbar;		// true if toolbar was visible before full-screen
	BOOL	m_ShowStatusBar;	// true if status bar was visible before full-screen
	CMenu	m_Menu;				// our menus: may be changed dynamically
	int		m_Timer;			// if non-zero, timer instance that drives OnTimer
	int		m_OrgMotion;		// origin motion; see enum above
	HHOOK	m_MouseHook;		// if non-zero, handle to mouse hook procedure
	HHOOK	m_KeybdHook;		// if non-zero, handle to keyboard hook procedure
	int		m_PreIconicTimer;	// pre-iconic timer state: 0 = off, 1 = on, -1 = invalid
	bool	m_PrePrintTimer;	// true if timer was enabled before print/print preview
	bool	m_DemoMode;			// true if we're in demo mode
	bool	m_IsVeejay;			// true if we're in VJ mode
	bool	m_IsCursorValid;	// true if view origin is tracking windows cursor
	double	m_PrevTapTime;		// timestamp of previous tap, in fractional seconds
	volatile	UINT	m_Clock;	// CRealTimer thread's callback increments this
	UINT	m_PrevClock;		// previous value of m_Clock, for detecting change
	volatile	DWORD	m_InputMode;	// input mode; see enum above
	DPoint	m_MousePos;			// absolute mouse position, in screen coordinates
	DPoint	m_TargetOrg;		// target origin, in normalized coordinates
	CAuxFrame	*m_AuxFrm;		// pointer to auxiliary frame, for dual-monitor
	CView	*m_AuxView;			// pointer to auxiliary view, for dual-monitor
	int		m_GrowDir;			// ring growth direction: 1 = outward, -1 = inward
	int		m_TempoMult;		// tempo multiplier: 0 = none, 1 = 2x, -1 = .5x
	double	m_Zoom;				// normalized zoom: 0 = nominal, 1 = max, -1 = min
	double	m_TargetZoom;		// zoom target value; may differ from m_Zoom when damped
	CMidiIO	m_MidiIO;			// wrapper for MIDI API
	int		m_MidiDev;			// currently selected MIDI device
	float	m_MastOfs[ROWS];	// parameter master offsets; written by MIDI thread
	CParmInfo	m_MastInfo;		// shadow of master offsets, for editing via CParmsDlg
	DWORD	m_HelpCookie;		// for HTMLHelp API
	bool	m_FirstNewDoc;		// true the first time OnNewDocument is called
	bool	m_HideChrome;		// true if menus and toolbars are hidden
	bool	m_IsExclusive;		// true if we're using DirectDraw exclusive mode
	bool	m_InHouse;			// true if hidden features are enabled
	double	m_PrevHue;			// previous MIDI hue: 360 degrees mapped to 0..1
	volatile	UINT	m_MidiClock;	// number of MIDI clocks since last reset
	UINT	m_MidiQuant;		// quantization in MIDI sync mode; 0 = shortest time
	bool	m_NoteToggle[MIDI_ROWS];	// note flip-flops for all MIDI properties
	BYTE	m_MPState[MISC_PROPS];	// shadows for momentary misc MIDI properties
	CKeyboardLeds	m_KbdLeds;	// read/write keyboard LEDs
	UINT	m_KbdLedMask;		// shadow state of keyboard LEDs
	int		m_PatchMode;		// patch change mode; see enum above
	bool	m_CycleVideo;		// true if videos are cycling in sync with tempo
	bool	m_VideoOrgMoving;	// true if video origin is seeking target origin
	bool	m_VideoRandOrg;		// true if video origin is making random jumps
	DPoint	m_TargetVideoOrg;	// video origin seeks this target origin
	CParmInfo	m_GlobParm;		// global parameter info
	COscillator	m_GlobOsc[ROWS];	// global oscillators
	double	m_GlobParmTarg[ROWS];	// global parameter targets
	bool	m_IsGlobParm[ROWS];	// true if parameter supports global control
	HACCEL	m_FocusAccel;		// extra keyboard accelerators when we have focus
	int		m_FrameRateTimer;	// frame rate timer instance
	DWORD	m_PrevFrameTime;	// timestamp of previous frame rate update
	int		m_FrameCount;		// total number of frames generated so far
	int		m_PrevFrameCount;	// previous total number of frames
	double	m_ActualFrameRate;	// actual frame frate, in Hz
	CString	m_ActualFrameRateStr;	// actual frame frate, as a string

// Helpers
	void	UpdateParmsDlgTitle();
	void	UpdateBars();
	void	UpdateOptions();
	void	UpdateTempo();
	void	UpdateMaster(int Idx);
	void	HideCursor(bool Enable);
	void	HideChrome(bool Enable);
	static	LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
	bool	HookMouse(bool Enable);
	static	LRESULT CALLBACK KeybdProc(int nCode, WPARAM wParam, LPARAM lParam);
	bool	HookKeybd(bool Enable);
	static	void	TimerCallback(LPVOID Cookie);
	void	GetMouseInput();
	bool	ViewHasFocus() const;
	void	SetMousePosFromOrigin();
	static	void CALLBACK MidiCallback(HMIDIIN handle, UINT uMsg, DWORD dwInstance,
								  DWORD dwParam1, DWORD dwParam2);
	void	SetMidiProperty(int Idx, int Val, bool Toggle);
	void	OnMidiProgChange(int ProgIdx);
	void	OnBankPatchSel(int Bank, int Patch);
	static	CString GetAppPath();
	static	DWORD	SetOrClear(bool Set, DWORD Value, DWORD Bitmask);
	static	bool	MakeUniquePath(LPCTSTR Folder, LPCTSTR Prefix, LPCTSTR Extension, CString& Path);
	bool	MiscPropChanged(int PropIdx, int Toggle, float CtrlVal);
	void	OnFrameTimer(DWORD dwTime);
	static	void CALLBACK FrameTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
};

inline CMainFrame::TimerState::TimerState(bool NewState)
{
	m_PrevState = CMainFrame::GetThis()->EnableTimer(NewState);
}

inline CMainFrame::TimerState::~TimerState()
{
	CMainFrame::GetThis()->EnableTimer(m_PrevState);
}

inline CParmsDlg& CMainFrame::GetParms()
{
	return(m_ParmsDlg);
}

inline CCrossDlg& CMainFrame::GetXFader()
{
	return(m_CrossDlg);
}

inline CMasterDlg& CMainFrame::GetMaster()
{
	return(m_MasterDlg);
}

inline CPlaylistDlg& CMainFrame::GetPlaylist()
{
	return(m_PlaylistDlg);
}

inline CMidiSetupDlg& CMainFrame::GetMidiSetup()
{
	return(m_MidiSetupDlg);
}

inline COptionsDlg&	CMainFrame::GetOptions()
{
	return(m_OptsDlg);
}

inline CMovieExportDlg&	CMainFrame::GetMovieExport()
{
	return(m_MovieExportDlg);
}

inline CMainFrame *CMainFrame::GetThis()
{
	return(m_This);
}

inline CWhorldDoc *CMainFrame::GetDoc() const
{
	return((CWhorldDoc *)m_View->GetDocument());
}

inline CWhorldView *CMainFrame::GetView() const
{
	return(m_View);
}

inline void CMainFrame::GetMasterInfo(MASTER_INFO& Master) const
{
	m_MasterDlg.GetInfo(Master);
}

inline bool	CMainFrame::IsPaused() const
{
	return(m_IsPaused);
}

inline int CMainFrame::GetOrgMotion() const
{
	return(m_OrgMotion);
}

inline bool CMainFrame::GetReverse() const
{
	return(m_GrowDir < 0);
}

inline bool CMainFrame::GetConvex() const
{
	return(m_View->GetConvex());
}

inline bool CMainFrame::GetAutoplay() const
{
	return(m_PlaylistDlg.IsPlaying());
}

inline double CMainFrame::GetXFadeFrac() const
{
	return(m_PlaylistDlg.GetXFadeFrac());
}

inline int CMainFrame::GetViewSel() const
{
	return(m_ViewSel);
}

inline int CMainFrame::GetEditSel() const
{
	return(m_EditSel);
}

inline int CMainFrame::GetDrawMode() const
{
	return(m_View->GetDrawMode());
}

inline bool CMainFrame::IsMirrored() const
{
	return(m_View->IsMirrored());
}

inline bool	CMainFrame::IsHueLooped() const
{
	return(m_View->IsHueLooped());
}

inline bool CMainFrame::IsColorInverted() const
{
	return(m_View->IsColorInverted());
}

inline int CMainFrame::SnapCount() const
{
	return(m_Snapshot.GetCount());
}

inline int CMainFrame::GetCurSnap() const
{
	return(m_Snapshot.GetCurSel());
}

inline double CMainFrame::GetSpeedNorm() const
{
	return(m_MasterDlg.GetSpeed().GetValNorm());
}

inline void CMainFrame::SetSpeedNorm(double Speed)
{
	m_MasterDlg.GetSpeed().SetValNorm(Speed);
	UpdateMaster(CMasterDlg::SPEED);
}

inline double CMainFrame::GetZoomNorm() const
{
	return(m_MasterDlg.GetZoom().GetValNorm());
}

inline void CMainFrame::SetZoomNorm(double Zoom)
{
	m_MasterDlg.GetZoom().SetValNorm(Zoom);
	UpdateMaster(CMasterDlg::ZOOM);
}

inline void CMainFrame::SetZoomNormDamped(double Zoom)
{
	if (m_IsPaused)
		SetZoomNorm(Zoom);
	else
		m_TargetZoom = Zoom;
}

inline double CMainFrame::GetDampingNorm() const
{
	return(m_MasterDlg.GetDamping().GetValNorm());
}

inline void CMainFrame::SetDampingNorm(double Damping)
{
	m_MasterDlg.GetDamping().SetValNorm(Damping);
	UpdateMaster(CMasterDlg::DAMPING);
}

inline double CMainFrame::GetTrailNorm() const
{
	return(m_MasterDlg.GetTrail().GetValNorm());
}

inline void CMainFrame::SetTrailNorm(double Trail)
{
	m_MasterDlg.GetTrail().SetValNorm(Trail);
	UpdateMaster(CMasterDlg::TRAIL);
}

inline double CMainFrame::GetRingsNorm() const
{
	return(m_MasterDlg.GetRings().GetValNorm());
}

inline void CMainFrame::SetRingsNorm(double Rings)
{
	m_MasterDlg.GetRings().SetValNorm(Rings);
	UpdateMaster(CMasterDlg::RINGS);
}

inline double CMainFrame::GetTempo()
{
	return(m_MasterDlg.GetTempo().GetVal());
}

inline int CMainFrame::GetTempoMult(int Mult) const
{
	return(m_TempoMult);
}

inline bool CMainFrame::IsViewDetached() const
{
	return(m_AuxFrm != NULL);
}

inline bool CMainFrame::ViewHasFocus() const
{
	CWnd	*wp = GetFocus();
	return(wp == m_View || wp == m_AuxView);
}

inline bool CMainFrame::IsVeejay() const
{
	return(m_IsVeejay);
}

inline CMenu *CMainFrame::GetMenu()
{
	return(&m_Menu);
}

inline double CMainFrame::GetTimerFreq() const
{
	return(m_OptsDlg.m_Display.GetFrameRate());
}

inline DWORD CMainFrame::SetOrClear(bool Set, DWORD Value, DWORD Bitmask)
{
	return(Set ? (Value | Bitmask) : (Value & ~Bitmask));
}

inline LPCTSTR CMainFrame::GetSelName(int Sel) const
{
	ASSERT(Sel >= 0 && Sel <= SELS);
	return(m_SelName[Sel]);
}

inline void CMainFrame::SetFill(bool Enable)
{
	SetDrawMode(SetOrClear(Enable, GetDrawMode(), CWhorldView::DM_FILL));
}

inline bool CMainFrame::GetFill() const
{
	return((GetDrawMode() & CWhorldView::DM_FILL) != 0);
}

inline void CMainFrame::SetOutline(bool Enable)
{
	SetDrawMode(SetOrClear(Enable, GetDrawMode(), CWhorldView::DM_OUTLINE));
}

inline bool CMainFrame::GetOutline() const
{
	return((GetDrawMode() & CWhorldView::DM_OUTLINE) != 0);
}

inline void CMainFrame::SetXRay(bool Enable)
{
	SetDrawMode(SetOrClear(Enable, GetDrawMode(), CWhorldView::DM_XRAY));
}

inline bool CMainFrame::GetXRay() const
{
	return((GetDrawMode() & CWhorldView::DM_XRAY) != 0);
}

inline bool CMainFrame::IsRecording() const
{
	return(m_Movie.IsOpen());
}

inline bool CMainFrame::IsFullScreen() const
{
	return(m_IsFullScreen);
}

inline bool CMainFrame::IsExclusive() const
{
	return(m_IsExclusive);
}

inline void CMainFrame::OnNewParms(int Sel)
{
	if (!m_IsVeejay && Sel == m_EditSel)	// if editing selection (except in VJ)
		m_ParmsDlg.SetInfo(*m_EditInfo);	// update parameters dialog
}

inline DWORD CMainFrame::GetInputMode() const
{
	return(m_InputMode);
}

inline bool CMainFrame::GetZoomCenter() const
{
	return(m_View->GetZoomType() == CWhorldView::ZT_WND_CENTER);
}

inline int CMainFrame::GetPatchMode() const
{
	return(m_PatchMode);
}

inline double CMainFrame::GetHueLoopLength() const
{
	return(m_MasterDlg.GetHueLoopLength().GetVal());
}

inline void CMainFrame::SetHueLoopLength(double Length)
{
	m_MasterDlg.GetHueLoopLength().SetVal(Length);
	UpdateMaster(CMasterDlg::HUE_LOOP);
}

inline double CMainFrame::GetCanvasScale() const
{
	return(m_MasterDlg.GetCanvasScale().GetVal());
}

inline void CMainFrame::SetCanvasScale(double Scale)
{
	m_MasterDlg.GetCanvasScale().SetVal(Scale);
	UpdateMaster(CMasterDlg::CANVAS);
}

inline double CMainFrame::GetCopies() const
{
	return(m_MasterDlg.GetCopies().GetVal());
}

inline void CMainFrame::SetCopies(double Copies)
{
	m_MasterDlg.GetCopies().SetVal(Copies);
	UpdateMaster(CMasterDlg::COPIES);
}

inline double CMainFrame::GetSpread() const
{
	return(m_MasterDlg.GetSpread().GetVal());
}

inline void CMainFrame::SetSpread(double Spread)
{
	m_MasterDlg.GetSpread().SetVal(Spread);
	UpdateMaster(CMasterDlg::SPREAD);
}

inline bool	CMainFrame::IsVideoCycling() const
{
	return(m_CycleVideo);
}

inline int CMainFrame::GetVideoCycleLength() const
{
	return(m_View->GetVideo().GetCycleLength());
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__F54EFF6B_1538_48D8_83C5_2B526B2A8248__INCLUDED_)
