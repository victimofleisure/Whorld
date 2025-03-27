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
		11		01mar25	add learn mode
		12		05mar25	indicate learn mode by changing list selection color
		13		19mar25	make mapping range real instead of integer
		14		27mar25	move selection range maker to globals
		
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

// CMappingBar

IMPLEMENT_DYNAMIC(CMappingBar, CMyDockablePane)

const CGridCtrl::COL_INFO CMappingBar::m_arrColInfo[COLUMNS] = {
	#define MAPPINGDEF_INCLUDE_NUMBER
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		{IDS_MAPPING_COL_##name, LVCFMT_##align, width},
	#include "MappingDef.h"	// generate list column info
};

const CMappingBar::COL_RANGE CMappingBar::m_arrColRange[COLUMNS] = {
	#define MAPPINGDEF_INCLUDE_NUMBER
	#define MAPPINGDEF(name, align, width, type, prefix, member, initval, minval, maxval) \
		{minval, maxval},
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

void CMappingBar::OnFrameGetMinMaxInfo(HWND hFrameWnd, MINMAXINFO *pMMI)
{
	CMainFrame::OnFrameGetMinMaxInfo(this, hFrameWnd, pMMI);	// delegate to common handler
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
	int	nMappings = midiMaps.GetCount();
	m_grid.SetItemCountEx(nMappings, 0);
}

void CMappingBar::UpdateGrid(int iMapping)
{
	m_grid.RedrawItem(iMapping);
}

void CMappingBar::UpdateGrid(int iMapping, int iProp)
{
	ASSERT(iProp >= 0 && iProp < PROPERTIES);
	m_grid.RedrawSubItem(iMapping, iProp + 1);	// compensate for number column
}

void CMappingBar::UpdateGrid(const CIntArrayEx& arrSelection, int iProp)
{
	int	nSels = arrSelection.GetSize();
	for (int iSel = 0; iSel < nSels; iSel++) {
		int	iItem = arrSelection[iSel];
		if (iProp >= 0) {	// if property specified
			m_grid.RedrawSubItem(iItem, iProp + 1);	// compensate for number column
		} else {	// all properties
			m_grid.RedrawItem(iItem);
		}
	}
	m_grid.SetSelection(arrSelection);	// also restore selection
}

void CMappingBar::SetModifiedFlag(bool bModified)
{
	theApp.m_pPlaylist->SetModifiedFlag(bModified);
}

void CMappingBar::SetProperty(int iMapping, int iProp, VARIANT_PROP prop)
{
	NotifyUndoableEdit(iMapping, MAKELONG(UCODE_PROPERTY, iProp));
	midiMaps.SetProperty(iMapping, iProp, prop);
	SetModifiedFlag();
}

void CMappingBar::SetProperty(const CIntArrayEx& arrSelection, int iProp, VARIANT_PROP prop)
{
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(iProp, UCODE_MULTI_PROPERTY);
	m_parrSelection = NULL;
	midiMaps.SetProperty(arrSelection, iProp, prop);
	SetModifiedFlag();
}

void CMappingBar::Copy(const CIntArrayEx& arrSelection)
{
	midiMaps.GetSelection(arrSelection, m_clipboard);
}

void CMappingBar::Cut(const CIntArrayEx& arrSelection)
{
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_CUT);
	m_parrSelection = NULL;
	midiMaps.GetSelection(arrSelection, m_clipboard);
	midiMaps.Delete(arrSelection);
	SetModifiedFlag();
	UpdateGrid();
	m_grid.Deselect();
}

void CMappingBar::Paste(int iInsert)
{
	midiMaps.Insert(iInsert, m_clipboard);
	SetModifiedFlag();
	UpdateGrid();
	CIntArrayEx	arrSelection;
	MakeSelectionRange(arrSelection, iInsert, m_clipboard.GetSize());
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_PASTE);
	m_parrSelection = NULL;
}

void CMappingBar::Insert(int iInsert)
{
	CMappingArray	arrMapping;
	arrMapping.SetSize(1);
	arrMapping[0].SetDefaults();
	Insert(iInsert, arrMapping);
}

void CMappingBar::Insert(int iInsert, CMappingArray& arrMapping)
{
	midiMaps.Insert(iInsert, arrMapping);
	SetModifiedFlag();
	UpdateGrid();
	CIntArrayEx	arrSelection;
	MakeSelectionRange(arrSelection, iInsert, 1);
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_INSERT);
	m_parrSelection = NULL;
}

