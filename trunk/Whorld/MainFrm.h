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
		07		02mar25	add globals pane
		08		03mar25	add render queue full handler
		09		06mar25	add snapshot path array
		10		07mar25	add snapshot submenu handlers
		11		08mar25	add wait for posted message
		12		14mar25	add movie recording and playback
		13		16mar25	add movie export
		14		17mar25	add movie bar
		15		20mar25	add movie record time to status bar

*/

// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "WhorldBase.h"
#include "ParamsBar.h"
#include "MasterBar.h"
#include "MappingBar.h"
#include "GlobalsBar.h"
#include "MovieBar.h"

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
class CProgressDlg;

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
		SBP_RECORD_TIME,
		SBP_RING_COUNT,
		SBP_FRAME_RATE,
		STATUS_BAR_PANES
	};

// Attributes
	HACCEL	GetAccelTable() const;
	bool	InRenderFullError() const;
	bool	IsCurrentSnapshotValid();
	bool	IsMovieOpen() const;
	bool	IsMovieRecording() const;
	bool	IsMoviePlaying() const;
	bool	IsPaused() const;

// Operations
	void	OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	static void	OnFrameGetMinMaxInfo(CDockablePane* pPane, HWND hFrameWnd, MINMAXINFO *pMMI);
	bool	PlayMovie(LPCTSTR pszMoviePath, bool bPaused = false);
	bool	RecordMovie(LPCTSTR pszMoviePath);
	bool	PauseMovie(bool bEnable);

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
	enum {	// timers
		FRAME_RATE_TIMER_ID = 2025,
		FRAME_RATE_TIMER_PERIOD = 1000,
	};
	enum {	// movie input/output states
		MOVIE_NONE = -1,	// no movie
		MOVIE_PLAYING,		// playing a movie
		MOVIE_RECORDING,	// recording a movie
	};
	static const LPCTSTR m_pszExportExt;
	static const LPCTSTR m_pszExportFilter;
	static const LPCTSTR m_pszSnapshotExt;
	static const LPCTSTR m_pszSnapshotFilter;
	static const LPCTSTR m_pszMovieExt;
	static const LPCTSTR m_pszMovieFilter;

// Data members
	UINT_PTR	m_nPrevFrameCount;	// previous frame count, for measuring frame rate
	CBenchmark	m_benchFrameRate;	// benchmark timer, for measuring frame rate
	CString	m_sRecordTime;			// ring count status pane string
	CString	m_sRingCount;			// ring count status pane string
	CString	m_sFrameRate;			// frame rate status pane string
	CStringArrayEx	m_aOutputPath;	// array of output paths
	CStringArrayEx	m_aSnapshotPath;	// array of snapshot paths
	int		m_iCurSnapshot;			// index of snapshot currently being displayed 
	bool	m_bInRenderFullError;	// true if handling render command queue full error
	int		m_nMovieIOState;		// movie input/output state; see enum above
	bool	m_bIsMoviePaused;		// true if movie playback is paused
	int		m_nTaskDoneID;			// ID of last completed task
	
// Helpers
	BOOL	CreateDockingWindows();
	bool	FastSetPaneText(CMFCStatusBar& bar, int nIndex, const CString& sText, int& nCurTextLength);
	void	ApplyOptions(const COptions *pPrevOptions);
	bool	WriteSnapshot(const CSnapshot *pSnapshot);
	bool	PromptingForExport() const;
	bool	MakeUniqueExportPath(CString& sExportPath, LPCTSTR pszExt);
	static bool	WaitForPostedMessage(UINT message, CProgressDlg& dlgProgress);

// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg BOOL OnDeviceChange(UINT nEventType, W64ULONG dwData);
	afx_msg LRESULT OnDelayedCreate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnHandleDlgKey(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRenderWndClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFullScreenChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnDeviceNodeChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnBitmapCapture(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnSnapshotCapture(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnParamChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnMasterPropChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnMainPropChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetDrawMode(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnRenderQueueFull(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnRenderTaskDone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFileExport();
	afx_msg void OnUpdateFileExport(CCmdUI* pCmdUI);
	afx_msg void OnFileTakeSnapshot();
	afx_msg void OnUpdateFileTakeSnapshot(CCmdUI* pCmdUI);
	afx_msg void OnFileLoadSnapshot();
	afx_msg void OnUpdateFileLoadSnapshot(CCmdUI* pCmdUI);
	afx_msg void OnPlaylistNew();
	afx_msg void OnPlaylistOpen();
	afx_msg void OnPlaylistSave();
	afx_msg void OnPlaylistSaveAs();
	afx_msg void OnPlaylistMru(UINT nID);
	afx_msg void OnUpdatePlaylistMru(CCmdUI* pCmdUI);
	afx_msg void OnImageRandomPhase();
	afx_msg void OnSnapshotFirst();
	afx_msg void OnSnapshotLast();
	afx_msg void OnSnapshotNext();
	afx_msg void OnSnapshotPrev();
	afx_msg void OnSnapshotExportAll();
	afx_msg void OnSnapshotInfo();
	afx_msg void OnUpdateSnapshot(CCmdUI *pCmdUI);
	#define MAINDOCKBARDEF(name, width, height, style) \
		afx_msg void OnViewBar##name(); \
		afx_msg void OnUpdateViewBar##name(CCmdUI *pCmdUI);
	#include "MainDockBarDef.h"	// generate docking bar message handlers
	afx_msg void OnViewOptions();
	afx_msg void OnViewMidiLearn();
	afx_msg void OnUpdateViewMidiLearn(CCmdUI *pCmdUI);
	afx_msg void OnWindowFullscreen();
	afx_msg void OnUpdateWindowFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnWindowDetach();
	afx_msg void OnUpdateWindowDetach(CCmdUI* pCmdUI);
	afx_msg void OnWindowPause();
	afx_msg void OnUpdateWindowPause(CCmdUI *pCmdUI);
	afx_msg void OnWindowStep();
	afx_msg void OnUpdateWindowStep(CCmdUI *pCmdUI);
	afx_msg void OnWindowClear();
	afx_msg void OnUpdateWindowClear(CCmdUI *pCmdUI);
	afx_msg void OnWindowResetLayout();
	afx_msg void OnMovieRecord();
	afx_msg void OnUpdateMovieRecord(CCmdUI *pCmdUI);
	afx_msg void OnMoviePlay();
	afx_msg void OnUpdateMoviePlay(CCmdUI *pCmdUI);
	afx_msg void OnMovieExport();
	afx_msg void OnUpdateMovieExport(CCmdUI *pCmdUI);
	afx_msg void OnMovieRewind();
	afx_msg void OnUpdateMovieRewind(CCmdUI *pCmdUI);
};

inline HACCEL CMainFrame::GetAccelTable() const
{
	return m_hAccelTable;
}

inline bool CMainFrame::InRenderFullError() const
{
	return m_bInRenderFullError;
}

inline bool CMainFrame::IsCurrentSnapshotValid()
{
	return m_iCurSnapshot < m_aSnapshotPath.GetSize();
}

inline bool CMainFrame::IsMovieOpen() const
{
	return m_nMovieIOState >= MOVIE_PLAYING;
}

inline bool CMainFrame::IsMovieRecording() const
{
	return m_nMovieIOState > MOVIE_PLAYING;
}

inline bool CMainFrame::IsMoviePlaying() const
{
	return m_nMovieIOState == MOVIE_PLAYING;
}

inline bool CMainFrame::IsPaused() const
{
	if (IsMoviePlaying()) {	// if playing movie
		return m_bIsMoviePaused;	// movie owns the pause state
	} else {	// not playing movie
		return theApp.IsPaused();	// do normal pause behavior
	}
}
