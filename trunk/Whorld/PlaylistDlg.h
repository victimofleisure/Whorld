// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun05	initial version
        01      23feb06	move clipboard support into edit list control
		02		23mar06	change INFO booleans from int to bool
        03      17apr06	rename link to CPatchLink
		04		05may06	allow patches to be videos
		05		06may06	add ten extra banks
        06      03jun06	add thumbnails
        07      26jun06	add export thumbnails
		08		28jan08	support Unicode

        playlist dialog
 
*/

#if !defined(AFX_PLAYLISTDLG_H__5F732D3C_5945_4CFD_9CD1_834D993AF676__INCLUDED_)
#define AFX_PLAYLISTDLG_H__5F732D3C_5945_4CFD_9CD1_834D993AF676__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlaylistDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlaylistDlg dialog

#include "PatchLink.h"
#include "ToolDlg.h"
#include "NumEdit.h"
#include "NumSpin.h"
#include "RandList.h"
#include "Patch.h"
#include "ClickSliderCtrl.h"
#include "EditListCtrl.h"
#include "HotKeyCtrlEx.h"
#include "CtrlResize.h"
#include "afxadv.h"	// for CRecentFileList
#include "PlaylistMidiDlg.h"
#include "ThumbList.h"

class CMainFrame;

class CPlaylistDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CPlaylistDlg);
// Construction
public:
	CPlaylistDlg(CWnd* pParent = NULL);
	~CPlaylistDlg();
	BOOL	Create(UINT nIDTemplate, CMainFrame *pFrame);

// Constants
	enum {
		BANKS = 20			// number of patch banks
	};

// Types
	typedef CPlaylistMidiDlg::INFO MIDI_INFO;
	typedef struct tagINFO {
		MIDI_INFO	Midi;	// MIDI mapping dialog state
		double	PatchSecs;	// in auto-play, duration of each patch, in seconds
		double	XFadeSecs;	// crossfade duration, in seconds; may exceed PatchSecs
		bool	Playing;	// true if auto-playing
		bool	Shuffle;	// true if auto-play should randomize playlist order
		int		CurBank;	// index of current bank
		int		CurPatch;	// index of currently playing patch; -1 if none
		int		CurPatchBank;	// index of current patch's bank
	} INFO;

// Attributes
	void	GetDefaults(INFO& Info) const;
	void	GetInfo(INFO& Info) const;
	void	SetInfo(const INFO& Info);
	void	GetMidiInfo(MIDI_INFO& Info) const;
	const	MIDI_INFO& GetMidiInfo() const;
	void	SetMidiInfo(const MIDI_INFO& Info);
	bool	IsPlaying() const;
	void	SetShuffle(bool Enable);
	bool	GetShuffle() const;
	void	SetTiming(double PatchSecs, double XFadeSecs);
	void	GetTiming(double& PatchSecs, double& XFadeSecs) const;
	void	SetPatchSecs(double Secs);
	double	GetPatchSecs() const;
	void	SetXFadeSecs(double Secs);
	double	GetXFadeSecs() const;
	void	SetXFadeFrac(double Frac);
	double	GetXFadeFrac() const;
	void	SetBank(int Bank);
	int		GetBank() const;
	int		GetCount() const;
	int		GetCount(int Bank) const;
	CString	GetName(int Patch) const;
	LPCTSTR	GetPath(int Patch) const;
	DWORD	GetHotKey(int Patch) const;
	LPCTSTR	GetListPath() const;
	int		GetCurPatch() const;
	void	SetModify(bool Enable = TRUE);
	bool	IsModified() const;
	bool	IsCached() const;
	int		GetNoteMapping(int Chan, int Note) const;
	int		GetProgMapping(int Chan, int Prog) const;
	void	SetCurPatchState(bool Fading);

