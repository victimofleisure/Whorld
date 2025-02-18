// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31aug05	initial version
		01		10sep05	add save frame
		02		27sep05	disable origin clamping
		03		25oct05	disable timer during OnSave
		04		25oct05	catch exceptions in OnTimer
		05		26oct05	add support for full-screen exclusive mode
		06		09feb06	add open button, support drop files
		07		10feb06	display all times in mm:ss:ff format
		08		29mar06	remove origin clamping
		09		03may06	add selection range and export
		10		07jun06	set view before opening movie
		11		16sep06	in OnTimer, fix off by one errors
		12		21dec07	rename GetMainFrame to GetThis
		13		28jan08	support Unicode

        snapshot movie player
 
*/

// MoviePlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MoviePlayerDlg.h"
#include "MainFrm.h"
#include "shlwapi.h"
#include "MultiFileDlg.h"
#include "MovieExportDlg.h"	// for time conversion functions

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMoviePlayerDlg dialog

IMPLEMENT_DYNAMIC(CMoviePlayerDlg, CPersistDlg);

CMoviePlayerDlg::CMoviePlayerDlg(CWnd* pParent /*=NULL*/)
	: CPersistDlg(CMoviePlayerDlg::IDD, 0, _T("MoviePlayerDlg"), pParent)
{
	//{{AFX_DATA_INIT(CMoviePlayerDlg)
	//}}AFX_DATA_INIT
	m_Frm = NULL;
	m_Transport = STOP;
	m_Timer = 0;
	m_Period = 0;
	m_IsFullScreen = FALSE;
	m_Hidden = FALSE;
	m_RangeIn = -1;
	m_RangeOut = -1;
}

bool CMoviePlayerDlg::Open(LPCTSTR Path)
{
	if (!m_Movie.Open(Path, TRUE))
		return(FALSE);
	m_Path = Path;
	SetTitle();
	if (IsWindow(m_hWnd)) {
		m_Pos.SetRange(0, m_Movie.GetFrameCount() - 1);
		ClearRange();
		int	Trans = m_Transport;
		SetTransport(STOP);		// display initial frame
		SetTransport(Trans);	// restore previous transport
	}
	return(TRUE);
}

void CMoviePlayerDlg::SetTransport(int State)
{
	m_Play.SetCheck(State == PLAY);
	m_Play.SetIcon(AfxGetApp()->LoadIcon(State == PLAY ? IDI_PLAYD : IDI_PLAYU));
	m_Pause.SetCheck(State == PAUSE);
	m_Pause.SetIcon(AfxGetApp()->LoadIcon(State == PAUSE ? IDI_PAUSED : IDI_PAUSEU));
	m_Stop.SetCheck(State == STOP);
	m_Stop.SetIcon(AfxGetApp()->LoadIcon(State == STOP ? IDI_STOPD : IDI_STOPU));
	switch (State) {
	case PLAY:
		if (IsOpen())
			EnableTimer(round(1000.0 / m_Movie.GetFrameRate()));
		break;
	case STOP:
		EnableTimer(0);
		m_Movie.Rewind();
		m_Movie.Read();	// display initial frame
		m_Movie.Rewind();
		UpdatePos();
		Hide(FALSE);
		break;
	case PAUSE:
		EnableTimer(0);
		break;
	}
	m_Transport = State;
}

void CMoviePlayerDlg::SetLoop(bool Enable)
{
	m_Loop.SetIcon(AfxGetApp()->LoadIcon(Enable ? IDI_LOOPD : IDI_LOOPU));
	m_Loop.SetCheck(Enable);
}

void CMoviePlayerDlg::EnableTimer(int Period)
{
	if ((Period != 0) == (m_Timer != 0))
		return;	// nothing to do
	if (Period) {
		m_Period = Period;
		m_Timer = SetTimer(TIMER_ID, Period, 0);
	} else {
		KillTimer(m_Timer);
		m_Timer = 0;
	}
}

