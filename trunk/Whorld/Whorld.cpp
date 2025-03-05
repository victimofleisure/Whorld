// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		21feb25	add options
		02		26feb25	add MIDI input
		03		28feb25	add playlist
		04		03mar25	if command queue is full, worker thread now retries

*/

// Whorld.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Whorld.h"
#include "MainFrm.h"
#include "AboutDlg.h"

#include "WhorldDoc.h"
#include "WhorldView.h"
#include "Win32Console.h"
#include "Persist.h"
#include "SaveObj.h"
#include "FocusEdit.h"
#include "afxregpath.h"
#include "AppRegKey.h"
#include "OptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ON_ERROR(x) OnError(x, __FILE__, __LINE__, __DATE__);

#define RK_RENDER_WND _T("RenderWnd")
#define RK_RESOURCE_VERSION _T("nResourceVersion")

const int CWhorldApp::m_nNewResourceVersion = 6;	// increment if resource change breaks customization

// CWhorldApp construction

CWhorldApp::CWhorldApp()
{
	m_bHiColorIcons = TRUE;
	SetAppID(_T("Whorld"));
	m_pView = NULL;
	m_pDocument = NULL;
	m_bIsDetached = false;
	m_bIsFullScreen = false;
	m_bIsFullScreenChanging = false;
	m_bDetachedPreFullScreen = false;
	m_bIsDualMonitor = false;
	m_bIsPaused = false;
	m_hKeyboardHook = NULL;
	m_nOldResourceVersion = 0;
}

// The one and only CWhorldApp object

CWhorldApp theApp;

// CWhorldApp initialization

BOOL CWhorldApp::InitInstance()
{
	InitWhorldBase();	// initialize Whorld base class

#if _DEBUG
	Win32Console::Create();	// create console window for debugging
#endif

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	LPCTSTR	pPrevAppName = m_pszAppName;
	m_pszAppName = _T("Whorld2");	// create new profile for version 2
	SetRegistryKey(_T("Anal Software"));
	m_pszAppName = pPrevAppName;
	m_nOldResourceVersion = GetProfileInt(REG_SETTINGS, RK_RESOURCE_VERSION, 0);
	m_options.ReadProperties();	// get options from registry
	LoadStdProfileSettings(m_options.m_General_nMRUItems);  // Load standard INI file options (including MRU)
	m_midiMgr.Initialize();
	m_pPlaylist.Attach(new CPlaylist);
	if (m_pPlaylist == NULL)	// if can't create playlist
		return FALSE;

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWhorldDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWhorldView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The stock code shows and updates the main window here, but this makes the
	// view flicker due to being painted twice; it's solved by moving show/update
	// to CMainFrame::OnDelayedCreate which runs after the window sizes stabilize

	// now that we're up, check for resource version change, and update profile if needed
	if (m_nNewResourceVersion != m_nOldResourceVersion) {	// if resource version changed
		WriteProfileInt(REG_SETTINGS, RK_RESOURCE_VERSION, m_nNewResourceVersion);
	}

	return TRUE;
}

int CWhorldApp::ExitInstance()
{
	m_midiMgr.CloseInputDevice();	// close MIDI input device; no more callbacks
	m_thrRender.DestroyThread();
	m_wndRender.DestroyWindow();
	m_options.WriteProperties();	// save options to registry
	m_midiMgr.WriteDevices();	// save MIDI device state to registry
	AfxOleTerm(FALSE);
	if (m_bCleanStateOnExit) {
		ResetWindowLayout();	// delete window layout keys
		CleanState();	// delete workspace key
		RestartApp();	// launch new instance of app
	}
	return CWinAppEx::ExitInstance();
}

