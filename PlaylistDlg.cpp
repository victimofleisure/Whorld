// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun05	initial version
		01		10jun05	pass frame to ctor
		02		10jun05	add list ctrl and shortcut keys
		03		16jun05	add missing files dialog
		04		27jun05	add drag reordering
		05		01jul05	add multi file dialog
		06		27jul05	add playlist object
		07		29jul05	add hot key control
		08		30jul05	add resizing
		09		02aug05	add state icons
		10		04aug05	pressing hotkey should trigger patch
		11		04aug05	add clear key and auto-assign keys
		12		05aug05	add drop files
		13		06aug05	add current patch to playlist info
		14		15aug05	Insert wasn't checking for GetSelectionMark < 0
		15		18aug05	add caching to reduce disk usage
		16		21aug05	clamp secs per file positive
		17		09sep05	add most recent file list
		18		02oct05	add import MIDI setup
		19		05oct05	add MIDI mapping dialog
		20		07oct05	add send to A/B/doc
		21		08oct05	in Write, if patch is local, store filename only
		22		14oct05	don't use isalpha on virtual keys
		23		20oct05	add banks
		24		23oct05	add clipboard support
		25		26oct05	remove key now operates on selection
		26		29oct05	in FadeTo, must use frame's SetViewSel
		27		02nov05	in Insert, add InsCount to insert position
		28		03nov05	display read patch errors unless in VJ mode
        29      23feb06	move clipboard support into edit list control
		30		23mar06	change INFO booleans from int to bool
		31		12apr06	in Read and Write, add wait cursor
        32      17apr06	rename link to CPatchLink
		33		04may06	bump current patch after inserting below it
		34		05may06	allow patches to be videos
		35		06may06	add ten extra banks
		36		18may06	add video caching
		37		30may06	fix post-insert bump test; on or below
		38		02jun06	move video open to main frame
        39      03jun06	add thumbnails
		40		05jun06	set background color of state icons
		41		21jun06	don't set minmax until m_InitRect is set
        42      26jun06	add export thumbnails
		43		28jan08	support Unicode
		44		29jan08	in Insert, create CPatchLink explicitly to fix warning
		45		29jan08	make DragQueryFile arg unsigned to fix warning
		46		29jan08	in PopulateList, remove unused local var
		47		29jan08	comment out unused pNMListView defs to fix warnings
		48		30jan08	while dragging, if Esc pressed, cancel drag

        playlist dialog
 
*/

// PlaylistDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "Playlist.h"
#include "PlaylistDlg.h"
#include "Persist.h"
#include "PathStr.h"
#include "MainFrm.h"
#include "MissingFilesDlg.h"
#include "MultiFileDlg.h"
#include "FormatIO.h"
#include "ProgressDlg.h"
#include "WhorldRCDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	EXT_THUMBS	_T(".wrc")	// for thumbnail export

/////////////////////////////////////////////////////////////////////////////
// CPlaylistDlg dialog

IMPLEMENT_DYNAMIC(CPlaylistDlg, CToolDlg);

const CCtrlResize::CTRL_LIST CPlaylistDlg::m_CtrlList[] = {
	// list matches tab order EXCEPT captions come AFTER their controls
	{IDC_PL_LIST,				BIND_ALL},
	{IDC_PL_BANK,				BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_BANK_CAP,			BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_HOTKEY,				BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_HOTKEY_CAP,			BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_SHUFFLE,			BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_PLAY,				BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_AUTOPLAY_CAP,		BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_PATCH_SECS_EDIT,	BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_PATCH_SECS_SPIN,	BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_PATCH_SECS_CAP,		BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_XFADE_SECS_EDIT,	BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_XFADE_SECS_SPIN,	BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_XFADE_SLIDER,		BIND_LEFT | BIND_BOTTOM},
	{IDC_PL_XFADE_SECS_CAP,		BIND_LEFT | BIND_BOTTOM},
	{0}	// list terminator
};

const int CPlaylistDlg::m_StateIcon[STATES] = {	// must match state enum
	IDI_PL_NORMAL,
	IDI_PL_PLAYING,
	IDI_PL_XFADING
};

const CPlaylistDlg::COLDATA CPlaylistDlg::m_ColData[COLUMNS] = {
	{IDS_PL_COL_KEY,	LVCFMT_LEFT,	30},
	{IDS_PL_COL_NAME,	LVCFMT_LEFT,	150},
	{IDS_PL_COL_PATH,	LVCFMT_LEFT,	300}
};

const CPlaylistDlg::INFO CPlaylistDlg::m_Defaults = {
	{0},	// Midi
	DEFAULT_PATCH_SECS,	// PatchSecs
	DEFAULT_PATCH_SECS * DEFAULT_XFADE_PCT * 100,	// XFadeSecs
	FALSE,	// Playing
	FALSE,	// Shuffle 
	0,		// CurBank
	-1,		// CurPatch
	0		// CurPatchBank
};

const SIZE CPlaylistDlg::m_ThumbSizePreset[THUMB_SIZES] = {
	{50,	37},	// tiny
	{80,	60},	// small
	{100,	75},	// medium
	{160,	120},	// large
	{200,	150},	// huge
};

CPlaylistDlg::CPlaylistDlg(CWnd* pParent) : 
	CToolDlg(CPlaylistDlg::IDD, IDR_MAINFRAME, _T("PlaylistDlg"), pParent),
	m_RecentFile(0, _T("Recent Playlists"), _T("Playlist%d"), MAX_RECENT_FILES)
{
	//{{AFX_DATA_INIT(CPlaylistDlg)
	//}}AFX_DATA_INIT
	m_InitRect.SetRectEmpty();
	m_RecentFile.ReadList();
	ZeroMemory(&m_Midi, sizeof(m_Midi));
	m_Frm = NULL;
	m_Patch = &m_Bank[0];
	m_ToolDlgAccel = LoadAccelerators(AfxGetResourceHandle(), 
		MAKEINTRESOURCE(IDR_PLAYLIST));
	m_Timer = 0;
	m_CurBank = 0;
	m_CurPatch = -1;
	m_CurPatchBank = 0;
	m_XFadeFrac = DEFAULT_XFADE_PCT / 100.0;
	m_ModFlag = FALSE;
	m_IsCached = FALSE;
	m_List.SetFormat(_T("Whorld PlaylistDlg"), sizeof(CBDATA));
	m_ThumbView = FALSE;
	m_ThumbSize = m_ThumbSizePreset[THUMB_MEDIUM];
	DWORD	sz = sizeof(SIZE);
	CPersist::GetBinary(REG_SETTINGS, _T("ThumbSize"), &m_ThumbSize, &sz);
}

CPlaylistDlg::~CPlaylistDlg()
{
	m_RecentFile.WriteList();
}

