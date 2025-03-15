// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		20feb25	add file export
		02		21feb25	add options
        03      22feb25	add snapshot capture and load
		04		25feb25	disable disruptive commands in full screen single monitor
		05		25feb25	add frame min/max info handler for row view panes
		06		26feb25	add MIDI input
		07		27feb25	restore patch on exiting snapshot mode
		08		02mar25	add globals pane
		09		03mar25	add render queue full handler
		10		06mar25	implement drop files for all supported file types
		11		07mar25	use prompt for multiple files dialog for snapshots
		12		08mar25	add export all snapshots
		13		10mar25	add prompting for export all
		14		11mar25	disable take snapshot command while in snapshot mode
		15		12mar25	add snapshot info command; handle set draw mode
		16		14mar25	add movie recording and playback

*/

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Whorld.h"
#include "MainFrm.h"
#include "WhorldDoc.h"
#include "WhorldView.h"
#include "OptionsDlg.h"
#include "PathStr.h"
#include "ExportDlg.h"
#include "FocusEdit.h"
#include "RowDlg.h"	// for row view frame min/max info
#include "dbt.h"	// for device change types
#include "Midi.h"
#include "SaveObj.h"
#include "ProgressDlg.h"
#include "FolderDialog.h"
#include "MovieExportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_STATUS_PANE_RING_COUNT,
	ID_STATUS_PANE_FRAME_RATE,
};

enum {	// application looks; alpha order to match corresponding resource IDs
	APPLOOK_OFF_2003,
	APPLOOK_OFF_2007_AQUA,
	APPLOOK_OFF_2007_BLACK,
	APPLOOK_OFF_2007_BLUE,
	APPLOOK_OFF_2007_SILVER,
	APPLOOK_OFF_XP, 
	APPLOOK_VS_2005,
	APPLOOK_VS_2008,
	APPLOOK_WINDOWS_7,
	APPLOOK_WIN_2000,
	APPLOOK_WIN_XP,
	APP_LOOKS
};

#define ID_VIEW_APPLOOK_FIRST ID_VIEW_APPLOOK_OFF_2003
#define ID_VIEW_APPLOOK_LAST ID_VIEW_APPLOOK_WIN_XP

#define ID_SNAPSHOT_RANGE_FIRST ID_SNAPSHOT_EXPORT_ALL
#define ID_SNAPSHOT_RANGE_LAST ID_SNAPSHOT_PREV

const UINT CMainFrame::m_arrDockingBarNameID[DOCKING_BARS] = {
	#define MAINDOCKBARDEF(name, width, height, style) IDS_BAR_##name,
	#include "MainDockBarDef.h"	// generate docking bar names
};

const LPCTSTR CMainFrame::m_pszExportExt = _T("png");
const LPCTSTR CMainFrame::m_pszExportFilter = _T("PNG Files (*.png)|*.png|All Files (*.*)|*.*||");
const LPCTSTR CMainFrame::m_pszSnapshotExt = _T("whs");
const LPCTSTR CMainFrame::m_pszSnapshotFilter = _T("Snapshot Files (*.whs)|*.whs|All Files (*.*)|*.*||");
const LPCTSTR CMainFrame::m_pszMovieExt = _T("whm");
const LPCTSTR CMainFrame::m_pszMovieFilter = _T("Movie Files (*.whm)|*.whm|All Files (*.*)|*.*||");

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
	theApp.m_pMainWnd = this;
	m_nPrevFrameCount = 0;
	m_iCurSnapshot = 0;
	m_bInRenderFullError = false;
	m_nMovieIOState = MOVIE_NONE;
	m_bIsMoviePaused = false;
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

#if 0	// disable user-defined toolbar images
	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}
#endif

#if 0	// disable menu personalization of most-recently used commands
	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_FILE_PRINT);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);
#endif

	// blank status bar panes
	int	nPrevTextLen = 0;
	FastSetPaneText(m_wndStatusBar, SBP_RING_COUNT, m_sRingCount, nPrevTextLen);
	FastSetPaneText(m_wndStatusBar, SBP_FRAME_RATE, m_sFrameRate, nPrevTextLen);

	// accept dropped files
	DragAcceptFiles();

	// apply initial options
	ApplyOptions(NULL);

	// do delayed initialization after message loop settles down
	PostMessage(UWM_DELAYED_CREATE);

	return 0;
}

BOOL CMainFrame::DestroyWindow()
{
	theApp.SetFullScreen(false);
	theApp.DestroyRenderWnd();
	return CFrameWndEx::DestroyWindow();
}

