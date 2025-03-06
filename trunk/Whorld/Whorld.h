// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
        01      20feb25	add bitmap capture and write
		02		21feb25	add options
		03		26feb25	add MIDI input
		04		28feb25	add playlist

*/

// Whorld.h : main header file for the Whorld application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "WinAppCK.h"
#include "RenderWnd.h"
#include "WhorldThread.h"
#include "Options.h"
#include "MidiManager.h"
#include "Playlist.h"

// CWhorldApp:
// See Whorld.cpp for the implementation of this class
//

class CMainFrame;
class CWhorldView;
class CWhorldDoc;

class CWhorldApp : public CWinAppCK, public CWhorldBase
{
// Construction
public:
	CWhorldApp();

// Attributes
	CMainFrame	*GetMainFrame() const;
	void	SetView(CWhorldView *pView);
	void	SetDocument(CWhorldDoc *pDocument);
	CWhorldView	*GetView();
	CWhorldDoc	*GetDocument();
	bool	IsDetached() const;
	bool	IsFullScreen() const;
	bool	IsFullScreenChanging() const;
	bool	IsDualMonitor() const;
	bool	IsSingleMonitor() const;
	bool	IsFullScreenSingleMonitor() const;
	bool	IsRenderWndCreated() const;
	bool	IsPaused() const;
	void	SetPause(bool bEnable);
	bool	IsSnapshotMode() const;
	void	SetSnapshotMode(bool bEnable);
	bool	ResourceVersionChanged() const;
	static bool	IsMainThread();

// Operations
	void	ApplyOptions(const COptions *pPrevOptions);
	bool	WriteLogEntry(CString sLogEntry);
	bool	SetDetached(bool bEnable);
	bool	SetFullScreen(bool bEnable);
	bool	SetSingleMonitorExclusive(bool bEnable);
	bool	CreateRenderWnd(DWORD dwStyle, CRect& rWnd, CWnd *pParentWnd);
	void	DestroyRenderWnd();
	void	ResizeRenderWnd(int cx, int cy);
	void	OnError(CString sErrorMsg, LPCSTR pszSrcFileName, int nLineNum, LPCSTR pszSrcFileDate);
	void	OnFullScreenChanged(bool bIsFullScreen, bool bResult);
	LRESULT	OnTrackingHelp(WPARAM wParam, LPARAM lParam);
	bool	HandleDlgKeyMsg(MSG* pMsg);
	bool	UpdateFrameRate();
	CString	GetTimestampFileName() const;
	void	MidiInit();
	bool	LoadSnapshot(LPCTSTR pszPath);

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	bool	m_bCleanStateOnExit;	// if true, clean state before exiting
	CWhorldThread	m_thrRender;	// render thread
	COptions	m_options;			// options data
	CMidiManager	m_midiMgr;		// MIDI manager
	CAutoPtr<CPlaylist>	m_pPlaylist;	// pointer to playlist
	WCritSec	m_csErrorLog;		// critical section for error log file

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

protected:
// Member data
	CWhorldView	*m_pView;		// pointer to one and only view; app is SDI
	CWhorldDoc	*m_pDocument;	// pointer to one and only document; app is SDI
	CRenderWnd	m_wndRender;	// render window
	bool	m_bIsDetached;		// true if render window is detached
	bool	m_bIsFullScreen;	// true if app is full-screen
	bool	m_bIsFullScreenChanging;	// true if full-screen mode switch is in progress
	bool	m_bDetachedPreFullScreen;	// true if render window was already detached when full-screen started
	bool	m_bIsDualMonitor;	// true if render window is on a different monitor than user interface
	bool	m_bIsPaused;		// true if render updates are paused from main thread's point of view
	CAutoPtr<CPatch> m_pPreSnapshotModePatch;	// backup of patch before we entered snapshot mode
	HHOOK	m_hKeyboardHook;	// handle to keyboard hook
	int		m_nOldResourceVersion;	// previous resource version number
	static const int	m_nNewResourceVersion;	// current resource version number

// Helpers
	static BOOL	GetNearestMonitorInfo(HWND hWnd, MONITORINFOEX& monInfo);
	bool	GetMonitorConfig(bool& bIsDualMonitor);
	bool	SetKeyboardHook();
	bool	RemoveKeyboardHook();
	static LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam);
	void	ResetWindowLayout();

// Message handlers
	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnAppHomePage();
};

extern CWhorldApp theApp;

inline CMainFrame* CWhorldApp::GetMainFrame() const
{
	return reinterpret_cast<CMainFrame*>(m_pMainWnd);
}

inline void CWhorldApp::SetView(CWhorldView *pView)
{
	m_pView = pView;
}

inline CWhorldView *CWhorldApp::GetView()
{
	return m_pView;
}

inline void CWhorldApp::SetDocument(CWhorldDoc *pDocument)
{
	m_pDocument = pDocument;
}

inline CWhorldDoc *CWhorldApp::GetDocument()
{
	return m_pDocument;
}

inline bool CWhorldApp::IsDetached() const
{
	return m_bIsDetached;
}

inline bool CWhorldApp::IsFullScreen() const
{
	return m_bIsFullScreen;
}

inline bool CWhorldApp::IsFullScreenChanging() const
{
	return m_bIsFullScreenChanging;
}

inline bool CWhorldApp::IsDualMonitor() const
{
	return m_bIsDualMonitor;
}

inline bool CWhorldApp::IsSingleMonitor() const
{
	return !m_bIsDualMonitor;
}

inline bool CWhorldApp::IsFullScreenSingleMonitor() const
{
	return m_bIsFullScreen && !m_bIsDualMonitor;
}

inline bool CWhorldApp::IsRenderWndCreated() const
{
	return m_wndRender.m_hWnd != 0;
}

inline bool CWhorldApp::IsPaused() const
{
	return m_bIsPaused;
}

inline bool CWhorldApp::IsSnapshotMode() const
{
	return m_pPreSnapshotModePatch != NULL;
}

inline bool	CWhorldApp::ResourceVersionChanged() const
{
	return m_nNewResourceVersion != m_nOldResourceVersion;
}

inline bool	CWhorldApp::IsMainThread()
{
	// assume app has only ONE user-interface thread
	return GetCurrentThreadId() == theApp.m_nThreadID;
}