BOOL CPlaylistDlg::Create(UINT nIDTemplate, CMainFrame *pFrame)
{
	m_Frm = pFrame;
	return CToolDlg::Create(nIDTemplate, pFrame);
}

void CPlaylistDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaylistDlg)
	DDX_Control(pDX, IDC_PL_BANK, m_BankCombo);
	DDX_Control(pDX, IDC_PL_HOTKEY_CAP, m_HotkeyCap);
	DDX_Control(pDX, IDC_PL_BANK_CAP, m_BankCap);
	DDX_Control(pDX, IDC_PL_AUTOPLAY_CAP, m_AutoplayCap);
	DDX_Control(pDX, IDC_PL_PATCH_SECS_CAP, m_PatchSecsCap);
	DDX_Control(pDX, IDC_PL_XFADE_SECS_CAP, m_XFadeSecsCap);
	DDX_Control(pDX, IDC_PL_PATCH_SECS_SPIN, m_PatchSecsSpin);
	DDX_Control(pDX, IDC_PL_XFADE_SECS_SPIN, m_XFadeSecsSpin);
	DDX_Control(pDX, IDC_PL_HOTKEY, m_HotKey);
	DDX_Control(pDX, IDC_PL_LIST, m_List);
	DDX_Control(pDX, IDC_PL_XFADE_SECS_EDIT, m_XFadeSecsEdit);
	DDX_Control(pDX, IDC_PL_XFADE_SLIDER, m_XFadeSlider);
	DDX_Control(pDX, IDC_PL_PLAY, m_Play);
	DDX_Control(pDX, IDC_PL_SHUFFLE, m_Shuffle);
	DDX_Control(pDX, IDC_PL_PATCH_SECS_EDIT, m_PatchSecsEdit);
	//}}AFX_DATA_MAP
}

inline bool CPlaylistDlg::IsVideo(LPCTSTR Path)
{
	return(!_tcsicmp(PathFindExtension(Path), EXT_AVI));
}

void CPlaylistDlg::CPlaylistCtrl::GetInfo(int Pos, void *Info)
{
	CPlaylistDlg *dlg = (CPlaylistDlg *)GetParent();
	CBDATA	*cbd = (CBDATA *)Info;
	cbd->Bank = dlg->GetBank();
	cbd->HotKey = dlg->GetHotKey(Pos);
	_tcsncpy(cbd->Path, dlg->GetPath(Pos), MAX_PATH);
	cbd->Path[MAX_PATH] = 0;	// in case of truncation
}

void CPlaylistDlg::CPlaylistCtrl::InsertInfo(int Pos, const void *Info)
{
	CPlaylistDlg *dlg = (CPlaylistDlg *)GetParent();
	const CBDATA	*cbd = (const CBDATA *)Info;
	int	key = cbd->HotKey;
	if (dlg->KeyToPatch(key) >= 0)	// if key is already assigned
		key = 0;	// avoid duplicate assignment
	dlg->Insert(dlg->GetBank(), Pos, cbd->Path, key);
}

void CPlaylistDlg::CPlaylistCtrl::DeleteInfo(int Pos)
{
	CPlaylistDlg *dlg = (CPlaylistDlg *)GetParent();
	dlg->Delete(dlg->GetBank(), Pos);
}

void CPlaylistDlg::GetDefaults(INFO& Info) const
{
	Info = m_Defaults;
	CPlaylistMidiDlg::GetDefaults(Info.Midi);
}

void CPlaylistDlg::GetInfo(INFO& Info) const
{
	GetMidiInfo(Info.Midi);
	GetTiming(Info.PatchSecs, Info.XFadeSecs);
	Info.Playing = IsPlaying();
	Info.Shuffle = GetShuffle();
	Info.CurBank = m_CurBank;
	Info.CurPatch = m_CurPatch;
	Info.CurPatchBank = m_CurPatchBank;
}

void CPlaylistDlg::SetInfo(const INFO& Info)
{
	SetMidiInfo(Info.Midi);
	SetTiming(Info.PatchSecs, Info.XFadeSecs);
	Play(Info.Playing);
	SetShuffle(Info.Shuffle);
	SetBank(Info.CurBank);
	if (m_Frm->GetViewSel() == CMainFrame::SEL_MIX)
		FadeToBank(Info.CurPatchBank, Info.CurPatch, 0);
}

void CPlaylistDlg::RestoreDefaults()
{
	INFO	Info;
	GetDefaults(Info);
	SetInfo(Info);
}

void CPlaylistDlg::SetModify(bool Enable)
{
	m_ModFlag = Enable;
}

void CPlaylistDlg::SetStateIcon(int Idx, int State)
{
	m_List.SetItemState(Idx, INDEXTOSTATEIMAGEMASK(State + 1), LVIS_STATEIMAGEMASK);
}

void CPlaylistDlg::SetCurPatch(int Bank, int Patch, int State)
{
	if (GetCurPatch() >= 0)
		SetStateIcon(m_CurPatch, TS_NORMAL);
	if (Bank == m_CurBank) {
		if (State >= 0)
			SetStateIcon(Patch, State);
		m_List.SetCurSel(Patch);
	}
	m_CurPatch = Patch;
	m_CurPatchBank = Bank;
}

void CPlaylistDlg::RemoveAll()
{
	m_List.DeleteAllItems();
	for (int i = 0; i < BANKS; i++)
		m_Bank[i].RemoveAll();
	m_Thumb.RemoveAll();
	m_Frm->GetView()->GetVideo().RemoveAll();
}

bool CPlaylistDlg::MakeThumbs()
{
	if (!m_Thumb.IsCreated() && !m_Thumb.Create(m_ThumbSize))
		return(FALSE);
	CStringArray	PathList;
	CDWordArray	ImgIdx;
	int	Bank, Patches;
	for (Bank = 0; Bank < BANKS; Bank++) {
		Patches = GetCount(Bank);
		for (int i = 0; i < Patches; i++)
			PathList.Add(m_Bank[Bank][i].m_Path);
	}
	bool	retc = m_Thumb.Add(PathList, ImgIdx);
	m_List.SetImageList(m_Thumb.GetImageList(), LVSIL_NORMAL);
	int	j = 0;
	for (Bank = 0; Bank < BANKS; Bank++) {
		int Patches = GetCount(Bank);
		for (int i = 0; i < Patches; i++)
			m_Bank[Bank][i].m_ImgIdx = ImgIdx[j++];	// copy image indices
	}
	return(retc);
}

void CPlaylistDlg::InsertItem(int Idx, LPCTSTR Path, DWORD HotKey, int ImgIdx)
{
	CPathStr	Name(PathFindFileName(Path));
	Name.RemoveExtension();
	CString	KeyStr(TCHAR(HotKey), 1);
	if (m_ThumbView)
		KeyStr = Name + "\n" + KeyStr;
	int pos = m_List.InsertItem(Idx, KeyStr, ImgIdx);
	m_List.SetItemText(pos, COL_NAME, Name);
	CPathStr	Folder(Path);
	Folder.RemoveFileSpec();
	m_List.SetItemText(pos, COL_PATH, Folder);
}