void CWhorldApp::ResetWindowLayout()
{
	// registry keys listed here will be deleted
	static const LPCTSTR pszCleanKey[] = {
		#define MAINDOCKBARDEF(name, width, height, style) RK_##name##Bar,
		#include "MainDockBarDef.h"	// generate keys for main dockable bars
		REG_SETTINGS,
		_T("Options\\Expand"),
	};
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);
	int	nKeys = _countof(pszCleanKey);
	for (int iKey = 0; iKey < nKeys; iKey++)	// for each listed key
		reg.DeleteKey(AFXGetRegPath(pszCleanKey[iKey]));
}

void CWhorldApp::OnError(CString sErrorMsg, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate)
{
	DWORD	nError = GetLastError();
	if (nError != ERROR_SUCCESS) {	// if last error isn't success
		// add error code and its translation to message
		CString	sErrorCode;
		sErrorCode.Format(_T("\nSystem Error %d (0x%x).\n"), nError, nError);
		sErrorMsg += sErrorCode + FormatSystemError(nError);
	}
	CString	sSrcFileName(pszSrcFileName);	// convert to Unicode
	CString	sSrcFileDate(pszSrcFileDate);
	if (sErrorMsg.Right(1) != '\n')
		sErrorMsg += '\n';
	CString sContext;
	sContext.Format(_T("%s line %d (%s)"), sSrcFileName.GetString(), nLineNum, sSrcFileDate.GetString());
	sErrorMsg += sContext;
	Log(sErrorMsg);
	AfxMessageBox(sErrorMsg);
}

bool CWhorldApp::HandleDlgKeyMsg(MSG* pMsg)
{
	static const LPCSTR	EditBoxCtrlKeys = "ACHVX";	// Z reserved for app undo
	CMainFrame	*pMain = GetMainFrame();
	ASSERT(pMain != NULL);	// main frame must exist
	switch (pMsg->message) {
	case WM_KEYDOWN:
		{
			int	nVKey = INT64TO32(pMsg->wParam);
			bool	bTryMainAccels = false;	// assume failure
			if ((nVKey >= VK_F1 && nVKey <= VK_F24) || nVKey == VK_ESCAPE) {
				bTryMainAccels = true;	// function key or escape
			} else {
				bool	bIsAlpha = nVKey >= 'A' && nVKey <= 'Z';
				CEdit	*pEdit = CFocusEdit::GetEdit();
				if (pEdit != NULL) {	// if an edit control has focus
					if ((bIsAlpha								// if (alpha key
					&& strchr(EditBoxCtrlKeys, nVKey) == NULL	// and unused by edit
					&& (GetKeyState(VK_CONTROL) & GKS_DOWN))	// and Ctrl is down)
					|| (!bIsAlpha								// or (non-alpha key
					&& (GetKeyState(VK_CONTROL) & GKS_DOWN))	// and Ctrl is down)
					|| (nVKey == VK_SPACE						// or (space key
					&& ((GetKeyState(VK_CONTROL) & GKS_DOWN))	// and Ctrl is down)
					|| (GetKeyState(VK_SHIFT) & GKS_DOWN)))	{	// or Shift is down
						bTryMainAccels = true;	// give main accelerators a try
					}
				} else {	// non-edit control has focus
					if (bIsAlpha								// if alpha key
					|| nVKey == VK_SPACE						// or space key
					|| (GetKeyState(VK_CONTROL) & GKS_DOWN)		// or Ctrl is down
					|| (GetKeyState(VK_SHIFT) & GKS_DOWN)) {	// or Shift is down
						bTryMainAccels = true;	// give main accelerators a try
					}
				}
			}
			if (bTryMainAccels) {
				HACCEL	hAccel = pMain->GetAccelTable();
				if (hAccel != NULL && TranslateAccelerator(pMain->m_hWnd, hAccel, pMsg)) {
					return true;	// message was translated, stop dispatching
				}
			}
		}
		break;
	case WM_SYSKEYDOWN:
		{
			if (GetKeyState(VK_SHIFT) & GKS_DOWN)	// if context menu
				return false;	// keep dispatching (false alarm)
			pMain->SetFocus();	// main frame must have focus to display menus
			HACCEL	hAccel = pMain->GetAccelTable();
			if (hAccel != NULL && TranslateAccelerator(pMain->m_hWnd, hAccel, pMsg)) {
				return true;	// message was translated, stop dispatching
			}
		}
		break;
	}
	return false;	// continue dispatching
}

