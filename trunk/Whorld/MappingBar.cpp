// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		20mar20	initial version
		01		29mar20	add learn multiple mappings
		02		01apr20	standardize context menu handling
		03		05apr20	add track step mapping
		04		07sep20	add preset and part mapping
		05		19nov20	add sender argument to set mapping property
		06		15feb21	add mapping targets for transport commands
		07		20jun21	remove dispatch edit keys
		08		25oct21	add descending sort via Shift key
		09		26feb25	adapt for Whorld
		10		27feb25	add undo
		
*/

#include "stdafx.h"
#include "Whorld.h"
#include "MappingBar.h"
#include "MainFrm.h"
#include "WhorldDoc.h"
#include "UndoCodes.h"
#include "PopupCombo.h"
#include "PopupNumEdit.h"
#include "Midi.h"
#include "AppRegKey.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMappingBar

IMPLEMENT_DYNAMIC(CMappingBar, CMyDockablePane)

const CGridCtrl::COL_INFO CMappingBar::m_arrColInfo[COLUMNS] = {
	#define MAPPINGDEF_INCLUDE_NUMBER
	#define MAPPINGDEF(name, align, width, member, minval, maxval) {IDS_MAPPING_COL_##name, align, width},
	#include "MappingDef.h"	// generate list column info
};

const CMappingBar::COL_RANGE CMappingBar::m_arrColRange[COLUMNS] = {
	#define MAPPINGDEF_INCLUDE_NUMBER
	#define MAPPINGDEF(name, align, width, member, minval, maxval) {minval, maxval},
	#include "MappingDef.h"	// generate list column info
};

#define IDS_EDIT_PROPERTY 0
#define IDS_EDIT_MULTI_PROPERTY 0

const int CMappingBar::m_arrUndoTitleId[MAPPING_UNDO_CODES] = {
	#define MAPPINGUNDODEF(name) IDS_EDIT_##name,
	#include "MappingDef.h"	
};

const CIntArrayEx*	CMappingBar::m_parrSelection;

#define midiMaps theApp.m_midiMgr.m_midiMaps

#define RK_COL_ORDER _T("ColOrder")
#define RK_COL_WIDTH _T("ColWidth")

CMappingBar::CMappingBar()
{
	m_UndoMgr.SetRoot(this);
	SetUndoManager(&m_UndoMgr);
}

CMappingBar::~CMappingBar()
{
}

void CMappingBar::AddMidiChannelComboItems(CComboBox& wndCombo)
{
	CString	sChan;
	for (int iChan = 0; iChan < MIDI_CHANNELS; iChan++) {
		sChan.Format(_T("%d"), iChan + 1);	// make one-based
		wndCombo.AddString(sChan);
	}
}

void CMappingBar::OnShowChanged(bool bShow)
{
	// we only receive document updates if we're visible; see CMainFrame::OnUpdate
	if (bShow)	// if showing bar
		OnUpdate(NULL, CWhorldDoc::HINT_NONE);	// repopulate grid
}

void CMappingBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	UNREFERENCED_PARAMETER(pHint);
//	printf("CMappingBar::OnUpdate %x %d %x\n", pSender, lHint, pHint);
	switch (lHint) {
	case CWhorldDoc::HINT_NONE:
		UpdateGrid();
		m_grid.Deselect();	// remove selection
		break;
	}
}

void CMappingBar::UpdateGrid()
{
	int	nMappings;
	nMappings = midiMaps.GetCount();
	m_grid.SetItemCountEx(nMappings, 0);
}

void CMappingBar::UpdateGrid(int iMapping, int iProp)
{
	ASSERT(iProp >= 0 && iProp < CMapping::PROPERTIES);
	m_grid.RedrawSubItem(iMapping, iProp + 1);	// compensate for number column
}

void CMappingBar::UpdateGrid(const CIntArrayEx& arrSelection, int iProp)
{
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {
		int	iItem = arrSelection[iSel];
		m_grid.RedrawSubItem(iItem, iProp + 1);	// compensate for number column
	}
	m_grid.SetSelection(arrSelection);	// also restore selection
}

void CMappingBar::SetModifiedFlag()
{
	theApp.m_pPlaylist->SetModifiedFlag();
}

void CMappingBar::OnMidiLearn()
{
	m_grid.Invalidate();	// redraw all grid items
/*@@@	if (!theApp.m_bIsMidiLearn)	// if learn disabled
		m_arrPrevSelection.RemoveAll();
		*/
}

