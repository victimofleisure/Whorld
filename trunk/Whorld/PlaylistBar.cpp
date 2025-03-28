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
        49		27mar25	refactor for V2

*/

#include "stdafx.h"
#include "Whorld.h"
#include "PlaylistBar.h"
#include "PathStr.h"
#include "AppRegKey.h"
#include "MainFrm.h"

// CPlaylistBar

IMPLEMENT_DYNAMIC(CPlaylistBar, CMyDockablePane)

const CListCtrlExSel::COL_INFO CPlaylistBar::m_arrColInfo[COLUMNS] = {
	{IDS_PLAYLIST_COL_PATCH_NAME, LVCFMT_LEFT, 150},
	{IDS_PLAYLIST_COL_PATCH_FOLDER, LVCFMT_LEFT, 250},
};

#define RK_COL_WIDTH _T("ColWidth")

#define mappingBar theApp.GetMainFrame()->m_wndMappingBar

const CIntArrayEx*	CPlaylistBar::m_parrSelection;

CPlaylistBar::CPlaylistBar()
{
}

CPlaylistBar::~CPlaylistBar()
{
}

inline CPlaylist* CPlaylistBar::GetPlaylist()
{
	ASSERT(theApp.m_pPlaylist != NULL);
	return theApp.m_pPlaylist;
}

void CPlaylistBar::UpdateList()
{
	int	nPatches = GetPlaylist()->m_arrPatch.GetSize();
	m_list.SetItemCountEx(nPatches, 0);
}

void CPlaylistBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	UNREFERENCED_PARAMETER(lHint);
	UNREFERENCED_PARAMETER(pHint);
	UpdateList();
}

void CPlaylistBar::Insert(int iInsert)
{
	// array of file type filter specifications, as IFileOpenDialog expects
	static const COMDLG_FILTERSPEC	aFilter[2] = {
		{L"Patch Files", L"*.whp"},	// Unicode only!
		{L"All Files", L"*.*"},
	};
	CStringArrayEx	aPatchPath;
	HRESULT hr = PromptForFiles(aPatchPath, _countof(aFilter), aFilter);
	if (SUCCEEDED(hr) && !aPatchPath.IsEmpty()) {	// if prompt succeeded
		CPlaylist::CPatchLinkArray	aPatchLink;
		int	nPatches = aPatchPath.GetSize();
		aPatchLink.SetSize(nPatches);
		for (int iPatch = 0; iPatch < nPatches; iPatch++) {
			aPatchLink[iPatch].m_sPath = aPatchPath[iPatch];
		}
		Insert(iInsert, aPatchLink);
	}
}

void CPlaylistBar::Insert(int iInsert, CPlaylist::CPatchLinkArray& aPatchLink)
{
	CPlaylist	*pPlaylist = GetPlaylist();
	pPlaylist->m_arrPatch.InsertAt(iInsert, &aPatchLink);
	pPlaylist->SetModifiedFlag();
	UpdateList();
	CIntArrayEx	arrSelection;
	MakeSelectionRange(arrSelection, iInsert, aPatchLink.GetSize());
	m_list.SetSelection(arrSelection);
	m_parrSelection = &arrSelection;
	mappingBar.NotifyUndoableEdit(0, CMapping::UCODE_PLAYLIST_INSERT);
	m_parrSelection = NULL;
}

void CPlaylistBar::Delete(const CIntArrayEx& arrSelection)
{
	m_parrSelection = &arrSelection;
	mappingBar.NotifyUndoableEdit(0, CMapping::UCODE_PLAYLIST_DELETE);
	m_parrSelection = NULL;
	CPlaylist	*pPlaylist = GetPlaylist();
	pPlaylist->m_arrPatch.DeleteSelection(arrSelection);
	pPlaylist->SetModifiedFlag();
	UpdateList();
	m_list.Deselect();
}

void CPlaylistBar::Move(const CIntArrayEx& arrSelection, int iDropPos)
{
	m_parrSelection = &arrSelection;
	mappingBar.NotifyUndoableEdit(iDropPos, CMapping::UCODE_PLAYLIST_MOVE);
	m_parrSelection = NULL;
	CPlaylist	*pPlaylist = GetPlaylist();
	pPlaylist->m_arrPatch.MoveSelection(arrSelection, iDropPos);
	pPlaylist->SetModifiedFlag();
	m_list.Invalidate();
	m_list.SelectRange(iDropPos, arrSelection.GetSize());
}

void CPlaylistBar::Play(int iPatch)
{
	CPatch	patch;
	const CPlaylist	*pPlaylist = GetPlaylist();
	if (patch.Read(pPlaylist->m_arrPatch[iPatch].m_sPath)) {
		theApp.m_thrRender.SetPatch(patch);
	}
}

// CPlaylistBar undo