void CPlaylistDlg::Insert(int Bank, int Patch, LPCTSTR Path, DWORD HotKey)
{
	ASSERT(Bank >= 0 && Bank < BANKS);
	int	ImgIdx = m_ThumbView ? m_Thumb.Add(Path) : -1;
	CPatchLink	pl(Bank, HotKey, Path, ImgIdx);
	m_Bank[Bank].InsertAt(Patch, pl);
	if (Bank == m_CurBank)	// if bank is current
		InsertItem(Patch, Path, HotKey, ImgIdx);	// update list control
	if (Patch <= GetCurPatch())	// if we're on or below current patch
		m_CurPatch++;	// bump current patch
	SetModify();
}

void CPlaylistDlg::Delete(int Bank, int Patch)
{
	ASSERT(Bank >= 0 && Bank < BANKS);
	if (Bank == m_CurBank) {	// if bank is current
		m_List.DeleteItem(Patch);	// update list control
		if (Patch == m_CurPatch)	// if we deleted current patch
			m_CurPatch = -1;			// mark it invalid
		else {
			if (Patch < m_CurPatch)		// if we're below current patch
				m_CurPatch--;				// move current patch down one
		}
	}
	m_Bank[Bank].RemoveAt(Patch);
	SetModify();
}

bool CPlaylistDlg::Insert(int Patch, const CStringArray& List, CString& Errs)
{
	CWaitCursor	wc;
	if (Patch < 0) {	// if no insert position specified
		if (m_List.GetSelectedCount())	// if there's a selection
			Patch = m_List.GetSelectionMark();	// insert before selection
		if (Patch < 0)	// if no selection available
			Patch = GetCount();	// insert after last element
	}
	int	InsCount = 0;
	// iterate through the results
	Errs.Empty();
	for (int i = 0; i < List.GetSize(); i++) {
		CString	Path = List[i];
		CPatch	PatchData;
		if (ReadPatch(Path, PatchData)) {
			Insert(m_CurBank, Patch + InsCount, Path, 0);
			InsCount++;
		} else
			Errs += "\n" + Path;
	}
	if (m_ThumbView)
		PopulateList();	// auto-arrange doesn't handle insert
	return(Errs.IsEmpty() != 0);
}

void CPlaylistDlg::Insert()
{
	CMultiFileDlg	fd(TRUE, EXT_PATCH, NULL, 
		OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT,
		LDS(IDS_FILTER_PATCH_LINK), LDS(IDS_PL_INSERT_PATCHES));
	if (fd.DoModal() == IDOK) {
		CString	Errs;
		if (!Insert(-1, *fd.GetPathArray(), Errs))
			AfxMessageBox(LDS(IDS_PL_BAD_PATCH) + Errs);
	}
}

int CPlaylistDlg::KeyToPatch(DWORD HotKey) const
{
	int	Patches = GetCount();
	for (int i = 0; i < Patches; i++) {
		if (HotKey == (*m_Patch)[i].m_HotKey)
			return(i);
	}
	return(-1);
}

int CPlaylistDlg::KeyToBank(DWORD HotKey) const
{
	if (HotKey < '0' || HotKey > '9')
		return(-1);
	int	Bank = HotKey - '0' - 1;
	if (Bank < 0)
		Bank = 9;	// map 0 key to tenth bank
	return(Bank);
}

bool CPlaylistDlg::ReadPatch(LPCTSTR Path, CPatch& Patch)
{
	if (IsVideo(Path))
		return(PathFileExists(Path) != 0);
	CStdioFile	fp;
	return(fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite) && Patch.Read(fp));
}

void CPlaylistDlg::UpdateUI()
{
	int	PatchSecs = m_PatchSecsEdit.GetIntVal();
	m_XFadeSlider.SetRange(0, PatchSecs, TRUE);
	m_XFadeSlider.SetDefaultPos(PatchSecs / 2);
	m_XFadeSlider.SetPos(round(GetXFadeSecs()));
	if (GetPatchSecs())
		m_XFadeFrac = GetXFadeSecs() / GetPatchSecs();
}

bool CPlaylistDlg::FadeToBank(int Bank, int Patch, double Secs)
{
	if (Bank < 0 || Bank >= BANKS)
		return(FALSE);
	if (Patch < 0 || Patch >= GetCount(Bank))
		return(FALSE);
	CString	Path = m_Bank[Bank][Patch].m_Path;
	if (IsVideo(Path)) {
		bool	retc = m_Frm->OpenVideo(Path);
		if (retc)
			SetCurPatch(Bank, Patch, TS_PLAYING);
		return(retc);
	}
	CString	Name;
	Name = m_List.GetItemText(Patch, COL_NAME);
	SetCurPatch(Bank, Patch, -1);
	CPatch	PatchData, *pp;
	if (m_IsCached)
		pp = &m_Cache[Bank][Patch];
	else {
		if (!ReadPatch(Path, PatchData)) {
			if (!m_Frm->IsVeejay())
				AfxMessageBox(LDS(IDS_PL_BAD_PATCH) + "\n" + Path);
			return(FALSE);
		}
		pp = &PatchData;
	}
	if (Secs < 0)	// if no override
		Secs = GetXFadeSecs();
	m_Frm->SetViewSel(CCrossDlg::SEL_MIX);	// assume we want to view mix
	m_Frm->GetXFader().FadeTo(*pp, Name, Secs, TRUE);
	return(TRUE);
}

bool CPlaylistDlg::HandleHotKey(DWORD HotKey)
{
	DWORD	Mode = m_Frm->GetInputMode();
	int	Bank = KeyToBank(HotKey);
	if (Bank >= 0) {
		if (Mode & CMainFrame::IM_CONTROL)
			return(FALSE);
		if (Mode & CMainFrame::IM_SHIFT)	// shift selects ten extra banks
			Bank += 10;
		if (!GetCount(Bank))	// don't switch to an empty bank
			return(FALSE);	
		SetBank(Bank);
		return(TRUE);
	}
	if (Mode & (CMainFrame::IM_CONTROL | CMainFrame::IM_SHIFT))
		return(FALSE);
	int	Patch = KeyToPatch(HotKey);
	if (Patch < 0)
		return(FALSE);
	return(FadeTo(Patch, -1));
}

void CPlaylistDlg::SetCurPatchState(bool Fading)
{
	if (GetCurPatch() >= 0)
		SetStateIcon(m_CurPatch, Fading ? TS_XFADING : TS_PLAYING);	
}

