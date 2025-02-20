// Copyleft 2025 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06feb25	initial version
		01		20feb25	add file export

*/

// WhorldView.cpp : implementation of the CWhorldView class
//

#include "stdafx.h"
#include "Whorld.h"

#include "WhorldDoc.h"
#include "WhorldView.h"
#include "MainFrm.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWhorldView

IMPLEMENT_DYNCREATE(CWhorldView, CView)

BEGIN_MESSAGE_MAP(CWhorldView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_MESSAGE(UWM_DELAYED_CREATE, OnDelayedCreate)
	ON_COMMAND(ID_WINDOW_PAUSE, OnWindowPause)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PAUSE, OnUpdateWindowPause)
	ON_COMMAND(ID_WINDOW_STEP, OnWindowStep)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_STEP, OnUpdateWindowStep)
	ON_COMMAND(ID_WINDOW_CLEAR, OnWindowClear)
	ON_COMMAND(ID_IMAGE_RANDOM_PHASE, OnImageRandomPhase)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
END_MESSAGE_MAP()

// CWhorldView construction/destruction

CWhorldView::CWhorldView()
{
	theApp.SetView(this);	// SDI shortcut
}

CWhorldView::~CWhorldView()
{
}

BOOL CWhorldView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// CWhorldView drawing

void CWhorldView::OnDraw(CDC* /*pDC*/)
{
}

void CWhorldView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CWhorldView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

void CWhorldView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CWhorldDoc	*pDoc = GetDocument();
	switch (lHint) {
	case HINT_NONE:
		{
			CRenderCmd	cmd(RC_SET_PATCH);
			// dynamically allocate patch data and enqueue a pointer to it;
			// recipient (render thread) is responsible for deleting buffer
			CPatch&	patch = *pDoc;	// upcast from document to patch data
			CPatch	*pPatch = new CPatch(patch);	// allocate buffer
			cmd.m_prop.byref = pPatch;	// command property is pointer to patch
			theApp.PushRenderCommand(cmd);
		}
		break;
	case HINT_PARAM:
		{
			CParamHint	*pParamHint = static_cast<CParamHint*>(pHint);
			CRenderCmd	cmd(RC_SET_PARAM_Val + pParamHint->m_iProp, pParamHint->m_iParam);
			pDoc->GetParamRC(pParamHint->m_iParam, pParamHint->m_iProp, cmd.m_prop);
			theApp.PushRenderCommand(cmd);
		}
		break;
	case HINT_MASTER:
		{
			CParamHint	*pParamHint = static_cast<CParamHint*>(pHint);
			CRenderCmd	cmd(RC_SET_MASTER, pParamHint->m_iParam);
			cmd.m_prop.dblVal = pDoc->GetMasterProp(pParamHint->m_iParam);
			theApp.PushRenderCommand(cmd);
		}
		break;
	case HINT_MAIN:
		{
			CParamHint	*pParamHint = static_cast<CParamHint*>(pHint);
			CRenderCmd	cmd(RC_SET_MAIN, pParamHint->m_iParam);
			pDoc->GetMainProp(pParamHint->m_iParam, cmd.m_prop);
			theApp.PushRenderCommand(cmd);
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

// CWhorldView message handlers

void CWhorldView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	if (cx > 0 && cy > 0) {
		if (!theApp.IsDetached()) {
			theApp.ResizeRenderWnd(cx, cy);
		}
	}
}

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

void CWhorldView::OnWindowPause()
{
	CRenderCmd	cmd(RC_SET_PAUSE, !theApp.IsPaused());
	theApp.PushRenderCommand(cmd);
}

void CWhorldView::OnUpdateWindowPause(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.IsPaused());
}

void CWhorldView::OnWindowStep()
{
	CRenderCmd	cmd(RC_SINGLE_STEP);
	theApp.PushRenderCommand(cmd);
}

void CWhorldView::OnUpdateWindowStep(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(theApp.IsPaused());
}

void CWhorldView::OnWindowClear()
{
	CRenderCmd	cmd(RC_SET_EMPTY, !theApp.IsPaused());
	theApp.PushRenderCommand(cmd);
}

void CWhorldView::OnImageRandomPhase()
{
	CRenderCmd	cmd(RC_RANDOM_PHASE);
	theApp.PushRenderCommand(cmd);
}

void CWhorldView::OnFileExport()
{
	CSize	szExport(1920, 1080);//@@@ get size from somewhere; need flags too
	// render thread captures bitmap and posts it to our main window for writing
	CRenderCmd	cmd(RC_CAPTURE_BITMAP);
	cmd.m_prop.szVal = szExport;
	theApp.PushRenderCommand(cmd);	// start capture ASAP in case we're unpaused
	CPathStr	sExportFolder;
	theApp.GetSpecialFolderPath(CSIDL_MYPICTURES, sExportFolder);//@@@ belongs in options init
	if (!PathFileExists(sExportFolder)) {
		AfxMessageBox(_T("Invalid folder."));
		return;
	}
	CString	sExportFileName(theApp.GetTimestampFileName());
	CPathStr sExportPath(sExportFolder);
	sExportPath.Append(sExportFileName);
	CString	sExportExt(_T(".png"));
	sExportPath += sExportExt;
	theApp.GetMainFrame()->AddImageExportPath(sExportPath);
}