void CPlaylistBar::SaveSelectedPatches(CUndoState& State) const
{
	if (mappingBar.UndoMgrIsIdle()) {	// if initial state
		ASSERT(m_parrSelection != NULL);
		CRefPtr<CUndoSelectedPatches>	pInfo;
		pInfo.CreateObj();
		pInfo->m_arrSelection = *m_parrSelection;
		CPlaylist	*pPlaylist = GetPlaylist();
		pPlaylist->m_arrPatch.GetSelection(*m_parrSelection, pInfo->m_arrPatch);
		State.SetObj(pInfo);
		switch (LOWORD(State.GetCode())) {
		case CMapping::UCODE_PLAYLIST_DELETE:
			State.m_Val.p.x.i = CUndoManager::UA_UNDO;	// undo inserts, redo deletes
			break;
		default:
			State.m_Val.p.x.i = CUndoManager::UA_REDO;	// undo deletes, redo inserts
		}
	}
}

void CPlaylistBar::RestoreSelectedPatches(const CUndoState& State)
{
	CUndoSelectedPatches	*pInfo = static_cast<CUndoSelectedPatches*>(State.GetObj());
	bool	bInserting = mappingBar.GetUndoAction() == State.m_Val.p.x.i; 
	CPlaylist	*pPlaylist = GetPlaylist();
	if (bInserting) {	// if inserting
		pPlaylist->m_arrPatch.InsertSelection(pInfo->m_arrSelection, pInfo->m_arrPatch);
	} else {	// deleting
		pPlaylist->m_arrPatch.DeleteSelection(pInfo->m_arrSelection);
	}
	UpdateList();
	if (bInserting) {
		m_list.SetSelection(pInfo->m_arrSelection);
	} else {
		m_list.Deselect();
	}
}

void CPlaylistBar::SavePatchMove(CUndoState& State) const
{
	const CIntArrayEx	*parrSelection;
	if (State.IsEmpty()) {	// if initial state
		ASSERT(m_parrSelection != NULL);
		parrSelection = m_parrSelection;	// get fresh selection
	} else {	// undoing or redoing; selection may have changed, so don't rely on it
		const CUndoSelection	*pInfo = static_cast<CUndoSelection*>(State.GetObj());
		parrSelection = &pInfo->m_arrSelection;	// use edit's original selection
	}
	CRefPtr<CUndoSelection>	pInfo;
	pInfo.CreateObj();
	pInfo->m_arrSelection = *parrSelection;
	State.SetObj(pInfo);
}

void CPlaylistBar::RestorePatchMove(const CUndoState& State)
{
	int	iDropPos = State.GetCtrlID();
	const CUndoSelection	*pInfo = static_cast<CUndoSelection*>(State.GetObj());
	int	nSels = pInfo->m_arrSelection.GetSize();
	CPlaylist::CPatchLinkArray	arrTempPatch;
	CPlaylist	*pPlaylist = GetPlaylist();
	CPlaylist::CPatchLinkArray&	arrPatch = pPlaylist->m_arrPatch;
	CIntArrayEx	arrSelection;
	if (mappingBar.IsUndoing()) {	// if undoing
		arrPatch.GetRange(iDropPos, nSels, arrTempPatch);
		arrPatch.RemoveAt(iDropPos, nSels);
		arrPatch.InsertSelection(pInfo->m_arrSelection, arrTempPatch);
		arrSelection = pInfo->m_arrSelection;
	} else {	// redoing
		arrPatch.GetSelection(pInfo->m_arrSelection, arrTempPatch);
		arrPatch.DeleteSelection(pInfo->m_arrSelection);
		arrPatch.InsertAt(iDropPos, &arrTempPatch);
		MakeSelectionRange(arrSelection, iDropPos, nSels);
	}
	UpdateList();
	m_list.SetSelection(arrSelection);
}

void CPlaylistBar::SaveUndoState(CUndoState& State)
{
	switch (LOWORD(State.GetCode())) {
	case CMapping::UCODE_PLAYLIST_INSERT:
	case CMapping::UCODE_PLAYLIST_DELETE:
		SaveSelectedPatches(State);
		break;
	case CMapping::UCODE_PLAYLIST_MOVE:
		SavePatchMove(State);
		break;
	default:
		NODEFAULTCASE;
	}
}

void CPlaylistBar::RestoreUndoState(const CUndoState& State)
{
	switch (LOWORD(State.GetCode())) {
	case CMapping::UCODE_PLAYLIST_INSERT:
	case CMapping::UCODE_PLAYLIST_DELETE:
		RestoreSelectedPatches(State);
		break;
	case CMapping::UCODE_PLAYLIST_MOVE:
		RestorePatchMove(State);
		break;
	default:
		NODEFAULTCASE;
	}
}

// CPlaylistBar message map

