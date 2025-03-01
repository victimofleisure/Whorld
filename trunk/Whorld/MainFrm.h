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
		04		25feb25	add frame min/max info handler for row view panes
		05		26feb25	add MIDI input
		06		27feb25	add pre-snapshot mode patch member
		07		01mar25	add globals pane

*/

// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "WhorldBase.h"
#include "ParamsBar.h"
#include "MasterBar.h"
#include "MappingBar.h"
#include "GlobalsBar.h"

// docking bar IDs are relative to AFX_IDW_CONTROLBAR_FIRST
enum {	// docking bar IDs; don't change, else bar placement won't be restored
	ID_APP_DOCKING_BAR_START = AFX_IDW_CONTROLBAR_FIRST + 40,
	#define MAINDOCKBARDEF(name, width, height, style) ID_BAR_##name,
	#include "MainDockBarDef.h"	// generate docking bar IDs
	ID_APP_DOCKING_BAR_END,
	ID_APP_DOCKING_BAR_FIRST = ID_APP_DOCKING_BAR_START + 1,
	ID_APP_DOCKING_BAR_LAST = ID_APP_DOCKING_BAR_END - 1,
};

class CAuxFrame;
class CAuxView;

class CMainFrame : public CFrameWndEx, public CWhorldBase
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

public:
// Constants
	enum {	// enumerate docking bars
		#define MAINDOCKBARDEF(name, width, height, style) DOCKING_BAR_##name,
		#include "MainDockBarDef.h"	// generate docking bar enumeration
		DOCKING_BARS
	};
	enum {	// status bar panes
		SBP_HINT,
		SBP_RING_COUNT,
		SBP_FRAME_RATE,
		STATUS_BAR_PANES
	};

// Attributes
	HACCEL	GetAccelTable() const;

// Operations
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	static void	OnFrameGetMinMaxInfo(CDockablePane* pPane, HWND hFrameWnd, MINMAXINFO *pMMI);

// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Docking bars
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
#if 0 // disable user-defined toolbar images
	CMFCToolBarImages m_UserImages;
#endif
	#define MAINDOCKBARDEF(name, width, height, style) C##name##Bar m_wnd##name##Bar;
	#include "MainDockBarDef.h"	// generate docking bar members

protected:
// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Constants
	static const UINT m_arrDockingBarNameID[DOCKING_BARS];	// array of docking bar name IDs
	enum {
		FRAME_RATE_TIMER_ID = 2025,
		FRAME_RATE_TIMER_PERIOD = 1000,
	};
	static const LPCTSTR m_pszExportExt;
	static const LPCTSTR m_pszExportFilter;
	static const LPCTSTR m_pszSnapshotExt;
	static const LPCTSTR m_pszSnapshotFilter;

// Data members
	UINT_PTR	m_nPrevFrameCount;	// previous frame count, for measuring frame rate
	CBenchmark	m_benchFrameRate;	// benchmark timer, for measuring frame rate
	CString	m_sRingCount;			// ring count status pane string
	CString	m_sFrameRate;			// frame rate status pane string
	CStringArrayEx	m_saOutputPath;	// array of output paths
	CAutoPtr<CPatch> m_pPreSnapshotModePatch;	// backup of patch before we entered snapshot mode

// Helpers
	BOOL	CreateDockingWindows();
	bool	FastSetPaneText(CMFCStatusBar& bar, int nIndex, const CString& sText, int& nCurTextLength);
	void	ApplyOptions(const COptions *pPrevOptions);
	bool	WriteSnapshot(CSnapshot *pSnapshot);
	static bool	MakeExportPath(CString& sExportPath, LPCTSTR pszExt);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	#define MAINDOCKBARDEF(name, width, height, style) \
		afx_msg void OnViewBar##name(); \
		afx_msg void OnUpdateViewBar##name(CCmdUI *pCmdUI);
	#include "MainDockBarDef.h"	// generate docking bar message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnWindowFullscreen();
	afx_msg void OnUpdateWindowFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnWindowDetach();
	afx_msg void OnUpdateWindowDetach(CCmdUI* pCmdUI);
	afx_msg LRESULT OnDelayedCreate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRenderWndClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFullScreenChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnHandleDlgKey(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT	OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnDeviceNodeChange(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnDeviceChange(UINT nEventType, W64ULONG dwData);
	afx_msg void OnWindowResetLayout();
	afx_msg LRESULT	OnBitmapCapture(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnSnapshotCapture(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnParamValChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnMasterPropChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnMainPropChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnViewOptions();
	afx_msg void OnWindowPause();
	afx_msg void OnUpdateWindowPause(CCmdUI *pCmdUI);
	afx_msg void OnWindowStep();
	afx_msg void OnUpdateWindowStep(CCmdUI *pCmdUI);
	afx_msg void OnWindowClear();
	afx_msg void OnImageRandomPhase();
	afx_msg void OnFileExport();
	afx_msg void OnFileTakeSnapshot();
	afx_msg void OnFileLoadSnapshot();
	afx_msg void OnPlaylistOpen();
	afx_msg void OnPlaylistSave();
	afx_msg void OnPlaylistSaveAs();
	afx_msg void OnPlaylistNew();
	afx_msg void OnPlaylistMru(UINT nID);
	afx_msg void OnUpdatePlaylistMru(CCmdUI* pCmdUI);
	afx_msg void OnViewMidiLearn();
	afx_msg void OnUpdateViewMidiLearn(CCmdUI *pCmdUI);
};

inline HACCEL CMainFrame::GetAccelTable() const
{
	return m_hAccelTable;
}

