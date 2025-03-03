// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08feb25	initial version
		01		25feb25	override frame min/max info handler
		02		26feb25	remove row reordering

*/

#include "stdafx.h"
#include "Whorld.h"
#include "ParamsBar.h"
#include "ParamsRowDlg.h"
#include "WhorldDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CParamsView

IMPLEMENT_DYNCREATE(CParamsView, CMyRowView)

CParamsView::CParamsView()
{
}

CParamsView::~CParamsView()
{
}

CRowDlg	*CParamsView::CreateRow(int Idx)
{
	CParamsRowDlg	*pRow = new CParamsRowDlg;
	ASSERT(pRow != NULL);
	pRow->SetRowIndex(Idx);
	pRow->Create(IDD_PARAM_ROW, m_pParent);
	return(pRow);
}

void CParamsView::UpdateRow(int Idx)
{
	CWhorldDoc	*pDoc = theApp.GetDocument();
	ASSERT(pDoc != NULL);
	CParamsRowDlg	*pRow = STATIC_DOWNCAST(CParamsRowDlg, GetRow(Idx));
	ASSERT(pRow != NULL);
	pRow->Update(pDoc->GetParamRow(Idx));
}

void CParamsView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UNREFERENCED_PARAMETER(pSender);
	switch (lHint) {
	case HINT_NONE:
		for (int iParam = 0; iParam < PARAM_COUNT; iParam++) {	// for each parameter row
			UpdateRow(iParam);	// update all of row's controls
		}
		break;
	case HINT_PARAM:
		{
			CParamHint	*pParamHint = reinterpret_cast<CParamHint*>(pHint);
			int	iParam = pParamHint->m_iParam;
			int	iProp = pParamHint->m_iProp;
			CWhorldDoc	*pDoc = theApp.GetDocument();
			// update row's control for specified property only
			GetRow(iParam)->Update(pDoc->GetParamRow(iParam), iProp);
		}
		break;
	}
}

BEGIN_MESSAGE_MAP(CParamsView, CMyRowView)
	ON_WM_CREATE()
END_MESSAGE_MAP()
	
int CParamsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyRowView::OnCreate(lpCreateStruct) == -1)
		return -1;
	theApp.GetDocument()->AddView(this);	// add view to document
	return 0;
}

// CParamsBar

const CRowView::COLINFO CParamsBar::m_arrColInfo[COLUMNS] = {
	#define PARAMCOLDEF(name) {IDC_PROW_##name, IDS_PROW_##name},
	#include "WhorldDef.h"	// generate array init list
};

CParamsBar::CParamsBar()
{
	m_pView = NULL;
}

CParamsBar::~CParamsBar()
{
}

void CParamsBar::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (pSender != m_pView)	{	// if sender isn't us
		m_pView->OnUpdate(pSender, lHint, pHint);
	}
}

void CParamsBar::OnFrameGetMinMaxInfo(HWND hFrameWnd, MINMAXINFO *pMMI)
{
	CMainFrame::OnFrameGetMinMaxInfo(this, hFrameWnd, pMMI);	// delegate to common handler
}

// CParamsBar message map

BEGIN_MESSAGE_MAP(CParamsBar, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

// CParamsBar message handlers

int CParamsBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CParamsView);
	m_pView = DYNAMIC_DOWNCAST(CParamsView, pFactory->CreateObject());
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
    CRect rc(0, 0, 0, 0);	// arbitrary initial size
	if (!m_pView->Create(NULL, NULL, dwStyle, rc, this, IDS_BAR_Params, NULL))
		return -1;
	m_pView->m_pParent = this;
	m_pView->SetNotifyWnd(this);
	m_pView->CreateCols(COLUMNS, m_arrColInfo, IDD_PARAM_ROW);
	m_pView->CreateRows(PARAM_COUNT);
	m_pView->SetNotifyWnd(this);
	CMainFrame	*pMainFrm = theApp.GetMainFrame();
	m_pView->SetAccel(pMainFrm->m_hAccelTable, pMainFrm);
	return 0;
}

void CParamsBar::OnDestroy()
{
	CMyDockablePane::OnDestroy();
}

void CParamsBar::OnSize(UINT nType, int cx, int cy)
{
	CMyDockablePane::OnSize(nType, cx, cy);
	if (m_pView != NULL)
		m_pView->MoveWindow(0, 0, cx, cy);
}

void CParamsBar::OnSetFocus(CWnd* pOldWnd)
{
	CMyDockablePane::OnSetFocus(pOldWnd);
	m_pView->SetFocus();	// delegate focus
}
