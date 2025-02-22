// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		20feb25	add bitmap capture and write
		02		21feb25	add options

*/

// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "WhorldBase.h"
#include "ParamsBar.h"
#include "MasterBar.h"

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
	void	AddImageExportPath(CString sPath);

// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

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

// Data members
	UINT_PTR	m_nPrevFrameCount;	// previous frame count, for measuring frame rate
	CBenchmark	m_benchFrameRate;	// benchmark timer, for measuring frame rate
	CString	m_sRingCount;			// ring count status pane string
	CString	m_sFrameRate;			// frame rate status pane string
	CStringArrayEx	m_saImageExportPath;	// array of image export paths

// Helpers
	BOOL	CreateDockingWindows();
	bool	FastSetPaneText(CMFCStatusBar& bar, int nIndex, const CString& sText, int& nCurTextLength);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnWindowFullscreen();
	afx_msg void OnUpdateWindowFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnWindowDetach();
	afx_msg void OnUpdateWindowDetach(CCmdUI* pCmdUI);
	afx_msg LRESULT OnDelayedCreate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRenderWndClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFullScreenChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnHandleDlgKey(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditCopy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT	OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnWindowResetLayout();
	afx_msg LRESULT	OnBitmapCapture(WPARAM wParam, LPARAM lParam);
	#define MAINDOCKBARDEF(name, width, height, style) \
		afx_msg void OnViewBar##name(); \
		afx_msg void OnUpdateViewBar##name(CCmdUI *pCmdUI);
	#include "MainDockBarDef.h"	// generate docking bar message handlers
	afx_msg void OnViewOptions();
};

inline HACCEL CMainFrame::GetAccelTable() const
{
	return m_hAccelTable;
}