BEGIN_MESSAGE_MAP(CPlaylistBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_PATCH_LIST, OnListGetdispinfo)
	ON_COMMAND(ID_LIST_COL_HDR_RESET, OnListColHdrReset)
	ON_NOTIFY(ULVN_REORDER, IDC_PATCH_LIST, OnListReorder)
	ON_NOTIFY(NM_DBLCLK, IDC_PATCH_LIST, OnListDblclk)
	ON_COMMAND(ID_EDIT_INSERT, OnEditInsert)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_PLAYLIST_PLAY, OnPlay)
	ON_UPDATE_COMMAND_UI(ID_PLAYLIST_PLAY, OnUpdatePlay)
END_MESSAGE_MAP()

// CPlaylistBar message handlers

int CPlaylistBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD	dwStyle = WS_CHILD | WS_VISIBLE 
		| LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER;
	if (!m_list.Create(dwStyle, CRect(0, 0, 0, 0), this, IDC_PATCH_LIST))
		return -1;
	m_list.SendMessage(WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)));
	DWORD	dwListExStyle = LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT;
	m_list.SetExtendedStyle(dwListExStyle);
	m_list.CreateColumns(m_arrColInfo, COLUMNS);
	m_list.LoadColumnWidths(RK_PlaylistBar, RK_COL_WIDTH);

	return 0;
}

void CPlaylistBar::OnDestroy()
{
	m_list.SaveColumnWidths(RK_PlaylistBar, RK_COL_WIDTH);
	CMyDockablePane::OnDestroy();
}

void CPlaylistBar::OnSize(UINT nType, int cx, int cy)
{
	CMyDockablePane::OnSize(nType, cx, cy);
	m_list.MoveWindow(0, 0, cx, cy);
}

void CPlaylistBar::OnSetFocus(CWnd* pOldWnd)
{
	CMyDockablePane::OnSetFocus(pOldWnd);
	m_list.SetFocus();	// delegate focus to child control
}

void CPlaylistBar::OnListGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pResult);
	const NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	const LVITEM&	item = pDispInfo->item;
	int	iItem = item.iItem;
	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
		case COL_PATCH_NAME:
			{
				CPathStr	sName(PathFindFileName(GetPlaylist()->m_arrPatch[iItem].m_sPath));
				sName.RemoveExtension();
				_tcscpy_s(item.pszText, item.cchTextMax, sName);
			}
			break;
		case COL_PATCH_FOLDER:
			{
				CPathStr	sFolder(GetPlaylist()->m_arrPatch[iItem].m_sPath);
				sFolder.RemoveFileSpec();
				_tcscpy_s(item.pszText, item.cchTextMax, sFolder);
			}
			break;
		}
	}
}

void CPlaylistBar::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (FixListContextMenuPoint(pWnd, m_list, point))
		return;
	DoGenericContextMenu(IDR_PLAYLIST_CTX, point, this);
}

void CPlaylistBar::OnListColHdrReset()
{
	m_list.ResetColumnHeader(m_arrColInfo, COLUMNS);
}

void CPlaylistBar::OnListReorder(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);	// NMLISTVIEW
	UNREFERENCED_PARAMETER(pResult);
	CIntArrayEx	arrSelection;
	m_list.GetSelection(arrSelection);
	if (arrSelection.GetSize()) {	// if selection exists
		int	iDropPos = m_list.GetCompensatedDropPos();
		if (iDropPos >= 0) {	// if items are actually moving
			Move(arrSelection, iDropPos);
		}
	}
}

void CPlaylistBar::OnListDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pitem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int	iItem = pitem->iItem;
	Play(iItem);
	*pResult = 0;
}

void CPlaylistBar::OnEditInsert()
{
	int	iInsert = m_list.GetInsertPos();
	Insert(iInsert);
}

void CPlaylistBar::OnEditDelete()
{
	CIntArrayEx	arrSelection;
	m_list.GetSelection(arrSelection);
	Delete(arrSelection);
}

void CPlaylistBar::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_list.GetSelectedCount());
}

void CPlaylistBar::OnEditUndo()
{
	mappingBar.OnCmdMsg(ID_EDIT_UNDO, CN_COMMAND, NULL, NULL);
}

void CPlaylistBar::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	mappingBar.OnCmdMsg(ID_EDIT_UNDO, CN_UPDATE_COMMAND_UI, pCmdUI, NULL);
}

void CPlaylistBar::OnEditRedo()
{
	mappingBar.OnCmdMsg(ID_EDIT_REDO, CN_COMMAND, NULL, NULL);
}

void CPlaylistBar::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	mappingBar.OnCmdMsg(ID_EDIT_REDO, CN_UPDATE_COMMAND_UI, pCmdUI, NULL);
}

void CPlaylistBar::OnUpdateEditSelectAll(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_list.GetSelectedCount() > 0);
}

void CPlaylistBar::OnPlay()
{
	int iItem = m_list.GetSelection();
	if (iItem >= 0) {
		Play(iItem);
	}
}

void CPlaylistBar::OnUpdatePlay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_list.GetSelectedCount() > 0);
}