CWnd *CMappingBar::CModGridCtrl::CreateEditCtrl(LPCTSTR pszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	UNREFERENCED_PARAMETER(pszText);
	UNREFERENCED_PARAMETER(dwStyle);
	UNREFERENCED_PARAMETER(pParentWnd);
	UNREFERENCED_PARAMETER(nID);
	int	nVal = midiMaps.GetProperty(m_iEditRow, m_iEditCol - 1);	// skip number column
	switch (m_iEditCol) {
	case COL_IN_EVENT:
	case COL_IN_CHANNEL:
	case COL_OUT_EVENT:
		{
			CPopupCombo	*pCombo = CPopupCombo::Factory(0, rect, this, 0, 100);
			if (pCombo == NULL)
				return NULL;
			switch (m_iEditCol) {
			case COL_IN_CHANNEL:
				AddMidiChannelComboItems(*pCombo);
				break;
			case COL_IN_EVENT:
				{
					// start from one to exclude note off
					for (int iEvent = 1; iEvent < CMapping::INPUT_EVENTS; iEvent++) {
						pCombo->AddString(CMapping::GetInputEventName(iEvent));
					}
					nVal--;	// compensate for excluding note off
				}
				break;
			case COL_OUT_EVENT:
				{
					for (int iEvent = 0; iEvent < CMapping::OUTPUT_EVENTS; iEvent++) {
						pCombo->AddString(CMapping::GetOutputEventName(iEvent));
					}
				}
				break;
			default:
				NODEFAULTCASE;
			}
			pCombo->SetCurSel(nVal);
			pCombo->ShowDropDown();
			return pCombo;
		}
		break;
	default:
		CPopupNumEdit	*pEdit = new CPopupNumEdit;
		pEdit->SetFormat(CNumEdit::DF_INT | CNumEdit::DF_SPIN);
		if (!pEdit->Create(dwStyle, rect, this, nID)) {	// create edit control
			delete pEdit;
			return NULL;
		}
		pEdit->SetWindowText(pszText);
		pEdit->SetSel(0, -1);	// select entire text
		if (m_arrColRange[m_iEditCol].nMin != m_arrColRange[m_iEditCol].nMax)	// if range specified
			pEdit->SetRange(m_arrColRange[m_iEditCol].nMin, m_arrColRange[m_iEditCol].nMax);
		return pEdit;
	}
}