void CMappingBar::Delete(const CIntArrayEx& arrSelection)
{
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_DELETE);
	m_parrSelection = NULL;
	midiMaps.Delete(arrSelection);
	SetModifiedFlag();
	UpdateGrid();
	m_grid.Deselect();
}

void CMappingBar::Move(const CIntArrayEx& arrSelection, int iDropPos)
{
	NotifyUndoableEdit(iDropPos, UCODE_MOVE);
	midiMaps.Move(arrSelection, iDropPos);
	SetModifiedFlag();
	UpdateGrid();
	m_grid.SelectRange(iDropPos, arrSelection.GetSize());
}

void CMappingBar::Sort(int iProp, bool bDescending)
{
	NotifyUndoableEdit(0, UCODE_SORT);
	midiMaps.Sort(iProp, bDescending);
	UpdateGrid();
}

CWnd *CMappingBar::CModGridCtrl::CreateEditCtrl(LPCTSTR pszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	UNREFERENCED_PARAMETER(pParentWnd);
	int	iEditProp = m_iEditCol - 1;	// skip number column
	VARIANT_PROP prop = midiMaps.GetProperty(m_iEditRow, iEditProp);
	switch (m_iEditCol) {
	case COL_EVENT:
	case COL_CHANNEL:
	case COL_TARGET:
	case COL_PROPERTY:
		{
			if (m_iEditCol == COL_PROPERTY) {	// if property column
				// target doesn't have a property, so property combo is inapplicable
				if (!TargetHasProperty(midiMaps.GetProperty(m_iEditRow, PROP_TARGET).intVal)) {
					return NULL;	// tell caller to skip this column
				}
			}
			CPopupCombo	*pCombo = CPopupCombo::Factory(0, rect, this, 0, 100);
			if (pCombo == NULL)
				return NULL;
			switch (m_iEditCol) {
			case COL_CHANNEL:
				AddMidiChannelComboItems(*pCombo);
				break;
			case COL_EVENT:
				{
					// start from one to exclude note off
					for (int iEvent = 1; iEvent < EVENTS; iEvent++) {
						pCombo->AddString(GetEventName(iEvent));
					}
					prop.intVal--;	// compensate for excluding note off
				}
				break;
			case COL_TARGET:
				{
					for (int iEvent = 0; iEvent < TARGETS; iEvent++) {
						pCombo->AddString(GetTargetName(iEvent));
					}
				}
				break;
			case COL_PROPERTY:
				{
					for (int iProp = 0; iProp < PARAM_PROP_COUNT; iProp++) {
						pCombo->AddString(GetParamPropName(iProp));
					}
				}
				break;
			default:
				NODEFAULTCASE;
			}
			pCombo->SetCurSel(prop.intVal);
			pCombo->ShowDropDown();
			return pCombo;
		}
		break;
	default:
		CPopupNumEdit	*pEdit = new CPopupNumEdit;
		int	nFormat = CNumEdit::DF_SPIN;
		if (CMapping::IsIntegerProperty(iEditProp)) {	// if integer property
			nFormat |= CNumEdit::DF_INT;	// restrict input to integers
		}
		pEdit->SetFormat(nFormat);
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
	VARIANT_PROP prop = {0};	// clear entire property
	int	iMapping = m_iEditRow;
	int	iEditProp = m_iEditCol - 1;	// skip number column
	VARIANT_PROP propPrev = midiMaps.GetProperty(iMapping, iEditProp);
	switch (m_iEditCol) {
	case COL_EVENT:
	case COL_CHANNEL:
	case COL_TARGET:
	case COL_PROPERTY:
		{
			CPopupCombo	*pCombo = STATIC_DOWNCAST(CPopupCombo, m_pEditCtrl);
			int	iSelItem = pCombo->GetCurSel();	// index of changed item
			if (iSelItem < 0)
				return;
			switch (m_iEditCol) {
			case COL_EVENT:
				iSelItem++;	// compensate for excluding note off
				break;
			case COL_TARGET:
				// if target had a property and now doesn't, or vice versa
				if (TargetHasProperty(iSelItem) != TargetHasProperty(propPrev.intVal)) {
					RedrawSubItem(m_iEditRow, COL_PROPERTY);	// update property
				}
				break;
			}
			prop.intVal = iSelItem;
		}
		break;
	default:
		CPopupNumEdit	*pEdit = STATIC_DOWNCAST(CPopupNumEdit, m_pEditCtrl);
		if (CMapping::IsIntegerProperty(iEditProp)) {	// if integer property
			prop.intVal = pEdit->GetIntVal();	// retrieve integer
		} else {	// not integer property
			prop.dblVal = pEdit->GetVal();	// retrieve double
		}
	}
	// if property value actually changed
	if (CMapping::Compare(iEditProp, prop, propPrev)) {
		CMappingBar*	pParent = STATIC_DOWNCAST(CMappingBar, GetParent());
		CIntArrayEx	arrSelection;
		GetSelection(arrSelection);
		// if multiple mappings selected and edit is within selection
		if (arrSelection.GetSize() > 1 && arrSelection.Find(m_iEditRow) >= 0) {
			pParent->SetProperty(arrSelection, iEditProp, prop);
		} else {	// edit single mapping
			pParent->SetProperty(iMapping, iEditProp, prop);
		}
	}
}

void CMappingBar::OnLearnMode()
{
	DWORD	dwRemove = 0, dwAdd = 0;
	// for more on style change, see OnCreate and OnListCustomDraw
	if (theApp.m_midiMgr.IsLearnMode()) {	// if learning MIDI input
		dwRemove = LVS_SHOWSELALWAYS;	// disable show selection always
	} else {	// not learning
		dwAdd = LVS_SHOWSELALWAYS;	// enable show selection always
		m_arrPrevSelection.RemoveAll();	// reset change detection
	}
	m_grid.ModifyStyle(dwRemove, dwAdd);	// apply style chosen above
	UpdateGrid();	// redraw all items
}

// CMappingBar undo

void CMappingBar::SaveProperty(CUndoState& State) const
{
	int	iMapping = State.GetCtrlID();
	int	iProp = HIWORD(State.GetCode());
	State.m_Val.i64 = midiMaps.GetProperty(iMapping, iProp).llVal;
}

void CMappingBar::RestoreProperty(const CUndoState& State)
{
	int	iMapping = State.GetCtrlID();
	int	iProp = HIWORD(State.GetCode());
	VARIANT_PROP	prop = {State.m_Val.i64};
	midiMaps.SetProperty(iMapping, iProp, prop);
	UpdateGrid(iMapping, iProp);
	if (iProp == PROP_TARGET) {	// if restoring target
		UpdateGrid(iMapping, PROP_PROPERTY);	// target can affect property
	}
}

void CMappingBar::SaveMultiProperty(CUndoState& State) const
{
	int	iProp = State.GetCtrlID();
	const CIntArrayEx	*parrSelection;
	if (State.IsEmpty()) {	// if initial state
		ASSERT(m_parrSelection != NULL);
		parrSelection = m_parrSelection;	// get fresh selection
	} else {	// undoing or redoing; selection may have changed, so don't rely on it
		const CUndoMultiVariantProp	*pInfo = static_cast<CUndoMultiVariantProp*>(State.GetObj());
		parrSelection = &pInfo->m_arrSelection;	// use edit's original selection
	}
	CRefPtr<CUndoMultiVariantProp>	pInfo;
	pInfo.CreateObj();
	pInfo->m_arrSelection = *parrSelection;
	pInfo->m_arrProp.SetSize(parrSelection->GetSize());
	midiMaps.GetProperty(*parrSelection, iProp, pInfo->m_arrProp);
	State.SetObj(pInfo);
}

void CMappingBar::RestoreMultiProperty(const CUndoState& State)
{
	int	iProp = State.GetCtrlID();
	const CUndoMultiVariantProp	*pInfo = static_cast<CUndoMultiVariantProp*>(State.GetObj());
	midiMaps.SetProperty(pInfo->m_arrSelection, iProp, pInfo->m_arrProp);
	UpdateGrid(pInfo->m_arrSelection, iProp);
	if (iProp == PROP_TARGET) {	// if restoring target
		UpdateGrid(pInfo->m_arrSelection, PROP_PROPERTY);	// target can affect property
	}
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
	pInfo->m_arrMapping = midiMaps.GetArray();
	m_grid.GetSelection(pInfo->m_arrSelection);
	State.SetObj(pInfo);
}

void CMappingBar::RestoreMappings(const CUndoState& State)
{
	const CUndoSelectedMappings* pInfo = static_cast<CUndoSelectedMappings*>(State.GetObj());
	midiMaps.SetArray(pInfo->m_arrMapping);
	UpdateGrid();
	m_grid.SetSelection(pInfo->m_arrSelection);
}

void CMappingBar::SaveLearn(CUndoState& State) const
{
	int	iMapping = State.GetCtrlID();
	State.m_Val.p.x.u = midiMaps.GetAt(iMapping).GetInputMidiMsg();
}

void CMappingBar::RestoreLearn(const CUndoState& State)
{
	int	iMapping = State.GetCtrlID();
	midiMaps.SetInputMidiMsg(iMapping, State.m_Val.p.x.u);
	UpdateGrid(iMapping);
}

void CMappingBar::SaveLearnMulti(CUndoState& State) const
{
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
	midiMaps.GetInputMidiMsg(*parrSelection, pInfo->m_arrProp);
	State.SetObj(pInfo);
}

void CMappingBar::RestoreLearnMulti(const CUndoState& State)
{
	const CUndoMultiIntegerProp	*pInfo = static_cast<CUndoMultiIntegerProp*>(State.GetObj());
	midiMaps.SetInputMidiMsg(pInfo->m_arrSelection, pInfo->m_arrProp);
	UpdateGrid(pInfo->m_arrSelection, -1);
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
	case UCODE_LEARN:
		SaveLearn(State);
		break;
	case UCODE_LEARN_MULTI:
		SaveLearnMulti(State);
		break;
	default:
		NODEFAULTCASE;	// missing case
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
	case UCODE_LEARN:
		RestoreLearn(State);
		break;
	case UCODE_LEARN_MULTI:
		RestoreLearnMulti(State);
		break;
	default:
		NODEFAULTCASE;	// missing case
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

// CMappingBar message map

BEGIN_MESSAGE_MAP(CMappingBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_MAPPING_GRID, OnListGetdispinfo)
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(UWM_MIDI_EVENT, OnMidiEvent)
	ON_COMMAND(ID_LIST_COL_HDR_RESET, OnListColHdrReset)
	ON_NOTIFY(ULVN_REORDER, IDC_MAPPING_GRID, OnListReorder)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_MAPPING_GRID, OnListColumnClick)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_MAPPING_GRID, OnListCustomDraw)
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
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
END_MESSAGE_MAP()