BOOL CMainFrame::CreateDockingWindows()
{
	CString sTitle;
	DWORD	dwBaseStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CBRS_FLOAT_MULTI;
	#define MAINDOCKBARDEF(name, width, height, style) \
		sTitle.LoadString(IDS_BAR_##name); \
		if (!m_wnd##name##Bar.Create(sTitle, this, CRect(0, 0, width, height), TRUE, ID_BAR_##name, style)) { \
			TRACE("Failed to create %s bar\n", #name); \
			return FALSE; \
		} \
		m_wnd##name##Bar.EnableDocking(CBRS_ALIGN_ANY); \
		DockPane(&m_wnd##name##Bar);
	#include "MainDockBarDef.h"	// generate code to create docking windows
	return TRUE;
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

#if 0	// ck: disable toolbar customization for now
	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}
#endif

#ifdef _DEBUG
	bool	bResetCustomizations = true;	// customizations are too confusing during development
#else
	// in Release only, reset customizations if resource version changed
	bool	bResetCustomizations = theApp.ResourceVersionChanged();
#endif
	if (bResetCustomizations) {	// if resetting UI to its default state
#if _MFC_VER < 0xb00
		m_wndMenuBar.RestoreOriginalstate();
		m_wndToolBar.RestoreOriginalstate();
#else	// MS fixed typo
		m_wndMenuBar.RestoreOriginalState();
		m_wndToolBar.RestoreOriginalState();
#endif
		theApp.GetKeyboardManager()->ResetAll();
		theApp.GetContextMenuManager()->ResetState();
	}
	return TRUE;
}

void CMainFrame::ApplyOptions(const COptions *pPrevOptions)
{
	theApp.ApplyOptions(pPrevOptions);
}

void CMainFrame::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// views that are descendents of main frame get updates automatically;
	// other objects that need document updates must be added explicitly
	m_wndParamsBar.OnUpdate(pSender, lHint, pHint);
	m_wndMasterBar.OnUpdate(pSender, lHint, pHint);
	m_wndGlobalsBar.OnUpdate(pSender, lHint, pHint);
}

bool CMainFrame::FastSetPaneText(CMFCStatusBar& bar, int nIndex, const CString& sText, int& nCurTextLength)
{
	// derived from SetPaneText, but doesn't reallocate pane's text
	// buffer unless text gets bigger, and doesn't call UpdateWindow
	ASSERT(nIndex < bar.m_nCount);
	CMFCStatusBarPaneInfo* pSBP = ((CMFCStatusBarPaneInfo*)bar.m_pData) + nIndex;
	int	nNewLen = sText.GetLength();
	if (pSBP->lpszText != NULL) {	// if text buffer allocated
		if (_tcscmp(pSBP->lpszText, sText) == 0) {	// if text unchanged
			return true;	// nothing to do
		}
		if (nNewLen > nCurTextLength) {	// if text got bigger
			free((LPVOID)pSBP->lpszText);	// free text buffer
			pSBP->lpszText = (LPCTSTR)malloc((nNewLen + 1) * sizeof(TCHAR));
			if (pSBP->lpszText == NULL)
				return false;	// allocation failed
			nCurTextLength = nNewLen;	// update pane's current text length
		}
	} else {	// text buffer not allocated
		pSBP->lpszText = (LPCTSTR)malloc((nNewLen + 1) * sizeof(TCHAR));
		if (pSBP->lpszText == NULL)
			return false;	// allocation failed
		nCurTextLength = nNewLen;	// update pane's current text length
	}
	// copy new text to pane's text buffer
	_tcscpy_s(const_cast<LPTSTR>(pSBP->lpszText), nNewLen + 1, sText);
	CRect rect = pSBP->rect;
	if (!(pSBP->nStyle & SBPS_NOBORDERS))
		rect.InflateRect(-AFX_CX_BORDER, -AFX_CY_BORDER);
	else
		rect.top -= AFX_CY_BORDER;  // base line adjustment
	bar.InvalidateRect(rect, FALSE);	// invalidate pane
	// caller is responsible for calling UpdateWindow if desired
	return true;
}

inline bool CMainFrame::PromptingForExport() const
{
	return theApp.m_options.m_Export_bPromptUser	// true if user wants to be prompted
		&& !theApp.IsFullScreenSingleMonitor();		// and prompting is permissible
}

bool CMainFrame::WriteSnapshot(const CSnapshot *pSnapshot)
{
	ASSERT(pSnapshot != NULL);
	CString	sSnapshotPath;
	if (PromptingForExport()) {
		CFileDialog	fd(false, m_pszSnapshotExt, NULL, OFN_OVERWRITEPROMPT, m_pszSnapshotFilter);
		if (fd.DoModal() != IDOK) {	// display file dialog
			return false;	// user canceled
		}
		sSnapshotPath = fd.GetPathName();
	} else {	// not prompting
		if (!MakeUniqueExportPath(sSnapshotPath, m_pszSnapshotExt))	// generate path
			return false;	// unable to generate path
	}
	pSnapshot->Write(pSnapshot, sSnapshotPath);
	return true;
}

bool CMainFrame::MakeUniqueExportPath(CString& sExportPath, LPCTSTR pszExt)
{
	CPathStr	sPath(theApp.m_options.m_Export_sImageFolder);
	if (!PathFileExists(sPath)) {	// if export image folder doesn't exist
		AfxMessageBox(IDS_APP_ERR_BAD_EXPORT_IMAGE_FOLDER);
		return false;
	}
	// automatically assign a filename based on date and time
	sPath.Append(theApp.GetTimestampFileName());	// append filename to folder
	int	nPreExtPathLen = sPath.GetLength();	// save pre-extension path length
	sExportPath = sPath + '.' + CString(pszExt);	// append file extension to filename
	int	nRetries = 0;
	// while path identifies an existing file, or is found in output path array
	while (PathFileExists(sExportPath) || m_aOutputPath.Find(sExportPath) >= 0) {
		CString	sSuffix;	// numeric suffix added to filename to make it unique
		sSuffix.Format(_T("(%d)"), ++nRetries);	// pre-increment retry count
		// append suffix to file name, re-append file extension, and try again
		sExportPath = sPath.Left(nPreExtPathLen) + sSuffix + '.' + CString(pszExt);
	}
	return true;
}

bool CMainFrame::WaitForPostedMessage(UINT message, CProgressDlg& dlgProgress)
{
	// loop until the specified posted message is received and processed,
	// or the user cancels in the progress dialog, or an error occurs
	while (1) {
		MSG msg;
		// check if we have a posted message, and read it if we do
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {	// if message is quit
				return false;	// epic fail
			}
			TranslateMessage(&msg);	// process the message
			DispatchMessage(&msg);
			// if it's the message we're waiting for
			if (msg.message == message) {
				return true;	// success
			}
		} else {	// no posted message
			if (dlgProgress.Canceled()) {	// if user canceled
				return false;	// fail
			}
			WaitMessage();	// suspend until a message arrives
		}
	}
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// if we're running full screen in a single monitor configuration,
	// disable commands that steal focus and disrupt full screen mode
	if (theApp.IsFullScreenSingleMonitor()) {
		switch (nID) {
		#define MAINDOCKBARDEF(name, width, height, style) case ID_VIEW_BAR_##name:
		#include "MainDockBarDef.h"	// generate docking bar view commands
		case ID_FILE_NEW:
		case ID_FILE_OPEN:
		case ID_FILE_SAVE:
		case ID_FILE_SAVE_AS:
		case ID_VIEW_CUSTOMIZE:
		case ID_HELP_FINDER:
		case ID_APP_HOME_PAGE:
		case ID_APP_ABOUT:
			return true;	// disable command
		}
	}
	switch (nID) {
	case ID_EDIT_UNDO:
	case ID_EDIT_REDO:
	case ID_EDIT_COPY:
	case ID_EDIT_CUT:
	case ID_EDIT_PASTE:
	case ID_EDIT_INSERT:
	case ID_EDIT_DELETE:
	case ID_EDIT_SELECT_ALL:
		if (nCode == CN_COMMAND || nCode == CN_UPDATE_COMMAND_UI) {
			HWND	hFocusWnd = ::GetFocus();
			CEdit	*pEdit = CFocusEdit::GetEdit(hFocusWnd);
			if (pEdit != NULL) {	// if edit control has focus, it has top priority
				switch (nID) {
				case ID_EDIT_UNDO:
				case ID_EDIT_REDO:
					break;	// but undo/redo are exceptions
				default:
					CFocusEdit::OnCmdMsg(nID, nCode, pExtra, pEdit);	// let edit control handle editing commands
					return TRUE;
				}
			}
			CMainFrame	*pFrame = theApp.GetMainFrame();
			// if dockable bar that wants editing commands has focus and is visible, it has priority over framework
			#define MAINDOCKBARDEF_WANTEDITCMDS(name) \
				if (hFocusWnd == pFrame->m_wnd##name##Bar.GetListCtrl().m_hWnd && pFrame->m_wnd##name##Bar.FastIsVisible()) { \
					return pFrame->m_wnd##name##Bar.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo); \
				}
			#include "MainDockBarDef.h"	// generate hooks for dockable bars that want editing commands
		}
		break;
	}
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnFrameGetMinMaxInfo(CDockablePane* pPane, HWND hFrameWnd, MINMAXINFO *pMMI)
{
	ASSERT(pPane != NULL);
	ASSERT(hFrameWnd);
	ASSERT(pMMI != NULL);
	// apply minimum size to all panes
	pMMI->ptMinTrackSize = CPoint(GetSystemMetrics(SM_CXMIN), GetSystemMetrics(SM_CYMIN));
	CRect	rFrame;
	::GetWindowRect(hFrameWnd, rFrame);
	CRect	rBar;
	pPane->GetWindowRect(rBar);
	CSize	szNCBrass = rFrame.Size() - rBar.Size();
	CWnd*	pWndChild = pPane->GetWindow(GW_CHILD);	// get pane's child
	CRowView*	pRowView = DYNAMIC_DOWNCAST(CRowView, pWndChild);
	if (pRowView != NULL) {	// if pane's child is a row view
		int	iRow = pRowView->GetRows() - 1;
		CRowDlg *pRowDlg = pRowView->GetRow(iRow);
		CRect	rRowDlg;
		pRowDlg->GetWindowRect(rRowDlg);
		pPane->ScreenToClient(rRowDlg);
		CPoint	ptBR(rRowDlg.BottomRight());
		CSize	szScrollBars(GetSystemMetrics(SM_CXHSCROLL), GetSystemMetrics(SM_CXVSCROLL));
		ptBR.Offset(szNCBrass + szScrollBars);
		pMMI->ptMaxTrackSize = ptBR;
	}
}

bool CMainFrame::PlayMovie(LPCTSTR pszMoviePath, bool bPaused)
{
	if (IsMovieOpen()) {	// if movie is open
		ASSERT(IsPlayingMovie());	// else we shouldn't be here
		theApp.m_thrRender.MoviePlay(NULL);	// stop playback
		m_nMovieIOState = MOVIE_NONE;
		theApp.SetPause(false);	// exit snapshot mode and resume patch
	} else {	// movie is closed
		ASSERT(pszMoviePath != NULL);	// path is required
		if (!theApp.m_thrRender.MoviePlay(pszMoviePath, bPaused)) {	// start playback
			return false;
		}
		// can't display snapshots while playing movie
		m_aSnapshotPath.RemoveAll();	// empty snapshot array
		theApp.SetSnapshotMode(true);
		m_nMovieIOState = MOVIE_PLAYING;
	}
	return true;
}

bool CMainFrame::RecordMovie(LPCTSTR pszMoviePath)
{
	if (IsMovieOpen()) {	// if movie is open
		ASSERT(IsRecordingMovie());	// else we shouldn't be here
		theApp.m_thrRender.MovieRecord(NULL);	// stop recording
		m_nMovieIOState = MOVIE_NONE;
	} else {	// movie is closed
		ASSERT(pszMoviePath != NULL);	// path is required
		if (!theApp.m_thrRender.MovieRecord(pszMoviePath)) {	// start recording
			return false;
		}
		m_nMovieIOState = MOVIE_RECORDING;
	}
	return true;
}

bool CMainFrame::PauseMovie(bool bEnable)
{
	if (bEnable == m_bIsMoviePaused) {	// if already in requested state
		return true;	// nothing to do
	}
	if (!theApp.m_thrRender.MoviePause(bEnable)) {
		return false;
	}
	m_bIsMoviePaused = bEnable;
	return true;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

// CMainFrame message map

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_FIRST, ID_VIEW_APPLOOK_LAST, OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_FIRST, ID_VIEW_APPLOOK_LAST, OnUpdateApplicationLook)
	ON_WM_CLOSE()
	ON_WM_DROPFILES()
	ON_WM_DEVICECHANGE()
	ON_MESSAGE(UWM_DELAYED_CREATE, OnDelayedCreate)
	ON_MESSAGE(UWM_HANDLE_DLG_KEY, OnHandleDlgKey)
	ON_MESSAGE(UWM_RENDER_WND_CLOSED, OnRenderWndClosed)
	ON_MESSAGE(UWM_FULL_SCREEN_CHANGED, OnFullScreenChanged)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_MESSAGE(UWM_DEVICE_NODE_CHANGE, OnDeviceNodeChange)
	ON_MESSAGE(UWM_BITMAP_CAPTURE, OnBitmapCapture)
	ON_MESSAGE(UWM_SNAPSHOT_CAPTURE, OnSnapshotCapture)
	ON_MESSAGE(UWM_PARAM_CHANGE, OnParamChange)
	ON_MESSAGE(UWM_MASTER_PROP_CHANGE, OnMasterPropChange)
	ON_MESSAGE(UWM_MAIN_PROP_CHANGE, OnMainPropChange)
	ON_MESSAGE(UWM_SET_DRAW_MODE, OnSetDrawMode)
	ON_MESSAGE(UWM_RENDER_QUEUE_FULL, OnRenderQueueFull)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_FILE_TAKE_SNAPSHOT, OnFileTakeSnapshot)
	ON_UPDATE_COMMAND_UI(ID_FILE_TAKE_SNAPSHOT, OnUpdateFileTakeSnapshot)
	ON_COMMAND(ID_FILE_LOAD_SNAPSHOT, OnFileLoadSnapshot)
	ON_UPDATE_COMMAND_UI(ID_FILE_LOAD_SNAPSHOT, OnUpdateFileLoadSnapshot)
	ON_COMMAND(ID_PLAYLIST_NEW, OnPlaylistNew)
	ON_COMMAND(ID_PLAYLIST_OPEN, OnPlaylistOpen)
	ON_COMMAND(ID_PLAYLIST_SAVE, OnPlaylistSave)
	ON_COMMAND(ID_PLAYLIST_SAVE_AS, OnPlaylistSaveAs)
	ON_COMMAND_RANGE(ID_PLAYLIST_MRU_FILE1, ID_PLAYLIST_MRU_FILE4, OnPlaylistMru)
	ON_UPDATE_COMMAND_UI(ID_PLAYLIST_MRU_FILE1, OnUpdatePlaylistMru)
	ON_COMMAND(ID_IMAGE_RANDOM_PHASE, OnImageRandomPhase)
	ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
	ON_COMMAND(ID_VIEW_MIDI_LEARN, OnViewMidiLearn)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MIDI_LEARN, OnUpdateViewMidiLearn)
	ON_COMMAND(ID_WINDOW_FULLSCREEN, OnWindowFullscreen)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_FULLSCREEN, OnUpdateWindowFullscreen)
	ON_COMMAND(ID_WINDOW_DETACH, OnWindowDetach)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_DETACH, OnUpdateWindowDetach)
	ON_COMMAND(ID_WINDOW_PAUSE, OnWindowPause)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PAUSE, OnUpdateWindowPause)
	ON_COMMAND(ID_WINDOW_STEP, OnWindowStep)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_STEP, OnUpdateWindowStep)
	ON_COMMAND(ID_WINDOW_CLEAR, OnWindowClear)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CLEAR, OnUpdateWindowClear)
	ON_COMMAND(ID_WINDOW_RESET_LAYOUT, OnWindowResetLayout)
	ON_COMMAND(ID_SNAPSHOT_FIRST, OnSnapshotFirst)
	ON_COMMAND(ID_SNAPSHOT_LAST, OnSnapshotLast)
	ON_COMMAND(ID_SNAPSHOT_NEXT, OnSnapshotNext)
	ON_COMMAND(ID_SNAPSHOT_PREV, OnSnapshotPrev)
	ON_COMMAND(ID_SNAPSHOT_EXPORT_ALL, OnSnapshotExportAll)
	ON_COMMAND(ID_SNAPSHOT_INFO, OnSnapshotInfo)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SNAPSHOT_RANGE_FIRST, ID_SNAPSHOT_RANGE_LAST, OnUpdateSnapshot)
	ON_COMMAND(ID_MOVIE_RECORD, OnMovieRecord)
	ON_UPDATE_COMMAND_UI(ID_MOVIE_RECORD, OnUpdateMovieRecord)
	ON_COMMAND(ID_MOVIE_PLAY, OnMoviePlay)
	ON_UPDATE_COMMAND_UI(ID_MOVIE_PLAY, OnUpdateMoviePlay)
	ON_COMMAND(ID_MOVIE_EXPORT, OnMovieExport)
	ON_UPDATE_COMMAND_UI(ID_MOVIE_EXPORT, OnUpdateMovieExport)
	ON_COMMAND(ID_MOVIE_REWIND, OnMovieRewind)
	ON_UPDATE_COMMAND_UI(ID_MOVIE_REWIND, OnUpdateMovieRewind)
	// dock bar handlers confuse IDE's code completion, so keep them last
	#define MAINDOCKBARDEF(name, width, height, style) \
		ON_COMMAND(ID_VIEW_BAR_##name, OnViewBar##name) \
		ON_UPDATE_COMMAND_UI(ID_VIEW_BAR_##name, OnUpdateViewBar##name)
	#include "MainDockBarDef.h"	// generate docking bar message map entries
END_MESSAGE_MAP()

// CMainFrame message handlers

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

#if 0
	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
#endif
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnClose()
{
	if (!theApp.m_pPlaylist->CanCloseFrame(this)) {	// if save check fails
		return;	// cancel close
	}
	CFrameWndEx::OnClose();
}

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	// if the index parameter is 0xFFFFFFFF, DragQueryFile returns a count
	// of the files dropped
	UINT	nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	CString	sPatchPath;
	CString	sPlaylistPath;
	CStringArrayEx	aSnapshotPath;	// can load multiple snapshots at once
	CString	sMoviePath;
	for (UINT iFile = 0; iFile < nFiles; iFile++) {	// for each dropped file
		UINT	nPathLen = DragQueryFile(hDropInfo, iFile, NULL, 0);
		if (nPathLen > 0) {	// if valid path length
			// if the buffer parameter is NULL, DragQueryFile returns the
			// required size of the buffer in characters, not including
			// the terminating null character (hence the plus one below)
			CString	sPath;
			LPTSTR	pszPath = sPath.GetBuffer(nPathLen + 1);	// one extra for terminator
			DragQueryFile(hDropInfo, iFile, pszPath, MAX_PATH);
			sPath.ReleaseBuffer(nPathLen + 1);
			CString	sExt = PathFindExtension(sPath) + 1;	// skip dot 
			// if dropped file has the playlist extension
			if (!_tcsicmp(sExt, CPlaylist::m_pszPlaylistExt)) {
				sPlaylistPath = sPath;	// store playlist path
			} else {	// not a playlist
				// if dropped file has the snapshot extension
				if (!_tcsicmp(sExt, m_pszSnapshotExt)) {
					aSnapshotPath.Add(sPath);	// add to list of snapshots
				} else {	// not a snapshot
					// if dropped file has the movie extension
					if (!_tcsicmp(sExt, m_pszMovieExt)) {
						sMoviePath = sPath;	// store movie path
					} else {	// not a movie
						// assume dropped file is a patch
						sPatchPath = sPath;	// store patch path
					}
				}
			}
		}
	}
	if (!sPatchPath.IsEmpty()) {	// if patch was dropped
		theApp.OpenDocumentFile(sPatchPath);	// open patch
	}
	if (!sPlaylistPath.IsEmpty()) {	// if playlist was dropped
		theApp.m_pPlaylist->Open(sPlaylistPath);	// open playlist			
	}
	if (!aSnapshotPath.IsEmpty()) {	// if snapshots were dropped
		VERIFY(theApp.LoadSnapshot(aSnapshotPath[0]));	// load first snapshot
		m_aSnapshotPath.Swap(aSnapshotPath);	// swap array pointers, avoiding array copy
		m_iCurSnapshot = 0;	// reset current snapshot index
	} else {	// snapshots weren't dropped
		PlayMovie(sMoviePath);
	}
}

BOOL CMainFrame::OnDeviceChange(UINT nEventType, W64ULONG dwData)
{
//	_tprintf(_T("OnDeviceChange %x %x\n"), nEventType, dwData);
	BOOL	retc = CFrameWnd::OnDeviceChange(nEventType, dwData);
	if (nEventType == DBT_DEVNODES_CHANGED) {
		// use post so device change completes before our handler runs
		PostMessage(UWM_DEVICE_NODE_CHANGE);
	}
	return retc;	// true to allow device change
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case FRAME_RATE_TIMER_ID:
		{
			int	nPrevLen = m_sRingCount.GetLength();
			m_sRingCount.Format(_T("%lld"), theApp.m_thrRender.GetRingCount());
			FastSetPaneText(m_wndStatusBar, SBP_RING_COUNT, m_sRingCount, nPrevLen);
			double	fElapsedSecs = m_benchFrameRate.Reset();
			UINT_PTR	nFrameCount = theApp.m_thrRender.GetFrameCount();
			double	fFrameRate = (nFrameCount - m_nPrevFrameCount) * (1 / fElapsedSecs);
			m_nPrevFrameCount = nFrameCount;	// update previous frame count
			nPrevLen = m_sFrameRate.GetLength();
			m_sFrameRate.Format(_T("%.02f"), fFrameRate);
			FastSetPaneText(m_wndStatusBar, SBP_FRAME_RATE, m_sFrameRate, nPrevLen);
		}
		return;	// don't relay timer message to base class
	}
	CFrameWndEx::OnTimer(nIDEvent);
}