int CPlaylistDlg::GetNext()
{
	int	Patches = GetCount();
	if (!Patches)
		return(-1);	// no patches in current bank
	int	Next;
	if (m_Shuffle.GetCheck()) {
		if (m_Rand.GetSize() != Patches)
			m_Rand.Init(Patches);
		Next = m_Rand.GetNext();
	} else {
		Next = GetCurPatch();
		if (Next >= 0) {	// if current patch is in current bank
			Next++;
			if (Next >= Patches)
				Next = 0;
		} else
			Next = 0;
	}
	return(Next);
}

void CPlaylistDlg::SetShuffle(bool Enable)
{
	m_Shuffle.SetCheck(Enable);
}

void CPlaylistDlg::SetTiming(double PatchSecs, double XFadeSecs)
{
	m_XFadeSecsEdit.SetVal(XFadeSecs);
	SetPatchSecs(PatchSecs);
}

void CPlaylistDlg::SetPatchSecs(double Secs)
{
	m_PatchSecsEdit.SetVal(Secs);
	UpdateUI();
	if (IsPlaying())
		InitTimer();
}

void CPlaylistDlg::SetXFadeSecs(double Secs)
{
	m_XFadeSecsEdit.SetVal(Secs);
	UpdateUI();
}

void CPlaylistDlg::SetXFadeFrac(double Frac)
{
	if (Frac < 1e-6)	// prevent infinitesimal
		Frac = 0;
	m_XFadeSecsEdit.SetVal(GetPatchSecs() * Frac);
	UpdateUI();
}

bool CPlaylistDlg::PlayNext(double Secs)
{
	int	next = GetNext();
	if (next < 0)
		return(FALSE);
	return(FadeTo(next, Secs));
}

bool CPlaylistDlg::InitTimer()
{
	if (m_Timer)
		KillTimer(m_Timer);
	int	Period = round(GetPatchSecs() * 1000);
	if (Period <= 0)
		return(FALSE);
	m_Timer = SetTimer(TIMER_ID, Period, 0);
	return(m_Timer != 0);
}

void CPlaylistDlg::Play(bool Enable)
{
	if (Enable == IsPlaying())
		return;
	if (Enable) {
		if (GetCount()) {
			InitTimer();
			PlayNext();
		} else {
			if (!m_Frm->IsVeejay())
				AfxMessageBox(IDS_PL_NO_PATCHES);
		}
	} else {
		KillTimer(m_Timer);
		m_Timer = 0;
		m_Frm->GetXFader().Play(FALSE);
	}
	m_Play.SetCheck(IsPlaying());
	m_Play.SetIcon(AfxGetApp()->LoadIcon(IsPlaying() ? IDI_PLAYD : IDI_PLAYU));
}

void CPlaylistDlg::SetListPath(LPCTSTR Path)
{
	m_ListPath = Path;
	CString	Name = Path != NULL ? PathFindFileName(Path) : m_Untitled;
	SetWindowText(m_Caption + " - " + Name); 
}

CString CPlaylistDlg::GetName(int Patch) const
{
	CPathStr	Name(PathFindFileName(GetPath(Patch)));
	Name.RemoveExtension();
	return(Name);
}

void CPlaylistDlg::PopulateList()
{
	m_List.DeleteAllItems();
	int	Patches = GetCount();
	for (int i = 0; i < Patches; i++) {
		CPatchLink	*p = &(*m_Patch)[i];
		InsertItem(i, p->m_Path, p->m_HotKey, p->m_ImgIdx);
	}
	SetCurPatchState(m_Frm->GetXFader().IsPlaying());	
}

bool CPlaylistDlg::Read(LPCTSTR Path)
{
	CWaitCursor	wc;
	CString	Errs;
	CPlaylist	pl;
	if (!pl.Read(Path))
		return(FALSE);
	RemoveAll();
	CPathStr	ListFolder(Path);
	ListFolder.RemoveFileSpec();
	int	Patches = pl.m_Patch.GetSize();
	CStringArray	PatchPath;
	PatchPath.SetSize(Patches);
	int	i;
	for (i = 0; i < Patches; i++) {
		PatchPath[i] = pl.m_Patch[i].m_Path;
		if (PathIsFileSpec(PatchPath[i])) {	// if filename only
			CPathStr	Path(ListFolder);	// assume patch is in playlist folder
			Path.Append(PatchPath[i]);
			PatchPath[i] = Path;
		}
	}
	CString	Filter((LPCTSTR)IDS_FILTER_PATCH_LINK);	// dialog stores pointer to filter
	CMissingFilesDlg	dlg(PatchPath, EXT_PATCH, Filter);
	if (dlg.Check() == IDCANCEL)
		return(FALSE);
	CPatch	PatchData;
	bool	PrevThumbView = m_ThumbView;	// save view type
	m_ThumbView = FALSE;	// prevent insert from adding thumbnails one at a time
	for (i = 0; i < Patches; i++) {
		if (!PatchPath[i].IsEmpty()) {
			if (ReadPatch(PatchPath[i], PatchData)) {
				int	Bank = CLAMP(pl.m_Patch[i].m_Bank, 0, BANKS - 1);
				Insert(Bank, GetCount(Bank), PatchPath[i], pl.m_Patch[i].m_HotKey);
			} else
				Errs += "\n" + PatchPath[i];
		}
	}
	m_ThumbView = PrevThumbView;	// restore view type
	if (!Errs.IsEmpty())
		AfxMessageBox(LDS(IDS_PL_BAD_PATCH) + Errs);
	m_Frm->SetInfo(pl.m_Main);
	SetListPath(Path);
	SetModify(FALSE);
	if (m_ThumbView) {
		MakeThumbs();	// add thumbnails all at once
		PopulateList();
	}
	return(Errs.IsEmpty() != 0);
}

bool CPlaylistDlg::Write(LPCTSTR Path)
{
	CWaitCursor	wc;
	CPathStr	ListFolder(Path);
	ListFolder.RemoveFileSpec();
	CPlaylist	pl;
	for (int i = 0; i < BANKS; i++) {
		int	Patches = GetCount(i);	// count of patches in this bank
		for (int j = 0; j < Patches; j++) {
			LPCTSTR	PatchPath = m_Bank[i][j].m_Path;
			CPathStr	PatchFolder = PatchPath;
			PatchFolder.RemoveFileSpec();
			if (PatchFolder == ListFolder)	// if patch is playlist folder
				PatchPath = PathFindFileName(PatchPath);	// store filename only
			CPatchLink	Patch;
			Patch.m_Bank = i;
			Patch.m_HotKey = m_Bank[i][j].m_HotKey;
			Patch.m_Path = PatchPath;
			pl.m_Patch.Add(Patch);
		}
	}
	m_Frm->GetInfo(pl.m_Main);
	if (!pl.Write(Path))
		return(FALSE);
	SetListPath(Path);
	SetModify(FALSE);
	return(TRUE);
}

bool CPlaylistDlg::Save()
{
	if (m_ListPath.IsEmpty())
		return(SaveAs());
	return(Write(m_ListPath));
}