// CWhorldApp customization load/save methods

void CWhorldApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CWhorldApp::LoadCustomState()
{
}

void CWhorldApp::SaveCustomState()
{
}

void CWhorldApp::ApplyOptions(const COptions *pPrevOptions)
{
	int	iPrevInputDev = m_midiMgr.GetDeviceIdx(CMidiDevices::INPUT);
	m_midiMgr.SetDeviceIdx(CMidiDevices::INPUT, m_options.m_Midi_iInputDevice - 1);
	if (pPrevOptions != NULL) {
		if (m_midiMgr.GetDeviceIdx(CMidiDevices::INPUT) != iPrevInputDev) {	// if MIDI input device changed
			m_midiMgr.ReopenInputDevice();
		}
		// if maximum number of recently used items changed
		if (m_options.m_General_nMRUItems != pPrevOptions->m_General_nMRUItems) {
			SetRecentFileListSize(m_options.m_General_nMRUItems);
			SetRecentFileListSize(m_pPlaylist->m_listRecentFile, m_options.m_General_nMRUItems);
		}
	}
}

void CWhorldApp::Log(CString sMsg)
{
#if _DEBUG
	SYSTEMTIME	sysTime;
	GetSystemTime(&sysTime);
	CString	sTime;
	sTime.Format(_T("%02d:%02d:%02d.%03d"), sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	sMsg.Replace('\n', ' ');	// remove internal newlines
	sMsg += '\n';	// append trailing newline
	_fputts(sTime + ' ' + sMsg, stdout);
#endif
}

bool CWhorldApp::CreateRenderWnd(DWORD dwStyle, CRect& rWnd, CWnd *pParentWnd)
{
	// create rendering window
	if (!m_wndRender.CreateWnd(dwStyle, rWnd, pParentWnd)) {
		ON_ERROR(LDS(IDS_APP_ERR_CANT_CREATE_RENDER_WINDOW));
		return false;
	}
	// create rendering thread
	if (!m_thrRender.CreateThread(m_wndRender.m_hWnd, m_pMainWnd)) {
		ON_ERROR(LDS(IDS_APP_ERR_CANT_CREATE_RENDER_THREAD));
		return false;
	}
	return true;
}

void CWhorldApp::DestroyRenderWnd()
{
	m_thrRender.DestroyThread();	// destroy render thread first as it uses render window
	if (m_wndRender.m_hWnd) {	// if render window exists
		if (m_bIsDetached) {	// if render window is detached
			// save render window's placement in registry
			CPersist::SaveWnd(REG_SETTINGS, &m_wndRender, RK_RENDER_WND);
		} else {	// render window is attached to view
			m_wndRender.SetParent(NULL);	// make render window a top-level window
			// change render window style's from child to overlapped and invisible
			m_wndRender.ModifyStyle(WS_CHILD | WS_VISIBLE, WS_OVERLAPPED);
		}
		m_wndRender.DestroyWindow();
	}
}

void CWhorldApp::ResizeRenderWnd(int cx, int cy)
{
	if (m_wndRender.m_hWnd) {	// if render window exists
		ASSERT(!m_bIsDetached);	// only if render window is a child
		m_wndRender.MoveWindow(0, 0, cx, cy);	// resize window
	}
}

bool CWhorldApp::SetDetached(bool bEnable)
{
	ASSERT(m_wndRender.m_hWnd);
	if (bEnable == m_bIsDetached)	// if already in requested state
		return true;	// nothing to do
	if (m_bIsFullScreen)	// if full screen
		return false;	// detach isn't allowed
	CRect	r;
	WINDOWPLACEMENT	wp = {};	// initalize to zero
	// SWP_FRAMECHANGED is required because we're changing frame styles;
	// SWP_NOCOPYBITS is advisable to reduce glitch as window changes size
	UINT	nSetPosFlags = SWP_FRAMECHANGED | SWP_NOCOPYBITS | SWP_NOZORDER;
	if (bEnable) {	// if detaching render window from view
		m_pView->GetWindowRect(r);	// get view's rect in screen coords
		m_wndRender.SetParent(NULL);	// make render window a top-level window
		m_wndRender.MoveWindow(r);	// prevent render window from jumping around
		// remove child style and add overlapped style, normally via the combined style
		// WS_OVERLAPPEDWINDOW which includes a caption bar, but if full screen mode is
		// changing, the caption would cause visual glitch so use WS_OVERLAPPED instead
		UINT	dwAddStyle = m_bIsFullScreenChanging ? WS_OVERLAPPED : WS_OVERLAPPEDWINDOW;
		m_wndRender.ModifyStyle(WS_CHILDWINDOW, dwAddStyle);
		// if full screen mode isn't changing, and window placement is available
		if (!m_bIsFullScreenChanging	
		&& CPersist::GetWndPlacement(REG_SETTINGS, RK_RENDER_WND, wp)) {
			nSetPosFlags |= SWP_NOMOVE | SWP_NOSIZE;	// don't move or size render window
		} else {	// offset render window from view
			int	nOffset = GetSystemMetrics(SM_CYCAPTION) * 2;
			r.OffsetRect(nOffset, nOffset);
		}
	} else {	// attaching render window to view
		CPersist::SaveWnd(REG_SETTINGS, &m_wndRender, RK_RENDER_WND);
		m_wndRender.SetParent(m_pView);
		m_wndRender.ModifyStyle(WS_OVERLAPPEDWINDOW, WS_CHILDWINDOW);
		m_pView->GetClientRect(r);
	}
	// If you change any of the frame styles, you must call SetWindowPos 
	// with the SWP_FRAMECHANGED flag for the cache to be updated properly.
	m_wndRender.SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), nSetPosFlags);
	if (wp.length) {	// if placement was obtained above
		m_wndRender.SetWindowPlacement(&wp);	// restore placement
	}
	if (!bEnable) {	// if attaching render window to view
		m_pView->SetFocus();	// set focus to view
	}
	m_bIsDetached = bEnable;	// update state data member
	return true;
}