LRESULT CMainFrame::OnDelayedCreate(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	// The main window has been initialized, so show and update it
	ShowWindow(theApp.m_nCmdShow);
	theApp.GetView()->PostMessage(UWM_DELAYED_CREATE);
	theApp.MidiInit();	// initialize MIDI devices
	ASSERT(theApp.m_pPlaylist != NULL);
	theApp.m_pPlaylist->New();	// create initial playlist
	SetTimer(FRAME_RATE_TIMER_ID, FRAME_RATE_TIMER_PERIOD, NULL);
	return 0;
}

LRESULT	CMainFrame::OnHandleDlgKey(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	return theApp.HandleDlgKeyMsg((MSG *)wParam);
}

LRESULT CMainFrame::OnRenderWndClosed(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	theApp.SetDetached(false);
	return 0;
}

LRESULT CMainFrame::OnFullScreenChanged(WPARAM wParam, LPARAM lParam)
{
	// wParam: true if full screen, lParam: true if change succeeded
	theApp.OnFullScreenChanged(wParam != 0, lParam != 0);
	return 0;
}

LRESULT	CMainFrame::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	theApp.UpdateFrameRate();
	return 0;
}

LRESULT	CMainFrame::OnDeviceNodeChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	theApp.m_midiMgr.OnDeviceChange();
	return 0;
}

