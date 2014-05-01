// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		28jun05	add snapshot file open
		02		07jul05	add playlist file open
		03		01aug05	disable and restore accessibility shortcuts
		04		18aug05	add mouse wheel to OnIdle excludes
		05		29aug05	add HTML help
		06		02jun06	add AVI to shell open
		07		24jul06	add command-line flag to start in VJ mode
		08		27jan08	add GetAppDataFolder

        Whorld application
 
*/

// Whorld.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Whorld.h"

#include "MainFrm.h"
#include "WhorldDoc.h"
#include "WhorldView.h"
#include "WhorldViewDD.h"
#include "AboutDlg.h"
#include "Win32Console.h"
#include "Persist.h"
#include "shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWhorldApp

BEGIN_MESSAGE_MAP(CWhorldApp, CWinApp)
	//{{AFX_MSG_MAP(CWhorldApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_HOME_PAGE, OnAppHomePage)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhorldApp construction

CWhorldApp::CWhorldApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWhorldApp object

CWhorldApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWhorldApp initialization

LONG __stdcall CWhorldApp::CrashHandler(EXCEPTION_POINTERS *ExceptionInfo)
{
	((CWhorldApp *)AfxGetApp())->m_NoAccess.Restore();	// restore accessibility
	return(EXCEPTION_CONTINUE_SEARCH);
}

BOOL CWhorldApp::InitInstance()
{
	SetUnhandledExceptionFilter(CrashHandler);	// trap unhandled exceptions

	AfxEnableControlContainer();

#ifdef _DEBUG
	Win32Console::Create();
#endif

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	SetRegistryKey(_T("Anal Software"));

	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWhorldDoc),
		RUNTIME_CLASS(CMainFrame),		// main SDI frame window
		CMainFrame::GetViewClass());	// GDI or DirectDraw
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	ParseCommandLine(m_cmdInfo);

	// if special file type, disable standard open document behavior
	bool	SpecialFile = FALSE;
	if (!m_cmdInfo.m_strFileName.IsEmpty()) {
		CString	Ext = PathFindExtension(m_cmdInfo.m_strFileName);
		if (!Ext.IsEmpty()) {
			if (!_tcsicmp(Ext, EXT_SNAPSHOT)
			|| !_tcsicmp(Ext, EXT_PLAYLIST)
			|| !_tcsicmp(Ext, EXT_MOVIE)
			|| !_tcsicmp(Ext, EXT_AVI)) {
				SpecialFile = TRUE;
				m_cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
			}
		}
	}

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(m_cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// notify frame about special file type
	if (SpecialFile) {
		LPCTSTR	Path = m_cmdInfo.m_strFileName;
		m_pMainWnd->PostMessage(UWM_SHELLOPEN, (LPARAM)Path, m_cmdInfo.m_StartVJ);
	}

	return TRUE;
}

CWhorldApp::CMyCommandLineInfo::CMyCommandLineInfo()
{
	m_StartVJ = FALSE;
}

void CWhorldApp::CMyCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag && !_tcsicmp(lpszParam, _T("vj")))
		m_StartVJ = TRUE;
	CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
}

/////////////////////////////////////////////////////////////////////////////
// CWhorldApp message handlers

// By default, CWinApp::OnIdle is called after WM_TIMER messages.  This isn't
// normally a problem, but if the application uses a short timer, OnIdle will
// be called frequently, seriously degrading performance.  Performance can be
// improved by overriding IsIdleMessage to return FALSE for WM_TIMER messages,
// which prevents them from triggering OnIdle.  This technique can be applied
// to any idle-triggering message that repeats frequently, e.g. WM_MOUSEMOVE.
//
BOOL CWhorldApp::IsIdleMessage(MSG* pMsg)
{
	if (CWinApp::IsIdleMessage(pMsg)) {
		switch (pMsg->message) {	// don't call OnIdle after these messages
		case WM_TIMER:
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		case WM_KEYDOWN:	// so keyboard auto-repeat doesn't hog CPU
		case WM_HSCROLL:	// Page Up/Down auto-repeats this message too
		case WM_MOUSEWHEEL:
			return(FALSE);
		default:
			return(TRUE);
		}
	} else
		return(FALSE);
}

// App command to run the dialog
void CWhorldApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CWhorldApp::OnAppHomePage() 
{
	CHyperlink::GotoUrl(CAboutDlg::HOME_PAGE_URL);
}

bool CWhorldApp::GetAppDataFolder(CString& Folder)
{
	LPTSTR	p = Folder.GetBuffer(MAX_PATH);
	bool	retc = SUCCEEDED(SHGetSpecialFolderPath(NULL, p, CSIDL_APPDATA, 0));
	Folder.ReleaseBuffer();
	return(retc);
}

// MakeSureDirectoryPathExists doesn't support Unicode; SHCreateDirectoryEx
// is a reasonable substitute, but our version of the SDK doesn't define it
#if defined(UNICODE) && !defined(SHCreateDirectoryEx)
int WINAPI SHCreateDirectoryExW(HWND hwnd, LPCWSTR pszPath, SECURITY_ATTRIBUTES *psa)
{
	int	retc = ERROR_INVALID_FUNCTION;
	typedef int (WINAPI* lpfnSHCreateDirectoryExW)(HWND hwnd, LPCWSTR pszPath, SECURITY_ATTRIBUTES *psa);
	HMODULE hShell = LoadLibrary(_T("shell32.dll"));
	lpfnSHCreateDirectoryExW lpfn = NULL;
	if (hShell) {
		lpfn = (lpfnSHCreateDirectoryExW)GetProcAddress(hShell, "SHCreateDirectoryExW");
		if (lpfn)
			retc = lpfn(hwnd, pszPath, psa);
		FreeLibrary(hShell);
	}
	return(retc);
}
#define SHCreateDirectoryEx SHCreateDirectoryExW
#endif