bool CWhorldApp::SetFullScreen(bool bEnable)
{
	if (bEnable == m_bIsFullScreen)	// if already in requested state
		return true;	// nothing to do
	if (m_bIsFullScreenChanging)	// if full screen change in progress
		return false;	// not allowed
	// set full screen change flag, but automatically restore its previous
	// state when CSaveObj goes out of scope, unless m_objOld is changed
	CSaveObj<bool>	objFullScreenChanging(m_bIsFullScreenChanging, true);
	if (bEnable) {	// if entering full screen mode
		if (!GetMonitorConfig(m_bIsDualMonitor))	// get monitor configuration
			return false;	// error already reported
		m_bDetachedPreFullScreen = m_bIsDetached;	// save detached state
		if (!SetDetached(true)) {	// detach render window from view
			ON_ERROR(LDS(IDS_APP_ERR_CANT_DETACH_RENDER_WINDOW));
			return false;
		}
		if (IsSingleMonitor()) {	// if single monitor configuration
			if (!SetSingleMonitorExclusive(true))
				return false;
		}
	} else {	// entering windowed mode
		if (IsSingleMonitor()) {	// if single monitor configuration
			SetSingleMonitorExclusive(false);
		}
	}
	m_thrRender.SetFullScreen(bEnable);
	m_bIsFullScreen = bEnable;	// update state data member
	objFullScreenChanging.m_objOld = true;	// set flag for real
	return true;
}