LRESULT	CMainFrame::OnBitmapCapture(WPARAM wParam, LPARAM lParam)
{
	// Assume this message was posted by the render thread in response to a
	// capture bitmap command. The message is posted even if capture failed.
	UNREFERENCED_PARAMETER(wParam);
	CComPtr<ID2D1Bitmap1> pBitmap(reinterpret_cast<ID2D1Bitmap1*>(lParam));
	if (!m_aOutputPath.IsEmpty()) {	// if output path available
		CString	sExportPath(m_aOutputPath[0]);	// copy oldest output path
		m_aOutputPath.RemoveAt(0);	// remove oldest output path from array
		if (pBitmap != NULL) {	// if capture succeeded
			CWhorldThread::WriteCapturedBitmap(pBitmap, sExportPath);	// export image
		}
	}
	return 0;
}

LRESULT	CMainFrame::OnSnapshotCapture(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	CAutoPtr<const CSnapshot> pSnapshot(reinterpret_cast<CSnapshot*>(lParam));
	if (pSnapshot != NULL) {	// if capture succeeded
		WriteSnapshot(pSnapshot);
	}
	return 0;
}

LRESULT	CMainFrame::OnParamChange(WPARAM wParam, LPARAM lParam)
{
	int	iParam = LOWORD(wParam);
	int	iProp = HIWORD(wParam);
	CComVariant	prop;
	switch (iProp) {
	case PARAM_PROP_Wave:	// waveform is assumed to be integer
		prop = static_cast<int>(lParam);
		break;
	default:	// all other properties are assumed to be float
		prop = CMidiManager::LParamToFloat(lParam);
	}
	CWhorldDoc*	pDoc = theApp.GetDocument();
	CWhorldDoc::CDisableUndo	noUndo(pDoc);
	pDoc->SetParam(iParam, iProp, prop, theApp.GetView());
	return 0;
}