bool CPlaylistDlg::SaveAs()
{
	CMultiFileDlg	fd(FALSE, EXT_PLAYLIST, NULL, OFN_OVERWRITEPROMPT, 
		LDS(IDS_FILTER_PLAYLIST), LDS(IDS_PL_SAVE_AS));
	if (fd.DoModal() != IDOK)
		return(FALSE);
	if (!Write(fd.GetPathName()))
		return(FALSE);
	m_RecentFile.Add(fd.GetPathName());
	return(TRUE);
}

bool CPlaylistDlg::SaveCheck()
{
	if (!IsModified())
		return(TRUE);
	CString	s;
	AfxFormatString1(s, IDS_PL_SAVE_PROMPT,
		m_ListPath.IsEmpty() ? m_Untitled : m_ListPath);
	int	retc = AfxMessageBox(s, MB_YESNOCANCEL);
	if (retc == IDCANCEL)
		return(FALSE);
	if (retc == IDNO)
		return(TRUE);
	return(Save());
}

bool CPlaylistDlg::New()
{
	if (!SaveCheck())
		return(FALSE);
	RemoveAll();
	m_ModFlag = FALSE;
	Play(FALSE);
	SetListPath(NULL);
	m_Frm->RestoreDefaults();
	return(TRUE);
}

bool CPlaylistDlg::Open(LPCTSTR Path)
{
	if (!SaveCheck())
		return(FALSE);
	if (!Read(Path))
		return(FALSE);
	m_RecentFile.Add(Path);
	return(TRUE);
}

bool CPlaylistDlg::Open()
{
	CMultiFileDlg	fd(TRUE, EXT_PLAYLIST, NULL, OFN_HIDEREADONLY,
		LDS(IDS_FILTER_PLAYLIST), LDS(IDS_PL_OPEN));
	if (fd.DoModal() != IDOK)
		return(FALSE);
	return(Open(fd.GetPathName()));
}

bool CPlaylistDlg::OpenRecent(int FileIdx)
{
	if (FileIdx < 0 && FileIdx >= m_RecentFile.GetSize())
		return(FALSE);
	if (!Open(m_RecentFile[FileIdx])) {
		m_RecentFile.Remove(FileIdx);
		return(FALSE);
	}
	return(TRUE);
}

void CPlaylistDlg::SetHotKey(int Patch, DWORD HotKey)
{
	(*m_Patch)[Patch].m_HotKey = HotKey;
	CString	KeyStr(TCHAR(HotKey), 1);
	if (m_ThumbView)
		KeyStr = GetName(Patch) + "\n" + KeyStr;
	m_List.SetItemText(Patch, COL_KEY, KeyStr);
}

bool CPlaylistDlg::AssignHotKey(int Patch, DWORD HotKey)
{
	if (Patch < 0 || Patch > GetCount())
		return(FALSE);
	if (HotKey == (*m_Patch)[Patch].m_HotKey)
		return(TRUE);	// nothing to do
	bool	retc = TRUE;	// assume success
	if (HotKey) {
		if (HotKey >= 'A' && HotKey <= 'Z') {
			int	Owner = KeyToPatch(HotKey);
			if (Owner >= 0) {
				CString	msg;
				msg.FormatMessage(IDS_PL_KEY_ASSIGNED, CString(char(HotKey)),
					m_List.GetItemText(Owner, COL_NAME),
					m_List.GetItemText(Patch, COL_NAME));
				if (AfxMessageBox(msg, MB_YESNO) == IDYES)
					SetHotKey(Owner, 0);
				else {
					m_HotKey.SetHotKey(0);	// avoids potential lockup on close
					retc = FALSE;
				}
				m_List.SetFocus();
			}
		} else {
			AfxMessageBox(IDS_PL_KEY_RESERVED);
			retc = FALSE;
		}
	}
	if (retc) {
		SetHotKey(Patch, HotKey);
		SetModify();
	}
	return(retc);
}

bool CPlaylistDlg::Cache(bool Enable)
{
	if (Enable == m_IsCached)
		return(TRUE);	// nothing to do
	if (Enable) {
		CWaitCursor	wc;
		CStringArray	VideoPath;
		int	i;
		for (i = 0; i < BANKS; i++) {
			int	count = m_Bank[i].GetSize();
			m_Cache[i].SetSize(count);
			for (int j = 0; j < count; j++) {
				LPCTSTR	Path = m_Bank[i][j].m_Path;
				if (!ReadPatch(Path, m_Cache[i][j])) {
					AfxMessageBox(LDS(IDS_PL_BAD_PATCH) + "\n" + Path);
					m_Cache[i].SetSize(0);	// all or nothing
					return(FALSE);
				}
				if (IsVideo(Path))
					VideoPath.Add(Path);
			}
		}
		int	Videos = VideoPath.GetSize();
		if (Videos) {
			CMainFrame::TimerState	ts(FALSE);	// freezes animation while in scope
			CProgressDlg	prog;
			prog.Create();	// show progress dialog
			prog.SetRange(0, Videos);
			prog.SetWindowText(LDS(IDS_PL_CACHING_VIDEOS));
			// we want to reduce the delay when selecting videos for the first
			// time, but we don't actually cache the CAviToBmp instances; it's
			// too complicated, and we can get a similar performance gain just
			// by opening and closing each video, presumably due to OS caching
			CAviToBmp	avi;
			for (i = 0; i < Videos; i++) {
				prog.SetPos(i);
				if (!avi.Open(VideoPath[i])) {
					CString	s;
					AfxFormatString2(s, IDS_PL_BAD_VIDEO, VideoPath[i], 
						avi.GetLastErrorString());
					AfxMessageBox(s);
					return(FALSE);
				}
				if (prog.Canceled())
					break;
			}
		}
	} else {
		for (int i = 0; i < BANKS; i++)
			m_Cache[i].SetSize(0);
	}
	m_IsCached = Enable;
	return(TRUE);
}

void CPlaylistDlg::SendTo(int SelIdx, int Patch)
{
	if (Patch < 0 || Patch >= GetCount())
		return;
	switch (SelIdx) {
	case CCrossDlg::SEL_DOC:
		AfxGetApp()->OpenDocumentFile((*m_Patch)[Patch].m_Path);
		break;
	default:
		if (m_IsCached)
			m_Frm->GetXFader().SetInfo(SelIdx, m_Cache[m_CurBank][Patch]);
		else
			m_Frm->GetXFader().Open(SelIdx, (*m_Patch)[Patch].m_Path);
	}
	return;
}

void CPlaylistDlg::LoadNext(int SelIdx)
{
	int	Patch = GetNext();
	SendTo(SelIdx, Patch);
	SetCurPatch(m_CurBank, Patch, TS_PLAYING);
}

