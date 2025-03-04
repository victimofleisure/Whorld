// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version

*/

// WhorldView.cpp : implementation of the CWhorldView class
//

#include "stdafx.h"
#include "Whorld.h"

#include "WhorldDoc.h"
#include "WhorldView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWhorldView

IMPLEMENT_DYNCREATE(CWhorldView, CView)

// CWhorldView construction/destruction

CWhorldView::CWhorldView()
{
	theApp.SetView(this);	// SDI shortcut
}

CWhorldView::~CWhorldView()
{
}

// CWhorldView overrides

BOOL CWhorldView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CWhorldView::OnDraw(CDC* /*pDC*/)
{
	// drawing is done by CRenderWnd
}

// CWhorldView operations

void CWhorldView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	// handle view updates by delegating them to the render thread
	CWhorldDoc	*pDoc = GetDocument();
	switch (lHint) {
	case HINT_NONE:
		theApp.m_thrRender.SetPatch(*pDoc);
		break;
	case HINT_PARAM:
		{
			CParamHint	*pParamHint = static_cast<CParamHint*>(pHint);
			VARIANT_PROP	prop;
			pDoc->GetParamRC(pParamHint->m_iParam, pParamHint->m_iProp, prop);
			theApp.m_thrRender.SetParam(pParamHint->m_iParam, pParamHint->m_iProp, prop);
		}
		break;
	case HINT_MASTER:
		{
			CPropHint	*pPropHint = static_cast<CPropHint*>(pHint);
			double	fVal = pDoc->GetMasterProp(pPropHint->m_iProp);
			theApp.m_thrRender.SetMasterProp(pPropHint->m_iProp, fVal);
		}
		break;
	case HINT_MAIN:
		{
			CPropHint	*pPropHint = static_cast<CPropHint*>(pHint);
			VARIANT_PROP	prop;
			pDoc->GetMainProp(pPropHint->m_iProp, prop);
			theApp.m_thrRender.SetMainProp(pPropHint->m_iProp, prop);
		}
		break;
	}
	// relay update to dockable panes via main frame
	theApp.GetMainFrame()->OnUpdate(pSender, lHint, pHint);
}

// CWhorldView diagnostics

#ifdef _DEBUG
void CWhorldView::AssertValid() const
{
	CView::AssertValid();
}

void CWhorldView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWhorldDoc* CWhorldView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWhorldDoc)));
	return (CWhorldDoc*)m_pDocument;
}
#endif //_DEBUG

// CWhorldView message map

BEGIN_MESSAGE_MAP(CWhorldView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_MESSAGE(UWM_DELAYED_CREATE, OnDelayedCreate)
END_MESSAGE_MAP()

// CWhorldView message handlers

LRESULT CWhorldView::OnDelayedCreate(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	CRect	rc;
	GetClientRect(rc);
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
	theApp.CreateRenderWnd(dwStyle, rc, this);
	return 0;
}

void CWhorldView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	if (cx > 0 && cy > 0) {
		if (!theApp.IsDetached()) {
			theApp.ResizeRenderWnd(cx, cy);
		}
	}
}

void CWhorldView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CWhorldView::OnContextMenu(CWnd* /* pWnd */, CPoint /* point */)
{
	// stock project displays a popup edit menu; disable that for now
	// theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}