// CMappingBar message handlers

int CMappingBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD	dwStyle = WS_CHILD | WS_VISIBLE 
		| LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS;
	// NOTE that LVS_SHOWSELALWAYS breaks CDIS_SELECTED handling in OnListCustomDraw,
	// but since it's a useful style, instead we disable it while in MIDI learn mode
	if (!m_grid.Create(dwStyle, CRect(0, 0, 0, 0), this, IDC_MAPPING_GRID))
		return -1;
	DWORD	dwListExStyle = LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP;
	m_grid.SetExtendedStyle(dwListExStyle);
	m_grid.CreateColumns(m_arrColInfo, COLUMNS);
	m_grid.SendMessage(WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)));
	m_grid.LoadColumnOrder(RK_MappingBar, RK_COL_ORDER);
	m_grid.LoadColumnWidths(RK_MappingBar, RK_COL_WIDTH);
	m_sNotApplicable.LoadString(IDS_NOT_APPLICABLE);
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

void CMappingBar::LearnMapping(int iMapping, DWORD nInMidiMsg, bool bCoalesceEdit)
{
	UINT	nUndoFlags = bCoalesceEdit ? UE_COALESCE : 0;
	NotifyUndoableEdit(iMapping, UCODE_LEARN, nUndoFlags);
	midiMaps.SetInputMidiMsg(iMapping, nInMidiMsg);
	SetModifiedFlag();
	UpdateGrid(iMapping);
}