void CPlaylistDlg::SetBank(int Bank)
{
	if (Bank < 0 || Bank >= BANKS || Bank == m_CurBank)
		return;	// nothing to do
	m_BankCombo.SetCurSel(Bank);
	m_CurBank = Bank;
	m_Patch = &m_Bank[Bank];
	PopulateList();
}

bool CPlaylistDlg::SetThumbView(bool Enable)
{
	if (Enable == m_ThumbView)
		return(TRUE);	// nothing to do
	if (Enable) {
		if (!MakeThumbs())	// if we can't create thumbnails
			return(FALSE);	// don't change list view type
	}
	m_ThumbView = Enable;
	m_List.ModifyStyle(LVS_TYPEMASK, m_ThumbView ? LVS_ICON : LVS_REPORT);
	PopulateList();
	return(TRUE);
}

void CPlaylistDlg::SetThumbSize(CSize Size)
{
	if (Size == m_ThumbSize)
		return;	// nothing to do
	m_ThumbSize = Size;
	m_Thumb.Create(Size);
	if (m_ThumbView) {
		MakeThumbs();
		PopulateList();
	}
}

BEGIN_MESSAGE_MAP(CPlaylistDlg, CToolDlg)
	//{{AFX_MSG_MAP(CPlaylistDlg)
	ON_BN_CLICKED(IDC_PL_PLAY, OnPlay)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_NOTIFY(NEN_CHANGED, IDC_PL_PATCH_SECS_EDIT, OnChangedSecsEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_PL_XFADE_SECS_EDIT, OnChangedXFadeEdit)
	ON_WM_HSCROLL()
	ON_NOTIFY(HKN_KILLFOCUS, IDC_PL_HOTKEY, OnKillFocusHotKey)
	ON_NOTIFY(ULVN_REORDER, IDC_PL_LIST, OnReorderList)
	ON_NOTIFY(NM_DBLCLK, IDC_PL_LIST, OnDblclkList)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_PL_LIST, OnBegindragList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_PL_LIST, OnKeydownList)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_PL_SHUFFLE, OnShuffle)
	ON_COMMAND(ID_PL_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_PL_FILE_INSERT_PATCHES, OnFileInsertPatches)
	ON_COMMAND(ID_PL_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_PL_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_PL_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_PL_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_PL_EDIT_REMOVE_KEY, OnEditRemoveKey)
	ON_COMMAND(ID_PL_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_PL_EDIT_AUTO_ASSIGN, OnEditAutoAssign)
	ON_WM_DROPFILES()
	ON_NOTIFY(NM_RCLICK, IDC_PL_LIST, OnRclickList)
	ON_UPDATE_COMMAND_UI(ID_PL_FILE_MRU_FILE1, OnUpdateFileMru)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_PL_FILE_IMPORT_MIDI_SETUP, OnFileImportMidiSetup)
	ON_COMMAND(ID_PL_EDIT_MIDI_MAPPING, OnEditMidiMapping)
	ON_COMMAND(ID_PL_EDIT_FADE_TO, OnEditFadeTo)
	ON_COMMAND(ID_PL_EDIT_SEND_TO_A, OnEditSendToA)
	ON_COMMAND(ID_PL_EDIT_SEND_TO_B, OnEditSendToB)
	ON_COMMAND(ID_PL_EDIT_SEND_TO_DOC, OnEditSendToDoc)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_DELETE, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_CBN_SELCHANGE(IDC_PL_BANK, OnSelchangeBank)
	ON_COMMAND(ID_PL_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_PL_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_PL_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_PASTE, OnUpdateEditPaste)
	ON_WM_SYSCOLORCHANGE()
	ON_COMMAND(ID_PL_VIEW_REFRESH_THUMBS, OnViewRefreshThumbs)
	ON_UPDATE_COMMAND_UI(ID_PL_VIEW_REFRESH_THUMBS, OnUpdateViewRefreshThumbs)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_FADE_TO, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_SEND_TO_A, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_SEND_TO_B, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_SEND_TO_DOC, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_REMOVE_KEY, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_COPY, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_CUT, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_AUTO_ASSIGN, OnUpdateEditSelectAll)
	ON_COMMAND(ID_PL_FILE_EXPORT_THUMBS, OnFileExportThumbs)
	ON_UPDATE_COMMAND_UI(ID_PL_FILE_EXPORT_THUMBS, OnUpdateFileExportThumbs)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_PL_FILE_MRU_FILE1, ID_PL_FILE_MRU_FILE4, OnFileMru)
	ON_COMMAND_RANGE(ID_PL_VIEW_TYPE_DETAILS, ID_PL_VIEW_TYPE_THUMBS, OnViewType)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PL_VIEW_TYPE_DETAILS, ID_PL_VIEW_TYPE_THUMBS, OnUpdateViewType)
	ON_COMMAND_RANGE(ID_PL_VIEW_THUMB_SIZE0, ID_PL_VIEW_THUMB_SIZE4, OnViewThumbSize)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PL_VIEW_THUMB_SIZE0, ID_PL_VIEW_THUMB_SIZE4, OnUpdateViewThumbSize)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaylistDlg message handlers

BOOL CPlaylistDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();
	
	GetWindowText(m_Caption);	// save dialog title for caption updating
	m_Untitled.LoadString(IDS_PL_UNTITLED);
	m_Play.SetIcon(AfxGetApp()->LoadIcon(IDI_PLAYU));	// load play button icon
	// create list control
	int	i;
	CString	s;
	for (i = 0; i < COLUMNS; i++) {
		const COLDATA	*cd = &m_ColData[i];
		s.LoadString(cd->TitleID);
		m_List.InsertColumn(i, s, cd->Align, cd->Width);
	}
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List.ModifyStyle(0, LVS_AUTOARRANGE);	// handles delete but not insert
	for (i = 0; i < BANKS; i++) {
		s.Format(_T("%d"), i + 1);
		m_BankCombo.AddString(s);
	}
	m_BankCombo.SetCurSel(0);
	m_StateImg.Create(12, 12, ILC_COLOR | ILC_MASK, STATES, STATES);	// build state icon list
	for (i = 0; i < STATES; i++)
		m_StateImg.Add(AfxGetApp()->LoadIcon(m_StateIcon[i]));
	m_List.SetImageList(&m_StateImg, LVSIL_STATE);
	m_PatchSecsEdit.SetRange(0, INT_MAX);
	m_XFadeSecsEdit.SetRange(0, INT_MAX);
	m_XFadeSlider.SetDefaultPos(DEFAULT_PATCH_SECS / 2);
	RestoreDefaults();
	UpdateUI();
	m_HotKey.SetRules((WORD)-1, 0);	// no key modifiers allowed
	GetWindowRect(m_InitRect);
	m_Resize.AddControlList(this, m_CtrlList);
	SetListPath(NULL);
	DragAcceptFiles();
	m_StateImg.SetBkColor(GetSysColor(COLOR_WINDOW));

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CPlaylistDlg::OnFileNew() 
{
	New();
}

