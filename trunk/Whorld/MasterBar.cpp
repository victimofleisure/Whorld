// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10feb25	initial version

*/

#include "stdafx.h"
#include "Whorld.h"
#include "MasterBar.h"
#include "MasterRowDlg.h"
#include "WhorldDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CMasterView

IMPLEMENT_DYNCREATE(CMasterView, CRowView)

#define IDS_MAST_NAME_STATIC IDS_PROW_NAME_STATIC
#define IDS_MAST_VAL_SLIDER IDS_PROW_VAL_SLIDER
#define IDS_MAST_VAL_EDIT IDS_PROW_Val

CMasterView::CMasterView()
{
}

CMasterView::~CMasterView()
{
}

CRowDlg	*CMasterView::CreateRow(int Idx)
{
	CMasterRowDlg	*pRow = new CMasterRowDlg;
	ASSERT(pRow != NULL);
	pRow->SetRowIndex(Idx);
	pRow->Create(IDD_MASTER_ROW, m_pParent);
	return(pRow);
}

void CMasterView::UpdateRow(int Idx)
{
	CWhorldDoc	*pDoc = GetDocument();
	ASSERT(pDoc != NULL);
	CMasterRowDlg	*pRow = STATIC_DOWNCAST(CMasterRowDlg, GetRow(Idx));
	ASSERT(pRow != NULL);
	pRow->Update(pDoc->m_master.a[Idx]);
}

#ifdef _DEBUG
CWhorldDoc* CMasterView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWhorldDoc)));
	return (CWhorldDoc*)m_pDocument;
}
#endif //_DEBUG

void CMasterView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	switch (lHint) {
	case HINT_NONE:
		for (int iMaster = 0; iMaster < MASTER_COUNT; iMaster++) {	// for each master row
			UpdateRow(iMaster);	// update all of row's controls
		}
		break;
	case HINT_MASTER:
		{
			CParamHint	*pParamHint = reinterpret_cast<CParamHint*>(pHint);
			int	iProp = pParamHint->m_iParam;	// master property index
			ASSERT(IsValidMasterProp(iProp));
			UpdateRow(iProp);	// update all of row's controls
		}
		break;
	case HINT_ZOOM:
		UpdateRow(MASTER_Zoom);
		break;
	}
}

BEGIN_MESSAGE_MAP(CMasterView, CRowView)
	ON_WM_MOUSEACTIVATE()
	ON_WM_CREATE()
END_MESSAGE_MAP()
	
int CMasterView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CRowView::OnCreate(lpCreateStruct) == -1)
		return -1;
	theApp.GetDocument()->AddView(this);	// add view to document
	return 0;
}

int CMasterView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	UNREFERENCED_PARAMETER(pDesktopWnd);
	UNREFERENCED_PARAMETER(nHitTest);
	UNREFERENCED_PARAMETER(message);
	return MA_NOACTIVATE;	// prevents assertion
}

// CMasterBar

const CRowView::COLINFO CMasterBar::m_arrColInfo[COLUMNS] = {
	#define MASTERCOLDEF(name) {IDC_MAST_##name, IDS_MAST_##name},
	#include "WhorldDef.h"	// generate array init list
};

CMasterBar::CMasterBar()
{
	m_pView = NULL;
}

CMasterBar::~CMasterBar()
{
}

void CMasterBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (pSender != m_pView)	{	// if sender isn't us
		m_pView->OnUpdate(pSender, lHint, pHint);
	}
}

// CMasterBar message map

BEGIN_MESSAGE_MAP(CMasterBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()

// CMasterBar message handlers

int CMasterBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CMasterView);
	m_pView = DYNAMIC_DOWNCAST(CMasterView, pFactory->CreateObject());
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
    CRect rc(0, 0, 0, 0);	// arbitrary initial size
    if (!m_pView->Create(NULL, NULL, dwStyle, rc, this, IDS_BAR_Master, NULL))
		return -1;
	m_pView->m_pParent = this;
	m_pView->SetNotifyWnd(this);
	m_pView->CreateCols(COLUMNS, m_arrColInfo, IDD_MASTER_ROW);
	m_pView->CreateRows(MASTER_COUNT);
	m_pView->SetNotifyWnd(this);
	CMainFrame	*pMainFrm = theApp.GetMainFrame();
	m_pView->SetAccel(pMainFrm->m_hAccelTable, pMainFrm);
	return 0;
}

void CMasterBar::OnDestroy()
{
	CMyDockablePane::OnDestroy();
}

void CMasterBar::OnSize(UINT nType, int cx, int cy)
{
	CMyDockablePane::OnSize(nType, cx, cy);
	if (m_pView != NULL)
		m_pView->MoveWindow(0, 0, cx, cy);
}

void CMasterBar::OnSetFocus(CWnd* pOldWnd)
{
	CMyDockablePane::OnSetFocus(pOldWnd);
	m_pView->SetFocus();	// delegate focus
}