void CMappingBar::LearnMappings(const CIntArrayEx& arrSelection, DWORD nInMidiMsg, bool bCoalesceEdit)
{
	UINT	nUndoFlags = bCoalesceEdit ? UE_COALESCE : 0;
	m_parrSelection = &arrSelection;
	NotifyUndoableEdit(0, UCODE_LEARN_MULTI, nUndoFlags);
	midiMaps.SetInputMidiMsg(arrSelection, nInMidiMsg);	// update selected mappings
	SetModifiedFlag();
	UpdateGrid(arrSelection, -1);
}

LRESULT CMappingBar::OnMidiEvent(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	if (!theApp.m_midiMgr.IsLearnMode()) {	// if not in MIDI learn mode
		return 0;	// we shouldn't be here
	}
	if (::GetFocus() != m_grid.m_hWnd) {	// if grid doesn't have focus
		return 0;	// too confusing otherwise
	}
	int	iMapping = m_grid.GetSelection();
	if (iMapping < 0) {	// if grid doesn't have a selection
		return 0;	// nothing to map
	}
	DWORD	nInMidiMsg = static_cast<DWORD>(wParam);
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	bool	bCoalesceEdit;
	if (arrSelection != m_arrPrevSelection) {	// if selection changed
		m_arrPrevSelection = arrSelection;
		bCoalesceEdit = false;	// don't coalesce edit; create a new undo state
	} else {	// selection hasn't changed
		bCoalesceEdit = true;	// coalesce edit to avoid a blizzard of undo states
	}
	int	iCmd = MIDI_CMD_IDX(nInMidiMsg);	// convert MIDI status to command index
	if (iCmd >= 0 && iCmd < MIDI_CHANNEL_VOICE_MESSAGES) {	// if channel voice message
		// remove data from message by clearing second or both parameters
		if (iCmd <= MIDI_CVM_CONTROL) {	// if message has a note/controller
			nInMidiMsg &= ~MIDI_P2_MASK;	// only clear its second parameter
		} else {	// message doesn't have a note/controller; only status matters
			nInMidiMsg &= ~(MIDI_P1_MASK | MIDI_P2_MASK);	// clear both parameters
		}
		if (arrSelection.GetSize() > 1) {	// if multiple selection
			LearnMappings(arrSelection, nInMidiMsg, bCoalesceEdit);
		} else {	// not multiple selection
			if (arrSelection.GetSize()) {	// if single selection
				LearnMapping(iMapping, nInMidiMsg, bCoalesceEdit);
			}
		}
	}
	return 0;
}