void CPlaylistDlg::OnFileOpen() 
{
	Open();
}

void CPlaylistDlg::OnFileSave() 
{
	if (m_ListPath.IsEmpty())
		SaveAs();
	else
		Save();
}

void CPlaylistDlg::OnFileSaveAs() 
{
	SaveAs();
}

void CPlaylistDlg::OnFileInsertPatches() 
{
	Insert();
}

void CPlaylistDlg::OnPlay() 
{
	Play(!IsPlaying());
}

void CPlaylistDlg::OnTimer(UINT nIDEvent) 
{
	CToolDlg::OnTimer(nIDEvent);
	PlayNext();
}

void CPlaylistDlg::OnDestroy() 
{
	CToolDlg::OnDestroy();
	if (m_Timer)
		KillTimer(m_Timer);
	CPersist::WriteBinary(REG_SETTINGS, _T("ThumbSize"), &m_ThumbSize, sizeof(SIZE));
}

void CPlaylistDlg::OnChangedSecsEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (IsPlaying())
		InitTimer();
	// keep ratio of crossfade seconds to patch seconds constant
	SetXFadeSecs(GetPatchSecs() * m_XFadeFrac);
	SetModify();
}

void CPlaylistDlg::OnChangedXFadeEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateUI();
	SetModify();
}

void CPlaylistDlg::OnBegindragList(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	for (int i = 0; i < m_List.GetItemCount(); i++)
		m_List.SetItemData(i, i);	// number the items
	*pResult = 0;
}

void CPlaylistDlg::OnReorderList(NMHDR* pNMHDR, LRESULT* pResult)
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	PATCH_LIST	tmp;
	tmp.Copy(*m_Patch);
	int	NewCurPatch = -1;
	int	CurPatch = GetCurPatch();
	for (int i = 0; i < m_List.GetItemCount(); i++) {
		int	OrigIdx = m_List.GetItemData(i);
		(*m_Patch)[i] = tmp[OrigIdx];
		if (OrigIdx == CurPatch) {	// if current patch
			NewCurPatch = i;			// store its new index
			CurPatch = -1;				// only one match allowed
		}
	}
	if (NewCurPatch >= 0)
		m_CurPatch = NewCurPatch;
	SetModify();
	if (m_ThumbView)
		PopulateList();	// redisplay thumbnails
	*pResult = 0;
}

void CPlaylistDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CWnd	*psb = pScrollBar;
	if (psb == &m_XFadeSlider) {
		SetXFadeSecs(m_XFadeSlider.GetPos());
		SetModify();
	}
	CToolDlg::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPlaylistDlg::OnKillFocusHotKey(NMHDR* pNMHDR, LRESULT* pResult)
{
	DWORD	Key = m_HotKey.GetHotKey();
	if (Key)	// else Alt+Tab clears current selection's key
		AssignHotKey(GetCurSel(), m_HotKey.GetHotKey());
	*pResult = 0;
}

void CPlaylistDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int	sel = GetCurSel();
	if (sel >= 0) {
		FadeTo(sel);
		if (IsPlaying())
			InitTimer();
	}
	*pResult = 0;
}

void CPlaylistDlg::OnOK()
{
	if (GetFocus() == &m_HotKey)	// if Enter pressed in hot key control
		SetFocus();	// force hot key assignment
	else
		CToolDlg::OnOK();
}

void CPlaylistDlg::OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	if (pLVKeyDow->flags & KF_REPEAT)	// discard auto-repeats
		return;
	DWORD	Key = pLVKeyDow->wVKey;
	if ((Key >= '0' && Key <= '9') || (Key >= 'A' && Key <= 'Z')) {
		DWORD	Mode = m_Frm->GetInputMode();
		if (!(Mode & (CMainFrame::IM_ALT | CMainFrame::IM_CONTROL))) {
			if (Mode & CMainFrame::IM_SHIFT)
				AssignHotKey(GetCurSel(), Key);
			else
				HandleHotKey(Key);
			*pResult = TRUE;	// inhibit base class
		}
	} else {
		switch (Key) {	// only while list control has focus
		case VK_BACK:
			AssignHotKey(GetCurSel(), 0);
			break;
		}
		*pResult = 0;
	}
}

void CPlaylistDlg::OnSize(UINT nType, int cx, int cy) 
{
	CToolDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CPlaylistDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (!m_InitRect.IsRectEmpty())
		lpMMI->ptMinTrackSize = CPoint(m_InitRect.Width(), m_InitRect.Height());
}

void CPlaylistDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CToolDlg::OnShowWindow(bShow, nStatus);
	if (bShow)
		m_Resize.OnSize();
}

void CPlaylistDlg::OnShuffle()
{
	SetModify();
}

void CPlaylistDlg::OnEditRemoveKey() 
{
	POSITION	pos = m_List.GetFirstSelectedItemPosition();
	while (pos != NULL)
		AssignHotKey(m_List.GetNextSelectedItem(pos), 0);
}

void CPlaylistDlg::OnEditSelectAll() 
{
	m_List.SelectAll();
}

void CPlaylistDlg::OnEditAutoAssign() 
{
	int	i;
	bool	Warned = FALSE;
	int	Items = GetCount();
	for (i = 0; i < Items; i++) {
		if ((*m_Patch)[i].m_HotKey) {	// if key assignments already exist, warn user
			if (!Warned && AfxMessageBox(IDS_PL_AUTO_ASSIGN_WARN, MB_YESNO) != IDYES)
				return;
			Warned = TRUE;
			AssignHotKey(i, 0);	// prevent key already assigned warning
		}
	}
	CString	AutoKey((LPCTSTR)IDS_PL_AUTO_ASSIGN_KEYS);
	for (i = 0; i < Items; i++) {
		if (i > AutoKey.GetLength())
			break;	// ran out of keys
		AssignHotKey(i, AutoKey[i]);
	}
}

void CPlaylistDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT	Files = DragQueryFile(hDropInfo, 0xFFFFFFFF, 0, 0);
	TCHAR	Path[MAX_PATH];
	DragQueryFile(hDropInfo, 0, Path, MAX_PATH);
	CStringArray	InsertList;
	for (UINT i = 0; i < Files; i++) {
		DragQueryFile(hDropInfo, i, Path, MAX_PATH);
		CString	Ext = PathFindExtension(Path); 
		if (!_tcsicmp(Ext, EXT_PLAYLIST))
			Open(Path);
		else
			InsertList.Add(Path);
	}
	if (InsertList.GetSize()) {
		CPoint	Cursor;
		GetCursorPos(&Cursor);
		m_List.ScreenToClient(&Cursor);
		int	Pos = m_List.HitTest(Cursor);
		CString	Errs;
		Insert(Pos, InsertList, Errs);
	}
}

void CPlaylistDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	CPoint	pt;
	GetCursorPos(&pt);
	CMenu	menu;
	menu.LoadMenu(IDR_PLAYLIST_CONTEXT);
	CMenu	*mp = menu.GetSubMenu(0);
	mp->TrackPopupMenu(0, pt.x, pt.y, this);
	*pResult = 0;
}

void CPlaylistDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	UpdateMenu(this, pPopupMenu);	// call menu's UI handlers
 	CToolDlg::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

void CPlaylistDlg::OnUpdateFileMru(CCmdUI* pCmdUI) 
{
	m_RecentFile.UpdateMenu(pCmdUI);
}

void CPlaylistDlg::OnFileMru(UINT nID)
{
	OpenRecent(nID - ID_PL_FILE_MRU_FILE1);
}

void CPlaylistDlg::OnFileImportMidiSetup() 
{
	CMultiFileDlg	fd(TRUE, EXT_PLAYLIST, NULL, OFN_HIDEREADONLY,
		LDS(IDS_FILTER_PLAYLIST), LDS(IDS_PL_IMPORT_MIDI_SETUP));
	if (fd.DoModal() == IDOK) {
		CPlaylist	src, dst;
		if (src.Read(fd.GetPathName())) {
			m_Frm->GetInfo(dst.m_Main);
			dst.m_Main.MidiSetup = src.m_Main.MidiSetup;
			m_Frm->SetInfo(dst.m_Main);
			SetModify(TRUE);
		}
	}
}

void CPlaylistDlg::OnFileExportThumbs() 
{
	CPathStr	Name(PathFindFileName(m_ListPath));
	Name.RenameExtension(EXT_THUMBS);
	CFileDialog	fd(FALSE, EXT_THUMBS, Name, OFN_OVERWRITEPROMPT, LDS(IDS_FILTER_THUMBS));
	if (fd.DoModal() != IDOK)
		return;
	CWaitCursor	wc;
	CWhorldRCDoc	doc;
	for (int i = 0; i < BANKS; i++)
		doc.m_Bank[i].Copy(m_Bank[i]);
	doc.m_ImgList.Create(m_Thumb.GetImageList());
	doc.m_ThumbSize = m_ThumbSize;
	CFile	fp;
	CFileException	e;
	if (!fp.Open(fd.GetPathName(), CFile::modeCreate | CFile::modeWrite, &e)) {
		e.ReportError();
		return;
	}
	CArchive	ar(&fp, CArchive::store);
	doc.Serialize(ar);
}

void CPlaylistDlg::OnUpdateFileExportThumbs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ThumbView);
}

void CPlaylistDlg::OnEditMidiMapping() 
{
	CPlaylistMidiDlg	md;
	md.SetInfo(m_Midi);
	if (md.DoModal() == IDOK) {
		md.GetInfo(m_Midi);
		SetModify();
	}
}

void CPlaylistDlg::OnEditFadeTo() 
{
	FadeTo(GetCurSel());
}

void CPlaylistDlg::OnEditSendToA() 
{
	SendTo(CCrossDlg::SEL_A, GetCurSel());
}

void CPlaylistDlg::OnEditSendToB() 
{
	SendTo(CCrossDlg::SEL_B, GetCurSel());
}

void CPlaylistDlg::OnEditSendToDoc() 
{
	SendTo(CCrossDlg::SEL_DOC, GetCurSel());
}

void CPlaylistDlg::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetFocus() == &m_List && m_List.GetSelectedCount());
}

void CPlaylistDlg::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetFocus() == &m_List && GetCount());
}

void CPlaylistDlg::OnSelchangeBank() 
{
	int	sel = m_BankCombo.GetCurSel();
	if (sel != LB_ERR)
		SetBank(sel);
}

void CPlaylistDlg::OnEditCut() 
{
	m_List.Cut();
}

void CPlaylistDlg::OnEditCopy() 
{
	m_List.Copy();
}

void CPlaylistDlg::OnEditPaste() 
{
	m_List.Paste();
	if (m_ThumbView)
		PopulateList();	// auto-arrange doesn't handle insert
}

void CPlaylistDlg::OnEditDelete() 
{
	m_List.Delete();
}

void CPlaylistDlg::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetFocus() == &m_List && m_List.CanPaste());
}

BOOL CPlaylistDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		if (m_List.IsDragging()) {	// if dragging
			if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
				m_List.CancelDrag();
			return(TRUE);	// override normal keyboard behavior
		}
		if (TranslateAccelerator(m_hWnd, m_ToolDlgAccel, pMsg))
			return(TRUE);
		if (pMsg->message == WM_KEYDOWN) {
			WORD	Key = (WORD)pMsg->wParam;
			// if list has focus and contains items, and key is alphanum
			if (GetFocus() == &m_List && GetCount()
			&& ((Key >= '0' && Key <= '9') || (Key >= 'A' && Key <= 'Z'))) {
				LV_KEYDOWN	kd;
				ZeroMemory(&kd, sizeof(kd));
				kd.wVKey = Key; 
				LRESULT	retc;
				OnKeydownList((NMHDR *)&kd, &retc);	// process hotkey
				return(TRUE);	// don't translate
			}
		}
	}
	return CToolDlg::PreTranslateMessage(pMsg);
}

void CPlaylistDlg::OnSysColorChange()
{
	m_StateImg.SetBkColor(GetSysColor(COLOR_WINDOW));
	CToolDlg::OnSysColorChange();
}

void CPlaylistDlg::OnViewType(UINT nID)
{
	bool	ThumbView = (nID - ID_PL_VIEW_TYPE_DETAILS) != 0;
	SetThumbView(ThumbView);
}

void CPlaylistDlg::OnUpdateViewType(CCmdUI* pCmdUI)
{
	bool	ThumbView = (pCmdUI->m_nID - ID_PL_VIEW_TYPE_DETAILS) != 0;
	pCmdUI->SetCheck(ThumbView == m_ThumbView);
}

void CPlaylistDlg::OnViewThumbSize(UINT nID)
{
	int	Idx = nID - ID_PL_VIEW_THUMB_SIZE0;
	ASSERT(Idx >= 0 && Idx < THUMB_SIZES);
	SetThumbSize(m_ThumbSizePreset[Idx]);
}

void CPlaylistDlg::OnUpdateViewThumbSize(CCmdUI* pCmdUI)
{
	int	Idx = pCmdUI->m_nID - ID_PL_VIEW_THUMB_SIZE0;
	ASSERT(Idx >= 0 && Idx < THUMB_SIZES);
	pCmdUI->SetCheck(CSize(m_ThumbSizePreset[Idx]) == m_ThumbSize);
}

void CPlaylistDlg::OnViewRefreshThumbs()
{
	m_Thumb.RemoveAll();
	MakeThumbs();
	PopulateList();
}

void CPlaylistDlg::OnUpdateViewRefreshThumbs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ThumbView);
}