LRESULT	CMainFrame::OnMasterPropChange(WPARAM wParam, LPARAM lParam)
{
	int	iProp = static_cast<int>(wParam);
	double	fVal = CMidiManager::LParamToFloat(lParam);
	CWhorldDoc*	pDoc = theApp.GetDocument();
	CWhorldDoc::CDisableUndo	noUndo(pDoc);
	pDoc->SetMasterProp(iProp, fVal, theApp.GetView());
	return 0;
}

LRESULT	CMainFrame::OnMainPropChange(WPARAM wParam, LPARAM lParam)
{
	int	iProp = static_cast<int>(wParam);
	VARIANT_PROP var;
	var.ullVal = lParam;
	CWhorldDoc*	pDoc = theApp.GetDocument();
	CWhorldDoc::CDisableUndo	noUndo(pDoc);
	pDoc->SetMainProp(iProp, var, theApp.GetView());
	return 0;
}

LRESULT CMainFrame::OnSetDrawMode(WPARAM wParam, LPARAM lParam)
{
	// draw mode is a special case due to being a bitmask; RWM of entire mode
	// wouldn't be thread-safe, so worker thread posts mask and value instead
	UINT	nMask = static_cast<UINT>(wParam);
	UINT	nValue = static_cast<UINT>(lParam);
	CWhorldDoc	*pDoc = theApp.GetDocument();
	UINT	nDrawMode = pDoc->m_main.nDrawMode;
	VARIANT_PROP var;
	// first clear any bits that are non-zero in the mask; then set
	// any bits that are non-zero in both the mask and the value
	var.uintVal = (nDrawMode & ~nMask) | (nValue & nMask);
	pDoc->SetMainProp(MAIN_DrawMode, var, theApp.GetView());
	return 0;
}