void CMoviePlayerDlg::SetTitle()
{
	if (IsWindow(m_hWnd)) {
		LPCTSTR	fname = PathFindFileName(m_Path);
		SetWindowText(IsOpen() ? m_Caption + " - " + fname : m_Caption);
	}
}

void CMoviePlayerDlg::SetRange(int In, int Out)
{
	if (In == Out) {
		In = -1;
		Out = -1;
	} else {
		if (In < 0)
			In = 0;
		if (Out < 0)
			Out = m_Movie.GetFrameCount() - 1;
		if (In > Out) {
			In = -1;
			Out = -1;
		}
	}
	m_RangeIn = In;
	m_RangeOut = Out;
	m_Pos.SetSelection(In, Out);
	m_Pos.RedrawWindow();	// else playback's CPU load could delay repaint
}

void CMoviePlayerDlg::Seek(int Frame)
{
	m_Movie.Seek(Frame);	// order matters here
	UpdatePos();
	if (m_Transport != PLAY)	// in play mode, assume timer will do read
		m_Movie.Read();
}

void CMoviePlayerDlg::UpdatePos()
{
	int	Frame = m_Movie.GetCurPos();
	m_Pos.SetPos(Frame);
	CString	CurTime, TotTime;
	CMovieExportDlg::FrameToTime(Frame, m_Movie.GetFrameRate(), CurTime);
	CMovieExportDlg::FrameToTime(m_Movie.GetFrameCount(), 
		m_Movie.GetFrameRate(), TotTime);
	m_Time.SetWindowText(CurTime + " / " + TotTime);
}

void CMoviePlayerDlg::Hide(bool Enable)
{
	if (Enable == m_Hidden)
		return;	// nothing to do
	CSize sz = Enable ? CSize(0, 0) : m_InitRect.Size();
	// topmost window of zero size is invisible but retains input focus
	SetWindowPos(Enable ? &CWnd::wndTopMost : &CWnd::wndNoTopMost,
		0, 0, sz.cx, sz.cy, SWP_NOMOVE);
	if (Enable)
		SetCapture();
	else {
		ReleaseCapture();
		FullScreen(FALSE);
	}
	m_Hidden = Enable;
}

bool CMoviePlayerDlg::Play(LPCTSTR Path)
{
	CMainFrame::TimerState	ts(FALSE);	// stop timer, dtor restores it
	CSnapshot::ViewBackup	vb;	// save view state, dtor restores it
	CMainFrame	*Frm = CMainFrame::GetThis();
	ASSERT(Frm != NULL);
	CWhorldView	*View = Frm->GetView();
	CMoviePlayerDlg	mp;
	mp.m_Movie.SetView(View);	// movie open uses view
	bool	retc = mp.Open(Path);
	if (retc) {
		Frm->ShowOwnedWindows(FALSE);
		mp.DoModal();
		Frm->ShowOwnedWindows(TRUE);
	}
	return(retc);
}

bool CMoviePlayerDlg::Play()
{
	CMainFrame::TimerState	ts(FALSE);	// stop timer, dtor restores it
	CMultiFileDlg	fd(TRUE, EXT_MOVIE, NULL, OFN_HIDEREADONLY,
		LDS(IDS_FILTER_MOVIE), LDS(IDS_MOVIE_OPEN));
	return(fd.DoModal() == IDOK && Play(fd.GetPathName()));
}