void CMappingBar::OnListGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pResult);
	const NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	const LVITEM&	item = pDispInfo->item;
	int	iItem = item.iItem;
	if (item.mask & LVIF_TEXT) {
		const CMapping&	map = midiMaps.GetAt(iItem);
		switch (item.iSubItem) {
		case COL_NUMBER:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), iItem + 1); // make one-based
			break;
		case COL_EVENT:
			_tcscpy_s(item.pszText, item.cchTextMax, GetEventName(map.m_iEvent)); 
			break;
		case COL_CHANNEL:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), map.m_iChannel + 1);	// make one-based
			break;
		case COL_CONTROL:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), map.m_iControl); 
			break;
		case COL_TARGET:
			_tcscpy_s(item.pszText, item.cchTextMax, GetTargetName(map.m_iTarget)); 
			break;
		case COL_PROPERTY:
			{
				LPCTSTR	pszPropName;
				if (map.TargetHasProperty()) {	// if target has a property
					pszPropName = GetParamPropName(map.m_iProp);
				} else {	// target doesn't have a property
					pszPropName = m_sNotApplicable;
				}
				_tcscpy_s(item.pszText, item.cchTextMax, pszPropName); 
			}
			break;
		case COL_START:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%g"), map.m_fStart); 
			break;
		case COL_END:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%g"), map.m_fEnd); 
			break;
		default:
			NODEFAULTCASE;	// missing column
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
			Move(arrSelection, iDropPos);
		}
	}
}

void CMappingBar::OnListColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pListView = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	int	iProp = pListView->iSubItem - 1;	// skip number column
	if (iProp >= 0) {
		bool	bDescending = (GetKeyState(VK_SHIFT) & GKS_DOWN) != 0;
		Sort(iProp, bDescending);
	}
	pResult = 0;
}

void CMappingBar::OnListCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;
	if (theApp.m_midiMgr.IsLearnMode()) {	// if learning MIDI input
		switch (pLVCD->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			// this will NOT work with LVS_SHOWSELALWAYS; see uItemState in NMCUSTOMDRAW doc
			if (pLVCD->nmcd.uItemState & CDIS_SELECTED) {	// if item selected
				pLVCD->clrTextBk = MIDI_LEARN_COLOR;	// customize item background color
				// trick system into using our custom color instead of selection color
				pLVCD->nmcd.uItemState &= ~CDIS_SELECTED;	// clear item's selected flag
			}
			break;
		}
	}
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
	Copy(arrSelection);
}

void CMappingBar::OnEditCut()
{
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	Cut(arrSelection);
}

void CMappingBar::OnEditPaste()
{
	int	iInsert = m_grid.GetInsertPos();
	Paste(iInsert);
}

void CMappingBar::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_clipboard.GetSize() > 0);
}

void CMappingBar::OnEditInsert()
{
	int	iInsert = m_grid.GetInsertPos();
	Insert(iInsert);
}

void CMappingBar::OnUpdateEditInsert(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(true);
}

void CMappingBar::OnEditDelete()
{
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	Delete(arrSelection);
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