void CMappingBar::CModGridCtrl::OnItemChange(LPCTSTR pszText)
{
	UNREFERENCED_PARAMETER(pszText);
	int	nVal;
	switch (m_iEditCol) {
	case COL_IN_EVENT:
	case COL_IN_CHANNEL:
	case COL_OUT_EVENT:
		{
			CPopupCombo	*pCombo = STATIC_DOWNCAST(CPopupCombo, m_pEditCtrl);
			int	iSelItem = pCombo->GetCurSel();	// index of changed item
			if (iSelItem < 0)
				return;
			switch (m_iEditCol) {
			case COL_IN_EVENT:
				iSelItem++;	// compensate for excluding note off
				break;
			}
			nVal = iSelItem;
		}
		break;
	default:
		CPopupNumEdit	*pEdit = STATIC_DOWNCAST(CPopupNumEdit, m_pEditCtrl);
		nVal = pEdit->GetIntVal();
	}
	int	iMapping = m_iEditRow;
	int	iProp = m_iEditCol - 1;	// skip number column
	int	nPrevVal = midiMaps.GetProperty(iMapping, iProp);
	if (nVal != nPrevVal) {	// if value actually changed
		CMappingBar*	pParent = STATIC_DOWNCAST(CMappingBar, GetParent());
		CIntArrayEx	arrSelection;
		GetSelection(arrSelection);
		// if multiple mappings selected and edit is within selection
		if (arrSelection.GetSize() > 1 && arrSelection.Find(m_iEditRow) >= 0) {
			m_parrSelection = &arrSelection;
			pParent->NotifyUndoableEdit(iProp, UCODE_MULTI_PROPERTY);
			m_parrSelection = NULL;
			midiMaps.SetProperty(arrSelection, iProp, nVal);
		} else {	// edit single mapping
			pParent->NotifyUndoableEdit(iMapping, MAKELONG(UCODE_PROPERTY, iProp));
			midiMaps.SetProperty(iMapping, iProp, nVal);
		}
		pParent->SetModifiedFlag();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMappingBar undo

void CMappingBar::MakeSelectionRange(CIntArrayEx& arrSelection, int iFirstItem, int nItems)
{
	arrSelection.SetSize(nItems);
	for (int iSel = 0; iSel < nItems; iSel++)	// for each item
		arrSelection[iSel] = iFirstItem + iSel;
}

void CMappingBar::SaveProperty(CUndoState& State) const
{
	int	iMapping = State.GetCtrlID();
	int	iProp = HIWORD(State.GetCode());
	State.m_Val.p.x.i = midiMaps.GetProperty(iMapping, iProp);
}

void CMappingBar::RestoreProperty(const CUndoState& State)
{
	int	iMapping = State.GetCtrlID();
	int	iProp = HIWORD(State.GetCode());
	midiMaps.SetProperty(iMapping, iProp, State.m_Val.p.x.i);
	UpdateGrid(iMapping, iProp);
}

void CMappingBar::SaveMultiProperty(CUndoState& State) const
{
	int	iProp = State.GetCtrlID();
	const CIntArrayEx	*parrSelection;
	if (State.IsEmpty()) {	// if initial state
		ASSERT(m_parrSelection != NULL);
		parrSelection = m_parrSelection;	// get fresh selection
	} else {	// undoing or redoing; selection may have changed, so don't rely on it
		const CUndoMultiIntegerProp	*pInfo = static_cast<CUndoMultiIntegerProp*>(State.GetObj());
		parrSelection = &pInfo->m_arrSelection;	// use edit's original selection
	}
	CRefPtr<CUndoMultiIntegerProp>	pInfo;
	pInfo.CreateObj();
	pInfo->m_arrSelection = *parrSelection;
	pInfo->m_arrProp.SetSize(parrSelection->GetSize());
	midiMaps.GetProperty(*parrSelection, iProp, pInfo->m_arrProp);
	State.SetObj(pInfo);
}

void CMappingBar::RestoreMultiProperty(const CUndoState& State)
{
	int	iProp = State.GetCtrlID();
	const CUndoMultiIntegerProp	*pInfo = static_cast<CUndoMultiIntegerProp*>(State.GetObj());
	midiMaps.SetProperty(pInfo->m_arrSelection, iProp, pInfo->m_arrProp);
	UpdateGrid(pInfo->m_arrSelection, iProp);
}

void CMappingBar::SaveSelectedMappings(CUndoState& State) const
{
	if (UndoMgrIsIdle()) {	// if initial state
		ASSERT(m_parrSelection != NULL);
		CRefPtr<CUndoSelectedMappings>	pInfo;
		pInfo.CreateObj();
		pInfo->m_arrSelection = *m_parrSelection;
		midiMaps.GetSelection(*m_parrSelection, pInfo->m_arrMapping);
		State.SetObj(pInfo);
		switch (LOWORD(State.GetCode())) {
		case UCODE_CUT:
		case UCODE_DELETE:
			State.m_Val.p.x.i = CUndoManager::UA_UNDO;	// undo inserts, redo deletes
			break;
		default:
			State.m_Val.p.x.i = CUndoManager::UA_REDO;	// undo deletes, redo inserts
		}
	}
}

void CMappingBar::RestoreSelectedMappings(const CUndoState& State)
{
	CUndoSelectedMappings	*pInfo = static_cast<CUndoSelectedMappings*>(State.GetObj());
	bool	bInserting = GetUndoAction() == State.m_Val.p.x.i; 
	if (bInserting) {	// if inserting
		midiMaps.Insert(pInfo->m_arrSelection, pInfo->m_arrMapping);
	} else {	// deleting
		midiMaps.Delete(pInfo->m_arrSelection);
	}
	UpdateGrid();
	if (bInserting) {
		m_grid.SetSelection(pInfo->m_arrSelection);
	} else {
		m_grid.Deselect();
	}
}

void CMappingBar::SaveMappings(CUndoState& State) const
{
	CRefPtr<CUndoSelectedMappings>	pInfo;
	pInfo.CreateObj();
	pInfo->m_arrMapping = theApp.m_midiMgr.m_midiMaps.GetArray();
	m_grid.GetSelection(pInfo->m_arrSelection);
	State.SetObj(pInfo);
}

void CMappingBar::RestoreMappings(const CUndoState& State)
{
	const CUndoSelectedMappings* pInfo = static_cast<CUndoSelectedMappings*>(State.GetObj());
	theApp.m_midiMgr.m_midiMaps.SetArray(pInfo->m_arrMapping);
	UpdateGrid();
	m_grid.SetSelection(pInfo->m_arrSelection);
}

void CMappingBar::SaveUndoState(CUndoState& State)
{
	switch (LOWORD(State.GetCode())) {
	case UCODE_PROPERTY:
		SaveProperty(State);
		break;
	case UCODE_MULTI_PROPERTY:
		SaveMultiProperty(State);
		break;
	case UCODE_CUT:
	case UCODE_PASTE:
	case UCODE_INSERT:
	case UCODE_DELETE:
		SaveSelectedMappings(State);
		break;
	case UCODE_MOVE:
	case UCODE_SORT:
		SaveMappings(State);
		break;
	default:
		ASSERT(0);	// missing case
	}
}

void CMappingBar::RestoreUndoState(const CUndoState& State)
{
	switch (LOWORD(State.GetCode())) {
	case UCODE_PROPERTY:
		RestoreProperty(State);
		break;
	case UCODE_MULTI_PROPERTY:
		RestoreMultiProperty(State);
		break;
	case UCODE_CUT:
	case UCODE_PASTE:
	case UCODE_INSERT:
	case UCODE_DELETE:
		RestoreSelectedMappings(State);
		break;
	case UCODE_MOVE:
	case UCODE_SORT:
		RestoreMappings(State);
		break;
	default:
		ASSERT(0);	// missing case
	}
}

CString	CMappingBar::GetUndoTitle(const CUndoState& State)
{
	CString	sTitle;
	int	nUndoCode = LOWORD(State.GetCode());
	switch (nUndoCode) {
	case UCODE_PROPERTY:
	case UCODE_MULTI_PROPERTY:
		{
			int	iProp = State.GetCtrlID() + 1;	// account for number column
			sTitle.LoadString(m_arrColInfo[iProp].nTitleID);
		}
		break;
	default:
		sTitle.LoadString(m_arrUndoTitleId[nUndoCode]);
	}
	return sTitle;
}

/////////////////////////////////////////////////////////////////////////////
// CMappingBar message map

BEGIN_MESSAGE_MAP(CMappingBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_MAPPING_GRID, OnListGetdispinfo)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_LIST_COL_HDR_RESET, OnListColHdrReset)
	ON_NOTIFY(ULVN_REORDER, IDC_MAPPING_GRID, OnListReorder)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_MAPPING_GRID, OnListColumnClick)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_COMMAND(ID_EDIT_INSERT, OnEditInsert)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT, OnUpdateEditInsert)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_COMMAND(ID_TOOLS_MIDI_LEARN, OnToolsMidiLearn)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_MIDI_LEARN, OnUpdateToolsMidiLearn)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMappingBar message handlers

int CMappingBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD	dwStyle = WS_CHILD | WS_VISIBLE 
		| LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS;
	// NOTE that LVS_SHOWSELALWAYS breaks CDIS_SELECTED handling in OnListCustomDraw
	if (!m_grid.Create(dwStyle, CRect(0, 0, 0, 0), this, IDC_MAPPING_GRID))
		return -1;
	DWORD	dwListExStyle = LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP;
	m_grid.SetExtendedStyle(dwListExStyle);
	m_grid.CreateColumns(m_arrColInfo, COLUMNS);
	m_grid.SendMessage(WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)));
	m_grid.LoadColumnOrder(RK_MappingBar, RK_COL_ORDER);
	m_grid.LoadColumnWidths(RK_MappingBar, RK_COL_WIDTH);
	return 0;
}

void CMappingBar::OnDestroy()
{
	m_grid.SaveColumnOrder(RK_MappingBar, RK_COL_ORDER);
	m_grid.SaveColumnWidths(RK_MappingBar, RK_COL_WIDTH);
	CMyDockablePane::OnDestroy();
}

void CMappingBar::OnSize(UINT nType, int cx, int cy)
{
	CMyDockablePane::OnSize(nType, cx, cy);
	m_grid.MoveWindow(0, 0, cx, cy);
}

void CMappingBar::OnSetFocus(CWnd* pOldWnd)
{
	CMyDockablePane::OnSetFocus(pOldWnd);
	m_grid.SetFocus();	// delegate focus to child control
}

void CMappingBar::OnListGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pResult);
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LVITEM&	item = pDispInfo->item;
	int	iItem = item.iItem;
	if (item.mask & LVIF_TEXT) {
		const CMapping&	map = midiMaps.GetAt(iItem);
		switch (item.iSubItem) {
		case COL_NUMBER:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), iItem + 1); // make one-based
			break;
		case COL_IN_EVENT:
			_tcscpy_s(item.pszText, item.cchTextMax, CMapping::GetInputEventName(map.m_nInEvent)); 
			break;
		case COL_IN_CHANNEL:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), map.m_nInChannel + 1);	// make one-based
			break;
		case COL_IN_CONTROL:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), map.m_nInControl); 
			break;
		case COL_OUT_EVENT:
			_tcscpy_s(item.pszText, item.cchTextMax, CMapping::GetOutputEventName(map.m_nOutEvent)); 
			break;
		case COL_RANGE_START:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), map.m_nRangeStart); 
			break;
		case COL_RANGE_END:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), map.m_nRangeEnd); 
			break;
		}
	}
}

