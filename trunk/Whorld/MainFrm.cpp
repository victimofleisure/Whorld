// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version

*/

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Whorld.h"

#include "MainFrm.h"
#include "WhorldDoc.h"
#include "WhorldView.h"

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

const UINT CMainFrame::m_arrDockingBarNameID[DOCKING_BARS] = {
	#define MAINDOCKBARDEF(name, width, height, style) IDS_BAR_##name,
	#include "MainDockBarDef.h"	// generate docking bar names
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
	theApp.m_pMainWnd = this;
	m_nPrevFrameCount = 0;
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

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
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
//	bool	bResetCustomizations = theApp.m_nNewResourceVersion != theApp.m_nOldResourceVersion;
	bool	bResetCustomizations = true; // @@@ temporary until we have resource versions
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

void CMainFrame::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// views that are descendents of main frame get updates automatically;
	// other objects that need document updates must be added explicitly
	m_wndParamsBar.OnUpdate(pSender, lHint, pHint);
	m_wndMasterBar.OnUpdate(pSender, lHint, pHint);
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
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_FIRST, ID_VIEW_APPLOOK_LAST, OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_FIRST, ID_VIEW_APPLOOK_LAST, OnUpdateApplicationLook)
	ON_COMMAND(ID_WINDOW_FULLSCREEN, OnWindowFullscreen)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_FULLSCREEN, OnUpdateWindowFullscreen)
	ON_COMMAND(ID_WINDOW_DETACH, OnWindowDetach)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_DETACH, OnUpdateWindowDetach)
	ON_MESSAGE(UWM_DELAYED_CREATE, OnDelayedCreate)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(UWM_RENDER_WND_CLOSED, OnRenderWndClosed)
	ON_MESSAGE(UWM_FULL_SCREEN_CHANGED, OnFullScreenChanged)
	ON_MESSAGE(UWM_HANDLE_DLG_KEY, OnHandleDlgKey)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_WM_TIMER()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_COMMAND(ID_WINDOW_RESET_LAYOUT, OnWindowResetLayout)
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

void CMainFrame::OnWindowDetach()
{
	theApp.SetDetached(!theApp.IsDetached());
}

void CMainFrame::OnUpdateWindowDetach(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(theApp.IsDetached());
	pCmdUI->Enable(!theApp.IsFullScreen());
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

LRESULT CMainFrame::OnDelayedCreate(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	// The main window has been initialized, so show and update it
	ShowWindow(theApp.m_nCmdShow);
	theApp.GetView()->PostMessage(UWM_DELAYED_CREATE);
	SetTimer(FRAME_RATE_TIMER_ID, FRAME_RATE_TIMER_PERIOD, NULL);
	return 0;
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

LRESULT	CMainFrame::OnHandleDlgKey(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	return theApp.HandleDlgKeyMsg((MSG *)wParam);
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case FRAME_RATE_TIMER_ID:
		{
			int	nPrevLen = m_sRingCount.GetLength();
			m_sRingCount.Format(_T("%lld"), theApp.GetRingCount());
			FastSetPaneText(m_wndStatusBar, SBP_RING_COUNT, m_sRingCount, nPrevLen);
			double	fElapsedSecs = m_benchFrameRate.Reset();
			UINT_PTR	nFrameCount = theApp.GetFrameCount();
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

void CMainFrame::OnEditCopy() //@@@ test only
{
#if _DEBUG
//	theApp.GetDocument()->SetParam(CWhorldBase::PARAM_AspectRatio, 0, 1.23);
//	theApp.GetDocument()->SetMasterProp(CWhorldBase::MASTER_CanvasScale, 1.23);
	AfxMessageBox(_T("TEST CMainFrame::OnEditCopy"));
#endif
}

LRESULT	CMainFrame::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	theApp.UpdateFrameRate();
	return 0;
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