// Operations
	void	RestoreDefaults();
	void	Play(bool Enable);
	bool	New();
	bool	Open(LPCTSTR Path);
	bool	Open();
	bool	OpenRecent(int FileIdx);
	bool	Save();
	bool	SaveAs();
	bool	SaveCheck();
	bool	FadeTo(int Patch, double Secs = -1);
	bool	FadeToBank(int Bank, int Patch, double Secs = -1);
	bool	HandleHotKey(DWORD HotKey);
	int		GetNext();
	bool	PlayNext(double Secs = -1);
	bool	Cache(bool Enable);
	void	UpdateRecentFileMenu(CCmdUI *pCmdUI);
	void	SendTo(int SelIdx, int Patch);
	void	LoadNext(int SelIdx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlaylistDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Nested classes
	class CPlaylistCtrl : public CEditListCtrl {
	protected:
		void	GetInfo(int Pos, void *Info);
		void	InsertInfo(int Pos, const void *Info);
		void	DeleteInfo(int Pos);
	};
	friend class CPlaylistCtrl;

// Dialog Data
	//{{AFX_DATA(CPlaylistDlg)
	enum { IDD = IDD_PLAYLIST };
	CComboBox	m_BankCombo;
	CStatic	m_HotkeyCap;
	CStatic	m_BankCap;
	CStatic	m_AutoplayCap;
	CStatic	m_PatchSecsCap;
	CStatic	m_XFadeSecsCap;
	CNumSpin	m_PatchSecsSpin;
	CNumSpin	m_XFadeSecsSpin;
	CHotKeyCtrlEx	m_HotKey;
	CPlaylistCtrl	m_List;
	CNumEdit	m_XFadeSecsEdit;
	CNumEdit	m_PatchSecsEdit;
	CClickSliderCtrl	m_XFadeSlider;
	CButton	m_Play;
	CButton	m_Shuffle;
	//}}AFX_DATA

	// Generated message map functions
	//{{AFX_MSG(CPlaylistDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPlay();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnChangedSecsEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedXFadeEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKillFocusHotKey(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReorderList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindragList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnShuffle();
	afx_msg void OnEditDelete();
	afx_msg void OnFileInsertPatches();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnEditRemoveKey();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditAutoAssign();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateFileMru(CCmdUI* pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnFileImportMidiSetup();
	afx_msg void OnEditMidiMapping();
	afx_msg void OnEditFadeTo();
	afx_msg void OnEditSendToA();
	afx_msg void OnEditSendToB();
	afx_msg void OnEditSendToDoc();
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnSelchangeBank();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnSysColorChange();
	afx_msg void OnViewRefreshThumbs();
	afx_msg void OnUpdateViewRefreshThumbs(CCmdUI* pCmdUI);
	afx_msg void OnFileExportThumbs();
	afx_msg void OnUpdateFileExportThumbs(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnFileMru(UINT nID);
	afx_msg void OnViewType(UINT nID);
	afx_msg void OnUpdateViewType(CCmdUI* pCmdUI);
	afx_msg void OnViewThumbSize(UINT nID);
	afx_msg void OnUpdateViewThumbSize(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagCOLDATA {
		int		TitleID;
		int		Align;
		int		Width;
	} COLDATA;
	typedef CArray<CPatchLink, CPatchLink&> PATCH_LIST;
	typedef CArray<CPatch, CPatch&> PATCH_CACHE;
	typedef struct tagCBDATA {	// clipboard data format
		int		Bank;
		DWORD	HotKey;
		TCHAR	Path[MAX_PATH + 1];
	} CBDATA;

// Constants
	enum {
		TIMER_ID = 1,				// identifies our timer instance
		DEFAULT_PATCH_SECS = 60,	// default duration of each patch in auto-play 
		DEFAULT_XFADE_PCT = 0,		// default crossfade time, as % of patch secs
		FILE_VERSION = 1,			// version number of playlist file format
		MAX_RECENT_FILES = 4		// how many recently used files to store
	};
	enum {	// list control columns
		COL_KEY,	// patch hotkey assignment 
		COL_NAME,	// patch name
		COL_PATH,	// patch path
		COLUMNS		// number of columns
	};
	enum {	// patch states
		TS_NORMAL,	// patch is normal
		TS_PLAYING,	// patch is currently playing
		TS_XFADING,	// patch is being crossfaded to
		STATES		// number of patch states
	};
	enum {	// preset thumbnail sizes
		THUMB_TINY,
		THUMB_SMALL,
		THUMB_MEDIUM,
		THUMB_LARGE,
		THUMB_HUGE,
		THUMB_SIZES
	};
	static const CCtrlResize::CTRL_LIST m_CtrlList[];
	static const int	m_StateIcon[STATES];
	static const COLDATA	m_ColData[COLUMNS];
	static const INFO	m_Defaults;
	static const SIZE	m_ThumbSizePreset[THUMB_SIZES];

// Data members
	CCtrlResize	m_Resize;	// dialog resizer
	CRect	m_InitRect;		// initial window rect, for minimum size
	PATCH_LIST	m_Bank[BANKS];	// array of patch lists, one per bank
	CRandList	m_Rand;		// list randomizer
	CString	m_ListPath;		// path of currently open playlist document
	CString	m_Caption;		// dialog's original caption, from resource
	CString	m_Untitled;		// name to display for an untitled playlist
	CImageList	m_StateImg;	// image list for patch state icons
	CThumbList	m_Thumb;	// thumbnail manager
	PATCH_CACHE	m_Cache[BANKS];	// array of patch caches, one per bank
	CRecentFileList	m_RecentFile;	// list of most recently used files
	MIDI_INFO	m_Midi;		// MIDI mapping state
	CMainFrame	*m_Frm;		// pointer to main frame window
	PATCH_LIST	*m_Patch;	// pointer to patch list for current bank
	HACCEL	m_ToolDlgAccel;	// accelerator handle for tool dialog shortcut keys
	int		m_Timer;		// if non-zero, our timer instance
	int		m_CurBank;		// index of current bank
	int		m_CurPatch;		// index of current patch; -1 if none
	int		m_CurPatchBank;	// index of current patch's bank
	double	m_XFadeFrac;	// crossfade time as fraction of patch seconds
	bool	m_ModFlag;		// true if playlist is modified
	bool	m_IsCached;		// true if playlist is cached
	bool	m_ThumbView;	// true if we're viewing thumbnails
	CSize	m_ThumbSize;	// thumbnail size, in pixels

// Helpers
	int		GetCurSel();
	bool	ReadPatch(LPCTSTR Path, CPatch& Patch);
	bool	InitTimer();
	void	UpdateUI();
	void	SetStateIcon(int Idx, int State);
	void	SetCurPatch(int Bank, int Patch, int State);
	void	RemoveAll();
	void	InsertItem(int Idx, LPCTSTR Path, DWORD HotKey, int ImgIdx = -1);
	void	Insert(int Bank, int Patch, LPCTSTR Path, DWORD HotKey);
	void	Delete(int Bank, int Patch);
	bool	Insert(int Patch, const CStringArray& List, CString& Errs);
	void	Insert();
	void	PopulateList();
	bool	Read(LPCTSTR Path);
	bool	Write(LPCTSTR Path);
	int		KeyToPatch(DWORD HotKey) const;
	int		KeyToBank(DWORD HotKey) const;
	void	SetHotKey(int Patch, DWORD HotKey);
	bool	AssignHotKey(int Patch, DWORD HotKey);
	void	SetListPath(LPCTSTR Path);
	bool	IsMapped(int Chan, int Parm) const;
	static	bool	IsVideo(LPCTSTR Path);
	bool	MakeThumbs();
	bool	SetThumbView(bool Enable);
	void	SetThumbSize(CSize Size);
};

inline int CPlaylistDlg::GetCurSel()
{
	return(m_List.GetCurSel());
}

inline void CPlaylistDlg::GetMidiInfo(MIDI_INFO& Info) const
{
	Info = m_Midi;
}

inline void CPlaylistDlg::SetMidiInfo(const MIDI_INFO& Info)
{
	m_Midi = Info;
}

inline bool CPlaylistDlg::IsPlaying() const
{
	return(m_Timer != 0);
}

inline bool CPlaylistDlg::GetShuffle() const
{
	return(m_Shuffle.GetCheck() != 0);
}

inline int CPlaylistDlg::GetBank() const
{
	return(m_CurBank);
}

inline int CPlaylistDlg::GetCount() const
{
	return((*m_Patch).GetSize());
}

inline int CPlaylistDlg::GetCount(int Bank) const
{
	ASSERT(Bank >= 0 && Bank < BANKS);
	return(m_Bank[Bank].GetSize());
}

inline LPCTSTR CPlaylistDlg::GetPath(int Patch) const
{
	return((*m_Patch)[Patch].m_Path);
}

inline DWORD CPlaylistDlg::GetHotKey(int Patch) const
{
	return((*m_Patch)[Patch].m_HotKey);
}

inline LPCTSTR CPlaylistDlg::GetListPath() const
{
	return(m_ListPath);
}

inline int CPlaylistDlg::GetCurPatch() const
{
	return(m_CurPatchBank == m_CurBank ? m_CurPatch : -1);
}

inline bool CPlaylistDlg::IsModified() const
{
	return(m_ModFlag);
}

inline bool CPlaylistDlg::IsCached() const
{
	return(m_IsCached);
}

inline void CPlaylistDlg::GetTiming(double& PatchSecs, double& XFadeSecs) const
{
	PatchSecs = GetPatchSecs();
	XFadeSecs = GetXFadeSecs();
}

inline double CPlaylistDlg::GetPatchSecs() const
{
	return(m_PatchSecsEdit.GetVal());
}

inline double CPlaylistDlg::GetXFadeSecs() const
{
	return(m_XFadeSecsEdit.GetVal());
}

inline double CPlaylistDlg::GetXFadeFrac() const
{
	return(m_XFadeFrac);
}

inline bool CPlaylistDlg::FadeTo(int Patch, double Secs)
{
	return(FadeToBank(m_CurBank, Patch, Secs));
}

inline void CPlaylistDlg::UpdateRecentFileMenu(CCmdUI *pCmdUI)
{
	m_RecentFile.UpdateMenu(pCmdUI);
}

inline bool CPlaylistDlg::IsMapped(int Chan, int Parm) const
{
	return(Chan == m_Midi.Chan && Parm >= m_Midi.First && Parm <= m_Midi.Last);
}

inline int CPlaylistDlg::GetNoteMapping(int Chan, int Note) const
{
	return(m_Midi.Scheme == CPlaylistMidiDlg::MS_NOTES
		&& IsMapped(Chan, Note) ? Note - m_Midi.First : -1);
}

inline int CPlaylistDlg::GetProgMapping(int Chan, int Prog) const
{
	return(m_Midi.Scheme == CPlaylistMidiDlg::MS_PROG_CHG
		&& IsMapped(Chan, Prog) ? Prog - m_Midi.First : -1);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYLISTDLG_H__5F732D3C_5945_4CFD_9CD1_834D993AF676__INCLUDED_)