LRESULT CMainFrame::OnRenderQueueFull(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if (m_bInRenderFullError) {	// if already handling this error
		return 0;	// reentry disallowed
	}
	// save and set the reentrance guard flag; the flag's state will be
	// restored automatically when the save object goes out of scope
	CSaveObj<bool>	save(m_bInRenderFullError, true);
	AfxMessageBox(IDS_APP_ERR_RENDER_QUEUE_FULL);	// display error message
	// if render thread's command queue has free space
	if (!theApp.m_thrRender.IsCommandQueueFull()) {
		return 0;	// success
	}
	// the command queue is still full; more drastic measures are needed
	int	nResult = AfxMessageBox(IDS_APP_ERR_RENDERING_TOO_SLOW, MB_YESNO);
	if (nResult == IDNO) {	// if user chickened out of doing a reset
		return 0;	// cancel
	}
	// if the MIDI input device is open, and at least one MIDI mapping exists
	if (theApp.m_midiMgr.IsInputDeviceOpen() 
	&& theApp.m_midiMgr.m_midiMaps.GetCount() > 0) {
		// remove all MIDI mappings by loading a new playlist; if current
		// playlist was modified, user is prompted to save their changes
		if (!theApp.m_pPlaylist->New()) {	// if new playlist fails
			return 0;	// user probably canceled out of saving changes
		}
	}
	// if the MIDI input callback was spamming the command queue, 
	// it's no longer doing so, because we nuked the MIDI mappings
	CProgressDlg	dlgProgress(IDD_PROGRESS, this);
	if (!dlgProgress.Create()) {	// create progress dialog
		AfxMessageBox(IDS_APP_ERR_CANT_CREATE_PROGRESS_DLG);
		return 0;
	}
	// we don't know how long this will take, so use a marquee progress bar
	dlgProgress.SetMarquee(true, 0);
	// loop until the command queue drains down at least halfway
	while (!theApp.m_thrRender.IsCommandQueueBelowHalfFull()) {
		if (dlgProgress.Canceled()) {	// if user canceled
			return 0;	// cancel
		}
		// pump message blocks waiting for messages, but it won't block
		// for long because we're running a timer to update the status bar
		theApp.PumpMessage();	// keeps UI responsive
	}
	// the command queue is now assumed to be half full at most, so there's
	// plenty of room for two commands: SetPatch (via OnFileNew) and SetEmpty
	theApp.m_pDocManager->OnFileNew();	// prompts user to save their changes
	theApp.m_thrRender.SetEmpty();	// removes all rings from the drawing
	// successful recovery
	return 0;
}

void CMainFrame::OnFileExport()
{
	CString	sExportPath;
	if (PromptingForExport()) {
		CPathStr sFileName;
		if (theApp.IsSnapshotMode()) {	// if in snapshot mode
			sFileName = PathFindFileName(m_aSnapshotPath[m_iCurSnapshot]);
		} else {	// not in snapshot mode
			if (!theApp.GetDocument()->GetPathName().IsEmpty()) {	// if document was opened
				sFileName = theApp.GetDocument()->GetTitle();
			}
		}
		sFileName.RemoveExtension();
		// prompt user for export path
		CFileDialog	fd(false, m_pszExportExt, sFileName, OFN_OVERWRITEPROMPT, m_pszExportFilter);
		if (fd.DoModal() != IDOK) {	// display file dialog
			return;	// user canceled
		}
		sExportPath = fd.GetPathName();
		// prompt user for export options
		CExportDlg	dlg;
		if (dlg.DoModal() != IDOK) {	// display export options dialog
			return;	// user canceled
		}
	}
	// render thread captures bitmap and posts it to our main window for writing;
	// enqueue command ASAP in case we're unpaused, in which case sooner is better
	theApp.m_thrRender.CaptureBitmap(theApp.m_options.GetExportFlags(), 
		theApp.m_options.GetExportImageSize());
	if (sExportPath.IsEmpty()) {	// if export path is unspecified
		if (!MakeUniqueExportPath(sExportPath, m_pszExportExt))	// generate path
			return;	// unable to generate path
	}
	m_aOutputPath.Add(sExportPath);
	// bitmap capture message may already be waiting for us in message queue
}

void CMainFrame::OnFileTakeSnapshot()
{
	theApp.m_thrRender.CaptureSnapshot();
}

void CMainFrame::OnUpdateFileTakeSnapshot(CCmdUI* pCmdUI)
{
	// don't allow taking snapshots while in snapshot mode, it's too confusing
	pCmdUI->Enable(!theApp.IsSnapshotMode());
}

void CMainFrame::OnFileLoadSnapshot()
{
	// array of file type filter specifications, as IFileOpenDialog expects
	static const COMDLG_FILTERSPEC	aFilter[2] = {
		{L"Snapshot Files", L"*.whs"},	// Unicode only!
		{L"All Files", L"*.*"},
	};
	HRESULT hr = PromptForFiles(m_aSnapshotPath, _countof(aFilter), aFilter);
	if (SUCCEEDED(hr)) {	// if prompt succeeded
		m_iCurSnapshot = 0;	// reset current snapshot index
		VERIFY(theApp.LoadSnapshot(m_aSnapshotPath[0]));	// load the first snapshot
	} else {	// prompt failed
		if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) {	// if user didn't cancel
			theApp.OnError(FormatSystemError(hr), __FILE__, __LINE__, __DATE__);
		}
	}
}

void CMainFrame::OnUpdateFileLoadSnapshot(CCmdUI* pCmdUI)
{
	// you can view snapshots or play a movie, but not both
	pCmdUI->Enable(!IsPlayingMovie());
}

void CMainFrame::OnPlaylistNew()
{
	theApp.m_pPlaylist->New();
}

void CMainFrame::OnPlaylistOpen()
{
	theApp.m_pPlaylist->OpenPrompt();
}

void CMainFrame::OnPlaylistSave()
{
	theApp.m_pPlaylist->DoFileSave();
}

void CMainFrame::OnPlaylistSaveAs()
{
	theApp.m_pPlaylist->DoSave(NULL);
}