void CWhorldApp::OnFullScreenChanged(bool bIsFullScreen, bool bResult)
{
	m_bIsFullScreen = bIsFullScreen;	// update state data member
	if (m_wndRender.m_hWnd) {	// if render window exists
		if (!bIsFullScreen) {	// if changed to windowed mode
			// if render window was attached to view when full-screen mode was entered
			if (!m_bDetachedPreFullScreen) {
				SetDetached(false);	// re-attach render window to view
			}
		}
	}
	m_bIsFullScreenChanging = false;
	if (!bResult) {	// if mode change failed
		SetSingleMonitorExclusive(false);	// do cleanup
	}
}

bool CWhorldApp::SetSingleMonitorExclusive(bool bEnable)
{
	if (bEnable) {	// if entering exclusive
		if (!SetKeyboardHook())	// set keyboard hook
			return false;
	} else {	// exiting exclusive
		RemoveKeyboardHook();	// remove keyboard hook
	}
	GetMainFrame()->m_wndMenuBar.ShowPane(!bEnable, false, false);	// show or hide main menus
	return true;
}

BOOL CWhorldApp::GetNearestMonitorInfo(HWND hWnd, MONITORINFOEX& monInfo)
{
	HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	ZeroMemory(&monInfo, sizeof(monInfo));
	monInfo.cbSize = sizeof(monInfo);
	return GetMonitorInfo(hMon, &monInfo);
}

bool CWhorldApp::GetMonitorConfig(bool& bIsDualMonitor)
{
	MONITORINFOEX monInfoRender;
	if (!GetNearestMonitorInfo(m_wndRender.m_hWnd, monInfoRender)) {
		ON_ERROR(LDS(IDS_APP_ERR_CANT_GET_MONITOR_INFO));
		return false;
	}
	MONITORINFOEX monInfoMain;
	if (!GetNearestMonitorInfo(m_pMainWnd->m_hWnd, monInfoMain)) {
		ON_ERROR(LDS(IDS_APP_ERR_CANT_GET_MONITOR_INFO));
		return false;
	}
	// if device names differ, it's a dual-monitor setup
	bIsDualMonitor = _tcsicmp(monInfoRender.szDevice, monInfoMain.szDevice) != 0;
	return true;
}

bool CWhorldApp::SetKeyboardHook()
{
	if (m_hKeyboardHook != NULL)	// if hooks already set
		return true;	// do nothing
	m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardHookProc, NULL, GetCurrentThreadId());
	if (m_hKeyboardHook == NULL) {
		ON_ERROR(LDS(IDS_APP_ERR_CANT_SET_KEYBOARD_HOOK));
		return false;
	}
	return true;
}

bool CWhorldApp::RemoveKeyboardHook()
{
	if (m_hKeyboardHook != NULL) {	// if hook was set
		BOOL	bResult = UnhookWindowsHookEx(m_hKeyboardHook);
		m_hKeyboardHook = NULL;
		if (!bResult) {
			ON_ERROR(LDS(IDS_APP_ERR_CANT_REMOVE_KEYBOARD_HOOK));
			return false;
		}
	}
	return true;
}

LRESULT CALLBACK CWhorldApp::KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case VK_MENU:
		return true;	// eat the key
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

LRESULT	CWhorldApp::OnTrackingHelp(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	return 0;
}

bool CWhorldApp::UpdateFrameRate()
{
	MONITORINFOEX monInfoRender;
	if (!GetNearestMonitorInfo(m_wndRender.m_hWnd, monInfoRender)) {
		ON_ERROR(LDS(IDS_APP_ERR_CANT_GET_MONITOR_INFO));
		return false;
	}
	DEVMODE	mode;
	ZeroMemory(&mode, sizeof(mode));
	mode.dmSize = sizeof(mode);
	if (!EnumDisplaySettings(monInfoRender.szDevice, ENUM_CURRENT_SETTINGS, &mode)) {
		ON_ERROR(LDS(IDS_APP_ERR_CANT_GET_DISPLAY_SETTINGS));
		return false;
	}
	if (mode.dmDisplayFrequency != m_thrRender.GetFrameRate()) {	// if display frequency changed
		m_thrRender.SetFrameRate(mode.dmDisplayFrequency);
	}
	return true;
}

