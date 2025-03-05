// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02mar25	initial version

*/

#include "stdafx.h"
#include "Whorld.h"
#include "GlobalsBar.h"
#include "GlobalsRowDlg.h"
#include "WhorldDoc.h"
#include "MainFrm.h"

// CGlobalsView

IMPLEMENT_DYNCREATE(CGlobalsView, CMyRowView)

CGlobalsView::CGlobalsView()
{
}

CGlobalsView::~CGlobalsView()
{
}

CRowDlg	*CGlobalsView::CreateRow(int Idx)
{
	CGlobalsRowDlg	*pRow = new CGlobalsRowDlg;
	ASSERT(pRow != NULL);
	pRow->SetRowIndex(Idx);
	pRow->SetRowPos(MapGlobalToParam(Idx));
	pRow->Create(IDD_GLOBAL_ROW, m_pParent);
	return(pRow);
}

void CGlobalsView::UpdateRow(int Idx)
{
	CWhorldDoc	*pDoc = theApp.GetDocument();
	ASSERT(pDoc != NULL);
	CGlobalsRowDlg	*pRow = STATIC_DOWNCAST(CGlobalsRowDlg, GetRow(Idx));
	ASSERT(pRow != NULL);
	int	iParam = MapGlobalToParam(Idx);
	pRow->Update(pDoc->GetGlobalParam(iParam));
}

void CGlobalsView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	switch (lHint) {
	case HINT_NONE:
		for (int iGlobal = 0; iGlobal < GLOBAL_COUNT; iGlobal++) {	// for each global row
			UpdateRow(iGlobal);	// update all of row's controls
		}
		break;
	case HINT_PARAM:
		{
			CParamHint	*pParamHint = reinterpret_cast<CParamHint*>(pHint);
			int	iParam = pParamHint->m_iParam;
			int	iProp = pParamHint->m_iProp;
			if (iProp == PARAM_PROP_Global) {
				CWhorldDoc	*pDoc = theApp.GetDocument();
				int	iRow = MapParamToGlobal(iParam);
				if (iRow >= 0) {	// if parameter has a global
					GetRow(iRow)->Update(pDoc->GetGlobalParam(iParam));
				}
			}
		}
		break;
	}
}

BEGIN_MESSAGE_MAP(CGlobalsView, CMyRowView)
	ON_WM_CREATE()
END_MESSAGE_MAP()
	
int CGlobalsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyRowView::OnCreate(lpCreateStruct) == -1)
		return -1;
	theApp.GetDocument()->AddView(this);	// add view to document
	return 0;
}

// CGlobalsBar

const CRowView::COLINFO CGlobalsBar::m_arrColInfo[COLUMNS] = {
	#define GLOBALCOLDEF(name) {IDC_PROW_##name, IDS_PROW_##name},
	#include "WhorldDef.h"	// generate array init list
};

CGlobalsBar::CGlobalsBar()
{
	m_pView = NULL;
}

CGlobalsBar::~CGlobalsBar()
{
}

void CGlobalsBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (pSender != m_pView)	{	// if sender isn't us
		m_pView->OnUpdate(pSender, lHint, pHint);
	}
}

void CGlobalsBar::OnFrameGetMinMaxInfo(HWND hFrameWnd, MINMAXINFO *pMMI)
{
	CMainFrame::OnFrameGetMinMaxInfo(this, hFrameWnd, pMMI);	// delegate to common handler
}

// CGlobalsBar message map

BEGIN_MESSAGE_MAP(CGlobalsBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CGlobalsBar message handlers

int CGlobalsBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CGlobalsView);
	m_pView = DYNAMIC_DOWNCAST(CGlobalsView, pFactory->CreateObject());
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
    CRect rc(0, 0, 0, 0);	// arbitrary initial size
	if (!m_pView->Create(NULL, NULL, dwStyle, rc, this, IDS_BAR_Globals, NULL))
		return -1;
	m_pView->m_pParent = this;
	m_pView->SetNotifyWnd(this);
	m_pView->CreateCols(COLUMNS, m_arrColInfo, IDD_GLOBAL_ROW);
	m_pView->CreateRows(GLOBAL_COUNT);
	m_pView->SetNotifyWnd(this);
	CMainFrame	*pMainFrm = theApp.GetMainFrame();
	m_pView->SetAccel(pMainFrm->m_hAccelTable, pMainFrm);
	return 0;
}

void CGlobalsBar::OnDestroy()
{
	CMyDockablePane::OnDestroy();
}

void CGlobalsBar::OnSize(UINT nType, int cx, int cy)
{
	CMyDockablePane::OnSize(nType, cx, cy);
	if (m_pView != NULL)
		m_pView->MoveWindow(0, 0, cx, cy);
}

void CGlobalsBar::OnSetFocus(CWnd* pOldWnd)
{
	CMyDockablePane::OnSetFocus(pOldWnd);
	m_pView->SetFocus();	// delegate focus
}