void CMainFrame::OnPlaylistMru(UINT nID)
{
	theApp.m_pPlaylist->OpenRecent(nID - ID_PLAYLIST_MRU_FILE1);
}

void CMainFrame::OnUpdatePlaylistMru(CCmdUI* pCmdUI)
{
	theApp.m_pPlaylist->UpdateMruMenu(pCmdUI);
}

void CMainFrame::OnImageRandomPhase()
{
	theApp.m_thrRender.RandomPhase();
}

void CMainFrame::OnSnapshotFirst()
{
	if (m_aSnapshotPath.GetSize() && m_iCurSnapshot > 0) {
		m_iCurSnapshot = 0;	// go to first
		theApp.LoadSnapshot(m_aSnapshotPath[m_iCurSnapshot]);
	}
}

void CMainFrame::OnSnapshotLast()
{
	if (m_iCurSnapshot < m_aSnapshotPath.GetSize() - 1) {
		m_iCurSnapshot = m_aSnapshotPath.GetSize() - 1;	// go to last
		theApp.LoadSnapshot(m_aSnapshotPath[m_iCurSnapshot]);
	}
}

void CMainFrame::OnSnapshotNext()
{
	if (m_iCurSnapshot < m_aSnapshotPath.GetSize() - 1) {
		m_iCurSnapshot++;	// go to next
		theApp.LoadSnapshot(m_aSnapshotPath[m_iCurSnapshot]);
	}
}

void CMainFrame::OnSnapshotPrev()
{
	if (m_aSnapshotPath.GetSize() && m_iCurSnapshot > 0) {
		m_iCurSnapshot--;	// go to previous
		theApp.LoadSnapshot(m_aSnapshotPath[m_iCurSnapshot]);
	}
}

void CMainFrame::OnSnapshotExportAll()
{
	int	nSnapshots = m_aSnapshotPath.GetSize();
	if (nSnapshots <= 0)	// if no snapshots loaded
		return;	// nothing to do
	CString	sExportFolder(theApp.m_options.m_Export_sImageFolder);
	if (PromptingForExport()) {
		// prompt user for export destination folder
		UINT	nFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
		CString	sFDlgTitle(LDS(IDS_EXPORT_ALL_SNAPSHOTS));
		if (!CFolderDialog::BrowseFolder(sFDlgTitle, sExportFolder, NULL, nFlags, sExportFolder))
			return;	// user canceled
		// prompt user for export options
		CExportDlg	dlg;
		if (dlg.DoModal() != IDOK) {	// display export options dialog
			return;	// user canceled
		}
	}
	if (!PathFileExists(sExportFolder)) {	// if export image folder doesn't exist
		AfxMessageBox(IDS_APP_ERR_BAD_EXPORT_IMAGE_FOLDER);
		return;	// fail
	}
	CProgressDlg	dlgProgress(IDD_PROGRESS, this);
	if (!dlgProgress.Create()) {
		AfxMessageBox(IDS_APP_ERR_CANT_CREATE_PROGRESS_DLG);
		return;	// fail
	}
	dlgProgress.SetRange(0, nSnapshots);
	for (int iSnapshot = 0; iSnapshot < nSnapshots; iSnapshot++) {	// for each snapshot
		CString	sSnapshotPath(m_aSnapshotPath[iSnapshot]);
		theApp.LoadSnapshot(sSnapshotPath);	// load snapshot in render thread
		// request render thread to capture snapshot as an image
		theApp.m_thrRender.CaptureBitmap(theApp.m_options.GetExportFlags(), 
			theApp.m_options.GetExportImageSize());
		CPathStr	sFilename(PathFindFileName(sSnapshotPath));
		sFilename.RenameExtension('.' + CString(m_pszExportExt));
		CPathStr	sExportPath(sExportFolder);
		sExportPath.Append(sFilename);
		m_aOutputPath.Add(sExportPath);	// push path for captured image
		// wait for render thread to post bitmap capture message
		if (!WaitForPostedMessage(UWM_BITMAP_CAPTURE, dlgProgress)) {
			m_aOutputPath.RemoveAll();	// clean up paths
			break;	// user canceled, or error
		}
		dlgProgress.SetPos(iSnapshot);	// pumps messages
	}
	// restore current snapshot
	theApp.LoadSnapshot(m_aSnapshotPath[m_iCurSnapshot]);
}

void CMainFrame::OnSnapshotInfo()
{
	if (m_iCurSnapshot < m_aSnapshotPath.GetSize()) {	// if we have a snapshot
		LPCTSTR	pszPath = m_aSnapshotPath[m_iCurSnapshot];
		// read snapshot and assign it to a smart pointer
		CAutoPtr<const CSnapshot>	pSnapshot(CSnapshot::Read(pszPath));
		if (pSnapshot != NULL) {	// if snapshot read succeeded
			CPathStr	sFilename(PathFindFileName(pszPath));
			sFilename.RemoveExtension();
			CString	sTargetSize(ValToString(pSnapshot->m_drawState.szTarget.width)
				+ _T(" x ") + ValToString(pSnapshot->m_drawState.szTarget.height));
			int	nVersion = !(pSnapshot->m_drawState.nFlags & CSnapshot::SF_V1) + 1;
			CString	sVersion(ValToString(nVersion));
			CString	sInfo(
				+ _T("Name:\t") + sFilename
				+ _T("\nSize:\t") + sTargetSize
				+ _T("\nZoom:\t") + ValToString(pSnapshot->m_drawState.fZoom)
				+ _T("\nRings:\t") + ValToString(pSnapshot->m_drawState.nRings)
				+ _T("\nVersion:\t") + sVersion
			);
			MessageBox(sInfo, _T("Snapshot"), MB_ICONINFORMATION);
		}
	}
}

void CMainFrame::OnUpdateSnapshot(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(theApp.IsSnapshotMode());
}