CString CWhorldApp::GetTimestampFileName() const
{
	SYSTEMTIME	t;
	GetSystemTime(&t);
	CString	sFileName;
	sFileName.Format(_T("-%04d-%02d-%02d-%02d-%02d-%02d-%03d"),
		t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
	return m_pszAppName + sFileName;
}

void CWhorldApp::MidiInit()
{
	m_midiMgr.ReadDevices();	// get MIDI devices from registry
	m_options.UpdateMidiDevices();	// copy MIDI devices to options
	m_midiMgr.OpenInputDevice();
}

void CWhorldApp::SetPause(bool bEnable)
{
	// The app pause state is only valid from within the main thread;
	// the render thread has its own paused state, which may differ.
	if (bEnable == m_bIsPaused)	// if already in requested state
		return;	// nothing to do
	m_thrRender.SetPause(bEnable);	// request render thread to enter specified pause state
	m_bIsPaused = bEnable;	// update our paused state
	if (!bEnable) {	// if we're unpausing
		SetSnapshotMode(false);
	}
}

bool CWhorldApp::LoadSnapshot(LPCTSTR pszPath)
{
	CSnapshot	*pSnapshot = CSnapshot::Read(pszPath);
	if (pSnapshot == NULL) {	// if read failed
		return false;	// error already handled
	}
	SetSnapshotMode(true);
	// update zoom in UI to snapshot's zoom
	CWhorldDoc*	pDoc = GetDocument();
	pDoc->m_master.fZoom = pSnapshot->m_state.fZoom;
	CWhorldDoc::CPropHint	hint(MASTER_Zoom);	// master property index
	// display snapshot command updates render thread's zoom, so specify
	// view as sender to prevent view from pushing needless zoom command
	pDoc->UpdateAllViews(GetView(), HINT_MASTER, &hint);
	return m_thrRender.DisplaySnapshot(pSnapshot);	// request render thread to display snapshot
}

void CWhorldApp::SetSnapshotMode(bool bEnable)
{
	if (bEnable == IsSnapshotMode())	// if already in requested mode
		return;	// nothing to do
	if (bEnable) {	// if entering snapshot mode
		SetPause(true);	// pause render updates while showing snapshot
		CWhorldDoc*	pDoc = GetDocument();
		if (m_pPreSnapshotModePatch == NULL) {	// if pre-snapshot mode backup unmade
			CPatch&	patch = *pDoc;	// upcast from document to patch data
			// create copy of current patch on heap and attach copy to member pointer
			m_pPreSnapshotModePatch.Attach(new CPatch(patch));
		}
	} else {	// exiting snapshot mode
		ASSERT(m_pPreSnapshotModePatch != NULL);	// otherwise logic error
		// snapshot is deleted when smart pointer goes out of scope
		CAutoPtr<CPatch> pOldPatch(m_pPreSnapshotModePatch);	// take ownership
		CWhorldDoc*	pDoc = GetDocument();
		pDoc->GetPatch() = *pOldPatch;	// copy previously saved patch to document
		pDoc->UpdateAllViews(NULL);	// notify views of new patch
	}
}

// CWhorldApp message map

BEGIN_MESSAGE_MAP(CWhorldApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CWhorldApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	ON_COMMAND(ID_APP_HOME_PAGE, OnAppHomePage)
END_MESSAGE_MAP()

// CWhorldApp message handlers

void CWhorldApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CWhorldApp::OnAppHomePage()
{
	GotoUrl(_T("https://whorld.sourceforge.net/"));
}