void CMappingBar::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (FixListContextMenuPoint(pWnd, m_grid, point))
		return;
	DoGenericContextMenu(IDR_MAPPING_CTX, point, this);
}

void CMappingBar::OnListColHdrReset()
{
	m_grid.ResetColumnHeader(m_arrColInfo, COLUMNS);
}

void CMappingBar::OnListReorder(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);	// NMLISTVIEW
	UNREFERENCED_PARAMETER(pResult);
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	if (arrSelection.GetSize()) {	// if selection exists
		int	iDropPos = m_grid.GetCompensatedDropPos();
		if (iDropPos >= 0) {	// if items are actually moving
			NotifyUndoableEdit(iDropPos, UCODE_MOVE);
			midiMaps.Move(arrSelection, iDropPos);
			SetModifiedFlag();
			UpdateGrid();
			m_grid.SelectRange(iDropPos, arrSelection.GetSize());
		}
	}
}

void CMappingBar::OnListColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pListView = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	int	iProp = pListView->iSubItem - 1;	// skip number column
	if (iProp >= 0) {
		bool	bDescending = (GetKeyState(VK_SHIFT) & GKS_DOWN) != 0;
		NotifyUndoableEdit(0, UCODE_SORT);
		midiMaps.Sort(iProp, bDescending);
		UpdateGrid();
	}
	pResult = 0;
}

void CMappingBar::OnEditUndo()
{
	m_UndoMgr.Undo();
	SetModifiedFlag();	// undo counts as modification
}

void CMappingBar::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(m_UndoMgr.m_sUndoMenuItem);
	pCmdUI->Enable(m_UndoMgr.CanUndo());
}

void CMappingBar::OnEditRedo()
{
	m_UndoMgr.Redo();
	SetModifiedFlag();	// redo counts as modification
}

void CMappingBar::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(m_UndoMgr.m_sRedoMenuItem);
	pCmdUI->Enable(m_UndoMgr.CanRedo());
}

void CMappingBar::OnEditCopy()
{
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	midiMaps.GetSelection(arrSelection, m_clipboard);
}

void CMappingBar::OnEditCut()
{
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_CUT);
	m_parrSelection = NULL;
	midiMaps.GetSelection(arrSelection, m_clipboard);
	midiMaps.Delete(arrSelection);
	SetModifiedFlag();
	UpdateGrid();
	m_grid.Deselect();
}

void CMappingBar::OnEditPaste()
{
	int	iInsert = m_grid.GetInsertPos();
	midiMaps.Insert(iInsert, m_clipboard);
	SetModifiedFlag();
	UpdateGrid();
	CIntArrayEx	arrSelection;
	MakeSelectionRange(arrSelection, iInsert, m_clipboard.GetSize());
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_PASTE);
	m_parrSelection = NULL;
}

void CMappingBar::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_clipboard.GetSize() > 0);
}

void CMappingBar::OnEditInsert()
{
	CMappingArray	arrMapping;
	arrMapping.SetSize(1);
	arrMapping[0].SetDefaults();
	int	iInsert = m_grid.GetInsertPos();
	midiMaps.Insert(iInsert, arrMapping);
	SetModifiedFlag();
	UpdateGrid();
	CIntArrayEx	arrSelection;
	MakeSelectionRange(arrSelection, iInsert, 1);
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_INSERT);
	m_parrSelection = NULL;
}

void CMappingBar::OnUpdateEditInsert(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(true);
}

void CMappingBar::OnEditDelete()
{
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_DELETE);
	m_parrSelection = NULL;
	midiMaps.Delete(arrSelection);
	SetModifiedFlag();
	UpdateGrid();
	m_grid.Deselect();
}

void CMappingBar::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_grid.GetSelectedCount());
}

void CMappingBar::OnEditSelectAll()
{
	m_grid.SelectAll();
}

void CMappingBar::OnUpdateEditSelectAll(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(midiMaps.GetCount());
}

void CMappingBar::OnToolsMidiLearn()
{
//@@@	theApp.m_bIsMidiLearn ^= 1;
	OnMidiLearn();
}

void CMappingBar::OnUpdateToolsMidiLearn(CCmdUI *pCmdUI)
{
//@@@	pCmdUI->SetCheck(theApp.m_bIsMidiLearn);
	pCmdUI->Enable(false);	// disable for now
}