#define MAINDOCKBARDEF(name, width, height, style) \
	void CMainFrame::OnViewBar##name() \
	{ \
		m_wnd##name##Bar.ToggleShowPane(); \
	} \
	void CMainFrame::OnUpdateViewBar##name(CCmdUI *pCmdUI) \
	{ \
		pCmdUI->SetCheck(m_wnd##name##Bar.IsVisible()); \
	}
#include "MainDockBarDef.h"	// generate docking bar message handlers

void CMainFrame::OnViewOptions()
{
	COptions	m_optsPrev(theApp.m_options);
	COptionsDlg	dlg;
	if (dlg.DoModal() == IDOK) {	// display options dialog
		// apply new options, passing pointer to old ones
		ApplyOptions(&m_optsPrev);
	}
}

void CMainFrame::OnViewMidiLearn()
{
	theApp.m_midiMgr.SetLearnMode(!theApp.m_midiMgr.IsLearnMode());
	m_wndMappingBar.OnLearnMode();
}

void CMainFrame::OnUpdateViewMidiLearn(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.m_midiMgr.IsLearnMode());
	pCmdUI->Enable(m_wndMappingBar.FastIsVisible() && theApp.m_midiMgr.m_midiMaps.GetCount());
}

void CMainFrame::OnWindowFullscreen()
{
	theApp.SetFullScreen(!theApp.IsFullScreen());
}

void CMainFrame::OnUpdateWindowFullscreen(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(theApp.IsFullScreen());
	pCmdUI->Enable(!theApp.IsFullScreenChanging());
}

void CMainFrame::OnWindowDetach()
{
	theApp.SetDetached(!theApp.IsDetached());
}

void CMainFrame::OnUpdateWindowDetach(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(theApp.IsDetached());
	pCmdUI->Enable(!theApp.IsFullScreen());
}

void CMainFrame::OnWindowPause()
{
	if (IsPlayingMovie()) {
		PauseMovie(!m_bIsMoviePaused);
	} else {
		theApp.SetPause(!theApp.IsPaused());
	}
}

void CMainFrame::OnUpdateWindowPause(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsPaused());
}

void CMainFrame::OnWindowStep()
{
	theApp.m_thrRender.SingleStep();
}

void CMainFrame::OnUpdateWindowStep(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPaused());
}

void CMainFrame::OnWindowClear()
{
	theApp.m_thrRender.SetEmpty();
}

void CMainFrame::OnUpdateWindowClear(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!IsPlayingMovie());
}

void CMainFrame::OnWindowResetLayout()
{
	UINT	nType = MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING;
	if (AfxMessageBox(IDS_WINDOW_RESET_LAYOUT_WARN, nType) == IDYES) {	// get confirmation
		// document overrides CanCloseFrame to reset flag if close is canceled
		theApp.m_bCleanStateOnExit = true;
		PostMessage(WM_CLOSE);
	}
}

void CMainFrame::OnMovieRecord()
{
	if (IsMovieOpen()) {	// if movie is open
		RecordMovie(NULL);	// stop recording
	} else {	// movie is closed
		CString	sMoviePath;
		if (PromptingForExport()) {
			CFileDialog	fd(false, m_pszMovieExt, NULL, OFN_OVERWRITEPROMPT, m_pszMovieFilter);
			if (fd.DoModal() != IDOK) {	// display file dialog
				return;	// user canceled
			}
			sMoviePath = fd.GetPathName();
		} else {	// not prompting
			if (!MakeUniqueExportPath(sMoviePath, m_pszMovieExt))	// generate path
				return;	// unable to generate path
		}
		RecordMovie(sMoviePath);	// start recording
	}
}

void CMainFrame::OnUpdateMovieRecord(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsRecordingMovie());
	pCmdUI->Enable(!IsPlayingMovie());
}

void CMainFrame::OnMoviePlay()
{
	if (IsMovieOpen()) {	// if movie is open
		PlayMovie(NULL);	// stop playback
	} else {	// movie is closed
		CFileDialog	fd(true, m_pszMovieExt, NULL, OFN_HIDEREADONLY, m_pszMovieFilter);
		if (fd.DoModal() == IDOK) {
			PlayMovie(fd.GetPathName());	// start playback
		}
	}
}

void CMainFrame::OnUpdateMoviePlay(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(IsPlayingMovie());
	pCmdUI->Enable(!IsRecordingMovie());
}

void CMainFrame::OnMovieExport()
{
	CString	sExportFolder(theApp.m_options.m_Export_sImageFolder);
	UINT	nFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
	CString	sFDlgTitle(LDS(IDS_EXPORT_MOVIE));
	if (!CFolderDialog::BrowseFolder(sFDlgTitle, sExportFolder, NULL, nFlags, sExportFolder)) {
		return;	// user canceled
	}
	CMovieExportDlg	dlgMovieExport;
	if (dlgMovieExport.DoModal() != IDOK) {
		return;	// user canceled
	}
	CProgressDlg	dlgProgress(IDD_PROGRESS, this);
	if (!dlgProgress.Create()) {
		AfxMessageBox(IDS_APP_ERR_CANT_CREATE_PROGRESS_DLG);
		return;	// fail
	}
	dlgProgress.SetWindowText(sFDlgTitle);
	LONGLONG	nFrames = theApp.m_thrRender.GetMovieFrameCount();
	dlgProgress.SetRange(0, static_cast<int>(nFrames));
	CMovieExportParams	mep;
	mep.m_sFolderPath = sExportFolder;
	mep.m_szFrame = dlgMovieExport.m_szFrame;
	mep.m_nStartFrame = 0;
	mep.m_nEndFrame = nFrames - 1;
	mep.m_nExportFlags = dlgMovieExport.m_nScalingType;
	LONG	nTaskID;
	if (!theApp.m_thrRender.MovieExport(mep, nTaskID)) {
		return;	// command queue was full and error recovery failed
	}
	// render thread pauses movie playback while exporting movie
	m_bIsMoviePaused = true;	// so keep UI consistent with that
	// We need to update the toolbar explicitly instead of relying on update
	// notifications, because we're modal and the progress dialog has focus.
	m_wndToolBar.OnUpdateCmdUI(this, false);	// update pause button
	LONGLONG	iFrame;
	// loop until all frames are exported or the user cancels
	while ((iFrame = theApp.m_thrRender.GetReadFrameIdx()) < nFrames) {
		dlgProgress.SetPos(static_cast<int>(iFrame));	// pumps messages
		if (dlgProgress.Canceled()) {	// if user clicked cancel
			theApp.m_thrRender.CancelTask(nTaskID);	// cancel task
			break;
		}
		WaitMessage();	// suspend until a message arrives
	}
}

void CMainFrame::OnUpdateMovieExport(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPlayingMovie());
}

void CMainFrame::OnMovieRewind()
{
	theApp.m_thrRender.MovieSeek(0);
}

void CMainFrame::OnUpdateMovieRewind(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPlayingMovie());
}
