// Copyleft 2020 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		20mar20	initial version
		01		29mar20	add learn multiple mappings; add map selected tracks
		02		01apr20	standardize context menu handling
		03		05apr20	add track step mapping
		04		07sep20	add preset and part mapping
		05		19nov20	add sender argument to set mapping property
		06		15feb21	add mapping targets for transport commands
		07		20jun21	remove dispatch edit keys
		08		25oct21	add descending sort via Shift key
		09		26feb25	adapt for Whorld
		
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

#define RK_COL_ORDER _T("ColOrder")
#define RK_COL_WIDTH _T("ColWidth")

CMappingBar::CMappingBar()
{
}

CMappingBar::~CMappingBar()
{
}

void CMappingBar::AddMidiChannelComboItems(CComboBox& wndCombo)
{
	CString	sChan;
	for (int iChan = 0; iChan < MIDI_CHANNELS; iChan++) {
		sChan.Format(_T("%d"), iChan + 1);
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
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
	int	nMappings;
	nMappings = midiMaps.GetCount();
	m_grid.SetItemCountEx(nMappings, 0);
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
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
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
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
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
	int	iProp = m_iEditCol - 1;	// skip number column
	int	nPrevVal = midiMaps.GetProperty(m_iEditRow, iProp);
	if (nVal != nPrevVal) {	// if value actually changed
		CIntArrayEx	arrSelection;
		GetSelection(arrSelection);
		// if multiple mappings selected and edit is within selection
		if (arrSelection.GetSize() > 1 && arrSelection.Find(m_iEditRow) >= 0) {
			midiMaps.SetProperty(arrSelection, iProp, nVal);
		} else {	// edit single mapping
			midiMaps.SetProperty(m_iEditRow, iProp, nVal);
		}
	}
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
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
	if (item.mask & LVIF_TEXT) {
		const CMapping&	map = midiMaps.GetAt(iItem);
		switch (item.iSubItem) {
		case COL_NUMBER:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), iItem + 1); 
			break;
		case COL_IN_EVENT:
			_tcscpy_s(item.pszText, item.cchTextMax, CMapping::GetInputEventName(map.m_nInEvent)); 
			break;
		case COL_IN_CHANNEL:
			_stprintf_s(item.pszText, item.cchTextMax, _T("%d"), map.m_nInChannel + 1); 
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
			CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
			midiMaps.Move(arrSelection, iDropPos);
			UpdateGrid();
		}
	}
}

void CMappingBar::OnListColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pListView = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	int	iProp = pListView->iSubItem - 1;
	if (iProp >= 0) {
		CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
		bool	bDescending = (GetKeyState(VK_SHIFT) & GKS_DOWN) != 0;
		midiMaps.Sort(iProp, bDescending);
		UpdateGrid();
	}
	pResult = 0;
}

void CMappingBar::OnEditCopy()
{
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	midiMaps.GetSelection(arrSelection, m_clipboard);
}

void CMappingBar::OnEditCut()
{
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	midiMaps.GetSelection(arrSelection, m_clipboard);
	midiMaps.Delete(arrSelection);
	UpdateGrid();
}

void CMappingBar::OnEditPaste()
{
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
	int	iInsert = m_grid.GetInsertPos();
	midiMaps.Insert(iInsert, m_clipboard);
	UpdateGrid();
}

void CMappingBar::OnUpdateEditPaste(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_clipboard.GetSize() > 0);
}

void CMappingBar::OnEditInsert()
{
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
	CMappingArray	arrMapping;
	arrMapping.SetSize(1);
	arrMapping[0].SetDefaults();
	int	iInsert = m_grid.GetInsertPos();
	midiMaps.Insert(iInsert, arrMapping);
	UpdateGrid();
}

void CMappingBar::OnUpdateEditInsert(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(true);
}

void CMappingBar::OnEditDelete()
{
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
	CIntArrayEx	arrSelection;
	m_grid.GetSelection(arrSelection);
	midiMaps.Delete(arrSelection);
	UpdateGrid();
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
	CSeqMapping&	midiMaps = theApp.m_midiMgr.m_midiMaps;
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
