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
	CWhorldView	*GetView();
	CWhorldDoc	*GetDocument();
	bool	IsDetached() const;
	bool	IsFullScreen() const;
	bool	IsFullScreenChanging() const;
	bool	IsDualMonitor() const;
	bool	IsSingleMonitor() const;
	bool	IsFullScreenSingleMonitor() const;
	bool	IsRenderWndCreated() const;
	UINT_PTR	GetRingCount() const;
	UINT_PTR	GetFrameCount() const;
	bool	IsPaused() const;
	DWORD	GetFrameRate() const;
	DPoint	GetOrigin() const;

// Operations
	void	Log(CString sMsg);
	void	PushRenderCommand(const CRenderCmd& cmd);
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
	bool	WriteCapturedBitmap(ID2D1Bitmap1* pBitmap, LPCTSTR pszImagePath);
	static CString	GetTimestampFileName();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	bool	m_bCleanStateOnExit;	// if true, clean state before exiting

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

protected:
// Member data
	CWhorldView	*m_pView;		// pointer to one and only view; app is SDI
	CRenderWnd	m_wndRender;	// render window
	CWhorldThread	m_thrRender;	// derived render thread
	bool	m_bIsDetached;		// true if render window is detached
	bool	m_bIsFullScreen;	// true if app is full-screen
	bool	m_bIsFullScreenChanging;	// true if full-screen mode switch is in progress
	bool	m_bDetachedPreFullScreen;	// true if render window was already detached when full-screen started
	bool	m_bIsDualMonitor;	// true if render window is on a different monitor than user interface
	HHOOK	m_hKeyboardHook;	// handle to keyboard hook

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
public:
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

inline UINT_PTR CWhorldApp::GetRingCount() const
{
	return m_thrRender.GetRingCount();
}

inline UINT_PTR CWhorldApp::GetFrameCount() const
{
	return m_thrRender.GetFrameCount();
}

inline bool CWhorldApp::IsPaused() const
{
	return m_thrRender.IsPaused();
}

inline DWORD CWhorldApp::GetFrameRate() const
{
	return m_thrRender.GetFrameRate();
}

inline DPoint CWhorldApp::GetOrigin() const
{
	return m_thrRender.GetOrigin();
}

inline bool CWhorldApp::WriteCapturedBitmap(ID2D1Bitmap1* pBitmap, LPCTSTR pszImagePath)
{
	return m_thrRender.WriteCapturedBitmap(pBitmap, pszImagePath);
}