void CMoviePlayerDlg::FullScreen(bool Enable)
{
	if (Enable == m_IsFullScreen)
		return;	// nothing to do
	if (m_Frm->AllowExclusive())	// if DirectDraw exclusive mode is allowed
		m_Frm->SetExclusive(Enable);	// use it
	else
		m_Frm->FullScreen(Enable);
	ShowCursor(!Enable);
	if (Enable)	// move to bottom of Z-order, else frame receives keyboard events
		SetWindowPos(&CWnd::wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	m_IsFullScreen = Enable;
}

void CMoviePlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMoviePlayerDlg)
	DDX_Control(pDX, IDC_MP_GOTO_END, m_GotoEnd);
	DDX_Control(pDX, IDC_MP_GOTO_START, m_GotoStart);
	DDX_Control(pDX, IDC_MP_SET_OUT, m_SetOut);
	DDX_Control(pDX, IDC_MP_SET_IN, m_SetIn);
	DDX_Control(pDX, IDC_MP_TAKE_SNAP, m_TakeSnap);
	DDX_Control(pDX, IDC_MP_OPEN, m_Open);
	DDX_Control(pDX, IDC_MP_SAVE, m_Save);
	DDX_Control(pDX, IDC_MP_TIME, m_Time);
	DDX_Control(pDX, IDC_MP_POS, m_Pos);
	DDX_Control(pDX, IDC_MP_LOOP, m_Loop);
	DDX_Control(pDX, IDC_MP_STOP, m_Stop);
	DDX_Control(pDX, IDC_MP_PLAY, m_Play);
	DDX_Control(pDX, IDC_MP_PAUSE, m_Pause);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMoviePlayerDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CMoviePlayerDlg)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_MP_PAUSE, OnPause)
	ON_BN_CLICKED(IDC_MP_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_MP_STOP, OnStop)
	ON_BN_CLICKED(IDC_MP_LOOP, OnLoop)
	ON_WM_ACTIVATEAPP()
	ON_WM_SYSCOMMAND()
	ON_WM_LBUTTONDOWN()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_MP_SAVE, OnSave)
	ON_BN_CLICKED(IDC_MP_OPEN, OnOpen)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_MP_TAKE_SNAP, OnTakeSnap)
	ON_BN_CLICKED(IDC_MP_SET_IN, OnSetIn)
	ON_BN_CLICKED(IDC_MP_SET_OUT, OnSetOut)
	ON_BN_CLICKED(IDC_MP_GOTO_START, OnGotoStart)
	ON_BN_CLICKED(IDC_MP_GOTO_END, OnGotoEnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMoviePlayerDlg message handlers

BOOL CMoviePlayerDlg::OnInitDialog() 
{
	CPersistDlg::OnInitDialog();
	
	GetWindowText(m_Caption);
	m_Frm = CMainFrame::GetThis();
	ASSERT(m_Frm != NULL);
	CWhorldView	*View = m_Frm->GetView();
	m_Movie.SetView(View);
	SetLoop(FALSE);
	SetTitle();
	GetWindowRect(m_InitRect);
	m_Hidden = FALSE;
	m_Transport = IsOpen() ? PLAY : STOP;	// if movie is open, auto-play it
	SetTransport(m_Transport);
	m_Open.SetIcon(AfxGetApp()->LoadIcon(IDI_OPEN));
	m_Save.SetIcon(AfxGetApp()->LoadIcon(IDI_SAVE));
	m_TakeSnap.SetIcon(AfxGetApp()->LoadIcon(IDI_TAKE_SNAP));
	m_SetIn.SetIcon(AfxGetApp()->LoadIcon(IDI_SET_IN));
	m_SetOut.SetIcon(AfxGetApp()->LoadIcon(IDI_SET_OUT));
	m_GotoStart.SetIcon(AfxGetApp()->LoadIcon(IDI_GOTO_START));
	m_GotoEnd.SetIcon(AfxGetApp()->LoadIcon(IDI_GOTO_END));
	m_Pos.SetRange(0, m_Movie.GetFrameCount() - 1);
	DragAcceptFiles(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMoviePlayerDlg::OnOK()
{
	if (m_Hidden)
		Hide(FALSE);
	else
		CPersistDlg::OnOK();
}

void CMoviePlayerDlg::OnCancel()
{
	if (m_Hidden)
		Hide(FALSE);
	else
		CPersistDlg::OnCancel();
}

void CMoviePlayerDlg::OnTimer(UINT nIDEvent) 
{
	TRY {
		if (IsOpen()) {
			UINT	EndPos = HasRange() && IsLooped() ?
				m_RangeOut : m_Movie.GetFrameCount() - 1;
			if (m_Movie.GetCurPos() <= EndPos) {	// end position is inclusive
				UpdatePos();	// display current position before read bumps it
				m_Movie.Read();
			} else {
				if (IsLooped()) {
					Seek(HasRange() ? m_RangeIn : 0);
					m_Movie.Read();	// seek skips read when we're in play mode
				} else
					SetTransport(STOP);
			}
		}
	}
	CATCH (CException, e) {
		EnableTimer(0);
		e->ReportError();
	}
	END_CATCH
	CPersistDlg::OnTimer(nIDEvent);
}

void CMoviePlayerDlg::OnDestroy() 
{
	EnableTimer(0);
	FullScreen(FALSE);
	CPersistDlg::OnDestroy();
}

void CMoviePlayerDlg::OnPause() 
{
	SetTransport(PAUSE);
}

void CMoviePlayerDlg::OnPlay() 
{
	SetTransport(PLAY);
}

void CMoviePlayerDlg::OnStop() 
{
	SetTransport(STOP);
}

void CMoviePlayerDlg::OnActivateApp(BOOL bActive, HTASK hTask)
{
	if (!bActive)	// if app is being deactivated
		Hide(FALSE);
}

void CMoviePlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID) {
	case SC_MAXIMIZE:
		Hide(TRUE);
		FullScreen(TRUE);
		break;
	case SC_MINIMIZE:
		Hide(TRUE);
		break;
	default:
		CPersistDlg::OnSysCommand(nID, lParam);
	}
}

void CMoviePlayerDlg::OnLoop() 
{
	SetLoop(!IsLooped());
}

void CMoviePlayerDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	Hide(FALSE);
	CPersistDlg::OnLButtonDown(nFlags, point);
}

void CMoviePlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (m_Movie.IsOpen())
		Seek(m_Pos.GetPos());
	CPersistDlg::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CMoviePlayerDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && m_Hidden) {
		Hide(FALSE);
		return(TRUE);
	}
	return CPersistDlg::PreTranslateMessage(pMsg);
}

void CMoviePlayerDlg::OnSave() 
{
	EnableTimer(0);
	m_Frm->GetMovieExport().Export(m_Path, m_RangeIn, m_RangeOut);
	if (m_Transport == PLAY)
		EnableTimer(m_Period);
}

void CMoviePlayerDlg::OnOpen() 
{
	EnableTimer(0);
	CMultiFileDlg	fd(TRUE, EXT_MOVIE, NULL, OFN_HIDEREADONLY,
		LDS(IDS_FILTER_MOVIE), LDS(IDS_MOVIE_OPEN));
	if (fd.DoModal() == IDOK) {
		Open(fd.GetPathName());
	}
	if (m_Transport == PLAY)
		EnableTimer(m_Period);
}

void CMoviePlayerDlg::OnDropFiles(HDROP hDropInfo) 
{
	TCHAR	Path[MAX_PATH];
	DragQueryFile(hDropInfo, 0, Path, MAX_PATH);
	Open(Path);
}

void CMoviePlayerDlg::OnTakeSnap() 
{
	EnableTimer(0);
	CMultiFileDlg	fd(FALSE, EXT_SNAPSHOT, NULL, OFN_OVERWRITEPROMPT,
		LDS(IDS_FILTER_SNAPSHOT), LDS(IDS_SNAP_TAKE));
	if (fd.DoModal() == IDOK)
		m_Movie.SaveFrame(fd.GetPathName());
	if (m_Transport == PLAY)
		EnableTimer(m_Period);
}

void CMoviePlayerDlg::OnSetIn() 
{
	SetRange(m_Pos.GetPos(), m_RangeOut);
}

void CMoviePlayerDlg::OnSetOut() 
{
	SetRange(m_RangeIn, m_Pos.GetPos());
}

void CMoviePlayerDlg::OnGotoStart() 
{
	Seek(m_RangeIn >= 0 ? m_RangeIn : 0);
}

void CMoviePlayerDlg::OnGotoEnd() 
{
	Seek(m_RangeOut >= 0 ? m_RangeOut : m_Movie.GetFrameCount() - 1);
}
